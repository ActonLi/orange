#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include "common.h"
#include "config.h"
#include "osa/osa_debug.h"

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif



#define ReadFromCfg_new(pSection, pKey, pValue, iLen)  GetValueFromEtcFile_new(CFG_BASIC, pSection, pKey, pValue, iLen)
#define WriteToCfg_new(pSection, pKey, pValue)         SetValueToEtcFile_new(CFG_BASIC, pSection, pKey, pValue)
#define MAX_CFG_SIZE 256
#define MAX_PASSWORD_LEN   32


//因为minigui中GetValueFromEtcFile和SetValueToEtcFile函数对pValue的长度限制为1024字节，
//这对一些应用造成影响，此处改写这两个函数来突破该限制
int GetValueFromEtcFile_new (const char* pEtcFile, 
                const char* pSection, const char* pKey, char* pValue, int iLen)
{
    FILE *file;
    char buf[MAX_CFG_SIZE], lineAsRead[MAX_CFG_SIZE];
	char *eol,*index,*pTmp;
	int findSection=0;
	int inSection=0;

    if(!(file = fopen(pEtcFile, "r")))
    {   
        //文件不存在
        return -1;
    }  

    while (fgets(buf, MAX_CFG_SIZE, file) != NULL)
    {
        for (index = buf; *index == ' ' || *index == '\t'; index++);
    	if (*index == '#' || *index == '\n')
    	{
    		continue;
    	}
    	eol = strrchr(index, '\n');
    	if (eol != NULL)
    	{
    		*eol = '\0';
    	}

        eol = strrchr(index, 0x0d);  //去除回车符
    	if (eol != NULL)
    	{
    		*eol = '\0';
    	}

    	strcpy(lineAsRead, buf);	
    	if(strchr(lineAsRead, '[') == lineAsRead)
    	{
            if(strstr(lineAsRead, pSection) != 0)
            {
                findSection = 1;
                inSection = 1;
            }
            else
            {
                inSection = 0;
            }
        }

        if(inSection && (pTmp = strchr(lineAsRead, '=')) && (strstr(lineAsRead, pKey) == lineAsRead))
        {
            memcpy(pValue, pTmp+1, min(iLen, strlen(lineAsRead)));
            fclose(file);
            return 0;
        }
    }
    fclose(file);
    return -1;
}

int SetValueToEtcFile_new (const char* pEtcFile, 
                const char* pSection, const char* pKey, char* pValue)
{
    FILE *fileOld, *fileNew;
    char buf[MAX_CFG_SIZE],lineAsRead[MAX_CFG_SIZE];
	char *eol,*index;
	int findSection=0;
	int inSection=0;
    int success=0;
    char tempfile[128] = {0};
    int outfd; 
    int pid = 0;

    if(!(fileOld = fopen(pEtcFile, "r")))
    {    //文件不存在
        if(!(fileNew = fopen(pEtcFile, "w+")))
        {
            return -1;
        }
        fprintf (fileNew, "[%s]\n%s=%s\n", pSection,pKey,pValue);
        fclose(fileNew);
        return 0;
    }
    pid = getpid();
    sprintf(tempfile,"./tmp%dXXXXXX",pid);
 
	int outf = mkstemp(tempfile);
    if(outf == -1)
    {
        sprintf(tempfile,"%s.tmp",pEtcFile);
    }
	if(!(fileNew = fopen(tempfile, "w+")))
    {
        fclose(fileOld);
        return -1;
	}
    while (fgets(buf, MAX_CFG_SIZE, fileOld) != NULL)
    {
        for (index = buf; *index == ' ' || *index == '\t'; index++);
        
        if (*index == '#' )
        {
    	    continue;
        }
        if (*index == '\n' )
        {
             continue;
        }
        
    	eol = strrchr(index, '\n');
    	if (eol != NULL)
    	{
    		*eol = '\0';
    	}
 
    	strcpy(lineAsRead, buf);	
    	if(strchr(lineAsRead, '[') == lineAsRead)
    	{
            fputc('\n', fileNew);
            if(strstr(lineAsRead, pSection) != 0)
            {            	
            	int lineLen = strlen(lineAsRead);
				int secLen = strlen(pSection);
//				OSA_DBG_MSG("lineLen=%d, secLen=%d", lineLen,secLen);
            	if(lineLen == secLen+2){	/*add by Andy 2015-03-24, avoid the phenomenon that pSection=a3 to match the value section = [a3xxx]*/
	                findSection = 1;
    	            inSection = 1;
            	}
				else
					inSection = 0;
            }
            else
            {
                inSection = 0;
            }
        }    
         
        if(inSection && (strchr(lineAsRead, '=') != 0) && (strstr(lineAsRead, pKey) == lineAsRead))
        {
            //找到对应的键值
            memset(buf, 0, sizeof(buf));
            strcpy(buf, pKey);
            strcat(buf, "=");
            strcat(buf, pValue);
            fputs(buf,  fileNew);
            fputc('\n', fileNew);
            success = 1;
        }
        else if(!success && findSection && !inSection)
        {
            //找到对应的块，但该块中没有该键值，即进入其它块时还未成功
            memset(buf, 0, sizeof(buf));
            strcpy(buf, pKey);
            strcat(buf, "=");
            strcat(buf, pValue);
            fputs(buf, fileNew);
            fputc('\n', fileNew);
            fputs(lineAsRead, fileNew);
            fputc('\n', fileNew);
            success = 1;
        }
        else
        {
            fputs(lineAsRead, fileNew);
            fputc('\n', fileNew);
        }
    }
    if(!success)   //只有不存在对应域时才有可能失败
    {
        if(inSection)
        {

            fprintf(fileNew, "%s=%s\n", pKey,pValue);
        }
        else
        {
            fprintf(fileNew, "\n[%s]\n%s=%s\n", pSection,pKey,pValue);
        }    
    }
    fprintf(fileNew, "%s", "\n");
    fclose(fileOld);
    fclose(fileNew);

    char shellstr[128] = {0};
    sprintf(shellstr,"cp %s %s ",tempfile,pEtcFile);
    //do_system1(shellstr,0);
    system(shellstr);
    sprintf(shellstr,"rm %s -f",tempfile);
    //do_system1(shellstr,0);
    system(shellstr);
    close(outf);
   // do_system1("chmod 777 -R ../*",0);
    //rename("./tmp.cfg", pEtcFile);
    return 0;
}

int DeleleSection(const char* pEtcFile, 
                const char* pSection)
{
    FILE *fileOld, *fileNew;
    char buf[MAX_CFG_SIZE],lineAsRead[MAX_CFG_SIZE];
	char *eol,*index;
	int findSection=0;
    char tempfile[128] = {0};
    int outfd; 
    int pid = 0;
    if(!(fileOld = fopen(pEtcFile, "r")))
    {    //文件不存在
 
        fclose(fileOld);
        return -1;
    }
    pid = getpid();
    sprintf(tempfile,"./tmp%dXXXXXX",pid);
	int outf = mkstemp(tempfile);
    if(outf == -1)
    {
        sprintf(tempfile,"%s.tmp",pEtcFile);
    }
    
    if(!(fileNew = fopen(tempfile, "w+")))
    {
        fclose(fileNew);
        return -1;
    }  
    while (fgets(buf, MAX_CFG_SIZE, fileOld) != NULL)
    {
        for (index = buf; *index == ' ' || *index == '\t'; index++);
        if (*index == '#' )
        {
    	    continue;
        }
    	eol = strrchr(index, '\n');
    	if (eol != NULL)
    	{
    		*eol = '\0';
    	}

        
    	strcpy(lineAsRead, buf);
//		OSA_DBG_MSGXX(" 	%s", lineAsRead);
		if(strchr(lineAsRead, '[') == lineAsRead)
    	{
            if(strstr(lineAsRead, pSection) != 0 )
            {
            	int lineLen = strlen(lineAsRead);
				int secLen = strlen(pSection);
				//OSA_DBG_MSG("lineLen=%d, secLen=%d", lineLen,secLen);
            	if(lineLen == secLen+2)
	                findSection = 1; 
				else
					findSection = 0;
            }
            else
            {
                findSection = 0; 
            }
        }    
        if(findSection == 0)
        {
            fputs(lineAsRead, fileNew);
            fputc('\n', fileNew);
        }
    }
    fclose(fileOld);
    fclose(fileNew);

    char shellstr[128] = {0};
    sprintf(shellstr,"cp %s %s ",tempfile,pEtcFile);
    system(shellstr);
    sprintf(shellstr,"rm %s -f",tempfile);
    system(shellstr);
    close(outf);
//	OSA_DBG_MSGXX("");
    return 0;
    //system("chmod 777 -R ../ini/*");
    //system("chmod 777 -R ../ini/*");
}




int SaveNetworkInfo(int mode, char *ip, char *mask, char *gw)
{
    char ipaddr1[4];
    char ipaddr2[4];
    char ipaddr3[4];
    char ipaddr4[4];

    char c_mode[2];  //0 or 1(static or dhcp)
    

    memset(c_mode, 0, sizeof(c_mode));
    snprintf(c_mode, 2, "%d", mode);
    if(0 != WriteToCfg_new("IP", "mode", c_mode))
    {
        return -1;
    }

    memset(ipaddr1, 0, sizeof(ipaddr1));
    memset(ipaddr2, 0, sizeof(ipaddr2));
    memset(ipaddr3, 0, sizeof(ipaddr3));
    memset(ipaddr4, 0, sizeof(ipaddr4));
    if(0==split_ip(ip, ipaddr1, ipaddr2, ipaddr3, ipaddr4))
    {
        WriteToCfg_new("IP", "IPaddr1", ipaddr1);
        WriteToCfg_new("IP", "IPaddr2", ipaddr2);
        WriteToCfg_new("IP", "IPaddr3", ipaddr3);
        WriteToCfg_new("IP", "IPaddr4", ipaddr4);
    }
    else
    {
        return -1;
    }    

    memset(ipaddr1, 0, sizeof(ipaddr1));
    memset(ipaddr2, 0, sizeof(ipaddr2));
    memset(ipaddr3, 0, sizeof(ipaddr3));
    memset(ipaddr4, 0, sizeof(ipaddr4));
    if(0==split_ip(mask, ipaddr1, ipaddr2, ipaddr3, ipaddr4))
    {
        WriteToCfg_new("IP", "submask1", ipaddr1);
        WriteToCfg_new("IP", "submask2", ipaddr2);
        WriteToCfg_new("IP", "submask3", ipaddr3);
        WriteToCfg_new("IP", "submask4", ipaddr4);
    }
    else
    {
        return -1;
    }

    memset(ipaddr1, 0, sizeof(ipaddr1));
    memset(ipaddr2, 0, sizeof(ipaddr2));
    memset(ipaddr3, 0, sizeof(ipaddr3));
    memset(ipaddr4, 0, sizeof(ipaddr4));
    if(0==split_ip(gw, ipaddr1, ipaddr2, ipaddr3, ipaddr4))
    {
        WriteToCfg_new("IP", "gateway1", ipaddr1);
        WriteToCfg_new("IP", "gateway2", ipaddr2);
        WriteToCfg_new("IP", "gateway3", ipaddr3);
        WriteToCfg_new("IP", "gateway4", ipaddr4);
    }
    else
    {
        return -1;
    }

    return 0;

}

int SaveCpInfo(char *cp1, char *cp2, char *cp3, char *cp4)
{
    char ipaddr1[4];
    char ipaddr2[4];
    char ipaddr3[4];
    char ipaddr4[4];
    
    memset(ipaddr1, 0, sizeof(ipaddr1));
    memset(ipaddr2, 0, sizeof(ipaddr2));
    memset(ipaddr3, 0, sizeof(ipaddr3));
    memset(ipaddr4, 0, sizeof(ipaddr4));
    if(0==split_ip(cp1, ipaddr1, ipaddr2, ipaddr3, ipaddr4))
    {
        WriteToCfg_new("CP", "CP_IP1_1", ipaddr1);
        WriteToCfg_new("CP", "CP_IP1_2", ipaddr2);
        WriteToCfg_new("CP", "CP_IP1_3", ipaddr3);
        WriteToCfg_new("CP", "CP_IP1_4", ipaddr4);
    }
    else
    {
        return -1;
    }    
    
    memset(ipaddr1, 0, sizeof(ipaddr1));
    memset(ipaddr2, 0, sizeof(ipaddr2));
    memset(ipaddr3, 0, sizeof(ipaddr3));
    memset(ipaddr4, 0, sizeof(ipaddr4));
    if(0==split_ip(cp2, ipaddr1, ipaddr2, ipaddr3, ipaddr4))
    {
        WriteToCfg_new("CP", "CP_IP2_1", ipaddr1);
        WriteToCfg_new("CP", "CP_IP2_2", ipaddr2);
        WriteToCfg_new("CP", "CP_IP2_3", ipaddr3);
        WriteToCfg_new("CP", "CP_IP2_4", ipaddr4);
    }
    else
    {
        return -1;
    }   

    memset(ipaddr1, 0, sizeof(ipaddr1));
    memset(ipaddr2, 0, sizeof(ipaddr2));
    memset(ipaddr3, 0, sizeof(ipaddr3));
    memset(ipaddr4, 0, sizeof(ipaddr4));
    if(0==split_ip(cp3, ipaddr1, ipaddr2, ipaddr3, ipaddr4))
    {
        WriteToCfg_new("CP", "CP_IP3_1", ipaddr1);
        WriteToCfg_new("CP", "CP_IP3_2", ipaddr2);
        WriteToCfg_new("CP", "CP_IP3_3", ipaddr3);
        WriteToCfg_new("CP", "CP_IP3_4", ipaddr4);
    }
    else
    {
        return -1;
    }    

    memset(ipaddr1, 0, sizeof(ipaddr1));
    memset(ipaddr2, 0, sizeof(ipaddr2));
    memset(ipaddr3, 0, sizeof(ipaddr3));
    memset(ipaddr4, 0, sizeof(ipaddr4));
    if(0==split_ip(cp4, ipaddr1, ipaddr2, ipaddr3, ipaddr4))
    {
        WriteToCfg_new("CP", "CP_IP4_1", ipaddr1);
        WriteToCfg_new("CP", "CP_IP4_2", ipaddr2);
        WriteToCfg_new("CP", "CP_IP4_3", ipaddr3);
        WriteToCfg_new("CP", "CP_IP4_4", ipaddr4);
    }
    else
    {
        return -1;
    }    

    return 0;

}

int SavePassword(char *pass)
{
    char password[MAX_PASSWORD_LEN+1];

    memset(password, 0, sizeof(password));
    memcpy(password, pass, MAX_PASSWORD_LEN);
    
    return WriteToCfg_new("PASS", "pass", password);
}


int SaveAdapterType(int type)
{
    char c_type[2];  //0 or 1
        
    memset(c_type, 0, sizeof(c_type));
    snprintf(c_type, 2, "%d", type);
    if(0 != WriteToCfg_new("TYPE", "MasterOrSlave", c_type))
    {
        return -1;
    }

    return 0;
}        

int SaveAdapterUnlockDevNo(int unlockDevNo)
{
    char c_unlockDevNo[2]; //0 or 1
        
    memset(c_unlockDevNo, 0, sizeof(c_unlockDevNo));
    snprintf(c_unlockDevNo, 2, "%d", unlockDevNo);
    if(0 != WriteToCfg_new("UNLOCKDEVNO", "UnlockDevNo", c_unlockDevNo))
    {
        return -1;
    }

    return 0;
}        

int SaveAdapterId(int id)
{
    char c_id[3]; //00 ~ 99
        
    memset(c_id, 0, sizeof(c_id));
    snprintf(c_id, 3, "%d", id);
    if(0 != WriteToCfg_new("ID", "Id", c_id))
    {
        return -1;
    }

    return 0;
}        

#if 0
int ModifyAdapterSerialno(char *fileName)
{
    FILE *fileOld, *fileNew;
    char buf[MAX_CFG_SIZE], newbuf[MAX_CFG_SIZE];
    char *ptr1, *ptr2;
    char mac[20];

    if(!(fileOld = fopen(fileName, "r")))
    {  
        return -1;
    }
    
    if(!(fileNew = fopen("./tmpfile", "w+")))
    {
        fclose(fileOld);
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    while (fgets(buf, MAX_CFG_SIZE, fileOld) != NULL)
    {
    	if(strstr(buf, "<UDN>") !=  NULL)
    	{
            memset(newbuf, 0, sizeof(newbuf));
            memset(mac, 0, sizeof(mac));
            getLocalMacAddr(mac, SIZE_OF_ARRAY(mac) );
            
            if(strstr(buf, mac)==NULL)
            {
                ptr1 = strchr(buf, '>');
                ptr2 = strchr(ptr1+1, '<');
                *(ptr1+1) = '\0';
                strcpy(newbuf, buf);
                strcat(newbuf, "uuid:");
                strcat(newbuf, mac);
                strcat(newbuf, ptr2);
                fputs(newbuf, fileNew);
                continue;
            }
        }
        fputs(buf, fileNew);
    }   

    fclose(fileOld);
    fclose(fileNew);

    rename("./tmpfile", "./web/adapterdevicedesc.xml");
    return 0;
}



#endif

void saveIdToCfgFile(char *unit, char *apartment, char *externIP){
	SetValueToEtcFile_new(CFG_BASIC,"community", "unit", unit);  
	SetValueToEtcFile_new(CFG_BASIC,"community", "apartment", apartment); 
	SetValueToEtcFile_new(CFG_BASIC,"community", "externIP", externIP);  	
}


int FindFirstVailUser()
{
    int i = 0;
    static char section[8] = {0};
    char user1[16] = {0};
    for(i=0;i<=2;i++)
    {
        sprintf(section,"%s_%d","user",i);
        int ret =GetValueFromEtcFile_new(CFG_USER, section,"user",user1, 16);    
        if(ret == -1)
        {
            return i;
        }
    }
     return 0;
}





