#ifndef EDIT_FILE
#define EDIT_FILE

extern int GetValueFromEtcFile_new (const char* pEtcFile, const char* pSection, const char* pKey, char* pValue, int iLen);
extern int SetValueToEtcFile_new (const char* pEtcFile,const char* pSection, const char* pKey, char* pValue);

#endif
