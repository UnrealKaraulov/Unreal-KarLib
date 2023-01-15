#include <amxmodx>
#include <karlab>

public plugin_init() {
	register_plugin("FASTDL AND MINISERVER", "1.0", "KARAULOV");
}

public plugin_precache() 
{
	karlib_init_mini_server(27015); // Порт минисервера
	karlib_activate_fastdl_server();
}

// Запрос файлов с miniserver
public karlib_mini_server_req(const ip[],const params[],const values[],const path[])
{
	new player = find_player("d",ip)
	if (player > 0 && player < 33)
	{
		new playername[64]
		new buf[256]
		get_user_name(player,playername,charsmax(playername))
		formatex(buf,charsmax(buf),"<center><h1> Hello %s, you use mini server from %s ip!</h1><div/> Your request value = %s ----- %s</center><div/> Requ path: %s ",playername,ip,params,values, path)
		karlib_mini_server_res(ip,buf)
	}
	else 
	{
		karlib_mini_server_res(ip,"<center><h1>Only real players can use mini server!</h1></center>")
	}
}

// Запрос файлов с fastdl
public karlib_mini_server_fastdl_req(const ip[],const path[])
{
	//log_amx("user %s request file %s",ip,path);
}

