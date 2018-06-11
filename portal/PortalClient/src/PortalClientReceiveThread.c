#include "PortalClient.h"

LOCAL OSA_ThrHndl Thread_portalconnection = {0};

LOCAL noPollCtx *ctx = NULL;
LOCAL noPollConn *conn =NULL;

LOCAL pthread_mutex_t  s_Mutex_Websocket;
LOCAL pthread_mutex_t event_mutex_cond =  PTHREAD_MUTEX_INITIALIZER; 
LOCAL pthread_cond_t event_cond = PTHREAD_COND_INITIALIZER;


LOCAL INT s_iConnectState = eStateDisconnected;
LOCAL INT s_iHeartBeatSendCount = 1;
LOCAL INT s_iheartTimerHandle = -1;
LOCAL INT s_iFragmentTotalLen = 0;
LOCAL char s_pWebSocketPayload[WEB_SOCKET_PAYLOAD_MAXLEN] = {0};  


noPollConn * GetNoPollConn(VOID)
{
	return conn;
}

VOID SetHeartBeatCount(INT iValue)
{
	s_iHeartBeatSendCount = iValue;
}

VOID LockWebsocketMutex(VOID)
{
	OSA_DBG_MSG("%s===========%s",DEBUG_HEADER_PORTALCLIENT,__func__);
	pthread_mutex_lock(&s_Mutex_Websocket);
}

VOID UnLockWebsocketMutex(VOID)
{
	OSA_DBG_MSG("%s===========%s",DEBUG_HEADER_PORTALCLIENT,__func__);
	pthread_mutex_unlock(&s_Mutex_Websocket);
}

VOID LockEventMutex(VOID)
{
	OSA_DBG_MSG("%s===========%s",DEBUG_HEADER_PORTALCLIENT,__func__);
	pthread_mutex_lock(&event_mutex_cond);
}

VOID UnLockEventMutex(VOID)
{
	OSA_DBG_MSG("%s===========%s",DEBUG_HEADER_PORTALCLIENT,__func__);
	pthread_mutex_unlock(&event_mutex_cond);
}

VOID UnBlockEventCond(VOID)
{
	OSA_DBG_MSG("%s===========%s",DEBUG_HEADER_PORTALCLIENT,__func__);
	pthread_cond_broadcast(&event_cond);
}

INT WaitRespForPushEventToPortalServer(struct timespec * pstWaittime)
{
	OSA_DBG_MSG("%s===========%s",DEBUG_HEADER_PORTALCLIENT,__func__);
	return pthread_cond_timedwait(&event_cond, &event_mutex_cond, pstWaittime);
}

INT GetConnectState(VOID)
{
	return s_iConnectState;
}

INT NoPollSafeLoopStop(VOID)
{ 
	s_iConnectState = eStateDisconnected;
	if(ctx != NULL)
	{
		nopoll_loop_stop(ctx);
	}

	return 0;
}

LOCAL INT HandleSendHeartbeat(timer_id id, void *user_data, int len)
{
	/*Push Ping Event*/
	InternalEventInfo stinfo;
	memset(&stinfo,0,sizeof(stinfo));
	stinfo.type = eEventPing;
	fnSendMsg2PortClientSendThread((BYTE*)&stinfo,sizeof(InternalEventInfo));

	OSA_DBG_MSG("%sSend Ping Count = %d\n",DEBUG_HEADER_PORTALCLIENT,s_iHeartBeatSendCount);
	
	if(s_iHeartBeatSendCount++ > 6)  //60s 内会发送6次心态包，如果都没有收到应答，则认为与Portal Server 的通讯异常，
	{
		OSA_DBG_MSG("%sDetect Network Abnormal, Disconnect Portal Server\n",DEBUG_HEADER_PORTALCLIENT);
		NoPollSafeLoopStop();
	}

	return 0;

}

LOCAL VOID KillHeartBeatTimer(VOID)
{
	OSA_KillTimer(s_iheartTimerHandle);
	s_iHeartBeatSendCount = 1;
}

LOCAL VOID ClearHeartBeatSendCount(VOID)
{
	s_iHeartBeatSendCount = 1;
}


LOCAL VOID HandlePeerEvent(char* strevent)
{
	cJSON *root = cJSON_Parse(strevent);
	if(root == NULL)
	{
		OSA_DBG_MSG("%sIt is Not a Valid Message In Joson Format, Discard it",DEBUG_HEADER_PORTALCLIENTRECEIVE);
		return ;
	}
	cJSON *uuID = cJSON_GetObjectItem(root,"id");
	cJSON *Type = cJSON_GetObjectItem(root,"type");
	cJSON *belongsto=  cJSON_GetObjectItem(root,"belongsTo");
	cJSON *payload = cJSON_GetObjectItem(root,"payload");
	cJSON *pSender = cJSON_GetObjectItem(root,"sender");
	char* pDecPayload = NULL;
	int b64payloadlen = 0;

	if(payload != NULL)
	{
		b64payloadlen = strlen(payload->valuestring);
		pDecPayload = (char* )OSA_MemMalloc(b64payloadlen);
		Base64Dec((unsigned char *)pDecPayload,payload->valuestring,b64payloadlen);
	}
	if(Type == NULL)
	{
		if(pDecPayload != NULL)
			OSA_MemFree(pDecPayload);
		cJSON_Delete(root);
		OSA_DBG_MSG("%sError:no type item in event \n",DEBUG_HEADER_PORTALCLIENTRECEIVE);
		return;
	}
	OSA_DBG_MSG("%sReceive Event Type = %s\n",DEBUG_HEADER_PORTALCLIENTRECEIVE,Type->valuestring);
	//处理事件推送成功应答
	if(strcmp(Type->valuestring,EVENT_TYPE_SUCCESS) == 0)
	{
		OSA_DBG_MSG("%sReceive Success Event to %s\n",DEBUG_HEADER_PORTALCLIENTRECEIVE,cJSON_GetObjectItem(root,"belongsto")->valuestring);
	
		if(uuID != NULL)
		{
			if(strcmp(belongsto->valuestring,GetEventUUID()) ==0)
			{
				OSA_DBG_MSG("%sReceive Success Response For UUID = %s\n",DEBUG_HEADER_PORTALCLIENTRECEIVE,GetEventUUID());
				LockEventMutex();
       		    UnBlockEventCond();
				UnLockEventMutex();
				ClearHeartBeatSendCount();
			}
		}
	} //处理手机的配对请求
	else if(strcmp(Type->valuestring,EVENT_TYPE_ACL_REQUEST) == 0)
	{
		OSA_DBG_MSG("%sRecv Event Payload: %s\n",DEBUG_HEADER_PORTALCLIENTRECEIVE,pDecPayload);
 		HandleWelcomeAppPairRequest(pDecPayload);
	}//处理手机的取消配对请求
	else if(strcmp(Type->valuestring,EVENT_TYPE_ACL_REVOKE) == 0)
	{
		OSA_DBG_MSG("%sRecv Event Payload: %s\n",DEBUG_HEADER_PORTALCLIENTRECEIVE,pDecPayload);
 		
		if(pSender != NULL)
		{
			char * pAppRevokeUUID = (char *)malloc(strlen(pSender->valuestring) + 1);
			sprintf(pAppRevokeUUID ,"%s",pSender->valuestring);
			OSA_ThrHndl ThreadACLRevoke;
			int ret = OSA_ThreadCreate(&ThreadACLRevoke, DeleteAppBySenderUUID, (void *)pAppRevokeUUID);
			if(ret!=OSA_SOK)
			{
				perror ("Create Cancle Pairing APP Thread Error!\n");
			}
		}
	}
	
	
	/*release mem*/
	if(pDecPayload != NULL)
	{
		OSA_MemFree(pDecPayload);
	}
	cJSON_Delete(root);

}

/*消息处理回调函数*/
LOCAL VOID DealMessageFromPortalServer(noPollCtx *ctx, noPollConn *conn,noPollMsg *msg, noPollPtr user_data) 
{
	if (! nopoll_conn_is_ok (GetNoPollConn())) 
	{
	     fprintf (stderr, "ERROR: received websocket connection close during wait reply..\n");    
		 return;
	}
	
	if(nopoll_msg_get_payload_opcode(msg) == NOPOLL_PONG_FRAME)
	{
		OSA_DBG_MSG("%sRecv msg:pong From Portal Server\n",DEBUG_HEADER_PORTALCLIENTRECEIVE);
		s_iHeartBeatSendCount = 1;
		return;
	}

	const noPollPtr pPayLoad = nopoll_msg_get_payload(msg);
	int iPayloadSize = nopoll_msg_get_payload_size(msg);

	if(strcmp(EVENT_PORTAL_DELETE_DEVICE,(char*)pPayLoad) == 0)  //client被删除
	{
		OSA_DBG_MSG("%s Receive msg:%s From Portal Server",DEBUG_HEADER_PORTALCLIENT,EVENT_PORTAL_DELETE_DEVICE);
		LogOutDeal();
		return;
	}
	
	//OSA_DBG_MSG("%siFragmentTotalLen =%d PayloadLen(this time)=%d\n",DEBUG_HEADER_PORTALCLIENT,s_iFragmentTotalLen,iPayloadSize);
	
	if(s_iFragmentTotalLen + iPayloadSize < WEB_SOCKET_PAYLOAD_MAXLEN)
	{
		memcpy(s_pWebSocketPayload + s_iFragmentTotalLen,pPayLoad,iPayloadSize);
		s_iFragmentTotalLen += iPayloadSize;
	}
	else
	{
		OSA_DBG_MSG("%sThe Payload Data'Size Large than %d ! Discard it\n",DEBUG_HEADER_PORTALCLIENT,WEB_SOCKET_PAYLOAD_MAXLEN);
		memset(s_pWebSocketPayload,0,10 * 1024);
		s_iFragmentTotalLen = 0;
		return;
	}
	
	if(nopoll_msg_is_final(msg) == nopoll_true)  //应该是分片传输，如果nopoll_true 代表完整报文接收成功
	{
		//OSA_DBG_MSG("%sRecv Msg From Portal Server: %100.100s  \n",DEBUG_HEADER_PORTALCLIENT, (char*)s_pWebSocketPayload);
		OSA_DBG_MSG("%sRecv Msg From Portal Server: %s  \n",DEBUG_HEADER_PORTALCLIENT, (char*)s_pWebSocketPayload);
		HandlePeerEvent((char*)s_pWebSocketPayload);
		memset(s_pWebSocketPayload,0,10 * 1024);
		s_iFragmentTotalLen = 0;
	}
}

LOCAL VOID cb_close_handler(noPollCtx  * ctx,noPollConn * conn, noPollPtr    user_data)
{
	OSA_DBG_MSG("%sConnection Close\n",DEBUG_HEADER_PORTALCLIENT);
}	


LOCAL VOID StartConnectWithPortalService(void *argv)
{
	INT iConnectWaitCount = 0;
	INT iConnectResult = 0;
		
	ctx = nopoll_ctx_new(); 
	
	if (! ctx) {   
	  OSA_DBG_MSG("%sCreate nopoll Context Error\n",DEBUG_HEADER_PORTALCLIENT);
	  nopoll_ctx_unref (ctx);
	  return;
	} 

	//nopoll_log_enable (ctx, nopoll_true);  //打开log 便于debug
	
	nopoll_ctx_set_certificate(ctx,"",PATH_GATEWAY_CLIENT_CERT,PATH_GATEWAY_PRIVIATE_KEY,FILE_CERT_CA);  //client 必须需要证书和私钥以及CA证书完成TLS流程

	while(1)
	{
		iConnectWaitCount = 0;
		iConnectResult = 0;
			
		if(access(PATH_GATEWAY_CLIENT_CERT,F_OK) != 0)  //没有证书延时10s重复验证，直到有注册成功后，执行连接
		{
			OSA_DBG_MSG("%sNot find signed certificate\n",DEBUG_HEADER_PORTALCLIENT);
			sleep(10);
			continue;
		}

		OSA_DBG_MSG("\n%sTry Connect To Portal Server %s\n",DEBUG_HEADER_PORTALCLIENT,GetPortalServerUrl());
		conn  = nopoll_conn_tls_new (ctx,NULL,(const char *)GetPortalDomain(),(const char *)PORTAL_WSS_PORT, NULL, (const char *)WSS_API, NULL, (const char *)"origin");
		if (! nopoll_conn_is_ok (GetNoPollConn())) {   //连接失败则延时1s钟重新连接
			 sleep(1);
	  		 fprintf(stderr, "%sConnect to portal failed due to DNS or tcp or ssl\n",DEBUG_HEADER_PORTALCLIENT);             
	    	 continue; 
        }
		
		while(1)
		{
			/*验证握手是否成功，最多轮训10次*/
			if (nopoll_true != nopoll_conn_is_ready (GetNoPollConn())) 
			{     // some error handling here }
				if(nopoll_conn_get_handshake_error() == 2) /*deleted by portal*/
				{
					OSA_DBG_MSG("%sRejected by Portal Server Due to IPGateway was Deleted",DEBUG_HEADER_PORTALCLIENT);
					//DeleleAllCertAndConfig();
					//DeleteAllPairedApp();
					LogOutDeal();
					break;
				}
				if(iConnectWaitCount < 10)
				{
					iConnectWaitCount++;
					sleep(1);
					continue;
				}
				else
				{					
					OSA_DBG_MSG("%sConnect to Portal Failed Due to Handshake Timeout\n",DEBUG_HEADER_PORTALCLIENT);
					break;
				}
			}
			else
			{
				iConnectResult = 1;
				break;
			}
		}
		
		if(iConnectResult == 0)
		{
			nopoll_conn_close(GetNoPollConn());
			continue;
		}

		SetPortalClientConnectStatus(PORTAL_CLIENT_STATUS_CONNECT_SUCCESS);

		//连接成功
 		s_iConnectState = eStateEstablish;
		
		//连接成功马上推送版本信息
		InternalEventInfo stinfo;
		memset(&stinfo,0,sizeof(stinfo));
		stinfo.type = eEventDeviceInfo;
		fnSendMsg2PortClientSendThread((unsigned char*)&stinfo,sizeof(InternalEventInfo));

		/*update local-address in config.ini */
		UpdateFlexisipServeIPAddress();
		
		/*update program button*/
		UpdateProgramButton();

		/*update route.ini*/
		UpdateRecordToRouteFile();

		//连接成功马上推送APP各自的ACL信息给各自的APP，如果ACL信息存在的话
		NotifyPortalServerACLForAllPairedApp();

		/*创建10s 定时器，用来发送心态包, 如果连发6次心跳包都没有收到应答，那么nopoll_loop_stop 会被执行*/
		s_iheartTimerHandle = OSA_SetTimer(HEARTBEAT_INTERVAL, eTimerContinued, HandleSendHeartbeat, NULL);

		/*设置消息处理回调函数*/
		nopoll_ctx_set_on_msg(ctx, DealMessageFromPortalServer, NULL);

		/*设置结束处理回调函数*/
		nopoll_conn_set_on_close(GetNoPollConn(), cb_close_handler, NULL);
		
		/*如果没有nopoll_loop_stop，程序 一直会在nopoll_loop_wait 函数中监听消息*/
		/*nopoll_loop_stop被调用的条件: (1)ping 6次没有收到pong (2)push event 没有应答(3)Client 被Portal Server Delete*/
		nopoll_loop_wait(ctx, 0); 

		/*如果nopoll_loop_stop 被调用，那么后面的流程会被执行*/
		KillHeartBeatTimer();

		LockWebsocketMutex();
		nopoll_conn_close(GetNoPollConn());
		conn = NULL;
		UnLockWebsocketMutex();
		
		SetPortalClientConnectStatus(PORTAL_CLIENT_STATUS_CONNECT_FAIL);
	}

	nopoll_ctx_unref (ctx);
	ctx = NULL;

}


/*与Portal Client 建立连接，并且监听处理Portal Server 的消息*/

VOID fnCreatePortalClientReceiveThread(VOID)
{
 	int iRet = OSA_ThrCreate(&Thread_portalconnection, (void *)StartConnectWithPortalService, OSA_THR_PRI_FIFO_MIN, NULL);

	if( iRet != OSA_SOK )
	{
		OSA_ERROR("%sCreate Portal Client Receive Thread Error!\n",DEBUG_HEADER_PORTALCLIENT);
		exit(1);
	}
}

