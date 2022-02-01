#ifndef WIN32
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#endif
#include "httplib.h"
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <fstream>

#include "api/moduleconfig.h"
#include <amxxmodule.h>

#include "k_rehlds_api.h"

#include <util.h>
#include <net.h>
#include <algorithm>
#include <thread>
#include <mutex>
#include <vector>
#include "quicktrace.h"



std::thread g_hSpeedTestThread;
std::thread g_hMiniServerThread;
std::thread g_hMiniTracer;


bool g_bStopMiniTracerThread = false;

int g_iSpeedTestPart = 0;
std::string g_last_error = "No error";
float g_flSpeedTestResult;
int g_hPlayerSpeedCaller;
int g_hTextMsg = 0;

bool IsPlayer(int Player)
{
	return Player >= 1 && Player <= gpGlobals->maxClients;
}

bool IsPlayerSafe(int Player)
{
	return IsPlayer(Player) && MF_IsPlayerIngame(Player);
}

void UTIL_TextMsg(edict_t* pPlayer, const char* message)
{
	if (g_hTextMsg == 0)
		g_hTextMsg = GET_USER_MSG_ID(PLID, "TextMsg", NULL);

	MESSAGE_BEGIN(MSG_ONE, g_hTextMsg, NULL, pPlayer);
	WRITE_BYTE(HUD_PRINTCONSOLE); // 1 = console, 2 = console, 3 = chat, 4 = center
	WRITE_STRING(message);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, g_hTextMsg, NULL, pPlayer);
	WRITE_BYTE(HUD_PRINTTALK); // 1 = console, 2 = console, 3 = chat, 4 = center
	WRITE_STRING(message);
	MESSAGE_END();
	/*
	The byte above seems to use these:
	#define HUD_PRINTNOTIFY		1
	#define HUD_PRINTCONSOLE	2
	#define HUD_PRINTTALK		3
	#define HUD_PRINTCENTER		4
	However both 1 and 2 seems to go to console with Steam CS.
	*/
}

void UTIL_TextMsg(int iPlayer, const char* message)
{
	if (!IsPlayerSafe(iPlayer))
	{
		MF_Log("%s", message);
		return;
	}
	edict_t* pPlayer = MF_GetPlayerEdict(iPlayer);

	UTIL_TextMsg(pPlayer, message);
}


void UTIL_TextMsg(int iPlayer, std::string message)
{
	if (!IsPlayerSafe(iPlayer))
	{
		MF_Log("%s", message.c_str());
		return;
	}
	UTIL_TextMsg(iPlayer, message.c_str());
}

httplib::Server g_hMiniServer;
int g_iWaitForListen = 0;
int g_iMiniServerPort = 1000;
int g_hReqForward = -1;
int g_hFastDlForward = -1;
struct sMiniServerStr_REQ
{
	std::string val;
	std::string par;
	std::string ip;
	std::string path;
};
std::vector<sMiniServerStr_REQ> g_MiniServerReqList;


struct sMiniServerStrFastDL_REQ
{
	std::string ip;
	std::string path;
};
std::vector<sMiniServerStrFastDL_REQ> g_MiniServerReqListFastDL;

struct sMiniServerStr_RES
{
	std::string res;
	std::string ip;
	bool valid;
};
std::vector<sMiniServerStr_RES> g_MiniServerResList;

bool g_bStopMiniServerThread = false;

bool g_bAllowFastDLServer = false;

void ReportFastDLNewFileRequest(std::string path,std::string ip)
{
	sMiniServerStrFastDL_REQ tmpsMiniServerStr_REQ = sMiniServerStrFastDL_REQ();
	tmpsMiniServerStr_REQ.ip = ip;
	tmpsMiniServerStr_REQ.path = path;
	g_MiniServerReqListFastDL.push_back(tmpsMiniServerStr_REQ);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	while (g_MiniServerReqListFastDL.size())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

bool fileExists(const std::string& fileName)
{
	if (FILE* file = fopen(fileName.c_str(), "r"))
	{
		fclose(file);
		return true;
	}
	return false;
}

char* loadFile(const std::string& fileName, int& length)
{
	if (!fileExists(fileName))
		return NULL;
	std::ifstream fin(fileName.c_str(), std::ios::binary);
	long long begin = fin.tellg();
	fin.seekg(0, std::ios::end);
	unsigned int size = (unsigned int)((int)fin.tellg() - begin);
	char* buffer = new char[size];
	fin.seekg(0, std::ios::beg);
	fin.read(buffer, size);
	fin.close();
	length = (int)size; // surely models will never exceed 2 GB
	return buffer;
}

struct tracelistitem
{
	int pid;
	std::string ip;
};

struct traceresultitem
{
	int pid;
	std::vector<std::string> result;
};

std::vector<tracelistitem> tracelist;

std::vector<traceresultitem> traceresult;

void mini_tracer_thread()
{
	while (!g_bStopMiniTracerThread)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		for (auto& s : tracelist)
		{
			if (s.pid != -1)
			{
				traceresultitem tmptraceresultitem = traceresultitem();
				tmptraceresultitem.pid = s.pid;
				quicktrace qt;
				int ret = qt.trace(s.ip.c_str(), 32, 30);   //max 32 hops, 100 packet per hop
				unsigned int dst = qt.get_target_address();
				if (ret == QTRACE_OK) {

					tmptraceresultitem.result.push_back(("Traceroute to " + s.ip + "  (" + std::string(inet_ntoa(*(struct in_addr*)&dst)) + ")"));

					int hop_count = qt.get_hop_count();
					unsigned int addr;
					char* str_addr;
					for (int i = 0; i < hop_count; i++) {
						addr = qt.get_hop_address(i);
						str_addr = inet_ntoa(*(struct in_addr*)&addr);
						tmptraceresultitem.result.push_back(std::string("Hop ") + std::to_string(i + 1) + " " +
							std::string(str_addr) + " " + std::to_string(qt.get_hop_latency(i)) + "ms");
					}

				}
				else
				{
					tmptraceresultitem.result.push_back("Traceroute to " + s.ip + " failed!");

				}


				traceresult.push_back(tmptraceresultitem);
				s.pid = -1;
			}
		}
	}
}

static cell AMX_NATIVE_CALL start_traceroute_back(AMX* amx, cell* params) // 1 pararam
{
	int index = params[1];
	if (!IsPlayerSafe(index))
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Cannot access player %d, it's not safe enough!", index);
		return 0;
	}
	int iLen;
	const char* ip = MF_GetAmxString(amx, params[2], 0, &iLen);
	if (!iLen || !ip || ip[0] == '\0')
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Cannot get dest addr!", index);
		return 0;
	}
	UTIL_TextMsg(index, "Init back traceroute. Please wait for finish.");

	tracelistitem tmptracelistitem = tracelistitem();
	tmptracelistitem.ip = ip;
	tmptracelistitem.pid = index;
	tracelist.push_back(tmptracelistitem);
	return 0;
}


std::string g_sGameDir = "/";

void mini_server_thread()
{
	g_hMiniServer.new_task_queue = [] { return new httplib::ThreadPool(64); };
	while (!g_bStopMiniServerThread)
	{
		while (g_iWaitForListen == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		g_hMiniServer.Get("/.*", [](const httplib::Request& req, httplib::Response& res)
			{
				std::string params, values, path = req.path;

				if (g_bAllowFastDLServer && req.params.size() == 0)
				{
					while (path.find("//") == 0)
						path.erase(path.begin());
					if (path.size() > 64)
					{

					}
					else if (path.find("..") != std::string::npos)
					{
						res.status = 404;
						res.set_content("404 - fucking hacker", "text/plain");
						return;
					}
					else if (path.find("/gfx/") == 0
						|| path.find("/maps/") == 0
						|| path.find("/overviews/") == 0
						|| path.find("/sound/") == 0
						|| path.find("/sprites/") == 0
						|| path.find("/models/") == 0 ||
						(path.rfind("/") == 0 && path.find(".wad") != std::string::npos))
					{
						ReportFastDLNewFileRequest(path, req.remote_addr);

						const size_t DATA_CHUNK_SIZE = 2048;
						char* filedata = NULL;
						int len = 0;

						if (fileExists(g_sGameDir + path))
						{
							filedata = loadFile(g_sGameDir + path, len);
						}
						else if (fileExists(g_sGameDir + "/../valve" + path))
						{
							filedata = loadFile(g_sGameDir + "/../valve" + path, len);
						}
						else
						{
							res.status = 404;
							res.set_content("404 - no file found at server", "text/plain");
							return;
						}

						if (len > 0 && filedata != NULL)
						{
							res.set_content_provider(
								len,
								"application/octet-stream",
								[filedata](size_t offset, size_t length, httplib::DataSink& sink) {
									sink.write(&filedata[offset], std::min(length, (size_t)2048));
									return true;
								},
								[filedata](bool success) {
									delete[] filedata; });

							return;
						}
						else
						{
							res.status = 404;
							res.set_content("404 - error while load file", "text/plain");
							if (filedata != NULL)
								delete[] filedata;
							filedata = NULL;
							return;
						}
					}
				}
				if (g_hReqForward != -1 && g_hFastDlForward != -1)
				{
					if (req.params.size())
					{
						for (auto const& s : req.params)
						{
							params += s.first + ";";
							values += s.second + ";";

						}
					}
					sMiniServerStr_REQ tmpsMiniServerStr_REQ = sMiniServerStr_REQ();
					tmpsMiniServerStr_REQ.ip = req.remote_addr;
					tmpsMiniServerStr_REQ.par = params;
					tmpsMiniServerStr_REQ.val = values;
					tmpsMiniServerStr_REQ.path = path;
					g_MiniServerReqList.push_back(tmpsMiniServerStr_REQ);
					int maxwait = 100;
					while (maxwait > 0)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
						if (maxwait > 0)
						{
							for (auto& response : g_MiniServerResList)
							{
								if (response.valid)
								{
									if (response.ip == req.remote_addr)
									{
										res.set_content(response.res, "text/html");
										response.valid = false;
										return;
									}
								}
							}
						}
						maxwait--;
					}

					if (maxwait == 0)
					{
						res.status = 404;
						res.set_content("No response found", "text/plain");
					}
				}
				else
				{
					res.status = 404;
					res.set_content("Web server not satarted", "text/plain");
				}
			});

		g_hMiniServer.listen("0.0.0.0", g_iMiniServerPort);
		g_iWaitForListen = 0;
	}
}

bool g_bStopSpeedTestThread = false;

void download_speed_thread()
{
	while (true && !g_bStopSpeedTestThread)
	{
		if (g_iSpeedTestPart == 1)
		{
			try
			{
				g_iSpeedTestPart = 3;
				httplib::Client cli("speedtest.selectel.ru", 80);
				using namespace std::chrono;
				g_iSpeedTestPart = 4;
				high_resolution_clock::time_point t1 = high_resolution_clock::now();
				g_iSpeedTestPart = 5;
				cli.set_connection_timeout(5);
				g_iSpeedTestPart = 6;
				auto res = cli.Get("/100MB");
				g_iSpeedTestPart = 7;
				high_resolution_clock::time_point t2 = high_resolution_clock::now();
				g_iSpeedTestPart = 8;

				duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
				g_iSpeedTestPart = 9;

				g_flSpeedTestResult = time_span.count();

				g_iSpeedTestPart = 10;
			}
			catch (const std::runtime_error& re)
			{
				g_iSpeedTestPart += 100;
				g_last_error = re.what();
			}
			catch (const std::exception& ex)
			{
				g_iSpeedTestPart += 100;
				g_last_error = ex.what();
			}
			catch (...)
			{
				g_iSpeedTestPart += 100;
				g_last_error = "Unhandled exception";
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void StartFrame(void)
{
	if (g_MiniServerReqList.size() > 0)
	{
		if (g_hReqForward != -1)
		{
			for (const auto& s : g_MiniServerReqList)
			{
				MF_ExecuteForward(g_hReqForward, s.ip.c_str(), s.par.c_str(), s.val.c_str(), s.path.c_str());
			}
		}
		g_MiniServerReqList.clear();
	}

	if (g_MiniServerReqListFastDL.size() > 0)
	{
		if (g_hFastDlForward != -1)
		{
			for (const auto& s : g_MiniServerReqListFastDL)
			{
				MF_ExecuteForward(g_hFastDlForward, s.ip.c_str(), s.path.c_str());
			}
		}
		g_MiniServerReqListFastDL.clear();
	}

	for (auto& s : traceresult)
	{
		if (s.pid != -1)
		{
			for(const auto & str : s.result)
				UTIL_TextMsg(s.pid, str);

			s.result.clear();
			s.pid = -1;
		}
	}

	if (g_iSpeedTestPart >= 2)
	{
		if (IsPlayerSafe(g_hPlayerSpeedCaller) && g_iSpeedTestPart == 10)
		{
			char tmpSpeedResult[256];
			snprintf(tmpSpeedResult, sizeof(tmpSpeedResult), "Result: download 100mb in %f seconds.", g_flSpeedTestResult);
			UTIL_TextMsg(g_hPlayerSpeedCaller, tmpSpeedResult);
			g_iSpeedTestPart = 0;
		}
		else if (IsPlayerSafe(g_hPlayerSpeedCaller) && g_iSpeedTestPart > 100)
		{
			char tmpSpeedResult[256];
			snprintf(tmpSpeedResult, sizeof(tmpSpeedResult), "%s - %i - message : %s", "Result: crash.", g_iSpeedTestPart, g_last_error.c_str());
			UTIL_TextMsg(g_hPlayerSpeedCaller, tmpSpeedResult);
			g_iSpeedTestPart = 0;
		}
	}
}



static cell AMX_NATIVE_CALL test_download_speed(AMX* amx, cell* params) // 1 pararam
{
	int index = params[1];
	if (!IsPlayerSafe(index))
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Cannot access player %d, it's not safe enough!", index);
		return 0;
	}
	UTIL_TextMsg(index, "Start download speed test. Please wait for finish.");
	if (g_iSpeedTestPart <= 0)
	{
		g_hPlayerSpeedCaller = index;
		g_iSpeedTestPart = 1;
	}
	return 0;
}



static cell AMX_NATIVE_CALL print_sys_info(AMX* amx, cell* params) // 1 pararam
{
	int index = params[1];
	if (!IsPlayerSafe(index))
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Cannot access player %d, it's not safe enough!", index);
		return 0;
	}
#ifndef WIN32
	char tmpSysInfoPrint[256];
	/* Conversion constants. */
	const long minute = 60;
	const long hour = minute * 60;
	const long day = hour * 24;
	const double megabyte = 1024 * 1024;
	/* Obtain system statistics. */
	struct sysinfo si;
	sysinfo(&si);


	snprintf(tmpSysInfoPrint, sizeof(tmpSysInfoPrint), "Up : %ld days, %ld:%02ld:%02ld. Ram/Free: %5.1f MB / %5.1f MB. CPU's:  %d", si.uptime / day, (si.uptime % day) / hour,
		(si.uptime % hour) / minute, si.uptime % minute, si.totalram / megabyte, si.freeram / megabyte, si.procs);
	UTIL_TextMsg(index, tmpSysInfoPrint);

	struct utsname sysinfo;
	uname(&sysinfo);

	std::string tmpSysString = "Sys: " + std::string(sysinfo.sysname) +
		". Host: " + std::string(sysinfo.nodename) +
		". Release: " + std::string(sysinfo.release);

	snprintf(tmpSysInfoPrint, sizeof(tmpSysInfoPrint), "%s", tmpSysString.c_str());
	UTIL_TextMsg(index, tmpSysInfoPrint);

	tmpSysString =
		". Build: " + std::string(sysinfo.version) +
		". Arch: " + std::string(sysinfo.machine) +
		". Name: " + std::string(sysinfo.domainname);
	snprintf(tmpSysInfoPrint, sizeof(tmpSysInfoPrint), "%s", tmpSysString.c_str());
	UTIL_TextMsg(index, tmpSysInfoPrint);
#else 
	/*SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	*/
	UTIL_TextMsg(index, "Win32 not supported!");
#endif

	return 0;
}

static cell AMX_NATIVE_CALL init_mini_server(AMX* amx, cell* params)  // 1 param
{
	int port = params[1];
	g_iMiniServerPort = port;
	g_iWaitForListen = 1;
	return 0;
}

static cell AMX_NATIVE_CALL stop_mini_server(AMX*, cell*)
{
	g_iWaitForListen = 0;
	g_hMiniServer.stop();
	return 0;
}

static cell AMX_NATIVE_CALL mini_server_res(AMX* amx, cell* params) // 2 params
{
	int iLen, iLen2;

	const char* ip = MF_GetAmxString(amx, params[1], 0, &iLen);
	const char* res = MF_GetAmxString(amx, params[2], 1, &iLen2);
	sMiniServerStr_RES tmpsMiniServerStr_RES = sMiniServerStr_RES();
	tmpsMiniServerStr_RES.ip = ip;
	tmpsMiniServerStr_RES.res = res;
	tmpsMiniServerStr_RES.valid = true;
	for (unsigned int i = 0; i < g_MiniServerResList.size(); i++)
	{
		if (!g_MiniServerResList[i].valid)
		{
			g_MiniServerResList[i].ip = ip;
			g_MiniServerResList[i].res = res;
			g_MiniServerResList[i].valid = true;
			return 0;
		}
	}
	g_MiniServerResList.push_back(tmpsMiniServerStr_RES);
	return 0;
}

static cell AMX_NATIVE_CALL test_regex_req(AMX* amx, cell* params) // 1 pararam
{
	int index = params[1];
	if (index == 0 || !IsPlayerSafe(index))
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Cannot access player %d, it's not safe enough!", index);
		return 0;
	}
	char tmpSysInfoPrint[256];
	int error = 0;
	try
	{
		error = 1;

		const static std::regex re("(HTTP/1\\.[01]) (\\d{3}) (.*?)\r\n");

		error = 2;
		std::cmatch m;
		error = 3;

		if (std::regex_match("HTTP/1.1 200 OK\r\n", m, re)) {
			error = 4;
			auto version = std::string(m[1]);
			auto status = std::stoi(std::string(m[2]));
			auto reason = std::string(m[3]);
			error = 5;
			snprintf(tmpSysInfoPrint, sizeof(tmpSysInfoPrint), "%s:%i:%s", version.c_str(), status, reason.c_str());
			error = 6;
			UTIL_TextMsg(index, tmpSysInfoPrint);
			error = 7;
		}
		else
			UTIL_TextMsg(index, "regex_match: false");
	}
	catch (...)
	{
		snprintf(tmpSysInfoPrint, sizeof(tmpSysInfoPrint), "%s:%i", "Fatal error", error);
		UTIL_TextMsg(index, tmpSysInfoPrint);
	}
	return 0;
}


static cell AMX_NATIVE_CALL test_view_angles(AMX* amx, cell* params) // 1 pararam
{
	int index = params[1];
	if (!IsPlayerSafe(index))
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Cannot access player %d, it's not safe enough!", index);
		return 0;
	}

	edict_t* pPlayer = MF_GetPlayerEdict(index);

	char msg[256];
	snprintf(msg, sizeof(msg), "Angle %f %f %f", pPlayer->v.v_angle[0], pPlayer->v.v_angle[1], pPlayer->v.v_angle[2]);

	UTIL_TextMsg(index, msg);


	return 0;
}


#define xBIT(ab)            (1<<(ab))
#define xBITSUM(ab)        (xBIT(ab)-1)
#define xRESOURCE_INDEX_BITS	12

unsigned int x_iWritingBits = 0;
unsigned int x_iCurrentByte = 0;

void xMSG_StartBitWriting()
{
	x_iWritingBits = 0;
	x_iCurrentByte = 0;
}

int num = 2;
void xMSG_WriteBits(unsigned int iValue, unsigned int iBits)
{
	x_iCurrentByte |= ((iValue & xBITSUM(iBits)) << x_iWritingBits);
	x_iWritingBits += iBits;

	while (x_iWritingBits >= 8)
	{
		WRITE_BYTE(x_iCurrentByte & xBITSUM(8));

		x_iCurrentByte >>= 8;
		x_iWritingBits -= 8;
	}
}

void xMSG_WriteBitData(unsigned char* p, int len)
{
	int i = 0;
	for (; i < len; i++)
	{
		xMSG_WriteBits(p[i], 8);
	}
}
void xMSG_WriteBitString(char* p)
{
	if (p && p[0] != '\0')
		xMSG_WriteBitData((unsigned char*)p, strlen(p));
	xMSG_WriteBits(0, 8);
}

void xMSG_WriteOneBit(unsigned int iBit)
{
	xMSG_WriteBits(iBit, 1);
}

void xMSG_EndBitWriting()
{
	if (x_iWritingBits)
	{
		WRITE_BYTE(x_iCurrentByte);
	}
}

static cell AMX_NATIVE_CALL activate_fastdl_server(AMX* amx, cell* params) // 1 pararam
{
	g_bAllowFastDLServer = true;
	return 0;
}

static cell AMX_NATIVE_CALL deactivate_fastdl_server(AMX* amx, cell* params) // 1 pararam
{
	g_bAllowFastDLServer = false;
	return 0;
}


AMX_NATIVE_INFO my_Natives[] =
{
	{"test_download_speed",	test_download_speed},
	{"print_sys_info",	print_sys_info},
	{"test_regex_req",	test_regex_req},
	{"init_mini_server",	init_mini_server},
	{"stop_mini_server",	stop_mini_server},
	{"mini_server_res",	mini_server_res},
	{"test_view_angles",	test_view_angles},
	{"activate_fastdl_server",	activate_fastdl_server},
	{"deactivate_fastdl_server",	deactivate_fastdl_server},
	{"start_traceroute_back",	start_traceroute_back},
	{NULL,			NULL},
};

void OnPluginsLoaded()
{
	g_hReqForward = MF_RegisterForward("mini_server_req", ET_IGNORE, FP_STRING, FP_STRING, FP_STRING, FP_STRING, FP_DONE);
	g_hFastDlForward = MF_RegisterForward("mini_server_fastdl_req", ET_IGNORE, FP_STRING, FP_STRING, FP_DONE);
}

bool g_initialized = false;

int	DispatchSpawnPre(edict_t* pent)
{
	if (g_initialized)
	{
		RETURN_META_VALUE(MRES_IGNORED, 0);
	}

	g_initialized = true;

	RETURN_META_VALUE(MRES_IGNORED, 0);
}


void ServerDeactivate_Post()
{
	g_hReqForward = -1;
	g_hFastDlForward = -1;
	g_initialized = false;
	RETURN_META(MRES_IGNORED);
}



void OnAmxxAttach() // Server start
{
	MF_AddNatives(my_Natives);
	g_hSpeedTestThread = std::thread(download_speed_thread);
	g_hMiniServerThread = std::thread(mini_server_thread);
	g_hMiniTracer = std::thread(mini_tracer_thread);
	RehldsApi_Init();

	g_sGameDir = GET_GAME_INFO(PLID, GINFO_GAMEDIR);
	if (g_sGameDir.back() == '\\' || g_sGameDir.back() == '/')
		g_sGameDir.pop_back();
}

void OnAmxxDetach() // Server stop
{
	g_hTextMsg = 0;
	g_iSpeedTestPart = -1;
	g_iWaitForListen = -1;
	g_bStopSpeedTestThread = true;
	g_bStopSpeedTestThread = true;
	g_bStopMiniTracerThread = true;
	g_hMiniServer.stop();
	g_hSpeedTestThread.join();
	g_hMiniServerThread.join();
}

