#ifndef __CSR_H_H__
#define __CSR_H_H__

#define JSON_DEVICE_TYPE_IPGATEWAY "com.abb.ispf.client.welcome.gateway"
#define JSON_DEVICE_TYPE_APP "com.abb.ispf.client.welcome.app"
#define JSON_DEVICE_TYPE_DESAP "com.abb.ispf.client.globalip.desap"
//#define JSON_DEVICE_TYPE_FREEATHOME "com.abb.ispf.client.freeathome.sysap"
#define JSON_DEVICE_TYPE_FREEATHOME "com.abb.ispf.client.freeathome.app.buschjaeger.ios"

/*CSR API*/
#define API_SIGNED_REQUEST "/api/certificate/request"
#define API_ADDITIONAL_SIGNED_REQUEST "/api/certificate/sign"
#define API_RENEW_CERT_REQUEST "/api/certificate/renew"
#define API_GET_REVOCATION_CERT_REQUEST "/api/certificate/revoked"
#define API_REVOKE_CERT "/api/certificate/revoke"

/*Portal Server Domain*/
//#define BJE_PORTAL_SERVER "my.busch-jaeger.de"
#define BJE_PORTAL_SERVER "my-staging.busch-jaeger.de"
//#define BJE_PORTAL_SERVER "localhost"

#define BJE_SIP_SERVER "sip.my.busch-jaeger.de"
#define ABB_PORTAL_SERVER "my.busch-jaeger.de"
#define ABB_SIP_SERVER "sip.my.busch-jaeger.de"
#define BJE_STAGING_PORTAL_SERVER "my-staging.busch-jaeger.de"

/*path for certificate/csr/private key/openssl configuration file*/
#ifdef _WIN32
#define PATH_USRER_PARTITION "./usr/mnt/userdata/"
#define PATH_CERT_ROOT PATH_USRER_PARTITION "./PortalClient/" //֤?????Ÿ?Ŀ¼
#else
#define PATH_USRER_PARTITION "/usr/mnt/userdata/"
#define PATH_CERT_ROOT PATH_USRER_PARTITION "PortalClient/" //֤?????Ÿ?Ŀ¼
//#define PATH_USRER_PARTITION "/usr/portal/"
//#define PATH_CERT_ROOT PATH_USRER_PARTITION"clientCerts/"
#endif
#define PATH_GATEWAY_PRIVIATE_KEY PATH_CERT_ROOT "private.key.pem"
//#define PATH_GATEWAY_PRIVIATE_KEY PATH_CERT_ROOT"client.key"
#define PATH_CSR_CLIENT PATH_CERT_ROOT "IPGatewayClient.csr"	 // IPGateway(Client)-Portal Server(Server)
#define PATH_CSR_SERVER PATH_CERT_ROOT "IPGatewayServer.csr"	 // Flexisip In IPGateway (server) -APP (Client)
#define PATH_GATEWAY_TMP_CLIENT_CERT PATH_CERT_ROOT "client.crt" // IPGateway(Client) ??ʱ֤??

#define PATH_GATEWAY_CLIENT_CERT PATH_CERT_ROOT "client.pem" // IPGateway(Client) ֤??
//#define PATH_GATEWAY_CLIENT_CERT PATH_CERT_ROOT"client.crt"

#define PATH_GATEWAY_SERVER_CERT PATH_CERT_ROOT "server.crt"	 // IPGateway(Server) ֤??
#define PATH_PORTAL_SERVER_CFG PATH_CERT_ROOT "PortalServer.ini" //??��???ŵ?¼Portal Servel ???û?????????sipname??ʱ??Ҫ??  ??Ĭ???ǿ??ļ?
#define PATH_ACCESS_TOKEN PATH_CERT_ROOT "AccessToken"

#define FILE_CERT_CA PATH_CERT_ROOT "ca-certificates.crt"
//#define FILE_CERT_CA PATH_CERT_ROOT"ca.crt"
#define PATH_OPENSSL_CFG PATH_CERT_ROOT "openssl.cfg" // openssl ???????????ļ?????PATH_CSR_CLIENT
#define OPENSSL_SECTION "req_distinguished_name"
#define OPENSSL_KEY_CN "commonName"

//?ļ?ϵͳ?б??????õ??ļ?
#ifdef _WIN32
#define FILE_CERT_CA_INIT "./usr/app/ca-certificates.crt" //??��??֤????????֤???Ƿ??Ϸ??????ڸ??ļ?ϵͳ
#define PATH_OPENSSL_CFG_INIT "./usr/app/openssl.cfg"	 //??��??֤????????֤???Ƿ??Ϸ??????ڸ??ļ?ϵͳ
#define PATH_WELCOME_CFG_INIT "./usr/app/config.ini"
#define PATH_PORTAL_SERVER_CFG_INIT "./usr/app/PortalServer.ini" //??��???ŵ?¼Portal Servel ???û?????????sipname??ʱ??Ҫ??  ??Ĭ???ǿ??ļ?
//#define PATH_FLEXISIP_ROOT PATH_USRER_PARTITION"Flexisip/"
#define PATH_FLEXISIP_ROOT "./usr/app/userconfig/ipgw/flexisip/"
#else
#define FILE_CERT_CA_INIT "/home/wipap/portal/app/ca-certificates.crt" //??��??֤????????֤???Ƿ??Ϸ??????ڸ??ļ?ϵͳ
#define PATH_OPENSSL_CFG_INIT "/home/wipap/portal/app/openssl.cfg"	 //??��??֤????????֤???Ƿ??Ϸ??????ڸ??ļ?ϵͳ
#define PATH_WELCOME_CFG_INIT "/home/wipap/portal/app/config.ini"
#define PATH_PORTAL_SERVER_CFG_INIT "/home/wipap/portal/app/PortalServer.ini" //??��???ŵ?¼Portal Servel ???û?????????sipname??ʱ??Ҫ??  ??Ĭ???ǿ??ļ?
//#define PATH_FLEXISIP_ROOT PATH_USRER_PARTITION"Flexisip/"
#define PATH_FLEXISIP_ROOT "/home/wipap/portal/app/userconfig/ipgw/flexisip/"
#endif

#define PATH_WELCOME_CFG PATH_FLEXISIP_ROOT "config.ini" //ͨ??Portal Ҫ????app???????ļ???????ϵͳ???ſڻ??б���???ڻ??б?
#define PATH_FLEXISIP_AUTH_CFG PATH_FLEXISIP_ROOT "auth_db"
#define PATH_FLEXISIP_ACL PATH_FLEXISIP_ROOT "acl.list"
#define PATH_GATEWAY_FLEXISIP_KEY PATH_FLEXISIP_ROOT "agent.pem" // PATH_GATEWAY_PRIVIATE_KEY + IPGateway(Server) ֤??
#define PATH_FLEXISIP_ROUTE_CFG PATH_FLEXISIP_ROOT "route.ini"

// b2bsip
#ifdef _WIN32
#define PATH_B2B_ROOT "./etc/b2bsip/"
#else
#define PATH_B2B_ROOT "/etc/b2bsip/"
#endif
#define PATH_B2B_LICENSE_ROOT PATH_B2B_ROOT "tls/"

#define PATH_B2B_CA PATH_B2B_LICENSE_ROOT "cafile.pem"
#define PATH_B2B_CERT PATH_B2B_LICENSE_ROOT "b2b_cert.pem"
#define PATH_B2B_KEY PATH_B2B_LICENSE_ROOT "b2b_cert.key"
#define PATH_B2B_CFG PATH_B2B_ROOT "b2bsip.conf"

#define PORTAL_SERVER_SECTION "PortalServer"
#define PORTAL_SERVER_USER_KEY "User"

#define HISTORY_LOCAL_ID "local_id"
#define HISTORY_LOCAL_NAME "local_name"
#define HISTORY_REMOTE_ID "remote_id"
#define HISTORY_REMOTE_NAME "remote_name"
#define HISTORY_REMOTE_UUID "remote_uuid"
#define HISTORY_REMOTE_ID "remote_id"

#define DEVICE_INFO_SW_VERSION "software-version"

#define WELCOME_KEY_GATEWAY_NAME "name"
#define WELCOME_KEY_OUTDOOR_NAME "name"
#define WELCOME_KEY_OUTDOOR_ADDRESS "address"
#define WELCOME_SECTION_NETWORK "network"
#define WELCOME_KEY_LOCAL_ID "localid"
#define WELCOME_KEY_DOMAIN "domain"
#define WELCOME_KEY_LOCAL_ADDRESS "local-address"
#define WELCOME_KEY_GLOBAL_ADDRESS "global-address"

#define WELCOME_KEY_UUID "uuid"

#define IPGW_SECTION_PORTAL "portal"
#define IPGW_KEY_PORTALNAME "name"
#define IPGW_KEY_PORTALDOMAIN "domain"
#define IPGW_KEY_SIPDOMAIN "sipdomain"

#define CSR_SIZE 2048
#define MAX_ECHO_PAYLOAD 1400
#define HEARTBEAT_INTERVAL 100 // 10s
#define CERT_RENEW_DETECT 24 * 3600 * 1000

#define LO_ADDR "127.0.0.1"

#define B2B_SECTION_CONF "config"
#define B2B_KEY_DOMAIN "domain"
#define B2B_KEY_EXTERNAL_URI "external_uri"
#define B2B_KEY_INTERNAL_URI "internal_uri"
#define B2B_KEY_PASSWD "password"

#define B2B_INTERNAL_URI "sip:127.0.0.1:1234;transport=tcp"

#define B2B_INTERNAL_DOMAIN LO_ADDR
#define B2B_INTERNAL_PORT 5060
#define B2B_INTERNAL_PROTOCAL "tcp"

#define B2B_EXTERNAL_DOMAIN "sip.linphone.org" //??ȷ?Ϻ??޸?
#define B2B_EXTERNAL_PORT 8889
#define B2B_EXTERNAL_PROTOCAL "tls"

#define WELCOME_GLOBAL_DOMAIN B2B_EXTERNAL_DOMAIN
#define WELCOME_GLOBAL_PORT B2B_EXTERNAL_PORT
#define WELCOME_GLOBAL_PROTOCAL B2B_EXTERNAL_PROTOCAL

#define WELCOME_LOCAL_PORT 5061
#define WELCOME_LOCAL_PROTOCAL "tls"

#define EVENT_DEBUG

enum http_method_type {
	ehttp_get = 0,
	ehttp_post,
};

enum connection_state {
	eStatePreConneted  = 0,
	eStateEstablish	= 1,
	eStateDisconnected = 2,
	eStateWaitResponse = 3,
};

typedef struct tagHttpRequest {
	char		  szTargetURL[512];
	unsigned char szContentType[64];
	char		  szClientCertPath[64];
	char*		  szContent;
	int			  iContentLen;
	int			  iSecured;
	int			  iVerifyClient;
	int			  iVerifyPeer;
	int			  iAuthType;
	int			  iMethod;
	unsigned char szPortalUserame[68];
	unsigned char szPortalPassword[132];
} HttpRequest;

typedef struct tagHttpResponse {
	int	iResponseCode;
	char   szContentType[64];
	char   szheader[128];
	char*  szContent;
	double iContentLen;
} HttpResponse;

extern VOID  SetPortalServerUrl(VOID);
extern VOID  SetPortalDomain(VOID);
extern char* GetPortalServerUrl(VOID);
extern char* GetPortalDomain(VOID);
extern char* GetDeviceDomain(VOID);
extern INT   RemoveDeviceFromPortalServer(VOID);
extern VOID GetUnformattedUUID(char* unformated_uuid, int len);
extern VOID GetUUID(char* uuidstring);
extern VOID UpdateDeviceDomain(VOID);
extern VOID ChangeAllConfigDomain(VOID);
extern INT SendAdditionnalCertSignedRequest(int newflag);
extern VOID UpdateDeviceToNewDomain(char* newdomain);
extern VOID InitB2bConf(VOID);

#endif
