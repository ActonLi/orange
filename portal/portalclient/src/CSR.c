/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "PortalClient.h"

LOCAL char g_ipgwdomain[128]	  = {0};
LOCAL char g_ipgwdomain_new[128]  = {0};
LOCAL char g_PortalServerUrl[256] = {0}; //"https://"PORTAL_DOMAIN
LOCAL char g_PortalDomain[256]	= {0}; // PORTAL_DOMAIN

LOCAL char g_desapdomain[128]	 = {0};
LOCAL char g_desapdomain_new[128] = {0};

LOCAL char g_SipServerDomain[256];

LOCAL pthread_mutex_t s_Mutex_PortalClientLogoutDeal;

LOCAL VOID SaveWelcomeGlobalAddress(char* domain, int port, char* protocal);
VOID SaveB2BExternalUri(char* domain, int port, char* protocal);
VOID SaveB2BInternalUri(char* domain, int port, char* protocal);

extern int LWSB64EncodeString(const char* in, int in_len, char* out, int out_size);

VOID GeneratePortalClientDirectory(VOID)
{
	if (OSA_DirIsExist(PATH_CERT_ROOT) == FALSE) {
		if (OSA_DirCreateEx(PATH_CERT_ROOT) == FALSE) {
			OSA_DBG_MSG("%sCreate %s Fail\n\n", DEBUG_HEADER_PORTALCLIENT, PATH_CERT_ROOT);
		}
	}

	if (OSA_DirIsExist(PATH_FLEXISIP_ROOT) == FALSE) {
		if (OSA_DirCreateEx(PATH_FLEXISIP_ROOT) == FALSE) {
			OSA_DBG_MSG("%sCreate %s Fail\n\n", DEBUG_HEADER_PORTALCLIENT, PATH_FLEXISIP_ROOT);
		}
	}

#ifdef flexsip
	if (OSA_DirIsExist(PATH_B2B_LICENSE_ROOT) == FALSE) {
		if (OSA_DirCreateEx(PATH_B2B_LICENSE_ROOT) == FALSE) {
			OSA_DBG_MSG("%sCreate %s Fail\n\n", DEBUG_HEADER_PORTALCLIENT, PATH_B2B_LICENSE_ROOT);
		}
	}
#endif

	/*CA֤?飬??��??֤??????֤??????Ч??*/

	if (OSA_FileIsExist(FILE_CERT_CA) == FALSE) {
		OSA_FileCopy(FILE_CERT_CA_INIT, FILE_CERT_CA);
	}

	/*??��????֤??ǩ???????ļ?*/
	if (OSA_FileIsExist(PATH_OPENSSL_CFG) == FALSE) {
		OSA_FileCopy(PATH_OPENSSL_CFG_INIT, PATH_OPENSSL_CFG);
	}

	/*????Ҫ????APP???????ļ?*/
	if (OSA_FileIsExist(PATH_WELCOME_CFG) == FALSE) {
		OSA_FileCopy(PATH_WELCOME_CFG_INIT, PATH_WELCOME_CFG);
	}

#ifdef flexsip
	/*??��????Portal ?????????û??????????û?????????APP ??sipname??*/
	if (OSA_FileIsExist(PATH_PORTAL_SERVER_CFG) == FALSE) {
		OSA_FileCopy(PATH_PORTAL_SERVER_CFG_INIT, PATH_PORTAL_SERVER_CFG);
	}
#endif
}

VOID GeneratePrivateKey(VOID)
{
	char pGenKeyCmd[1024];

	while (1) {
		if (OSA_FileIsExist(PATH_GATEWAY_PRIVIATE_KEY) == FALSE) {
			sprintf(pGenKeyCmd, "openssl genrsa -out %s 2048", PATH_GATEWAY_PRIVIATE_KEY);
			printf("%s:%d cmd: %s\n", __func__, __LINE__, pGenKeyCmd);
			system(pGenKeyCmd);
		}

		if (OSA_FileGetSizeEx(PATH_GATEWAY_PRIVIATE_KEY) == 0) {
			OSA_ERROR("The private key size of IPGateway is 0");
			OSA_FileDel(PATH_GATEWAY_PRIVIATE_KEY);
		} else {
			OSA_DBG_MSG("%sThe private key is existing", DEBUG_HEADER_PORTALCLIENT);
			break;
		}

		OSA_Sleep(5000);
	}
}

VOID SetPortalServerUrl(VOID)
{
	// sprintf(g_PortalServerUrl,"https://%s",BJE_STAGING_PORTAL_SERVER);
	sprintf(g_PortalServerUrl, "https://%s", BJE_PORTAL_SERVER);
}

char* GetPortalServerUrl(VOID)
{
	return g_PortalServerUrl;
}

VOID SetPortalDomain(VOID)
{
	// sprintf(g_PortalDomain,"%s",BJE_STAGING_PORTAL_SERVER);
	sprintf(g_PortalDomain, "%s", BJE_PORTAL_SERVER);
}

char* GetPortalDomain(VOID)
{
	return g_PortalDomain;
}

VOID SetSipServerDomain(VOID)
{
	OSA_MemSet(g_SipServerDomain, 0, sizeof(g_SipServerDomain));
	sprintf(g_SipServerDomain, "%s", BJE_SIP_SERVER);
}

VOID InitB2BSip(VOID)
{
	SetSipServerDomain();
	SaveWelcomeGlobalAddress(g_SipServerDomain, WELCOME_GLOBAL_PORT, WELCOME_GLOBAL_PROTOCAL);
	SaveB2BExternalUri(g_SipServerDomain, B2B_EXTERNAL_PORT, B2B_EXTERNAL_PROTOCAL);

	/*inertal_uri*/
	SaveB2BInternalUri(GetExternalIP(), B2B_INTERNAL_PORT, B2B_INTERNAL_PROTOCAL);
}

char* GetSipServerDomain(VOID)
{
	return g_SipServerDomain;
}

VOID GetUnformattedUUID(char* unformated_uuid, int len)
{
	char format_uuid[64] = {0};
	GetUUID(format_uuid);

	int i, j = 0;
	for (i = 0; format_uuid[i] != '\0' && i < len; i++) {
		if (format_uuid[i] != '-') {
			unformated_uuid[j++] = format_uuid[i];
		}
	}
}

LOCAL VOID GenerateDeviceDomainForCN(VOID)
{
	char uuid[16] = {0};
	GetUnformattedUUID(uuid, 14);

	if (strcmp(DEVICE_TYPE, JSON_DEVICE_TYPE_DESAP) == 0) {
		sprintf(g_desapdomain_new, "desap%s", uuid);
		OSA_DBG_MSG("%sGenerateDesAPDomainForCN(%s)\n", DEBUG_HEADER_PORTALCLIENT, g_desapdomain_new);
	} else {
		sprintf(g_ipgwdomain_new, "ipgw%s", uuid);
		OSA_DBG_MSG("%sGenerateIpgwDomainForCN(%s)\n", DEBUG_HEADER_PORTALCLIENT, g_ipgwdomain_new);
	}
}

LOCAL VOID GenerateClientCSRFile(VOID)
{
	char tmpstr[256] = {0};
	char uri[128]	= {0};

	if (strcmp(DEVICE_TYPE, JSON_DEVICE_TYPE_DESAP) == 0) {
		// sprintf(uri,"Shelly@portal",g_desapdomain_new);
		sprintf(uri, "desap@%s", g_desapdomain_new);
	} else {
		sprintf(uri, "ipgw@%s", g_ipgwdomain_new);
	}
	OSA_DBG_MSG("%sGenerate CSR commonName(%s)\n", DEBUG_HEADER_PORTALCLIENT, uri);
	SetValueToEtcFile(PATH_OPENSSL_CFG, OPENSSL_SECTION, OPENSSL_KEY_CN, uri); /*PATH_OPENSSL_CFG-/etc/appweb/ssl/openssl.cfg*/
	SaveIniData(PATH_OPENSSL_CFG);
	sprintf(tmpstr, "openssl req -new -key %s -out %s -config %s", PATH_GATEWAY_PRIVIATE_KEY, PATH_CSR_CLIENT,
			PATH_OPENSSL_CFG); /*/etc/appweb/ssl/server.key.pem ????????*/
	system(tmpstr);
}

/*Generate CSR in cjson format include client-type,client-csr,client-name*/
LOCAL char* GenerateCertSignedRquest(char* fname)
{
	char* pClientCsrPlainText;
	FILE* pClientCsrPlainFD;
	int   ReadClientCsrPlainTextLen = 0;

	char* pClientCsrB64EncryptText;

	cJSON* cJsonObject;
	char*  pCsrInJsonFmt;

	/*pClientCsrPlainText Store CSR Plain Text*/
	pClientCsrPlainText = (char*) malloc(3 * 1024);
	memset(pClientCsrPlainText, 0, 3 * 1024);

	/*set g_ipgwdomain_new to "ipgateway14byteuuid"*/
	GenerateDeviceDomainForCN();

	/*ʹ??openssl req  ????PATH_CSR_CLIENT --/etc/appweb/ssl/IPGWclient.csr*/
	GenerateClientCSRFile();

	/*map client csr file to mem*/
	pClientCsrPlainFD = fopen(PATH_CSR_CLIENT, "r");
	if (pClientCsrPlainFD == NULL) {
		printf("csr Plain file not exist err(%d)\n", errno);
		free(pClientCsrPlainText);
		return NULL;
	};

	ReadClientCsrPlainTextLen = fread(pClientCsrPlainText, 1, 2048, pClientCsrPlainFD);

	pClientCsrB64EncryptText = (char*) malloc(ReadClientCsrPlainTextLen * 4.0 / 3 + 4);
	LWSB64EncodeString(pClientCsrPlainText, ReadClientCsrPlainTextLen, pClientCsrB64EncryptText,
					   ReadClientCsrPlainTextLen * 4.0 / 3 + 4); /*JSON don't allow CRLF*/
	free(pClientCsrPlainText);

	/*construct json object*/
	cJsonObject = cJSON_CreateObject();
	// cJSON_AddStringToObject(cJsonObject,"client-type",JSON_DEVICE_TYPE_IPGATEWAY);
	cJSON_AddStringToObject(cJsonObject, "client-type", DEVICE_TYPE);
	cJSON_AddStringToObject(cJsonObject, "client-csr", pClientCsrB64EncryptText);
	cJSON_AddStringToObject(cJsonObject, "client-name", fname);
	pCsrInJsonFmt = cJSON_PrintUnformatted(cJsonObject);

	/*release mem*/
	fclose(pClientCsrPlainFD);
	cJSON_Delete(cJsonObject);
	free(pClientCsrB64EncryptText);
	return pCsrInJsonFmt;
}

LOCAL size_t CurlWriteFunctionCallBack(VOID* ptr, size_t size, size_t nmemb, VOID* stream)
{

	// printf("size = %d , nmemb = %d\n",size,nmemb );
	int writelen = size * nmemb;
	strncpy((char*) stream, (char*) ptr, writelen);
	// printf("stream = %s\n",stream);
	// strcat((char*)stream,(char*)ptr); /*should not do like this ,ptr has useless data at the rail end*/
	return writelen;
}

LOCAL size_t CurlHeaderFunctionCallBack(VOID* ptr, size_t size, size_t nmemb, VOID* userdata)
{
	int  r;
	char uuid[64] = {0};
	r			  = sscanf(ptr, "X-Abb-Ispf-Uuid: %s\n", uuid);
	if (r) {
		sprintf(userdata, "%s", uuid);
	}
	return size * nmemb;
}

LOCAL int SendHTTPRequest(HttpRequest* req, HttpResponse* resp)
{
	CURL*	curl;
	CURLcode res;
	char*	resptype;
	int		 ret = -1;

	if (NULL == req || NULL == resp) {
		printf("%shttp request or response is NULL, req = %p resp = %p \n", DEBUG_HEADER_PORTALCLIENT, req, resp);
		return ret;
	}

	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		if (req->iSecured) {
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, req->iVerifyPeer);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);
			if (OSA_FileIsExist(FILE_CERT_CA) == FALSE) {
				printf("%s%s is not exist\n", DEBUG_HEADER_PORTALCLIENT, FILE_CERT_CA);
			}
			curl_easy_setopt(curl, CURLOPT_CAINFO, FILE_CERT_CA); /*ca-certificates.crt*/
			/* validate client certificate */
			if (req->iVerifyClient) {
				OSA_printf("req->szClientCertPath = %s\n", req->szClientCertPath);
				curl_easy_setopt(curl, CURLOPT_SSLCERT, req->szClientCertPath);

				/*no password for accessing to cert*/
				// curl_easy_setopt(curl, CURLOPT_SSLCERTPASSWD,"123456");

				curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");

				curl_easy_setopt(curl, CURLOPT_SSLKEY, PATH_GATEWAY_PRIVIATE_KEY);

				/*no password for accessing to private key*/
				// curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD,"123456");

				curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
			}
		}

		curl_easy_setopt(curl, req->iMethod, 1);

		// authentication
		// CURLAUTH_DIGEST
		if (req->iAuthType != CURLAUTH_NONE) {
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, req->iAuthType);
			curl_easy_setopt(curl, CURLOPT_USERNAME, req->szPortalUserame);
			curl_easy_setopt(curl, CURLOPT_PASSWORD, req->szPortalPassword);
		}

		printf("req->szTargetURL = %s, req->szPortalUserame=%s  req->szPortalPassword=%s\n", req->szTargetURL, req->szPortalUserame, req->szPortalPassword);
		// url address
		curl_easy_setopt(curl, CURLOPT_URL, req->szTargetURL);
		if (req->szContent != NULL && req->iContentLen > 0) {
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->szContent);
			/* if we don't provide POSTFIELDSIZE, libcurl will strlen() by itself */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, req->iContentLen);
			// set value to 1,http header will be involved in response content
		}
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);

		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlHeaderFunctionCallBack);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void*) resp->szheader);

		/*callback to deal with reponse data*/
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteFunctionCallBack);

		/*set a pointer to response buffer */
		if (resp->szContent != NULL) {
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp->szContent);
		}
		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		printf("curl_easy_perform res = %d\n", res);
		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			// printf("resp->szContent = %s\n",resp->szContent);
			res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, (void*) &resp->iResponseCode);
			if ((res == CURLE_OK)) {
				res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &resptype);
				strncpy(resp->szContentType, resptype, sizeof(resp->szContentType));
				if (CURLE_OK == res) {
					res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &resp->iContentLen);
					ret = 0;
				}
			}
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return ret;
}

LOCAL VOID SaveTempClientFile(unsigned char* data, int datalen)
{
	FILE* cert_fd = NULL;

	cert_fd = fopen(PATH_GATEWAY_TMP_CLIENT_CERT, "w+");
	if (!cert_fd) {
		printf("open certificate file failed\n");
		return;
	}
	fwrite(data, 1, strlen((const char*) data), cert_fd);
	fsync((int) cert_fd);
	fclose(cert_fd);
}

VOID UpdateDeviceDomain(VOID)
{
	if (strcmp(DEVICE_TYPE, JSON_DEVICE_TYPE_DESAP) == 0) {
		sprintf(g_desapdomain, "%s", g_desapdomain_new);
		OSA_DBG_MSG("%sUpdateDesAPDomain %s\n", DEBUG_HEADER_PORTALCLIENT, g_desapdomain);
	} else {
		sprintf(g_ipgwdomain, "%s", g_ipgwdomain_new);
		OSA_DBG_MSG("%sUpdateIpgwDomain %s\n", DEBUG_HEADER_PORTALCLIENT, g_ipgwdomain);
	}
}

VOID LoadDeviceDomainFromCfgFile(VOID)
{
	if (strcmp(DEVICE_TYPE, JSON_DEVICE_TYPE_DESAP) == 0) {
		GetValueFromEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_DOMAIN, g_desapdomain, 128);
		printf("\n\n\n\nDesap Domain is %s\n", g_desapdomain);
	} else {
		GetValueFromEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_DOMAIN, g_ipgwdomain, 128);
		printf("\n\n\n\nIPGateway Domain is %s\n", g_ipgwdomain);
	}
}

char* GetDeviceDomain(VOID)
{
	if (strcmp(DEVICE_TYPE, JSON_DEVICE_TYPE_DESAP) == 0) {
		return g_desapdomain;
	} else {
		return g_ipgwdomain;
	}
}

LOCAL VOID SaveServerCertFile(unsigned char* data, int datalen)
{
	FILE* cert_fd		 = NULL;
	char  shellstr[1024] = {0};
	cert_fd				 = fopen(PATH_GATEWAY_SERVER_CERT, "w+");
	if (!cert_fd) {
		printf("open certificate file failed\n");
		return;
	}
	fwrite(data, 1, strlen((const char*) data), cert_fd);
	fsync((int) cert_fd);
	fclose(cert_fd);
	sprintf(shellstr, "cp %s %s -f", PATH_GATEWAY_PRIVIATE_KEY, PATH_GATEWAY_FLEXISIP_KEY);
	system(shellstr);

	sprintf(shellstr, "cat %s>>%s", PATH_GATEWAY_SERVER_CERT, PATH_GATEWAY_FLEXISIP_KEY);
	system(shellstr);

	OSA_printf("%sSave Server Cert File %s\n", DEBUG_HEADER_PORTALCLIENT, PATH_GATEWAY_FLEXISIP_KEY);
}

VOID UpdateAuthFileDomain(char* domain)
{
	char  sec_name[16]	 = {0};
	char  username[128]	= {0};
	char  password[16]	 = {0};
	char  authRecord[1024] = {0};
	FILE* authFile		   = NULL;
	int   i				   = 0;
	int   ret			   = 0;

	char pCmd[1024];
	OSA_MemSet(pCmd, 0, sizeof(pCmd));
	sprintf(pCmd, "rm %s -rf", PATH_FLEXISIP_AUTH_CFG);
	system(pCmd);

	authFile = fopen(PATH_FLEXISIP_AUTH_CFG, "w+");
	if (authFile) {
		for (i = 1; i <= 4; i++) {
			sprintf(sec_name, "user_%d", i);

			ret = GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST, sec_name, "sipname", username, 128);
			ret = GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST, sec_name, "password", password, 16);
			if (ret != -1) {
				OSA_printf("user %d username = %s password = %s\n", i, username, password);
				sprintf(authRecord, "%s@%s %s\n", username, domain, password);
				fwrite(authRecord, 1, strlen(authRecord), authFile);
			}
		}
		/*ret = GetValueFromEtcFile(PATH_B2B_CFG, "config", "password",b2bpass, 32);
		if(ret != -1)
		{
			sprintf(authRecord,"%s@%s %s\n","ipgw",domain,b2bpass);
			fwrite(authRecord,1,strlen(authRecord),authFile);
		}*/
	}

	fclose(authFile);
}

LOCAL VOID SaveWelcomeLocalAddress(char* domain, char* localip, int port, char* protocal)
{
	char local_address[128] = {0};
	sprintf(local_address, "<sip:%s:%d;maddr=%s;transport=%s>", domain, port, localip, protocal);
	OSA_DBG_MSG("%sUpdate [Section=%s] [Key=%s] [Value=%s] in File %s\n", DEBUG_HEADER_PORTALCLIENT, WELCOME_SECTION_NETWORK, WELCOME_KEY_LOCAL_ADDRESS,
				local_address, PATH_WELCOME_CFG);

	SetValueToEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_LOCAL_ADDRESS, local_address);
	SaveIniData(PATH_WELCOME_CFG);
}

LOCAL VOID SaveWelcomeGlobalAddress(char* domain, int port, char* protocal)
{
	char global_address[128] = {0};
	sprintf(global_address, "<sip:%s:%d;transport=%s>", domain, port, protocal);
	SetValueToEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_GLOBAL_ADDRESS, global_address);
	SaveIniData(PATH_WELCOME_CFG);
}

LOCAL VOID save_welcome_outdoor_address(char* domain)
{
	int  i			  = 0;
	char address[128] = {0};
	char section[32]  = {0};
	char osid[16]	 = {0};
	for (i = 0; i < 9; i++) {
		sprintf(section, "outdoorstation_%d", i);
		if (-1 != GetValueFromEtcFile(PATH_WELCOME_CFG, section, WELCOME_KEY_OUTDOOR_ADDRESS, address, 128)) {
			sscanf(address, "sip:%[^@]", osid);
			sprintf(address, "sip:%s@%s:%d", osid, domain, WELCOME_LOCAL_PORT);
			SetValueToEtcFile(PATH_WELCOME_CFG, section, WELCOME_KEY_OUTDOOR_ADDRESS, address);
			SaveIniData(PATH_WELCOME_CFG);

			OSA_DBG_MSG("%sUpdate [Section=%s] [Key=%s] [Value=%s] in File %s\n", DEBUG_HEADER_PORTALCLIENT, section, WELCOME_KEY_OUTDOOR_ADDRESS, address,
						PATH_WELCOME_CFG);
		}
	}
}

LOCAL char* GetInternalNetworkIP(VOID)
{
	char* pFlexisipServerIPAddr = NULL;

	// pFlexisipServerIPAddr = SystemDeviceInfo_GetEth1IPAddress();
	OSA_DBG_MSG("%sCall SystemDeviceInfo_GetEth1IPAddress To Get Flexisip server IP Address %s", DEBUG_HEADER_PORTALCLIENT, pFlexisipServerIPAddr);
	return pFlexisipServerIPAddr;
}

char* GetExternalIP(VOID)
{
	char* pExternalIPAddr = NULL;

	// pExternalIPAddr = SystemDeviceInfo_GetEth0IPAddress();
	OSA_DBG_MSG("%sCall SystemDeviceInfo_GetEth0IPAddress To Get External IP Address %s", DEBUG_HEADER_PORTALCLIENT, pExternalIPAddr);
	return pExternalIPAddr;
}

VOID UpdateFlexisipServeIPAddress(VOID)
{
	if (strcmp(DEVICE_TYPE, JSON_DEVICE_TYPE_DESAP)) {
		SaveWelcomeLocalAddress(g_desapdomain, GetInternalNetworkIP(), WELCOME_LOCAL_PORT, WELCOME_LOCAL_PROTOCAL);
	} else {
		SaveWelcomeLocalAddress(g_ipgwdomain, GetInternalNetworkIP(), WELCOME_LOCAL_PORT, WELCOME_LOCAL_PROTOCAL);
	}
}

VOID ChangeAllConfigDomain(VOID)
{
	/*update domain in welcome config.ini*/
	SetValueToEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_DOMAIN, GetDeviceDomain());
	OSA_DBG_MSG("%sUpdate [Section=%s] [Key=%s] [Value=%s] in File %s\n", DEBUG_HEADER_PORTALCLIENT, WELCOME_SECTION_NETWORK, WELCOME_KEY_DOMAIN, g_ipgwdomain,
				PATH_WELCOME_CFG);

	/*update domain for flexisip auth file*/
	UpdateAuthFileDomain(GetDeviceDomain());
	/*update local-address in config.ini */
	SaveWelcomeLocalAddress(GetDeviceDomain(), GetInternalNetworkIP(), WELCOME_LOCAL_PORT, WELCOME_LOCAL_PROTOCAL);

	// SaveWelcomeLocalAddress(g_ipgwdomain,GetInternalNetworkIP(),5060,"tcp");

	/*update domain for outdoor station*/
	save_welcome_outdoor_address(GetDeviceDomain());

/*update b2bsip.conf */
// save_b2b_domain(g_ipgwdomain);

/*route.ini*/
#ifdef flexsip
	UpdateRecordToRouteFile();
#endif
}

LOCAL VOID GenerateServerCSRFile(VOID)
{
	char tmpstr[256] = {0};

	/*save CN = domain ,then generate csr*/
	if (strcmp(DEVICE_TYPE, JSON_DEVICE_TYPE_DESAP) == 0) {
		SetValueToEtcFile(PATH_OPENSSL_CFG, OPENSSL_SECTION, OPENSSL_KEY_CN, g_desapdomain_new);
	} else {
		SetValueToEtcFile(PATH_OPENSSL_CFG, OPENSSL_SECTION, OPENSSL_KEY_CN, g_ipgwdomain_new);
	}
	SaveIniData(PATH_OPENSSL_CFG);
	sprintf(tmpstr, "openssl req -new -key %s -out %s -config %s", PATH_GATEWAY_PRIVIATE_KEY, PATH_CSR_SERVER, PATH_OPENSSL_CFG);
	system(tmpstr);
}

LOCAL char* GenerateAdditionalCertSignedRquest(int newflag)
{
	char* csrstring;
	FILE* csrfd_server;

	/*create CSR*/
	csrstring = (char*) malloc(3 * 1024);
	memset(csrstring, 0, 3 * 1024);

	if (newflag == 1) {
		GenerateServerCSRFile();
	}

	/*map client csr file to mem*/
	csrfd_server = fopen(PATH_CSR_SERVER, "r");
	// csrfd_server = fopen(PATH_CSR_SERVER_OLD_IPGATEWAY,"r");
	if (csrfd_server == NULL) {
		printf("csr file not exist err(%d)\n", errno);
		return NULL;
	};

	fread(csrstring, 1, 2048, csrfd_server);

	fclose(csrfd_server);
	return csrstring;
}

INT SendAdditionnalCertSignedRequest(int newflag)
{
	if (strcmp(DEVICE_TYPE, JSON_DEVICE_TYPE_DESAP) == 0) {
		return 0;
	}

	int			 ret = -1;
	HttpRequest  stCertSignReq;
	HttpResponse stCertSignResp;
	char		 pCSRRequestAPI[512] = {0};
	char*		 csr				 = GenerateAdditionalCertSignedRquest(newflag);
	if (csr == NULL) {
		OSA_ERROR("%sCreate Server CSR Failed\n", DEBUG_HEADER_PORTALCLIENT);
		return -1;
	}
	memset(&stCertSignReq, 0, sizeof(HttpRequest));
	memset(&stCertSignResp, 0, sizeof(HttpResponse));
	sprintf(pCSRRequestAPI, "%s%s", g_PortalServerUrl, API_ADDITIONAL_SIGNED_REQUEST);
	strncpy(stCertSignReq.szTargetURL, pCSRRequestAPI, strlen(pCSRRequestAPI));
	stCertSignReq.szContent		= csr;
	stCertSignReq.iContentLen   = strlen(csr);
	stCertSignReq.iAuthType		= CURLAUTH_NONE;
	stCertSignReq.iSecured		= 1;
	stCertSignReq.iVerifyPeer   = 0;
	stCertSignReq.iVerifyClient = 1;
	sprintf((char*) stCertSignReq.szClientCertPath, "%s", PATH_GATEWAY_TMP_CLIENT_CERT);
	stCertSignReq.iMethod	= CURLOPT_HTTPPOST;
	stCertSignResp.szContent = (char*) malloc(3 * 1024);
	memset(stCertSignResp.szContent, 0, 3 * 1024);
	res_init(); /*ˢ??????????????ַ????��????portal server??????/etc/resolv.conf ?ļ???ָ??nameserver*/

	if (SendHTTPRequest(&stCertSignReq, &stCertSignResp) == 0) {
		OSA_DBG_MSG("iResponseCode = %d", stCertSignResp.iResponseCode);
		if (stCertSignResp.iResponseCode == 201) {
			OSA_DBG_MSG("stCertSignResp = %s", stCertSignResp.szContent);
			if (strcmp(stCertSignResp.szContentType, "application/x-x509-user-cert") == 0) {
				SaveServerCertFile((unsigned char*) stCertSignResp.szContent, stCertSignResp.iContentLen);
				ret = 0;
			}
		}
	}

	free(stCertSignResp.szContent);
	free(csr);
	return ret;
}

// Access Token, For xmpp
INT SendAccessTokenRequest(VOID)
{
	int			 ret = -1;
	HttpRequest  stCertSignReq;
	HttpResponse stCertSignResp;
	char		 pAccessTokenRequestAPI[512] = {0};

	char pSendContent[512];
	char pUUID[128];

	OSA_MemSet(pSendContent, 0, sizeof(pSendContent));
	sprintf(pSendContent, "response_type=code");
	strcat(pSendContent, "&client_id=");
	OSA_MemSet(pUUID, 0, sizeof(pUUID));
	GetValueFromEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_UUID, pUUID, sizeof(pUUID));
	strcat(pSendContent, pUUID);

	memset(&stCertSignReq, 0, sizeof(HttpRequest));
	memset(&stCertSignResp, 0, sizeof(HttpResponse));
	// sprintf(pAccessTokenRequestAPI,"%s%s","https://my-staging.busch-jaeger.de/sso/authorize?",pSendContent);
	// sprintf(pAccessTokenRequestAPI,"%s","https://my-staging.busch-jaeger.de/api/user/access-token");
	// sprintf(pAccessTokenRequestAPI,"%s%s","https://my.busch-jaeger.de/sso/authorize?",pSendContent);
	sprintf(pAccessTokenRequestAPI, "%s%s", GetPortalServerUrl(), "/api/user/access-token");
	strncpy(stCertSignReq.szTargetURL, pAccessTokenRequestAPI, strlen(pAccessTokenRequestAPI));
	stCertSignReq.szContent		= "12345";
	stCertSignReq.iContentLen   = 5;
	stCertSignReq.iAuthType		= CURLAUTH_NONE;
	stCertSignReq.iSecured		= 1;
	stCertSignReq.iVerifyClient = 1;
	sprintf(stCertSignReq.szClientCertPath, "%s", PATH_GATEWAY_CLIENT_CERT);
	stCertSignReq.iMethod = CURLOPT_HTTPPOST;

	stCertSignResp.szContent = (char*) malloc(3 * 1024);
	memset(stCertSignResp.szContent, 0, 3 * 1024);

	if (SendHTTPRequest(&stCertSignReq, &stCertSignResp) == 0) {
		OSA_DBG_MSG("iResponseCode = %d", stCertSignResp.iResponseCode);
		OSA_DBG_MSG("stCertSignResp = %s", stCertSignResp.szContent);

		if (stCertSignResp.iResponseCode == 201) {
			OSA_DBG_MSG("stCertSignResp = %s", stCertSignResp.szContent);

			FILE* fAccessToken = NULL;

			fAccessToken = fopen(PATH_ACCESS_TOKEN, "w+");
			if (fAccessToken != NULL) {
				fwrite(stCertSignResp.szContent, strlen(stCertSignResp.szContent), 1, fAccessToken);
			}
			fclose(fAccessToken);

			ret = 0;
		}
	}

	free(stCertSignResp.szContent);
	return ret;
}

VOID UpdateDeviceToNewDomain(char* newdomain)
{
	FILE *fileOld, *fileNew;
	char  buf[256], lineAsRead[256];
	char *eol, *index;
	char  tempfile[128] = {0};
	char  pEtcFile[128] = PATH_WELCOME_CFG;
	int   pid			= 0;
	if (!(fileOld = fopen(pEtcFile, "r"))) {
		return;
	}
	// printf("UpdateDeviceToNewDomain 000\n");
	pid = getpid();
	sprintf(tempfile, "./tmp%dXXXXXX", pid);

	int outf = mkstemp(tempfile);
	if (outf == -1) {
		sprintf(tempfile, "%s.tmp", pEtcFile);
	}

	if (!(fileNew = fopen(tempfile, "w+"))) {
		fclose(fileOld);
		return;
	}
	// printf("UpdateDeviceToNewDomain 1111\n");
	while (fgets(buf, 256, fileOld) != NULL) {
		for (index = buf; *index == ' ' || *index == '\t'; index++)
			;

		if (*index == '#') {
			continue;
		}
		if (*index == '\n') {
			continue;
		}

		eol = strrchr(index, '\n');
		if (eol != NULL) {
			*eol = '\0';
		}

		strcpy(lineAsRead, buf);
		if (strchr(lineAsRead, '[') == lineAsRead) {
			fputc('\n', fileNew);
		} else if (strstr(lineAsRead, "address") == lineAsRead) {
			char  addr[128] = {0};
			char* p			= strchr(lineAsRead, '@');
			if (p) {
				// printf("old addr=%s\n",lineAsRead);
				strncpy(addr, lineAsRead, (p - lineAsRead));
				sprintf(buf, "%s@%s", addr, newdomain);
				// printf("new addr=%s\n",buf);
			}
		}

		fputs(buf, fileNew);
		fputc('\n', fileNew);
	}

	// printf("UpdateDeviceToNewDomain 2222\n");
	fprintf(fileNew, "%s", "\n");
	fclose(fileOld);
	fclose(fileNew);

	// printf("UpdateDeviceToNewDomain 3333\n");

	char shellstr[128] = {0};
	sprintf(shellstr, "cp %s %s ", tempfile, pEtcFile);
	system(shellstr);
	sprintf(shellstr, "rm %s -f", tempfile);
	system(shellstr);
	close(outf);
	// printf("UpdateDeviceToNewDomain 4444\n");
	// system("chmod 777 -R ../*");
	// rename("./tmp.cfg", pEtcFile);
	return;
}

LOCAL VOID SaveClientCertFile(VOID)
{
	char pShellCmd[1024] = {0};

	OSA_MemSet(pShellCmd, 0, sizeof(pShellCmd));
	sprintf(pShellCmd, "cp %s %s -f", PATH_GATEWAY_TMP_CLIENT_CERT, PATH_GATEWAY_CLIENT_CERT);
	system(pShellCmd);

#ifdef flexsip
	OSA_MemSet(pShellCmd, 0, sizeof(pShellCmd));
	sprintf(pShellCmd, "cp %s %s -f", PATH_GATEWAY_TMP_CLIENT_CERT, PATH_B2B_CERT);
	system(pShellCmd);
#endif

	OSA_MemSet(pShellCmd, 0, sizeof(pShellCmd));
	sprintf(pShellCmd, "rm %s -f", PATH_GATEWAY_TMP_CLIENT_CERT);
	system(pShellCmd);
}

LOCAL VOID AddUserToFlexisip(char* username, char* password)
{
	char  account[128] = {0};
	FILE* auth_fd	  = fopen(PATH_FLEXISIP_AUTH_CFG, "a+");
	if (!auth_fd) {
		printf("open certificate file failed\n");
		return;
	}
	sprintf(account, "%s@%s %s\n", username, (char*) GetDeviceDomain, password);
	int len = fwrite(account, 1, strlen(account), auth_fd);

	if (len <= 0) {
		OSA_ERROR("add user to flexisip failed");
	}
	fclose(auth_fd);
}

LOCAL VOID AddB2BUser(VOID)
{
	char passwd[16] = {0};
	OSA_MemSet(passwd, 0, sizeof(passwd));

	GetUnformattedUUID(passwd, 8);
	AddUserToFlexisip("ipgw", passwd);
	SetValueToEtcFile(PATH_B2B_CFG, B2B_SECTION_CONF, B2B_KEY_PASSWD, passwd);
	SaveIniData(PATH_B2B_CFG);
}

LOCAL VOID SaveB2BDomain(char* domain)
{
	SetValueToEtcFile(PATH_B2B_CFG, B2B_SECTION_CONF, B2B_KEY_DOMAIN, domain);
	SaveIniData(PATH_B2B_CFG);
}

VOID SaveB2BInternalUri(char* domain, int port, char* protocal)
{
	char internal_uri[128] = {0};
	OSA_MemSet(internal_uri, 0, sizeof(internal_uri));

	sprintf(internal_uri, "sip:%s:%d;transport=%s", domain, port, protocal);
	SetValueToEtcFile(PATH_B2B_CFG, B2B_SECTION_CONF, B2B_KEY_INTERNAL_URI, internal_uri);
	SaveIniData(PATH_B2B_CFG);

	// fnPortalClientSendMsg2IPGateway(PROCESS_OPER_IPGW_B2BSIPCONF_CHANGED, NULL, 0);
}

VOID SaveB2BExternalUri(char* domain, int port, char* protocal)
{
	char external_uri[128] = {0};
	OSA_MemSet(external_uri, 0, sizeof(external_uri));

	sprintf(external_uri, "sip:%s:%d;transport=%s", domain, port, protocal);
	SetValueToEtcFile(PATH_B2B_CFG, B2B_SECTION_CONF, B2B_KEY_EXTERNAL_URI, external_uri);
	SaveIniData(PATH_B2B_CFG);
}

VOID InitB2bConf(VOID)
{
	char pShellCmd[256] = {0};

	/*copy certificate/key file to b3821bsip folder*/
	if (access(PATH_GATEWAY_PRIVIATE_KEY, F_OK) == 0) {
		OSA_MemSet(pShellCmd, 0, sizeof(pShellCmd));
		sprintf(pShellCmd, "cp %s %s -f", PATH_GATEWAY_PRIVIATE_KEY, PATH_B2B_KEY);
		system(pShellCmd);

		if (access(PATH_B2B_CFG, F_OK) != 0) {
			OSA_MemSet(pShellCmd, 0, sizeof(pShellCmd));
			sprintf(pShellCmd, "touch %s", PATH_B2B_CFG);
			system(pShellCmd);
		}
	} else {
		OSA_ERROR("IPGW certificate/key not found\n");
		return;
	}

	/*add username and password for b2bsip*/
	AddB2BUser();

	/*domain*/
	SaveB2BDomain(g_ipgwdomain);

	/*exteranl uri*/
	SaveB2BExternalUri(g_SipServerDomain, B2B_EXTERNAL_PORT, B2B_EXTERNAL_PROTOCAL);

	/*inertal_uri*/
	SaveB2BInternalUri(GetExternalIP(), B2B_INTERNAL_PORT, B2B_INTERNAL_PROTOCAL);
}

int SendCertSignedRequest(char* username, char* password, char* fname)
{
	int			 ret = -1;
	HttpRequest  stCertSignReq;
	HttpResponse stCertSignResp;
	char		 pCSRRequestAPI[512] = {0};

	char* CsrIncJsonFmt = GenerateCertSignedRquest(fname); /*cjson include client-type,client-csr,client-name*/

	if (CsrIncJsonFmt == NULL) {
		printf("%sCreate CsrIncJsonFmt Failed\n", DEBUG_HEADER_PORTALCLIENT);
		return -1;
	}

	/*crs in json fmt string saved in /rundir/csr.json*/
	FILE* ftest = fopen("./csr.json", "w+");
	if (ftest != NULL) {
		fwrite(CsrIncJsonFmt, 1, strlen(CsrIncJsonFmt), ftest);
		fclose(ftest);
	} else {
		printf("%sCreate CsrIncJsonFmt Failed\n", DEBUG_HEADER_PORTALCLIENT);
		free(CsrIncJsonFmt);
		return -1;
	}

	if (username == NULL || password == NULL) {
		OSA_DBG_MSG("%susername or password is NULL", DEBUG_HEADER_PORTALCLIENT);
		free(CsrIncJsonFmt);
		return -1;
	}
	memset(&stCertSignReq, 0, sizeof(HttpRequest));
	memset(&stCertSignResp, 0, sizeof(HttpResponse));
	sprintf(pCSRRequestAPI, "%s%s", g_PortalServerUrl, API_SIGNED_REQUEST);
	strncpy(stCertSignReq.szTargetURL, pCSRRequestAPI, strlen(pCSRRequestAPI));
	stCertSignReq.szContent   = CsrIncJsonFmt;
	stCertSignReq.iContentLen = strlen(CsrIncJsonFmt);
	// stCertSignReq.iAuthType = CURLAUTH_DIGEST;
	stCertSignReq.iAuthType   = CURLAUTH_BASIC;
	stCertSignReq.iSecured	= 1;
	stCertSignReq.iVerifyPeer = 1;
	strncpy((char*) stCertSignReq.szPortalUserame, username, strlen(username));
	strncpy((char*) stCertSignReq.szPortalPassword, password, strlen(password));
	stCertSignResp.szContent = (char*) malloc(3 * 1024);
	memset((unsigned char*) stCertSignResp.szContent, 0, 3 * 1024);
	res_init();

	if (SendHTTPRequest(&stCertSignReq, &stCertSignResp) == 0) {
		OSA_DBG_MSG("%siResponseCode = %d", DEBUG_HEADER_PORTALCLIENT, stCertSignResp.iResponseCode);
		if (stCertSignResp.iResponseCode == 201) {
			OSA_DBG_MSG("%sstCertSignResp = %s", DEBUG_HEADER_PORTALCLIENT, stCertSignResp.szContent);
			if (strcmp(stCertSignResp.szContentType, "application/x-x509-user-cert") == 0) {
				SaveTempClientFile((unsigned char*) stCertSignResp.szContent, stCertSignResp.iContentLen);
				SaveClientCertFile();
				ret = 0;
#ifdef flexsip
				if (0 == SendAdditionnalCertSignedRequest(1)) {
					UpdateDeviceDomain();
					SaveClientCertFile();
					OSA_printf("%sSave Client Cert File %s \n", DEBUG_HEADER_PORTALCLIENT, PATH_GATEWAY_CLIENT_CERT);
					DeleteAllPairedApp();
					ChangeAllConfigDomain();

					InitB2bConf();

					// save portal name for generate sipname in further
					// SetValueToEtcFile(PATH_IPGW_CFG,IPGW_SECTION_PORTAL,IPGW_KEY_PORTALNAME,username);

					// save uuid in config.ini
					SetValueToEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_UUID, stCertSignResp.szheader);
					OSA_DBG_MSG("%sUpdate [Section=%s] [Key=%s] [Value=%s] in File %s\n", DEBUG_HEADER_PORTALCLIENT, WELCOME_SECTION_NETWORK, WELCOME_KEY_UUID,
								stCertSignResp.szheader, PATH_WELCOME_CFG);

					// save fname in config.ini
					SetValueToEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_GATEWAY_NAME, fname);
					OSA_DBG_MSG("%sUpdate [Section=%s] [Key=%s] [Value=%s] in File %s\n", DEBUG_HEADER_PORTALCLIENT, WELCOME_SECTION_NETWORK,
								WELCOME_KEY_GATEWAY_NAME, fname, PATH_WELCOME_CFG);

					// UpdateDeviceToNewDomain(g_ipgwdomain_new);  //What's Function
					SaveIniData(PATH_WELCOME_CFG);

					// Access Token Request, For xmpp
					// SendAccessTokenRequest();

					ret = 0;
				}
#endif
			}
		}
	}
	free(stCertSignResp.szContent);
	free(CsrIncJsonFmt);
	return ret;
}

LOCAL INT sendCertRevokeRequest(VOID)
{
	int			 ret = -1;
	HttpRequest  stCertSignReq;
	HttpResponse stCertSignResp;
	char		 request_api[512] = {0};
	memset(&stCertSignReq, 0, sizeof(HttpRequest));
	memset(&stCertSignResp, 0, sizeof(HttpResponse));
	sprintf(request_api, "%s%s", g_PortalServerUrl, API_REVOKE_CERT);
	strncpy(stCertSignReq.szTargetURL, request_api, strlen(request_api));
	stCertSignReq.szContent = (char*) malloc(3 * 1024);
	sprintf(stCertSignReq.szContent, "%s", "12345");
	stCertSignReq.iContentLen   = 5;
	stCertSignReq.iAuthType		= CURLAUTH_NONE;
	stCertSignReq.iSecured		= 1;
	stCertSignReq.iVerifyClient = 1;
	sprintf(stCertSignReq.szClientCertPath, "%s", PATH_GATEWAY_CLIENT_CERT);
	stCertSignReq.iMethod = CURLOPT_HTTPPOST;

	stCertSignResp.szContent = (char*) malloc(3 * 1024);
	memset(stCertSignResp.szContent, 0, 3 * 1024);

	if (SendHTTPRequest(&stCertSignReq, &stCertSignResp) == 0) {
		OSA_DBG_MSG("%sstCertSignResp.iResponseCode(%d)\n", DEBUG_HEADER_PORTALCLIENT, stCertSignResp.iResponseCode);

		if (stCertSignResp.iResponseCode == 200) {
			ret = 0;
		}
		OSA_DBG_MSG("%sRevoke Successfully Content(%s)\n", DEBUG_HEADER_PORTALCLIENT, stCertSignResp.szContent);
	}
	free(stCertSignReq.szContent);
	free(stCertSignResp.szContent);
	return ret;
}

VOID DeleleAllCertAndConfig(VOID)
{
	char pShellCmd[128] = {0};

#ifdef flexsip
	/*remove uuid in config.ini*/
	if (OSA_FileIsExist(PATH_WELCOME_CFG) == TRUE) {
		SetValueToEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_UUID, "");
		SaveIniData(PATH_WELCOME_CFG);
	}
#endif

	/*remove all certificates*/
	OSA_MemSet(pShellCmd, 0, sizeof(pShellCmd));
	sprintf(pShellCmd, "rm -f %s", PATH_GATEWAY_CLIENT_CERT); /*portal client */
	system(pShellCmd);

	OSA_MemSet(pShellCmd, 0, sizeof(pShellCmd));
	sprintf(pShellCmd, "rm -f %s", PATH_GATEWAY_FLEXISIP_KEY); /*flexisip */
	system(pShellCmd);

	OSA_MemSet(pShellCmd, 0, sizeof(pShellCmd));
	sprintf(pShellCmd, "rm -f %s", PATH_B2B_KEY); /*b2bsip */
	system(pShellCmd);

	/*stop b2bsip*/
	system("killall b2bsip");
}

INT RemoveDeviceFromPortalServer(VOID)
{
	int iRet = 0;

	iRet = sendCertRevokeRequest();
	LogOutDeal();
	return iRet;
}

VOID LogOutDeal(VOID)
{
	pthread_mutex_lock(&s_Mutex_PortalClientLogoutDeal);

#ifdef flexsip
	DeleleAllCertAndConfig();
#else

	if (OSA_FileIsExist(PATH_GATEWAY_CLIENT_CERT) == TRUE) {
		OSA_DBG_MSG("OSA_FileDel(PATH_GATEWAY_CLIENT_CERT)\n");
		OSA_FileDel(PATH_GATEWAY_CLIENT_CERT);
	}

#endif

	/*close websocket connection*/
	NoPollSafeLoopStop();

#ifdef flexsip
	/*remove apps*/
	DeleteAllPairedApp();
#endif

	pthread_mutex_unlock(&s_Mutex_PortalClientLogoutDeal);
}
