#include <amxmodx>
#include <fakemeta>
#include <karlab>

public plugin_init() {
	register_plugin("Karlab tester", "1.0", "BADMAN");
	register_concmd("test_speed","InitDownload", ADMIN_RCON, "InitDownload")
	register_concmd("test_info","InitTestInfo", ADMIN_RCON, "InitTestInfo")
	register_concmd("test_regex","InitTestRegex", ADMIN_RCON, "InitTestRegex")
	register_concmd("startserver8080","InitMiniServer8080", ADMIN_RCON, "InitTestInfo")
	register_concmd("test_view_angles","PrintViewAngles", ADMIN_RCON, "PrintViewAngles")
	
}

public InitMiniServer8080(Index)
{
	init_mini_server(8080)
}

public mini_server_req(ip[],params[],values[])
{
	new player = find_player("d",ip)
	if (player > 0 && player < 33)
	{
		new playername[64]
		new buf[256]
		get_user_name(player,playername,charsmax(playername))
		formatex(buf,charsmax(buf),"<center><h1> Hello %s, you use mini server from %s ip!</h1><div/> Your request value = %s ----- %s</center>",playername,ip,params,values)
		mini_server_res(ip,buf)
	}
	else 
	{
		mini_server_res(ip,"<center><h1>Only real players can use mini server!</h1></center>")
	}
}


public InitTestRegex(Index)
{
	test_regex_req(Index )
}

public InitDownload(Index)
{
	test_download_speed(Index )
}

public InitTestInfo(Index)
{
	print_sys_info(Index )
}

public PrintViewAngles(Index)
{
	test_view_angles(Index)
	new msg[256];
	new Float:Angles[3];
	pev(Index,pev_v_angle,Angles);
	client_print_color(Index,Index,"Amxx Angles: %f %f %f", Angles[0],Angles[1],Angles[2]);
}