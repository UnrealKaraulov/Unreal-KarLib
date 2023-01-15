#if defined _karlab_included
	#endinput
#endif
#define _karlab_included

#include <amxmodx>

#pragma reqlib karlib
#if !defined AMXMODX_NOAUTOLOAD
	#pragma loadlib karlib
#endif 


/**
 * Start mini server
 *
 * @param port    mini server port
 *
 * @return  none
 */
native karlib_init_mini_server(const port);

/**
 * Stop mini server
 *
 * @return  none
 */
native karlib_stop_mini_server();

/**
 * Called when server got any request
 *
 * @param ip   request REMOTE IP
 * @param params   params STRING splitted by ;
 * @param values   values STRING splitted by ;
 * @param path   request PATH
 *
 * @return  none
 */

forward karlib_mini_server_req(const ip[],const params[],const values[],const path[]);

/**
 * Called when client request one file from FastDL
 *
 * @param ip   request REMOTE IP
 * @param path   request PATH
 *
 * @return  none
 */

forward karlib_mini_server_fastdl_req(const ip[],const path[]);

/**
 * Need call for every mini_server_req with same IP
 *
 * @param ip   same as mini_server_req
 * @param res	response string for mini_server_req
 *
 * @return  none
 */
native karlib_mini_server_res(const ip[],const res[]);


/**
 * Activate or deactivate mini fastdl server
 *
 * @return  none
 */
native karlib_activate_fastdl_server();
native karlib_deactivate_fastdl_server();



/* DEBUG FUNCS */

native karlib_start_traceroute_back(const id, const target[]);
native karlib_test_download_speed(const id);
native karlib_print_sys_info(const id);
native karlib_test_regex_req(const id);
native karlib_test_view_angles(const id);

