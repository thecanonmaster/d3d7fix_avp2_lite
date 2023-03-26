#include "StdAfx.h"

LPDIRECTDRAW7 g_ddMainDDraw = NULL;
ILTClient* g_pLTClient = NULL;
CClientMgrBase* g_pClientMgr = NULL;
HWND g_hWindowHandle = NULL;
BOOL g_bWindowHooked = FALSE;

char* g_szConfigBytesCols = "INTEL_HD|RADEON|NO_BF_LIGHTS|DG_VOODOO|COOP_MOD_COMPAT";
char g_szConfigBytes[64] = "X00110X";

DWORD GetConfigValue(eConfigOption eOption)
{
	return g_szConfigBytes[eOption] - 0x30;
}

void SetConfigValue(eConfigOption eOption, DWORD dwSet)
{
	g_szConfigBytes[eOption] = (char)(dwSet + 0x30);
}

void EngineHack_WriteData(LPVOID lpAddr, BYTE* pNew, BYTE* pOld, DWORD dwSize)
{
	DWORD dwOldProtect, dwTemp;
	void* pContent = (DWORD*)lpAddr;
	
	VirtualProtect(lpAddr, dwSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	
	memcpy(pOld, pContent, dwSize);
	memcpy(pContent, pNew, dwSize);
	
	VirtualProtect(lpAddr, dwSize, dwOldProtect, &dwTemp);
}

void EngineHack_WriteFunction(LPVOID lpAddr, DWORD dwNew, DWORD& dwOld)
{
	DWORD dwOldProtect, dwTemp;
	DWORD* dwContent = (DWORD*)lpAddr;
	
	VirtualProtect(lpAddr, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);	
	dwOld = dwContent[0];
	dwContent[0] = dwNew;
	VirtualProtect(lpAddr, 4, dwOldProtect, &dwTemp);
}

void EngineHack_WriteCall(LPVOID lpAddr, DWORD dwNew, BOOL bStructCall)
{
	DWORD dwOldProtect, dwTemp;
	BYTE* pContent = (BYTE*)lpAddr;
	DWORD* pCodeContent = (DWORD*)(pContent + 1);
	DWORD dwCallCode = dwNew - (DWORD)lpAddr - 5;
	
	if (bStructCall)
	{
		VirtualProtect(lpAddr, 6, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		
		pContent[0] = 0xE8; 
		pCodeContent[0] = dwCallCode;
		pContent[5] = 0x90;
		
		VirtualProtect(lpAddr, 6, dwOldProtect, &dwTemp);
	}
	else
	{
		VirtualProtect(lpAddr, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		
		pContent[0] = 0xE8; 
		pCodeContent[0] = dwCallCode;
		
		VirtualProtect(lpAddr, 5, dwOldProtect, &dwTemp);
	}
}

void EngineHack_AllowWrite(LPVOID lpAddr, DWORD dwSize)
{
	DWORD dwOldProtect;	
	VirtualProtect(lpAddr, dwSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);	
}

void EngineHack_WriteJump(LPVOID lpAddr, DWORD dwNew)
{
	DWORD dwOldProtect, dwTemp;
	BYTE* pContent = (BYTE*)lpAddr;
	DWORD* pCodeContent = (DWORD*)(pContent + 1);
	DWORD dwCallCode = dwNew - (DWORD)lpAddr - 5;
	
	
	VirtualProtect(lpAddr, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	
	pContent[0] = 0xE9; 
	pCodeContent[0] = dwCallCode;
	
	VirtualProtect(lpAddr, 5, dwOldProtect, &dwTemp);
}

void logf(char *msg, ...)
{	
	FILE* pLogFile = fopen("ltmsg.log", "a");

	va_list argp;
	
	va_start(argp, msg);
	vfprintf(pLogFile, msg, argp);	
	va_end(argp);
	
	fflush(pLogFile);

	fclose(pLogFile);
}