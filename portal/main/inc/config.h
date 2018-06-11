#ifndef CONFIG_H
#define CONFIG_H




#ifndef  MAX_ID_SIZE
#define MAX_ID_SIZE 6
#endif
#define USERNAME_LENGTH 32+1

#define CFG_IPGW_CFG_DIR		   "/usr/app/userconfig/ipgw/"
#define CFG_IPGW_WEB_DIR		   "/usr/app/userconfig/web/"
#define CFG_WELCOME                "/usr/app/userconfig/web/config.ini"
#define CFG_WEB_AUTH               "/usr/app/userconfig/web/webauth.ini"
#define CFG_WEB_CERT			   "/usr/app/userconfig/web/cert.pem"
#define CFG_FLEXISIP_DIR		   "/usr/app/userconfig/ipgw/flexisip/"
#define CFG_FLEXISIP               "/usr/app/userconfig/ipgw/flexisip/flexisip.conf"
#define CFG_FLEXISIP_AUTH          "/usr/app/userconfig/ipgw/flexisip/auth_db"
#define CFG_ROUTE                  "/usr/app/userconfig/ipgw/flexisip/route.ini"
#define CFG_BASIC                  "/usr/app/userconfig/ipgw/ipgw.conf"
#define CFG_USER                   "/usr/app/userconfig/ipgw/usr.ini"
#define CFG_APPWEB                 "/usr/app/userconfig/appweb/appweb.conf"
#define CFG_HISTORY_DIR			   "/usr/mnt/userdata/history"
#define CFG_HISTORY_CONFIG_FILE    "/usr/mnt/userdata/history/history.ini"
#define SHELL_RESET                "/usr/app/reset2default.sh"
#define SHELL_GENE_CERT	     	   "/usr/app/userconfig/appweb/ssl/gererate_certification"

enum {
	eConfigMode = 0,
	eFullFuncMode,
}StartMode_tag;

extern int get_startMode();
extern void ipgwServiceCfgInit();
extern void get_local_ID(char* szLocal_ID);
extern void get_external_IP(char* szExternal_IP);
extern void get_internal_IP(char* szInternal_IP);
extern void get_IPS_IP(char* szIPS_IP);

extern void StartnetStausDetcDaemon();

extern void initIPGWConfigSevice();
extern void start_web();
extern void start_mrouted();
extern void start_flexisip();
extern void SetneedrestartFlexisip(int flag);


#endif
