extern LPDIRECTDRAW7 g_ddMainDDraw;

static char g_szRenderDll[64] = "d3d.ren";
static char g_szRenderWrapperDll[64] = "d3d.ren";
extern char g_szConfigBytes[64];

enum eConfigOption
{
	CO_INTEL_HD = 1,
	CO_RADEON_5700 = 2, 
	CO_NOBF_LIGHTS,
	CO_DGVOODOO_MODE,
	CO_MAX
};

#define LITHTECH			"lithtech"
#define LITHTECH_EXE		"lithtech.exe"
#define CRES_DLL_LOWER		"cres.dll"
#define CRES_DLL_UPPER		"CRES.DLL"

#define FIX_FLG_R5700_FLICKERING_MODELS		(1<<0)
#define FIX_FLG_R5700_BLACK_SCREEEN			(1<<1)

#define FLIPSCREEN_CANDRAWCONSOLE	(1<<0)
#define FLIPSCREEN_COPY				(1<<1)
#define FLIPSCREEN_DIRTY			(1<<2)

#define OT_LIGHT 4
#define FLAG_DONTLIGHTBACKFACING	(1<<6)

#define TLVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

#define MAX_RESTREES            20

struct LTRect
{
	int left;
	int top;
	int right;
	int bottom;
};

struct ObjectCreateStruct
{
	WORD			m_ObjectType;
	WORD			m_ContainerCode;
	DWORD			m_CreateFlags;
	DWORD			m_Flags;
	DWORD			m_Flags2;
	// more there
};

struct TLVertex
{	
	float m_VecX;
	float m_VecY;
	float m_VecZ;
	float rhw;	
	DWORD color1;
	DWORD color2;
	float tu, tv;
};

class ILTCSBase
{
public:

	virtual DWORD StartHMessageWrite()=0;
	// More stuff there

//protected:
	
	void			*m_MathLT;
	void			*m_pCommonLT;
	void			*m_pPhysicsLT;
	void			*m_pModelLT;
	void			*m_pTransformLT;
	void			*m_pLightAnimLT;
	void			*m_pSoundMgr;
};


class ILTClient: public ILTCSBase
{
public:

	DWORD			(*StartGame)(void *pRequest);
	BYTE			m_Data0[20]; // 24 52
	void			(*Shutdown)();
	void			(*ShutdownWithMessage)( char *pMsg, ... );
	DWORD			(*FlipScreen)(DWORD flags);	
	BYTE			m_Data1[14]; // 18 34
	DWORD			(*StartOptimized2D)();
	DWORD			(*EndOptimized2D)();
	DWORD			(*SetOptimized2DBlend)(DWORD blend);
	DWORD			(*GetOptimized2DBlend)(DWORD &blend);
	DWORD			(*SetOptimized2DColor)(DWORD hColor);
	DWORD			(*GetOptimized2DColor)(DWORD &hColor);
	DWORD			(*End3D)();
	void*			(*GetRenderModes)();
	void			(*RelinquishRenderModes)(void *pModes);
	DWORD			(*GetRenderMode)(void *pMode);
	DWORD			(*SetRenderMode)(void *pMode);
	DWORD			(*ShutdownRender)(DWORD flags);	
	void*			(*GetFileList)(char *pDirName);	
	void			(*FreeFileList)(void *pHead);	
	DWORD			(*GetWorldInfoString)(char *pFilename, char *pInfoString, DWORD maxLen, DWORD *pActualLen);
	DWORD			(*ReadConfigFile)(char *pFilename);
	DWORD			(*WriteConfigFile)(char *pFilename);
	void			(*GetAxisOffsets)(float *offsets);	
	BYTE			m_Data2[164]; // 200

	DWORD			(*CreateFont)(char *pFontName, int width, int height, BOOL bItalic, BOOL bUnderline, BOOL bBold);
	void			(*DeleteFont)(DWORD hFont);
	DWORD			(*SetFontExtraSpace)(DWORD hFont, int pixels);
	DWORD			(*GetFontExtraSpace)(DWORD hFont, int &pixels);
	DWORD			(*CreateColor)(float r, float g, float b, BOOL bTransparent);
	void			(*DeleteColor)(DWORD hColor);
	DWORD			(*SetupColor1)(float r, float g, float b, BOOL bTransparent);
	DWORD			(*SetupColor2)(float r, float g, float b, BOOL bTransparent);
	DWORD			(*GetBorderSize)(DWORD hSurface, DWORD hColor, LTRect *pRect);	
	
	DWORD			(*OptimizeSurface)(DWORD hSurface, DWORD hTransparentColor);
	DWORD			(*UnoptimizeSurface)(DWORD hSurface);	
	DWORD			(*GetScreenSurface)();
	DWORD			(*CreateSurfaceFromBitmap)(char *pBitmapName);
	DWORD			(*CreateSurfaceFromString)(DWORD hFont, DWORD hString, DWORD hForeColor, DWORD hBackColor, int extraPixelsX, int extraPixelsY);
	DWORD			(*CreateSurface)(DWORD width, DWORD height);
	DWORD			(*DeleteSurface)(DWORD hSurface);
	void*			(*GetSurfaceUserData)(DWORD hSurface);
	void			(*SetSurfaceUserData)(DWORD hSurface, void *pUserData);
	DWORD			(*GetPixel)(DWORD hSurface, DWORD x, DWORD y, DWORD *color);
	DWORD			(*SetPixel)(DWORD hSurface, DWORD x, DWORD y, DWORD color);
	void			(*GetStringDimensions)(DWORD hFont, DWORD hString, int *sizeX, int *sizeY);
	
	void			(*DrawStringToSurface)(DWORD hDest, DWORD hFont, DWORD hString, LTRect* pRect, DWORD hForeColor, DWORD hBackColor);	
	void			(*GetSurfaceDims)(DWORD hSurf, DWORD *pWidth, DWORD *pHeight);	
	DWORD			(*DrawBitmapToSurface)(DWORD hDest, char *pSourceBitmapName, LTRect *pSrcRect, int destX, int destY);	
	DWORD			(*DrawSurfaceMasked)(DWORD hDest, DWORD hSrc, DWORD hMask, LTRect *pSrcRect, int destX, int destY, DWORD hColor);
	DWORD			(*DrawSurfaceSolidColor)(DWORD hDest, DWORD hSrc, LTRect *pSrcRect, int destX, int destY, DWORD hTransColor, DWORD hFillColor);
	DWORD			(*DrawSurfaceToSurface)(DWORD hDest, DWORD hSrc, LTRect *pSrcRect, int destX, int destY);
	DWORD			(*DrawSurfaceToSurfaceTransparent)(DWORD hDest, DWORD hSrc, LTRect *pSrcRect, int destX, int destY, DWORD hColor);
	DWORD			(*ScaleSurfaceToSurface)(DWORD hDest, DWORD hSrc, LTRect *pDestRect, LTRect *pSrcRect);
	DWORD			(*ScaleSurfaceToSurfaceTransparent)(DWORD hDest, DWORD hSrc, LTRect *pDestRect, LTRect *pSrcRect, DWORD hColor);	
	DWORD			(*ScaleSurfaceToSurfaceSolidColor)(DWORD hDest, DWORD hSrc, LTRect *pDestRect, LTRect *pSrcRect, DWORD hTransColor, DWORD hFillColor); // 444
	BYTE			m_Data4[20];
	DWORD			(*FillRect)(DWORD hDest, LTRect *pRect, DWORD hColor);
	DWORD			(*GetSurfaceAlpha)(DWORD hSurface, float &alpha);
	DWORD			(*SetSurfaceAlpha)(DWORD hSurface, float alpha);
	DWORD			(*RegisterConsoleProgram)(char *pName, void* fn);	
	DWORD			(*UnregisterConsoleProgram)(char *pName);	
	DWORD			(*GetConsoleVar)(char *pName);
	BYTE			m_Data5[88]; 
	void			(*RunConsoleString)(char *pString);
	BYTE			m_Data6[32];
	DWORD			(*CreateObject)(ObjectCreateStruct *pStruct);
	BYTE			m_Data7[64]; // 100
	void			(*SetCameraFOV)(DWORD hObj, float fovX, float fovY);
};

extern ILTClient* g_pLTClient;

class CClientMgr
{	
public:
	
	BYTE			m_Data1[1220];
	ILTClient*		m_pLTClient;
	
	BYTE			m_Data11[344];
	void*			m_pClientShell;
	
	BYTE			m_Data2[3392]; // 3740
	float			m_CurTimeCopy;
	float			m_FrameTime;
	float			m_CurTime;
	BYTE			m_Data3[684];
	void*			m_pCurShell;
	BYTE			m_Data4[36];
	void*			m_pClientFileMgr;
	char*			m_ResTrees[MAX_RESTREES];
    DWORD			m_nResTrees;
	void*			m_pDemoMgr;
};

class CClientMgrBase
{
public:
	
	CClientMgr*		m_pClientMgr;
	//BYTE			m_Data0[388];
	//void*			m_pClientShell;
};

extern CClientMgrBase* g_pClientMgr;

DWORD GetConfigValue(eConfigOption eOption);
void SetConfigValue(eConfigOption eOption, DWORD dwSet);

void EngineHack_WriteData(HANDLE hProcess, LPVOID lpAddr, BYTE* pNew, BYTE* pOld, DWORD dwSize);
void EngineHack_WriteFunction(HANDLE hProcess, LPVOID lpAddr, DWORD dwNew, DWORD& dwOld);
void EngineHack_WriteCall(HANDLE hProcess, LPVOID lpAddr, DWORD dwNew, BOOL bStructCall);
void EngineHack_AllowWrite(HANDLE hProcess, LPVOID lpAddr, DWORD dwSize);
void EngineHack_WriteJump(HANDLE hProcess, LPVOID lpAddr, DWORD dwNew);
