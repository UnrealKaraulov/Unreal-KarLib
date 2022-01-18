#Library for create Web Server in AMXMODX (Interactive motd window, stats, register systems etc)
#Also possible to test real net speed.
 
				***NATIVES

 \* **Test server download speed (download 100mb test file)**
 \*
 \* @return  **none**
 \*
	`native test_download_speed(const id);

 \* **Print information about system
 \*
 \* @return  **none
 \*
	`native print_sys_info(const id);

 \* **Start mini server at selected port, accesss by http://ipaddress:port/miniserver?value=TEST
 \*
 \* @param port    **mini server port
 \*
 \* @return  **none

	`native init_mini_server(const port);

 \* **Stop mini server
 \*
 \* @return  **none

	`native stop_mini_server();

 \* **Called when server got any request
 \*
 \* @param ip  **request REMOTE IP
 \* @param req  **request STRING
 \*
 \* @return **none

	`forward mini_server_req(ip[],req[]);

 \* **Need call for every mini_server_req with same IP
 \*
 \* @param ip   **same as mini_server_req
 \* @param res	**response string for mini_server_req
 \*
 \* @return  **none

	`native mini_server_res(const ip[],const res[]);

*debug function

	`native test_regex_req(const id);
