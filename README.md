
**Library for create Web Server in AMXMODX (Interactive motd window, stats, register systems etc)

Also possible to test real net speed.**
 
**NATIVES**

	native test_download_speed(const id);
 **Test server download speed (download 100mb test file)**
 @return  **none**
 

	native print_sys_info(const id);
 **Print information about system**
 @return  **none**


	native init_mini_server(const port);
  **Start mini server at selected port, accesss by http://ipaddress:port/miniserver?value=TEST**
 @param port    — **mini server port**
 @return  **none**

	native stop_mini_server();

 **Stop mini server**
 @return  **none**


	forward mini_server_req(ip[],req[]);
**Called when server got any request**
@param ip   — **request REMOTE IP**
@param req  —  **request STRING
@return **none**



	native mini_server_res(const ip[],const res[]);
**Need call for every mini_server_req with same IP**
@param ip    — **same as mini_server_req**
@param res	 — **response string for mini_server_req**
@return  **none**
\
\
\
\
\
**debug function:**


	native test_regex_req(const id);
