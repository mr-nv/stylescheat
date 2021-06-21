#include "hashes.h"

typedef HANDLE (__stdcall* GetCurrentProcessFn)();
typedef HANDLE (__stdcall* CreateThreadFn)(LPSECURITY_ATTRIBUTES lpSecurityAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef LPVOID (__stdcall* VirtualAllocFn)(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
typedef bool (__stdcall* VirtualFreeFn)(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
typedef bool (__stdcall* VirtualFreeExFn)(HANDLE proc, LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
typedef bool (__stdcall* VirtualQueryFn)(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength);
typedef bool (__stdcall* VirtualProtectFn)(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
typedef DWORD (__stdcall* GetLastErrorFn)();
typedef void (__stdcall* SleepFn)(DWORD dwMilliseconds);
typedef int (__stdcall* MultiByteToWideCharFn)(DWORD CodePage, DWORD dwFlags, char* lpMultiByteStr, int cbMultiByte, wchar_t* lpWideCharStr, int cchWideChar);

typedef void (__stdcall* RtlExitUserThreadFn)(DWORD dwExitCode);

typedef void (__cdecl* wsprintfAFn)(char* out, char* in, ...);
typedef short (__stdcall* GetAsyncKeyStateFn)(int vKey);
typedef void (__stdcall* MessageBoxAFn)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

typedef long (__stdcall* RegCreateKeyExAFn)(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
typedef long (__stdcall* RegQueryInfoKeyAFn)(HKEY hKey, LPTSTR lpClass, LPDWORD lpcClass, DWORD Reserved, LPDWORD lpcSubKeys, LPDWORD lpcMaxSubKeyLen, LPDWORD lpcMaxClassLen, LPDWORD lpcValues, LPDWORD lpcMaxValueNameLen, LPDWORD lpcMaxValueLen, LPDWORD lpcbSecurityDescriptor, void* lpftLastWriteTime);
typedef long (__stdcall* RegEnumKeyExAFn)(HKEY hKey, DWORD dwIndex, LPTSTR lpName, LPDWORD lpcName, LPDWORD lpReserved, LPTSTR lpClass, LPDWORD lpcClass, void* lpftLastWriteTime);
typedef long (__stdcall* RegQueryValueExAFn)(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
typedef long (__stdcall* RegSetValueExAFn)(HKEY hKey, LPCTSTR lpValueName, DWORD Reserved, DWORD dwType, BYTE* lpData, DWORD cbData);
typedef long (__stdcall* RegCloseKeyFn)(HKEY hKey);
typedef long (__stdcall* RegDeleteKeyAFn)(HKEY hKey, LPCSTR lpSubKey);

//typedef HINSTANCE (__stdcall* ShellExecuteAFn)(HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCSTR lpParameters, LPCTSTR lpDirectory, int nShowCmd);

typedef HANDLE (__stdcall* CreateFileAFn)(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef bool (__stdcall* ReadFileFn)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
typedef bool (__stdcall* WriteFileFn)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
typedef bool (__stdcall* CreateDirectoryAFn)(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
typedef bool (__stdcall* CloseHandleFn)(HANDLE hObject);

typedef void (__cdecl* WarningFn)(const char* str, ...);

typedef void (__cdecl* RandomSeedFn)(int seed);
typedef float (__cdecl* RandomFloatFn)(float min, float max);
typedef int (__cdecl* RandomIntFn)(int min, int max);

class CBaseClass
{
public:
	DWORD mModuleBaseAddress;
	DWORD mModuleSize;

	char mTitle[64];
	
	int mNoInsecureLoad;

	// windows functions

	GetCurrentProcessFn GetCurrentProcess;
	CreateThreadFn CreateThread;
	VirtualAllocFn VirtualAlloc;
	VirtualFreeFn VirtualFree;
	VirtualFreeExFn VirtualFreeEx;
	VirtualQueryFn VirtualQuery;
	VirtualProtectFn VirtualProtect;
	GetLastErrorFn GetLastError;
	SleepFn Sleep;
	MultiByteToWideCharFn MultiByteToWideChar;

	RtlExitUserThreadFn RtlExitUserThread;

	wsprintfAFn wsprintfA;
	GetAsyncKeyStateFn GetAsyncKeyState;
	//MessageBoxAFn MessageBoxA;

	RegCreateKeyExAFn RegCreateKeyExA;
	RegQueryInfoKeyAFn RegQueryInfoKeyA;
	RegEnumKeyExAFn RegEnumKeyExA;
	RegQueryValueExAFn RegQueryValueExA;
	RegSetValueExAFn RegSetValueExA;
	RegCloseKeyFn RegCloseKey;
	RegDeleteKeyAFn RegDeleteKey;

	CreateFileAFn CreateFileA;
	ReadFileFn ReadFile;
	WriteFileFn WriteFile;
	CreateDirectoryAFn CreateDirectoryA;
	CloseHandleFn CloseHandle;

	// source functions

	WarningFn Warning;

	RandomSeedFn RandomSeed;
	RandomIntFn RandomInt;
	RandomFloatFn RandomFloat;

	// module bases
	// we don't want to leave any static offsets (as pointers should be solved by the loader)
	// so we don't use this for now

//	DWORD mClientDLL;
//	DWORD mEngineDLL;

	// source's interfaces

	// client

	void* mBaseClient;
	void* mEntList;
	void* mPrediction;
	void* mGameMovement;
	void* mClientMode;
	void* mInput;

	// engine
	
	void* mBaseEngine;
	void* mEngineTrace;
	void* mModelInfo;
	void* mVPhysics;
	void* mEngineCvar;
	void* mGameEvents;

	// render

	void* mModelRender;
	void* mView;
	void* mMaterialSystem;
	void* mStudioRender;

	// drawing

	void* mSurface;
	void* mEngineVgui;
	void* mDebugOverlay;

	// other

	GlobalVars* mGlobalVars;
	void* mCommandLine;
	DWORD mInPredictionReturn;
	DWORD mTimeDemoReturn;
	DWORD mSmokeCount;
	DWORD mClientEffectReg;

	// other (set at runtime)

	void* mMoveHelper;
	void* mRenderContext;

	void* mBaseClientHookTable;
	void* mBaseClientOldHookTable;

	void* mClientModeHookTable;
	void* mClientModeOldHookTable;

	void* mPredictionHookTable;
	void* mPredictionOldHookTable;

	void* mStudioRenderHookTable;
	void* mStudioRenderOldHookTable;

	void* mViewHookTable;
	void* mViewOldHookTable;

	void* mEngineVguiHookTable;
	void* mEngineVguiOldHookTable;

	void* mBaseEngineHookTable;
	void* mBaseEngineOldHookTable;

	DWORD mEncodeUsercmd;
	DWORD mClientCreateMove;
	DWORD mFrameStageNotify;
	DWORD mClientKeyEvent;

	DWORD mOverrideView;
	DWORD mCreateMove;
	DWORD mOverrideMouse;

	DWORD mInPrediction;
	DWORD mRunCommand;
	DWORD mGetLocalAngles;
	
	DWORD mDrawModel;
	DWORD mDrawWorldLists;

	DWORD mKeyEvent;
	DWORD mUpdateButton;
	DWORD mPaint;

	DWORD mTimeDemo;

	void* mEyePitchFunc;
	DWORD mEyePitchFuncOld;

	void* mEyeYawFunc;
	DWORD mEyeYawFuncOld;

	void* mBodyYawFunc;
	DWORD mBodyYawFuncOld;

	void* mFlashFunc;
	DWORD mFlashFuncOld;

	void* mSmokeEffectFunc;
	DWORD mSmokeEffectFuncOld;

	// netvars

	DWORD m_flSimulationTime;
	DWORD m_clrRender;
	DWORD m_iTeamNum;
	DWORD m_vecOrigin;
	DWORD m_vecMins;
	DWORD m_vecMaxs;

	DWORD m_nTickBase;
	DWORD m_lifeState;
	DWORD m_iHealth;
	DWORD m_fFlags;
	DWORD m_fEffects;
	DWORD m_aimPunchAngle;
	DWORD m_viewPunchAngle;
	DWORD m_vecViewOffset;
	DWORD m_vecVelocity;
	DWORD m_hLastWeapon;
	DWORD m_flDuckAmount;
	DWORD m_flDuckSpeed;

	DWORD m_MoveType;
	DWORD m_nWaterLevel;
	DWORD m_hGroundEntity;
	DWORD m_angEyeAngles;
	DWORD m_ArmorValue;
	DWORD m_bIsScoped;
	DWORD m_iShotsFired;
	DWORD m_iObserverMode;
	DWORD m_hObserverTarget;
	DWORD m_flLowerBodyYawTarget;
	DWORD m_bHasDefuser;
	DWORD m_bGunGameImmunity;

	DWORD m_bBurstMode;
	DWORD m_flPostponeFireReadyTime;
	DWORD m_fAccuracyPenalty;
	DWORD m_flRecoilIndex;

	DWORD m_flNextBurstFire;

	DWORD m_hActiveWeapon;
	DWORD m_hMyWeapons;
	DWORD m_flNextAttack;
	
	DWORD m_hOwner;
	DWORD m_iClip1;
	DWORD m_flNextPrimaryAttack;
	DWORD m_flNextSecondaryAttack;

	DWORD m_iItemDefinitionIndex;

	DWORD m_bClientSideAnimation;
	DWORD m_flPoseParameter;
	DWORD m_iv_flPoseParameter;
	DWORD m_flLastBoneSetupTime;
	DWORD m_iMostRecentModelBoneCounter;

	DWORD m_AnimOverlay;

	DWORD m_ModelAnimDistance;
	DWORD m_ModelAnimFlags;
	DWORD m_ModelAnimLastFrame;

	DWORD m_PlayerAnimState;

	DWORD m_bPinPulled;
	DWORD m_fThrowTime;
	DWORD m_flThrowStrength;

	DWORD m_flDefuseCountDown;

	DWORD m_fireXDelta;
	DWORD m_fireYDelta;

	DWORD m_bDidSmokeEffect;

	DWORD m_flC4Blow;

	DWORD m_iPing;

	typedef void (__thiscall* ModifyEyePositionFn)(void* thisptr, Vector* pos);
	ModifyEyePositionFn ModifyEyePosition;

	// base client

//	typedef void* (__thiscall* GetClientClassesFn)(void* thisptr);
	//GetClientClassesFn GetClientClasses;

	// entity list

	typedef Entity* (__thiscall* GetClientEntityFn)(void* thisptr, int index);
	GetClientEntityFn GetClientEntity;

	typedef Entity* (__thiscall* GetClientEntityByHandleFn)(void* thisptr, DWORD handle);
	GetClientEntityByHandleFn GetClientEntityByHandle;

	typedef int (__thiscall* GetHighestEntityIndexFn)(void* thisptr);
	GetHighestEntityIndexFn GetHighestEntityIndex;

	// base engine

	typedef void (__thiscall* GetScreenSizeFn)(void* thisptr, int* w, int* h);
	GetScreenSizeFn GetScreenSize;

	typedef bool (__thiscall* GetPlayerInfoFn)(void* thisptr, int index, PlayerInfo* info);
	GetPlayerInfoFn GetPlayerInfo;

	typedef bool (__thiscall* Con_IsVisibleFn)(void* thisptr);
	Con_IsVisibleFn Con_IsVisible;

	typedef int (__thiscall* GetLocalPlayerFn)(void* thisptr);
	GetLocalPlayerFn GetLocalPlayer;

	typedef void (__thiscall* GetViewAnglesFn)(void* thisptr, Vector* angles);
	GetViewAnglesFn GetViewAngles;

	typedef void (__thiscall* SetViewAnglesFn)(void* thisptr, Vector* angles);
	SetViewAnglesFn SetViewAngles;

	typedef bool (__thiscall* IsInGameFn)(void* thisptr);
	IsInGameFn IsInGame;

	typedef bool (__thiscall* IsConnectedFn)(void* thisptr);
	IsConnectedFn IsConnected;

	typedef Matrix* (__thiscall* GetScreenMatrixFn)(void* thisptr);
	GetScreenMatrixFn GetScreenMatrix;

	typedef char* (__thiscall* GetLevelNameFn)(void* thisptr);
	GetLevelNameFn GetLevelName;

	typedef void* (__thiscall* GetNetChannelFn)(void* thisptr);
	GetNetChannelFn GetNetChannel;

	typedef bool (__thiscall* IsPlayingDemoFn)(void* thisptr);
	IsPlayingDemoFn IsPlayingDemo;

	typedef void (__thiscall* ConCommandFn)(void* thisptr, char* cmd);
	ConCommandFn ConCommand;

	// clientmode

	typedef bool (__thiscall* ShouldDrawLocalPlayerFn)(void* thisptr, void* player);
	ShouldDrawLocalPlayerFn ShouldDrawLocalPlayer;

	typedef void* (__thiscall* GetMessagePanelFn)(void* thisptr);
	GetMessagePanelFn GetMessagePanel;

	// client prediction

	typedef void (__thiscall* SetLocalViewAnglesFn)(void* thisptr, Vector* angles);
	SetLocalViewAnglesFn SetLocalViewAngles;

	typedef void (__thiscall* SetupMoveFn)(void* thisptr, void* player, UserCmd* cmd, void* movehelper, void* movedata);
	SetupMoveFn SetupMove;

	typedef void (__thiscall* FinishMoveFn)(void* thisptr, void* player, UserCmd* cmd, void* movedata);
	FinishMoveFn FinishMove;

	// gamemovement

	typedef void (__thiscall* ProcessMovementFn)(void* thisptr, void* player, void* movedata);
	ProcessMovementFn ProcessMovement;

	// enginetrace

	typedef int (__thiscall* GetPointContentsFn)(void* thisptr, Vector* point, DWORD mask, void* a4);
	GetPointContentsFn GetPointContents;

	typedef void (__thiscall* TraceRayFn)(void* thisptr, RayData* ray, DWORD mask, TraceFilter* filter, TraceResult* tr);
	TraceRayFn TraceRay;

	// model

	typedef char* (__thiscall* GetModelNameFn)(void* thisptr, void* name);
	GetModelNameFn GetModelName;

	typedef StudioModel* (__thiscall* GetStudioModelFn)(void* thisptr, void* model);
	GetStudioModelFn GetStudioModel;

	// surfacedata

	typedef void* (__thiscall* GetSurfaceDataFn)(void* thisptr, int data);
	GetSurfaceDataFn GetSurfaceData;

	// cvar

	typedef ConCmdBase* (__thiscall* FindCommandFn)(void* thisptr, char* name);
	FindCommandFn FindCommand;

	typedef ConVar* (__thiscall* FindVarFn)(void* thisptr, char* name);
	FindVarFn FindVar;

	typedef CvarIteratorInternal* (__thiscall* FactoryInternalIteratorFn)(void* thisptr);
	FactoryInternalIteratorFn FactoryInternalIterator;

	// game events

	typedef bool (__thiscall* AddListenerFn)(void* thisptr, void* listener, char* name, bool serverside);
	AddListenerFn AddListener;

	typedef void (__thiscall* RemoveListenerFn)(void* thisptr, void* listener);
	RemoveListenerFn RemoveListener;

	// render

	typedef void (__thiscall* MaterialOverrideFn)(void* thisptr, Material* material, void* a3, void* a4);
	MaterialOverrideFn MaterialOverride;

	// view

	typedef void (__thiscall* SetBlendFn)(void* thisptr, float blend);
	SetBlendFn SetBlend;

	typedef void (__thiscall* SetColorModulationFn)(void* thisptr, float* clr);
	SetColorModulationFn SetColorModulation;

	// material

	typedef Material* (__thiscall* FindMaterialFn)(void* thisptr, char* name, char* group, bool complain, void* complain_prefix);
	FindMaterialFn FindMaterial;

	typedef unsigned short (__thiscall* FirstMaterialFn)(void* thisptr);
	FirstMaterialFn FirstMaterial;

	typedef unsigned short (__thiscall* NextMaterialFn)(void* thisptr, unsigned short handle);
	NextMaterialFn NextMaterial;

	typedef Material* (__thiscall* GetMaterialFn)(void* thisptr, unsigned short handle);
	GetMaterialFn GetMaterial;

	// surface

	typedef void (__thiscall* DrawSetColorFn)(void* thisptr, int r, int g, int b, int a);
	DrawSetColorFn DrawSetColor;

	typedef void (__thiscall* DrawFilledRectFn)(void* thisptr, int x1, int y1, int x2, int y2);
	DrawFilledRectFn DrawFilledRect;

	typedef void (__thiscall* DrawOutlinedRectFn)(void* thisptr, int x1, int y1, int x2, int y2);
	DrawOutlinedRectFn DrawOutlinedRect;

	typedef void (__thiscall* DrawLineFn)(void* thisptr, int x1, int y1, int x2, int y2);
	DrawLineFn DrawLine;

	typedef void (__thiscall* SetFontFn)(void* thisptr, int font);
	SetFontFn SetFont;

	typedef void (__thiscall* SetTextColorFn)(void* thisptr, int r, int g, int b, int a);
	SetTextColorFn SetTextColor;

	typedef void (__thiscall* SetTextPosFn)(void* thisptr, int x, int y);
	SetTextPosFn SetTextPos;

	typedef void (__thiscall* DrawTextFn)(void* thisptr, wchar_t* str, int length, int draw_type);
	DrawTextFn DrawText;

	typedef bool (__thiscall* IsCursorVisibleFn)(void* thisptr);
	IsCursorVisibleFn IsCursorVisible;

	typedef DWORD (__thiscall* CreateFontFn)(void* thisptr);
	CreateFontFn CreateFont;

	typedef void (__thiscall* SetFontGlyphSetFn)(void* thisptr, int font_index, char* font, int tall, int weight, int blur, int scanlines, int flags, int a9, int a10);
	SetFontGlyphSetFn SetFontGlyphSet;

	typedef void (__thiscall* GetTextSizeFn)(void* thisptr, int font, wchar_t* str, int* x, int* y);
	GetTextSizeFn GetTextSize;

	typedef bool (__thiscall* IsCursorLockedFn)(void* thisptr);
	IsCursorLockedFn IsCursorLocked;

	typedef void (__thiscall* GetCursorPosFn)(void* thisptr, int* x, int* y);
	GetCursorPosFn GetCursorPos;

	typedef void (__thiscall* StartDrawingFn)(void* thisptr);
	StartDrawingFn StartDrawing;

	typedef void (__thiscall* FinishDrawingFn)(void* thisptr);
	FinishDrawingFn FinishDrawing;

	// debug

	typedef void (__thiscall* DrawBoxFn)(void* thisptr, Vector* origin, Vector* mins, Vector* maxs, Vector* angle, int r, int g, int b, int a, float duration);
	DrawBoxFn DrawBox;

	typedef void (__thiscall* DrawCapsuleFn)(void* thisptr, Vector* mins, Vector* maxs, float* radius, int r, int g, int b, int a, float duration);
	DrawCapsuleFn DrawCapsule;
	
	// studio

	typedef void (__thiscall* StudioSetColorModulationFn)(void* thisptr, float* color);
	StudioSetColorModulationFn StudioSetColorModulation;

	// movehelper

	typedef void (__thiscall* SetHostFn)(void* thisptr, void* player);
	SetHostFn SetHost;

	// net channel

	typedef float (__thiscall* GetLatencyFn)(void* thisptr, int type);
	GetLatencyFn GetLatency;

	// command line

	typedef int (__thiscall* GetParmFn)(void* thisptr, char* parameter);
	GetParmFn GetParm;
};