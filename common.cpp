#include "StdAfx.h"

LPDIRECTDRAW7 g_ddMainDDraw = NULL;
ILTClient* g_pLTClient = NULL;
CClientMgrBase* g_pClientMgr = NULL;

char* g_szConfigBytesCols = "INTEL_HD|RADEON|NO_BF_LIGHTS|DG_VOODOO";
char g_szConfigBytes[64] = "X0310X";

DWORD GetConfigValue(eConfigOption eOption)
{
	return g_szConfigBytes[eOption] - 0x30;
}

void SetConfigValue(eConfigOption eOption, DWORD dwSet)
{
	g_szConfigBytes[eOption] = (char)(dwSet + 0x30);
}

void EngineHack_WriteData(HANDLE hProcess, LPVOID lpAddr, BYTE* pNew, BYTE* pOld, DWORD dwSize)
{
	DWORD dwOldProtect, dwTemp;
	void* pContent = (DWORD*)lpAddr;
	
	VirtualProtectEx(hProcess, lpAddr, dwSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	
	memcpy(pOld, pContent, dwSize);
	memcpy(pContent, pNew, dwSize);
	
	VirtualProtectEx(hProcess, lpAddr, dwSize, dwOldProtect, &dwTemp);
}

void EngineHack_WriteFunction(HANDLE hProcess, LPVOID lpAddr, DWORD dwNew, DWORD& dwOld)
{
	DWORD dwOldProtect, dwTemp;
	DWORD* dwContent = (DWORD*)lpAddr;
	
	VirtualProtectEx(hProcess, lpAddr, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);	
	dwOld = dwContent[0];
	dwContent[0] = dwNew;
	VirtualProtectEx(hProcess, lpAddr, 4, dwOldProtect, &dwTemp);
}

void EngineHack_WriteCall(HANDLE hProcess, LPVOID lpAddr, DWORD dwNew, BOOL bStructCall)
{
	DWORD dwOldProtect, dwTemp;
	BYTE* pContent = (BYTE*)lpAddr;
	DWORD* pCodeContent = (DWORD*)(pContent + 1);
	DWORD dwCallCode = dwNew - (DWORD)lpAddr - 5;
	
	if (bStructCall)
	{
		VirtualProtectEx(hProcess, lpAddr, 6, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		
		pContent[0] = 0xE8; 
		pCodeContent[0] = dwCallCode;
		pContent[5] = 0x90;
		
		VirtualProtectEx(hProcess, lpAddr, 6, dwOldProtect, &dwTemp);
	}
	else
	{
		VirtualProtectEx(hProcess, lpAddr, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		
		pContent[0] = 0xE8; 
		pCodeContent[0] = dwCallCode;
		
		VirtualProtectEx(hProcess, lpAddr, 5, dwOldProtect, &dwTemp);
	}
}

void EngineHack_AllowWrite(HANDLE hProcess, LPVOID lpAddr, DWORD dwSize)
{
	DWORD dwOldProtect;	
	VirtualProtectEx(hProcess, lpAddr, dwSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);	
}

void EngineHack_WriteJump(HANDLE hProcess, LPVOID lpAddr, DWORD dwNew)
{
	DWORD dwOldProtect, dwTemp;
	BYTE* pContent = (BYTE*)lpAddr;
	DWORD* pCodeContent = (DWORD*)(pContent + 1);
	DWORD dwCallCode = dwNew - (DWORD)lpAddr - 5;
	
	
	VirtualProtectEx(hProcess, lpAddr, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	
	pContent[0] = 0xE9; 
	pCodeContent[0] = dwCallCode;
	
	VirtualProtectEx(hProcess, lpAddr, 5, dwOldProtect, &dwTemp);
}
