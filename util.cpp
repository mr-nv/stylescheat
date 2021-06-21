#include "dll.h"

// SDL_stdlib.c
extern "C" void __declspec(naked) _ftol2_sse()
{
	__asm {
		push        ebp
		mov         ebp,esp
		sub         esp,20h
		and         esp,0FFFFFFF0h
		fld         st(0)
		fst         dword ptr [esp+18h]
		fistp       qword ptr [esp+10h]
		fild        qword ptr [esp+10h]
		mov         edx,dword ptr [esp+18h]
		mov         eax,dword ptr [esp+10h]
		test        eax,eax
		je          integer_QnaN_or_zero
arg_is_not_integer_QnaN:
		fsubp       st(1),st
		test        edx,edx
		jns         positive
		fstp        dword ptr [esp]
		mov         ecx,dword ptr [esp]
		xor         ecx,80000000h
		add         ecx,7FFFFFFFh
		adc         eax,0
		mov         edx,dword ptr [esp+14h]
		adc         edx,0
		jmp         localexit
positive:
		fstp        dword ptr [esp]
		mov         ecx,dword ptr [esp]
		add         ecx,7FFFFFFFh
		sbb         eax,0
		mov         edx,dword ptr [esp+14h]
		sbb         edx,0
		jmp         localexit
integer_QnaN_or_zero:
		mov         edx,dword ptr [esp+14h]
		test        edx,7FFFFFFFh
		jne         arg_is_not_integer_QnaN
		fstp        dword ptr [esp+18h]
		fstp        dword ptr [esp+18h]
localexit:
		leave
		ret
	}
}

extern "C" void* __cdecl memcpy(void*, const void*, size_t);
#pragma intrinsic(memcpy)

#pragma function(memcpy)
void* __cdecl memcpy(void* out, const void* in, size_t size)
{
	for (DWORD i = 0; i < size; i += 0x1)
		*(BYTE*)((DWORD)(out) + i) = *(BYTE*)((DWORD)(in) + i);

	return out;
}
// void* __cdecl memset(void* _Dst, int _Val, size_t Size)
extern "C" void* __cdecl memset(void*, int, size_t);
#pragma intrinsic(memset)

#pragma function(memset)
void* __cdecl memset(void* out, int value, size_t size)
{
	for (DWORD i = 0; i < size; i += 0x1)
		*(BYTE*)((DWORD)(out) + i) = (BYTE)(value);

	return out;
}

int string_len(char* in)
{
	int i = 0;

	for (i = 0; in[i] != '\0'; ++i);

	return i;
}

void string_cpy(char* out, char* in)
{
	memset(out, 0, string_len(out));

	for (int i = 0; in[i] != '\0'; ++i)
		out[i] = in[i];
}

bool string_find(char* in, char* test)
{
	for (int i = 0; in[i] != '\0'; ++i)
	{
		bool found = true;

		for (int n = 0; test[n] != '\0'; ++n)
		{
			if (test[n] != in[i + n])
			{
				found = false;
				break;
			}
		}

		if (found)
			return true;
	}
	
	return false;
}

__declspec(naked) void UnloadStub()
{
	__asm
	{
		push 500
		mov eax, 0x7331
		call eax // Sleep

		push 0x8000
		push 0
		push 0x1234

		mov eax, 0x1337
		call eax // VirtualFree

		//push 0
		//mov eax, 0x6969
		//call eax // RtlExitUserThread

		ret
	}
}

__declspec(naked) void UnloadStubEnd() { }

void ScheduleUnload()
{
	//gVars.render_asus.value = 0;
	//gRender.AsusThink();

	*(DWORD**)(base->mBaseClient) = (DWORD*)(base->mBaseClientOldHookTable);
	*(DWORD**)(base->mClientMode) = (DWORD*)(base->mClientModeOldHookTable);
	*(DWORD**)(base->mPrediction) = (DWORD*)(base->mPredictionOldHookTable);
	*(DWORD**)(base->mStudioRender) = (DWORD*)(base->mStudioRenderOldHookTable);
	*(DWORD**)(base->mView) = (DWORD*)(base->mViewOldHookTable);
	*(DWORD**)(base->mEngineVgui) = (DWORD*)(base->mEngineVguiOldHookTable);
	*(DWORD**)(base->mBaseEngine) = (DWORD*)(base->mBaseEngineOldHookTable);

	/*base->VirtualFree(base->mBaseClientHookTable, 0, MEM_RELEASE);
	base->VirtualFree(base->mClientModeHookTable, 0, MEM_RELEASE);
	base->VirtualFree(base->mPredictionHookTable, 0, MEM_RELEASE);
	base->VirtualFree(base->mStudioRenderHookTable, 0, MEM_RELEASE);
	base->VirtualFree(base->mViewHookTable, 0, MEM_RELEASE);
	base->VirtualFree(base->mEngineVguiHookTable, 0, MEM_RELEASE);
	base->VirtualFree(base->mBaseEngineHookTable, 0, MEM_RELEASE);
	*/
	base->RemoveListener(base->mGameEvents, &player_hurt);
	base->RemoveListener(base->mGameEvents, &bullet_impact);

	/**(DWORD*)(base->mEyePitchFunc) = base->mEyePitchFuncOld;
	*(DWORD*)(base->mEyeYawFunc) = base->mEyeYawFuncOld;
	*(DWORD*)(base->mBodyYawFunc) = base->mBodyYawFuncOld;
	*(DWORD*)(base->mFlashFunc) = base->mFlashFuncOld;
	*(DWORD*)(base->mSmokeEffectFunc) = base->mSmokeEffectFuncOld;
	*/
	/*void* net = base->GetNetChannel(base->mBaseEngine);

	if (net && (*(DWORD**)(net))[42] != gClient->mSendNetMsgAddress)
		(*(DWORD**)(net))[42] = (DWORD)(gClient->mSendNetMsgAddress);

	DWORD stub_size = (DWORD)(UnloadStubEnd) - (DWORD)(UnloadStub);
	*/
//	for (DWORD x = 0; x <= stub_size; x += 0x1)
	//	base->Warning("stub %x %x\n",x,*(DWORD*)((DWORD)(UnloadStub) + x));

	/**(DWORD*)((DWORD)(UnloadStub) + 0x6) = (DWORD)(base->Sleep);
	*(DWORD*)((DWORD)(UnloadStub) + 0x14) = (DWORD)(base->mModuleBaseAddress);
	*(DWORD*)((DWORD)(UnloadStub) + 0x19) = (DWORD)(base->VirtualFree);
//	*(DWORD*)((DWORD)(UnloadStub) + 0x22) = (DWORD)(base->RtlExitUserThread);

	void* stub = base->VirtualAlloc(nullptr, stub_size, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy(stub, (void*)(UnloadStub), stub_size);

	DWORD address = base->mModuleBaseAddress;

	CreateThreadFn _CreateThread = (CreateThreadFn)((DWORD)(base->CreateThread));
	GetLastErrorFn _GetLastError = (GetLastErrorFn)((DWORD)(base->GetLastError));
	WarningFn _Warning = (WarningFn)((DWORD)(base->Warning));
	VirtualFreeFn _VirtualFree = (VirtualFreeFn)((DWORD)(base->VirtualFree));

	_VirtualFree(base, 0, MEM_RELEASE);

	_CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)(stub), nullptr, 0, nullptr);*/
}