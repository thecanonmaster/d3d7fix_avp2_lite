#include "StdAfx.h"

typedef HRESULT (WINAPI* DirectDrawCreate_Type)(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
typedef HRESULT (WINAPI* DirectDrawCreateEx_Type)( GUID FAR *, LPVOID *, REFIID,IUnknown FAR *);
HMODULE WINAPI MyLoadLibraryA(LPCSTR lpFileName);

void HookEngineStuff1();

HRESULT __stdcall FakeD3DDevice_Load(LPDIRECT3DDEVICE7 lpDevice, LPDIRECTDRAWSURFACE7 lpDestTex,LPPOINT lpDestPoint,LPDIRECTDRAWSURFACE7 lpSrcTex,LPRECT lprcSrcRect,DWORD dwFlags)
{
	return lpDestTex->Blt(NULL, lpSrcTex, NULL, DDBLT_WAIT, 0);
}

char g_szParentExeFilename[MAX_PATH + 1];
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			GetModuleFileName(NULL, g_szParentExeFilename, MAX_PATH);

			if (strstr(g_szParentExeFilename, "lithtech"))
			{
				timeBeginPeriod(1);
				HookEngineStuff1();
			}
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			if (strstr(g_szParentExeFilename, "lithtech"))
			{
				timeEndPeriod(1);
			}
		}
		break;
	}
	return TRUE;
}

void ApplyIntelHD_RHW_Fix()
{
	DWORD dwDLLAddress = (DWORD)GetModuleHandle(g_szRenderDll);
	
	float fIntelHDFix = 0.5f;
	TLVertex* pVert = (TLVertex*)(dwDLLAddress + ADDR_D3D_LIGHTADD_POLY_RHW); // 0x58500
	pVert[0].rhw = fIntelHDFix;
	pVert[1].rhw = fIntelHDFix;
	pVert[2].rhw = fIntelHDFix;
	pVert[3].rhw = fIntelHDFix;
	
	pVert = (TLVertex*)(dwDLLAddress + ADDR_D3D_LIGHTSCALE_POLY_RHW); // 0x58668
	pVert[0].rhw = fIntelHDFix;
	pVert[1].rhw = fIntelHDFix;
	pVert[2].rhw = fIntelHDFix;
	pVert[3].rhw = fIntelHDFix;
}

DWORD (*OldCreateObject)(ObjectCreateStruct *pStruct);
DWORD MyCreateObject(ObjectCreateStruct *pStruct)
{
	if (pStruct->m_ObjectType == OT_LIGHT)
		pStruct->m_Flags |= FLAG_DONTLIGHTBACKFACING;
	
	return OldCreateObject(pStruct);
}

#ifdef PRIMAL_HUNT_BUILD
BYTE g_anLoadRenderLibCode1[6];
BYTE g_anLoadRenderLibCode2[6];
#endif

void HookEngineStuff1()
{	
	if (GetConfigValue(CO_DGVOODOO_MODE))
	{
		SetConfigValue(CO_INTEL_HD, 0);
		SetConfigValue(CO_RADEON_5700, 0);
	}
	
	DWORD dwRead;
	DWORD dwExeAddress = (DWORD)GetModuleHandle(LITHTECH_EXE);
	
	EngineHack_WriteFunction((LPVOID)(dwExeAddress + ADDR_LOAD_LIBRARY), (DWORD)MyLoadLibraryA, dwRead); // 0x0C6100

#ifdef PRIMAL_HUNT_BUILD
	memcpy(g_anLoadRenderLibCode1, (LPVOID)(dwExeAddress + ADDR_LOAD_RENDER_LIB_CODE1), 6); // 0x38219
	memcpy(g_anLoadRenderLibCode2, (LPVOID)(dwExeAddress + ADDR_LOAD_RENDER_LIB_CODE2), 6); // 0x75371
#endif
}

void HookEngineStuff2()
{
	HANDLE hProcess = GetCurrentProcess();
	DWORD dwExeAddress = (DWORD)GetModuleHandle(LITHTECH_EXE);
	DWORD dwDllAddress = (DWORD)GetModuleHandle(g_szRenderDll);

	g_pClientMgr = (CClientMgrBase*)(dwExeAddress + ADDR_CLIENT_MGR); // 0xDEFAC
	g_pLTClient = g_pClientMgr->m_pClientMgr->m_pLTClient;

	if (GetConfigValue(CO_NOBF_LIGHTS))
	{
		if (g_pLTClient->CreateObject != MyCreateObject)
		{
			OldCreateObject = g_pLTClient->CreateObject;		
			g_pLTClient->CreateObject = MyCreateObject;
		}
	}
}

DWORD g_dwOriginalD3D = 0;
HRESULT WINAPI FakeDirectDrawCreateEx(GUID FAR * lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter)
{
	HookEngineStuff2();
	
	if (GetConfigValue(CO_INTEL_HD))
		ApplyIntelHD_RHW_Fix();

	DirectDrawCreateEx_Type DirectDrawCreateEx_fn = (DirectDrawCreateEx_Type)g_dwOriginalD3D;

	LPVOID FAR dummy;
	
	HRESULT hResult = DirectDrawCreateEx_fn(lpGUID, &dummy, iid, pUnkOuter);

	*lplpDD = (LPDIRECTDRAW7) new FakeIDDraw7((LPDIRECTDRAW7)dummy);
	
	return hResult;
}

HMODULE WINAPI MyLoadLibraryA(LPCSTR lpFileName)
{
	HMODULE hModule = LoadLibraryA(lpFileName);
	
	if (_stricmp(lpFileName, g_szRenderWrapperDll) == 0)
	{ 
		DWORD dwDllAddress = (DWORD)GetModuleHandle(g_szRenderDll);
		
		EngineHack_WriteFunction((LPVOID)(dwDllAddress + ADDR_D3D_DDRAW_CREATE_EX), (DWORD)FakeDirectDrawCreateEx, g_dwOriginalD3D); // 0x46000
	}
#ifdef PRIMAL_HUNT_BUILD
	else if (strstr(lpFileName, CRES_DLL_LOWER) || strstr(lpFileName, CRES_DLL_UPPER))
	{
		BYTE anOld[6];
		DWORD dwExeAddress = (DWORD)GetModuleHandle(LITHTECH_EXE);

		EngineHack_WriteData((LPVOID)(dwExeAddress + ADDR_LOAD_RENDER_LIB_CODE1), g_anLoadRenderLibCode1, anOld, 6);
		EngineHack_WriteData((LPVOID)(dwExeAddress + ADDR_LOAD_RENDER_LIB_CODE2), g_anLoadRenderLibCode2, anOld, 6);
	}
#endif

	return hModule;
}
