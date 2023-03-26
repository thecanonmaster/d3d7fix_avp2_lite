#include "stdafx.h"

typedef HRESULT (__stdcall *Blt_Type) (LPDIRECTDRAWSURFACE7, LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX);
typedef HRESULT (__stdcall *BltFast_Type) (LPDIRECTDRAWSURFACE7, DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
typedef ULONG (__stdcall *Release_Type) (LPDIRECTDRAWSURFACE7);
typedef HRESULT (__stdcall *Flip_Type)(LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7, DWORD);

Release_Type g_pOrigLMRelease = NULL;
Blt_Type g_pOrigLMBlt = NULL;
BltFast_Type g_pOrigLMBltFast = NULL;

Release_Type g_pOrigPRRelease = NULL;
Blt_Type g_pOrigPRBlt = NULL;
Flip_Type g_pOrigPRFlip = NULL;

DWORD* g_pOrigLMSurfaceVTable = NULL;
DWORD* g_pOrigPRSurfaceVTable = NULL;
DWORD g_dwLMSurfaceVTable[49];
DWORD g_dwPRSurfaceVTable[49];

ULONG __stdcall FakeIDDrawSurface7LM_Release(LPDIRECTDRAWSURFACE7 f)
{
	((DWORD*)f)[0] = (DWORD)g_pOrigLMSurfaceVTable;
	g_pOrigLMRelease = NULL;
	
	return f->Release();
}

ULONG __stdcall FakeIDDrawSurface7PR_Release(LPDIRECTDRAWSURFACE7 f)
{
	((DWORD*)f)[0] = (DWORD)g_pOrigPRSurfaceVTable;
	g_pOrigPRRelease = NULL;
	
	return f->Release();
}

HRESULT __stdcall FakeIDDrawSurface7LM_Blt(LPDIRECTDRAWSURFACE7 f, LPRECT a,LPDIRECTDRAWSURFACE7 b, LPRECT c,DWORD d, LPDDBLTFX e)
{
	if (!b) 
	{			
		DDSURFACEDESC2 descDest;
		descDest.dwSize = sizeof(DDSURFACEDESC2);

		if (f->Lock(NULL, &descDest, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL) == DD_OK)
		{	
			memset(descDest.lpSurface, e->dwFillColor, (descDest.dwWidth * descDest.dwHeight) << 2);
	
			f->Unlock(NULL);
		}

		return DD_OK; 
	}

	return g_pOrigLMBlt(f, a, b, c, d, e);	
}

HRESULT __stdcall FakeIDDrawSurface7PR_Blt(LPDIRECTDRAWSURFACE7 f, LPRECT a,LPDIRECTDRAWSURFACE7 b, LPRECT c,DWORD d, LPDDBLTFX e)
{					
	if (f->IsLost())
		g_ddMainDDraw->RestoreAllSurfaces();
	
	return g_pOrigPRBlt(f, a, b, c, d, e);	
}

HRESULT __stdcall FakeIDDrawSurface7LM_BltFast(LPDIRECTDRAWSURFACE7 f,DWORD a,DWORD b,LPDIRECTDRAWSURFACE7 c, LPRECT d,DWORD e)
{		
	DDSURFACEDESC2 descSrc, descDest;
	descSrc.dwSize = sizeof(DDSURFACEDESC2);
	descDest.dwSize = sizeof(DDSURFACEDESC2);
		
	if (c->Lock(d, &descSrc, DDLOCK_WAIT | DDLOCK_READONLY, NULL) == DD_OK)
	{
		if (f->Lock(NULL, &descDest, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL) == DD_OK)
		{		
			DWORD srcWidth = d->right - d->left;
			DWORD srcHeight = d->bottom - d->top;
			BYTE* pSrcLine = (BYTE*)descSrc.lpSurface;

			BYTE* pDestLine = (BYTE*)descDest.lpSurface + (b * (descDest.dwWidth << 2) + (a << 2));

			int nCount = srcHeight;

			while (nCount) 
			{
				nCount--;
				memcpy(pDestLine, pSrcLine, srcWidth << 2);
					
				pSrcLine += descSrc.lPitch;
				pDestLine += descDest.lPitch; 
			}			

			c->Unlock(NULL);
			f->Unlock(NULL);
		}
	}
		
	return DD_OK;
}

HRESULT __stdcall FakeIDDrawSurface7PR_Flip(LPDIRECTDRAWSURFACE7 c,LPDIRECTDRAWSURFACE7 a, DWORD b)
{
	if (c->IsLost())
		g_ddMainDDraw->RestoreAllSurfaces();

	return g_pOrigPRFlip(c, a, b);
}

typedef LRESULT CALLBACK WindowProc_type(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef WindowProc_type *WindowProc_type_ptr;
WindowProc_type_ptr OldWindowProc;

LRESULT CALLBACK NewWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return OldWindowProc(hwnd, uMsg, wParam, lParam);
}

FakeIDDraw7::FakeIDDraw7(LPDIRECTDRAW7 pOriginal)
{
	m_pIDDraw = pOriginal;
	g_ddMainDDraw = pOriginal;
}

FakeIDDraw7::~FakeIDDraw7(void)
{

}

HRESULT __stdcall FakeIDDraw7::QueryInterface (REFIID a, LPVOID FAR * b)
{

	HRESULT hResult = m_pIDDraw->QueryInterface(a, b); 

	if (hResult == NOERROR) 
	{
		if (a == IID_IDirect3D7) 
			*b = new FakeID3D7((LPDIRECT3D7)*b);		
	}

	return hResult;
}

ULONG __stdcall FakeIDDraw7::AddRef(void)
{
	return m_pIDDraw->AddRef();
}

ULONG __stdcall FakeIDDraw7::Release(void)
{
	ULONG dwCount = m_pIDDraw->Release();

	m_pIDDraw = NULL;		
	delete(this); 

	return dwCount;
}

HRESULT  __stdcall FakeIDDraw7::Compact(void)
{
	return m_pIDDraw->Compact();
}

HRESULT  __stdcall FakeIDDraw7::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR* lplpDDClipper, IUnknown FAR *pUnkOuter)
{
	return m_pIDDraw->CreateClipper(dwFlags, lplpDDClipper, pUnkOuter);
}

HRESULT  __stdcall FakeIDDraw7::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE FAR* c, IUnknown FAR* d)
{
	return m_pIDDraw->CreatePalette(a, b, c, d);
}

HRESULT  __stdcall FakeIDDraw7::CreateSurface(LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPDIRECTDRAWSURFACE7 FAR * lplpDDSurface, IUnknown FAR * pUnkOuter)
{		
	if (GetConfigValue(CO_DGVOODOO_MODE))
	{
		if (lpDDSurfaceDesc2->dwMipMapCount == 0 && lpDDSurfaceDesc2->ddpfPixelFormat.dwRGBBitCount == 32 
			&& !(lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_ZBUFFER))
		{
			HRESULT hResult = m_pIDDraw->CreateSurface(lpDDSurfaceDesc2, lplpDDSurface, pUnkOuter);
			if (hResult == DD_OK)
			{							
				DWORD* pOrigTable = (DWORD*)*(DWORD*)*lplpDDSurface;
				
				if (!g_pOrigLMRelease)
				{
					memcpy(g_dwLMSurfaceVTable, pOrigTable, 49 * sizeof(DWORD));
					
					g_pOrigLMSurfaceVTable = pOrigTable;
					g_pOrigLMRelease = (Release_Type)pOrigTable[2];
					g_pOrigLMBlt = (Blt_Type)pOrigTable[5];
					g_pOrigLMBltFast = (BltFast_Type)pOrigTable[7];
					
					g_dwLMSurfaceVTable[2] = (DWORD)FakeIDDrawSurface7LM_Release;
					g_dwLMSurfaceVTable[5] = (DWORD)FakeIDDrawSurface7LM_Blt;
					g_dwLMSurfaceVTable[7] = (DWORD)FakeIDDrawSurface7LM_BltFast;
				}
				
				((DWORD*)*lplpDDSurface)[0] = (DWORD)g_dwLMSurfaceVTable;	
			}
			
			return hResult;
		}
	}
	
	if (GetConfigValue(CO_RADEON_5700) & FIX_FLG_R5700_BLACK_SCREEEN)
	{
		if (lpDDSurfaceDesc2->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
		{
			HRESULT hResult = m_pIDDraw->CreateSurface(lpDDSurfaceDesc2, lplpDDSurface, pUnkOuter);
			if (hResult == DD_OK)
			{							
				DWORD* pOrigTable = (DWORD*)*(DWORD*)*lplpDDSurface;
				
				if (!g_pOrigLMRelease)
				{
					memcpy(g_dwPRSurfaceVTable, pOrigTable, 49 * sizeof(DWORD));
					
					g_pOrigPRSurfaceVTable = pOrigTable;
					g_pOrigPRRelease = (Release_Type)pOrigTable[2];
					g_pOrigPRBlt = (Blt_Type)pOrigTable[5];
					g_pOrigPRFlip = (Flip_Type)pOrigTable[11];
					
					g_dwPRSurfaceVTable[2] = (DWORD)FakeIDDrawSurface7PR_Release;
					g_dwPRSurfaceVTable[5] = (DWORD)FakeIDDrawSurface7PR_Blt;
					g_dwPRSurfaceVTable[11] = (DWORD)FakeIDDrawSurface7PR_Flip;
				}
				
				((DWORD*)*lplpDDSurface)[0] = (DWORD)g_dwPRSurfaceVTable;	
			}
			
			return hResult;			
		}
	}

	return m_pIDDraw->CreateSurface(lpDDSurfaceDesc2, lplpDDSurface, pUnkOuter);
}

HRESULT  __stdcall FakeIDDraw7::DuplicateSurface(LPDIRECTDRAWSURFACE7 a, LPDIRECTDRAWSURFACE7 FAR * b)
{
	return m_pIDDraw->DuplicateSurface(a, b);
}

HRESULT  __stdcall FakeIDDraw7::EnumDisplayModes(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMMODESCALLBACK2 d)
{
	return m_pIDDraw->EnumDisplayModes(a, b, c, d);
}

HRESULT  __stdcall FakeIDDraw7::EnumSurfaces(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMSURFACESCALLBACK7 d)
{
	return m_pIDDraw->EnumSurfaces(a, b, c, d);
}

HRESULT  __stdcall FakeIDDraw7::FlipToGDISurface(void)
{
	return m_pIDDraw->FlipToGDISurface();
}

HRESULT  __stdcall FakeIDDraw7::GetCaps(LPDDCAPS lpDDSurfaceCaps, LPDDCAPS lpDDHELCaps)
{
	return m_pIDDraw->GetCaps(lpDDSurfaceCaps, lpDDHELCaps);
}

HRESULT  __stdcall FakeIDDraw7::GetDisplayMode(LPDDSURFACEDESC2 a)
{
	return m_pIDDraw->GetDisplayMode(a);
}

HRESULT  __stdcall FakeIDDraw7::GetFourCCCodes(LPDWORD a, LPDWORD b)
{
	return m_pIDDraw->GetFourCCCodes(a, b);
}

HRESULT  __stdcall FakeIDDraw7::GetGDISurface(LPDIRECTDRAWSURFACE7 FAR * a)
{
	return m_pIDDraw->GetGDISurface(a);
}

HRESULT  __stdcall FakeIDDraw7::GetMonitorFrequency(LPDWORD a)
{
	return m_pIDDraw->GetMonitorFrequency(a);
}

HRESULT  __stdcall FakeIDDraw7::GetScanLine(LPDWORD a)
{
	return m_pIDDraw->GetScanLine(a);
}

HRESULT  __stdcall FakeIDDraw7::GetVerticalBlankStatus(LPBOOL a)
{
	return m_pIDDraw->GetVerticalBlankStatus(a);
}

HRESULT  __stdcall FakeIDDraw7::Initialize(GUID FAR* a)
{
	return m_pIDDraw->Initialize(a);
}

HRESULT  __stdcall FakeIDDraw7::RestoreDisplayMode(void)
{
	return m_pIDDraw->RestoreDisplayMode();
}


HRESULT  __stdcall FakeIDDraw7::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	if ((GetConfigValue(CO_COOP_MOD_COMPAT) & FIX_COOP_COMPAT_WINDOW_PROC) && !g_bWindowHooked)
	{
		g_hWindowHandle = hWnd;

		LONG lResult = SetWindowLong(g_hWindowHandle, GWL_WNDPROC, (LONG)NewWindowProc);
		if (lResult)
			OldWindowProc = (WindowProc_type_ptr)lResult;
		
		g_bWindowHooked = TRUE;
	}
	
	return m_pIDDraw->SetCooperativeLevel(hWnd, dwFlags);
}

HRESULT  __stdcall FakeIDDraw7::SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e)
{
	return m_pIDDraw->SetDisplayMode(a, b, c, d, e); 
}

HRESULT  __stdcall FakeIDDraw7::WaitForVerticalBlank(DWORD a, HANDLE b)
{
	return m_pIDDraw->WaitForVerticalBlank(a, b);
}

HRESULT  __stdcall FakeIDDraw7::GetAvailableVidMem(LPDDSCAPS2 lpDDSCaps2, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	return m_pIDDraw->GetAvailableVidMem(lpDDSCaps2, lpdwTotal, lpdwFree);
}


HRESULT  __stdcall FakeIDDraw7::GetSurfaceFromDC(HDC a, LPDIRECTDRAWSURFACE7 * b)
{
	return m_pIDDraw->GetSurfaceFromDC(a, b);
}

HRESULT  __stdcall FakeIDDraw7::RestoreAllSurfaces(void)
{
	return m_pIDDraw->RestoreAllSurfaces();
}

HRESULT  __stdcall FakeIDDraw7::TestCooperativeLevel(void)
{
	return m_pIDDraw->TestCooperativeLevel();
}

HRESULT  __stdcall FakeIDDraw7::GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2 lpdddi, DWORD dwFlags)
{
	return m_pIDDraw->GetDeviceIdentifier(lpdddi, dwFlags);
}

HRESULT  __stdcall FakeIDDraw7::StartModeTest(LPSIZE a, DWORD b, DWORD c)
{
	return m_pIDDraw->StartModeTest(a, b, c);
}

HRESULT  __stdcall FakeIDDraw7::EvaluateMode(DWORD a, DWORD * b)
{
	return m_pIDDraw->EvaluateMode(a, b);
}
