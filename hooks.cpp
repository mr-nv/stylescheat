#include "dll.h"

extern "C" const int _fltused = 0;

static bool sLoaded = false;
static bool sUnload = false;

CBaseClass* base = nullptr;

CHurtEvent player_hurt;
CImpactEvent bullet_impact;

DWORD __cdecl GetBaseClassAddress()
{
	return 0x1337;
}

DWORD __cdecl GetBaseClassSize()
{
	return sizeof(CBaseClass);
}

bool __fastcall SendNetMsg(void* ecx, void* edx, NetMsg* netmsg, bool force_reliable, bool voice)
{
	int type = netmsg->GetType();

	if (gVars.misc_cvarbypass.value || gClient->mCvarsFixed)
	{
		if (type == 13)
			return false;
	}

	if (gVars.misc_nopure.value)
	{
		if (type == 14)
			return false;
	}

	typedef bool (__thiscall* dfn)(void* thisptr, NetMsg* netmsg, bool force_reliable, bool voice);
	dfn fn = (dfn)(gClient->mSendNetMsgAddress);

	bool result = fn(ecx, netmsg, force_reliable, voice);

	return result; // 47 - setchoked
}

char* bypass_cvars[] = {
"sv_cheats",
"mat_wireframe",
"r_drawothermodels",
"enable_skeleton_draw",
"r_drawbeams",
"r_drawbrushmodels",
"r_drawdetailprops",
"r_drawstaticprops",
"r_modelwireframedecal",
"r_shadowwireframe",
"r_slowpathwireframe",
"r_visocclusion",
"vcollide_wireframe",
"mp_radar_showall",
"radarvisdistance",
"mat_proxy",
"mat_drawflat",
"mat_norendering",
"mat_drawgray",
"mat_showmiplevels",
"mat_showlowresimage",
"mat_measurefillrate",
"mat_fillrate",
"mat_reversedepth",
"fog_override",
"r_drawentities",
"r_drawdisp",
"r_drawfuncdetail",
"r_drawworld",
"r_drawmodelstatsoverlay",
"r_drawopaqueworld",
"r_drawtranslucentworld",
"r_drawopaquerenderables",
"r_drawtranslucentrenderables",
"mat_normals",
"sv_allow_thirdperson",
"cl_mouseenable"};

Vector sLastAngles2 = Vector(0.0f, 0.0f, 0.0f);
int sLastSpaceName = 0;

// 0x18 = outsequencenr
// 0x1c = insequencenr

static char tabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
static int last_interp = 0;

void __fastcall HookFrameStageNotify(void* ecx, void* edx, int stage)
{
	if (stage == FRAME_START)
		gClient->HookPreThink();

	if (!base)
		return;

	if (!sLoaded && !gMenu.mIssueUnload)
	{
		base->AddListener(base->mGameEvents, &player_hurt, "player_hurt", false);
		base->AddListener(base->mGameEvents, &bullet_impact, "bullet_impact", false);

		base->Warning("%s loaded\n", base->mTitle);
		sLoaded = true;
	}

	if (sLoaded )//&& stage == FRAME_START)
	{
		if (!base->IsInGame(base->mBaseEngine))
		{
			base->mMoveHelper = nullptr;
			base->mRenderContext = nullptr;

			gClient->mPredictionPlayer = nullptr;
			gRender.mAsusReload = true;
		//	gClient->mNetPointer = nullptr;
		}
		else
		{
		/*	void* net = base->GetNetChannel(base->mBaseEngine);
			if (net)
			{
				if ((*(DWORD**)(net))[42] != gClient->mSendNetMsgHook)
				{
					base->VirtualProtect((LPVOID)(&(*(DWORD**)(net))[42]), 0x4, PAGE_EXECUTE_READWRITE, &gClient->mSendNetMsgProt);

					DWORD adr = (*(DWORD**)(net))[42];
					(*(DWORD**)(net))[42] = (DWORD)(&SendNetMsg);
					gClient->mSendNetMsgHook = (*(DWORD**)(net))[42];
					gClient->mSendNetMsgAddress = adr;
				}
			}*/
		}
	}

	if (stage == FRAME_RENDER_START)
	{
		gClient->mThirdPerson = false;

		Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));

		if (IsEntityValid(local))
		{
			if (base->ShouldDrawLocalPlayer(base->mClientMode, local))
			{
				gClient->UpdateLocalPlayerAnimations();

				gClient->mThirdPerson = true;
			}
		}

		if (gVars.aim_active.value && gVars.aim_noeffects.value)
		{
			if (base->mSmokeCount)
				*(int*)(base->mSmokeCount) = 0;
		}
	}

	if (stage == FRAME_RENDER_START)
	{
		if (!gClient->mCvarsFixed)
		{
			ConVar* sv_cheats = base->FindVar(base->mEngineCvar, "sv_cheats");

			if (sv_cheats && sv_cheats->GetFlags() != 0)
			{
				sv_cheats->GetFlags() = 0;

				ConVar* host_timescale = base->FindVar(base->mEngineCvar, "host_timescale");
				if (host_timescale)
					host_timescale->GetFlags() &= ~FCVAR_REPLICATED;

				ConVar* host_framerate = base->FindVar(base->mEngineCvar, "host_framerate");
				if (host_framerate)
					host_framerate->GetFlags() &= ~FCVAR_REPLICATED;

				CvarIteratorInternal* iter = base->FactoryInternalIterator(base->mEngineCvar);
				iter->SetFirst();

				while (true)
				{
					if (!iter->IsValid())
						break;

					iter->Get()->GetFlags() |= 8;
					iter->Get()->GetFlags() &= ~(FCVAR_UNREGISTERED|FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_NOT_CONNECTED);

					iter->Next();
				}

				for (int i = 0; i < 37; ++i)
				{
					ConVar* var = base->FindVar(base->mEngineCvar, bypass_cvars[i]);
					if (!var)
						continue;

					char cmd[128];
					base->wsprintfA(cmd, "setinfo _%s %i", bypass_cvars[i], var->GetValue());

					base->ConCommand(base->mBaseEngine, cmd);
				}

				char** cvars = (char**)(base->VirtualAlloc(nullptr, 36 * 4, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE));
				for (int i = 0; i < 36; ++i)
					cvars[i] = (char*)(base->VirtualAlloc(nullptr, 128, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE));

				for (int i = 0; i < 36; ++i)
				{
					ConVar* var = base->FindVar(base->mEngineCvar, bypass_cvars[i]);
					if (!var)
						continue;
				
					base->wsprintfA(cvars[i], "_%s", bypass_cvars[i]);

					ConVar* var2 = base->FindVar(base->mEngineCvar, cvars[i]);
					if (!var2)
						continue;

					memcpy(var2, var, 46);

					*var->GetName() = cvars[i];

					gClient->mCvarsFixed = true;
				}
				
				{
					char name[4096];
					utf8cpy(name, *base->FindVar(base->mEngineCvar, "name")->GetString(), 4096);

					char* cvar_name = (char*)(base->VirtualAlloc(nullptr, 128, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE));
					string_cpy(cvar_name, "_name");

					*base->FindVar(base->mEngineCvar, "name")->GetName() = cvar_name;

					char str[128];
					base->wsprintfA(str, "setinfo name \"%s\"", name);

					base->ConCommand(base->mBaseEngine, str);
				}

				{
					ConVar* var = base->FindVar(base->mEngineCvar, "cl_interp");
					*var->GetCallbackSize() = 0;
				}

				{
					ConVar* var = base->FindVar(base->mEngineCvar, "cl_interp_ratio");
					*var->GetCallbackSize() = 0;
				}

				{
					char* cvar_name = (char*)(base->VirtualAlloc(nullptr, 128, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE));
					string_cpy(cvar_name, "_cl_interp");

					*base->FindVar(base->mEngineCvar, "cl_interp")->GetName() = cvar_name;

					base->ConCommand(base->mBaseEngine, "setinfo cl_interp 0.031");
				}
			}
		}

		if (!base->IsInGame(base->mBaseEngine) && gVars.misc_force_interp.value != last_interp)
		{
			{
				if (gVars.misc_force_interp.value == 1)
					base->ConCommand(base->mBaseEngine, "setinfo cl_interp 0.063");
				else if (gVars.misc_force_interp.value == 2)
					base->ConCommand(base->mBaseEngine, "setinfo cl_interp 0.19");
				else
					base->ConCommand(base->mBaseEngine, "setinfo cl_interp 0.031");
			}

			{
				ConVar* var = base->FindVar(base->mEngineCvar, "cl_interp_ratio");
				if (var)
				{
					if (gVars.misc_force_interp.value == 1)
						var->SetValueInt(4);
					else if (gVars.misc_force_interp.value == 2)
						var->SetValueInt(5);
					else
						var->SetValueInt(2);
				}
			}
		}

		last_interp = gVars.misc_force_interp.value;
		
		Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));

		if (IsEntityValid(local))
		{
			if (gVars.misc_space_name.value && gVars.misc_space_name.value != sLastSpaceName)
			{
				char str[256];

				base->wsprintfA(str, "setinfo name \"%s\"", tabs);

				base->ConCommand(base->mBaseEngine, str);

				sLastSpaceName = gVars.misc_space_name.value;
			}

			for (int i = 1; i <= base->mGlobalVars->clients; ++i)
			{
				Entity* player = base->GetClientEntity(base->mEntList, i);
				if (!IsEntityValid(player))
					continue;

				if (player == local)
					continue;

				//if (gAim->mAimPlayers[i].mAnimating)
				//	*player->m_bClientSideAnimation() = false;
			}
		}
	}

	typedef void (__thiscall* dfn)(void* thisptr, int stage);
	((dfn)(base->mFrameStageNotify))(ecx, stage);

	if (stage == FRAME_RENDER_START)
	{
		for (int i = 1; i <= base->mGlobalVars->clients; ++i)
		{
			Entity* player = base->GetClientEntity(base->mEntList, i);
			if (!IsEntityValid(player))
				continue;

		//	*player->m_bClientSideAnimation() = true;
		}
	}

	if (stage == FRAME_RENDER_START)
		gClient->FixEngineSetupBones();

	if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
	{
		gClient->NetUpdateThink();

		Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
		if (IsEntityValid(local))
		{
			ConVar* mp_forcecamera = base->FindVar(base->mEngineCvar, "mp_forcecamera");
			if (mp_forcecamera && mp_forcecamera->GetValue() != 0 && gVars.misc_spectate.value)
			{
				if (gClient->mAliveAndProcessing)
				{
					*local->m_iObserverMode() = 0;
					*local->m_hObserverTarget() = -1;
				}
				else if (*local->m_hObserverTarget() != -1)
				{
					*local->m_iObserverMode() = 5;
				}
			}
		}
	}

	if (gMenu.mIssueUnload)
		sUnload = true;

//	if (sLoaded)
	{
		if (sUnload)
		{
			base->Warning("unloaded\n");
			ScheduleUnload();
		}
	}
}

void __fastcall HookOverrideView(void* ecx, void* edx, void* view)
{
	if (!base)
		return;

	ViewSetup* setup = (ViewSetup*)(view);
	if (gVars.render_fov.value)
		setup->fov = (float)(gVars.render_fov.value);

	typedef void (__thiscall* dfn)(void* thisptr, void* view);
	((dfn)(base->mOverrideView))(ecx, view);

	gClient->FixEngineSetupBones();
}

bool __fastcall HookCreateMove(void* ecx, void* edx, float time, UserCmd* cmd)
{
	if (!base)
		return false;

	typedef bool (__thiscall* dfn)(void* thisptr, float time, UserCmd* cmd);
	((dfn)(base->mCreateMove))(ecx, time, cmd);

	if (cmd->index == 0)
		return true;

	void* rEBP;
	__asm mov rEBP, ebp;
	
	gClient->mSendMove = (bool*)(*(bool**)(rEBP) - 0x1C);

	if (gVars.move_speedhack.value && gClient->mSpeedhackKey)
	{
		DWORD* ret = (DWORD*)(****(DWORD****)(rEBP) + 0x4);

		static int hold = 0;
		static int last_tick = 0;

		if (hold != 0)
		{
			*gClient->mSendMove = false;
			*ret -= 0x5;
			hold -= 1;
		}
		else
		{
			*gClient->mSendMove = true;
			hold = gVars.move_speedhack_factor.value;
		}

	//	if (base->mGlobalVars->ticks == last_tick)
		//	return false;

		last_tick = base->mGlobalVars->ticks;
	}

	gClient->mUserCmd = cmd;
	memcpy(&gClient->mLastCopyCmd, &gClient->mCopyCmd, sizeof(UserCmd));
	memcpy(&gClient->mCopyCmd, cmd, sizeof(UserCmd));

	gClient->CalculateMouse(cmd);

	if (!gClient->mCopyNextCmd)
		gClient->Think(cmd);

	gClient->RestrictCmd(cmd);

	/*if (*gClient->mSendMove && gClient->mSendNextCmd)
	{
		memcpy(&gClient->mNextUserCmd, cmd, sizeof(UserCmd));
		
		void* net = base->GetNetChannel(base->mBaseEngine);
		if (net)
			*(int*)((DWORD)(net) + 0x2C) -= 1;

		*(***(char****)(rEBP) + 0x4) -= 0x5;

		gClient->mCopyNextCmd = true;
		gClient->mSendNextCmd = false;
	}*/
	
	return false;
}

void __cdecl UpdateCommand()
{
	int num = gClient->mNextUserCmd.index;

	UserCmd* cmds = *(UserCmd**)((DWORD)(base->mInput) + 0xEC);
	memcpy(&cmds[num % 150], &gClient->mNextUserCmd, sizeof(UserCmd));

	VerifiedCmd* verified = *(VerifiedCmd**)((DWORD)(base->mInput) + 0xF0);
	memcpy(&(verified[num % 150].cmd), &cmds[num % 150], sizeof(UserCmd));
}

void __fastcall HookClientCreateMove(void* ecx, void* edx, int sequence, float time, bool active)
{
	if (!base)
		return;

	gClient->mFrameMult = base->mGlobalVars->frametime / base->mGlobalVars->interval;

	if (gClient->mCopyNextCmd)
	{
		//UpdateCommand();

		gClient->mCopyNextCmd = false;
	}
	else
	{
		typedef void (__thiscall* dfn)(void* thisptr, int sequence, float time, bool active);
		((dfn)(base->mClientCreateMove))(ecx, sequence, time, active);
	}
}

void __fastcall HookDrawModel(void* ecx, void* edx, void* a1, DrawModelInfo* info, void* a3, float* a4, float* a5, float* a6, int a7)
{
	typedef void (__thiscall* dfn)(void* thisptr, void* a1, void* info, void* a3, float* a4, float* a5, float* a6, int a7);

	if (!base)
		return;

	if (!info->entity)
	{
		((dfn)(base->mDrawModel))(ecx, a1, info, a3, a4, a5, a6, a7);
		return;
	}

	Entity* entity = (Entity*)((DWORD)(info->entity) - 0x4);
	int result = gRender.ProcessDrawModel(entity);

	if (result == DRAWMODEL_RETURN)
		return;

	if (result == DRAWMODEL_PLAYER)
	{
		int num_lods = info->hwdata->num_lods;

		info->hwdata->num_lods = 1;

		((dfn)(base->mDrawModel))(ecx, a1, info, a3, a4, a5, a6, a7);

		info->hwdata->num_lods = num_lods;

		return;
	}

	((dfn)(base->mDrawModel))(ecx, a1, info, a3, a4, a5, a6, a7);
}

void __fastcall HookDrawWorldLists(void* ecx, void* edx, void* a1, void* a2, void* a3, void* a4)
{
	if (!base)
		return;

	if (sLoaded)
		gRender.Think();

	typedef void (__thiscall* dfn)(void* thisptr, void* a1, void* a2, void* a3, void* a4);
	((dfn)(base->mDrawWorldLists))(ecx, a1, a2, a3, a4);
}

static DWORD sLastCallAddr = 0;

bool __fastcall HookInPrediction(void* ecx, void* edx)
{
	if (!base)
		return false;

	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (IsEntityValid(local))
	{
		void** rEBP;
		__asm MOV rEBP, EBP

		DWORD check_adr = ((DWORD)(rEBP[1]) - 0x1C);

		if (*(BYTE*)(check_adr) == 0x0F || *(BYTE*)(check_adr + 0x1) == 0x5B || *(BYTE*)(check_adr + 0x2) == 0xC0)
		{
			float* fov = (float*)(*(DWORD*)(rEBP) - 0x8);

			if (gVars.esp_no_scope.value)
				*fov = 90.0f;
		}

		if (((DWORD)(rEBP[1]) - sLastCallAddr) == 0xED)
		{
			Vector* origin = *(Vector**)(*(DWORD*)(rEBP) + 0x8);
			Vector* angles = *(Vector**)(*(DWORD*)(rEBP) + 0xC);

			gClient->mViewOriginEngine = origin;

			if (gVars.aim_active.value && gVars.aim_noeffects.value)
			{
				{
					Vector aim_punch = *local->m_aimPunchAngle();
					aim_punch *= 0.9f;

					ConVar* weapon_recoil_scale = base->FindVar(base->mEngineCvar, "weapon_recoil_scale");
					if (weapon_recoil_scale)
						aim_punch *= (float)(weapon_recoil_scale->GetValue()) / 2.0f;

					*angles -= aim_punch;
				}

				{
					Vector view_punch = *local->m_viewPunchAngle();
					*angles -= view_punch;
				}
			}
			else if (gClient->Cfg(gVars.l_rcs_active))
			{
				*angles -= gLegit.mPunchAngles;
			}

			//*angles = gLegit.mTargetViewangles;
		}

		sLastCallAddr = (DWORD)(rEBP[1]);
	}

	typedef bool (__thiscall* dfn)(void* thisptr);
	return ((dfn)(base->mInPrediction))(ecx);
}

void __fastcall HookRunCommand(void* ecx, void* edx, Entity* player, UserCmd* cmd, void* movehelper)
{
	if (!base)
		return;

	typedef void (__thiscall* dfn)(void* thisptr, Entity* player, UserCmd* cmd, void* movehelper);

	base->mMoveHelper = movehelper;
	base->SetHost = (CBaseClass::SetHostFn)((*(DWORD**)(movehelper))[1]);

	bool process = true;

	if (cmd->buttons & IN_AIRSTUCK)
		process = false;

	if (cmd->buttons & IN_DUCK)
	{
		if (gVars.misc_server.value & (1<<1))
			cmd->buttons &= ~IN_DUCK;
	}

	if (process)
		((dfn)(base->mRunCommand))(ecx, player, cmd, movehelper);

	//*player->m_bClientSideAnimation() = false;

	if (!cmd->predicted)
	{
		gClient->mPredictionPlayer = player;

		CPredFrame* frame = &gClient->mPredFrames[cmd->index % 150];
		frame->mIndex = cmd->index;

		base->SetupMove(ecx, player, cmd, nullptr, frame->mMoveData);

		frame->mFlags = *player->m_fFlags();
		frame->mGroundEntity = *player->m_hGroundEntity();
		frame->mTickBase = *player->m_nTickBase();
		frame->mAimPunch = *player->m_aimPunchAngle();
		frame->mDuckAmount = *player->m_flDuckAmount();
		frame->mDuckSpeed = *player->m_flDuckSpeed();
	}
}

void __fastcall HookGetLocalAngles(void* ecx, void* edx, Vector* angles)
{
	if (!base)
		return;
	
	typedef void (__thiscall* dfn)(void* thisptr, Vector* angles);
	((dfn)(base->mGetLocalAngles))(ecx, angles);

	if (gClient->mUserCmd)
		*angles = gClient->mCopyCmd.angles;
}

void __fastcall HookEncodeUsercmd(void* ecx, void* edx, int slot, void* buffer, int slot2)
{
	if (!base)
		return;
	
	UserCmd copy;

	if (gClient->mUserCmd)
	{
		memcpy(&copy, gClient->mUserCmd, sizeof(UserCmd));
		memcpy(gClient->mUserCmd, &gClient->mCopyCmd, sizeof(UserCmd));
	}

	typedef void (__thiscall* dfn)(void* thisptr, int slot, void* buffer, int slot2);
	((dfn)(base->mEncodeUsercmd))(ecx, slot, buffer, slot2);

	if (gClient->mUserCmd)
		memcpy(gClient->mUserCmd, &copy, sizeof(UserCmd));
}

bool __fastcall HookTimeDemo(void* ecx, void* edx)
{
	if (!base)
		return false;

	void** rEBP;
	_asm mov rEBP, ebp;

	typedef bool (__thiscall* dfn)(void* thisptr);
	return ((dfn)(base->mTimeDemo))(ecx);
}

void __fastcall HookPaint(void* ecx, void* edx, int mode)
{
	if (!base)
		return;

	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));

	bool scoped = false;

	if (IsEntityValid(local))
	{
		scoped = *local->m_bIsScoped();

		if (gVars.esp_no_scope.value)
			*local->m_bIsScoped() = false;
	}

	typedef void (__thiscall* dfn)(void* thisptr, int mode);
	((dfn)(base->mPaint))(ecx, mode);

	if (IsEntityValid(local))
		*local->m_bIsScoped() = scoped;

	if (!sLoaded)
		return;

	if (mode == 1)
	{
		base->StartDrawing(base->mSurface);

		gDraw.Think();

		base->FinishDrawing(base->mSurface);
	}
}

int __fastcall HookClientKeyEvent(void* ecx, void* edx, int down, int keynum, char* current_binding)
{
	if (!base)
		return 0;

	typedef int (__thiscall* dfn)(void* thisptr, int down, int keynum, char* current_binding);

	gMenu.BindThink((down == 1) ? 0 : 1, keynum);

	if (current_binding)
	{
		CRC32 crc = CRC32_Get(current_binding, string_len(current_binding));

		if (crc == 0x70150522) // "drop"
		{
			if (down)
				gAim->mDropPeriod = 2;

			return 0;
		}
	}
	
	return ((dfn)(base->mClientKeyEvent))(ecx, down, keynum, current_binding);
}

bool __fastcall HookKeyEvent(void* ecx, void* edx, void* k_event)
{
	if (!base)
		return true;

	typedef bool (__thiscall* dfn)(void* thisptr, void* k_event);

	if (!gMenu.ShouldHighlightElements())
		return true;

	if (gMenu.mDrawing)
	{
		int key = *(int*)((DWORD)(k_event) + 0x8);

		if (key == MOUSE_LEFT || key == MOUSE_RIGHT || key == MOUSE_WHEEL_UP || key == MOUSE_WHEEL_DOWN || key == KEY_ESCAPE)
			return true;
	}

	return ((dfn)(base->mKeyEvent))(ecx, k_event);
}

void __fastcall HookUpdateButton(void* ecx, void* edx, void* k_event)
{
	if (!base)
		return;

	typedef bool (__thiscall* dfn)(void* thisptr, void* k_event);

	int type = *(int*)((DWORD)(k_event));
	int key = *(int*)((DWORD)(k_event) + 0x8);

	gMenu.KeyThink(type, key);
	
	if (type == IE_ButtonPressed || type == IE_ButtonReleased)
		gMenu.BindThink(type, key);

	if (type == IE_ButtonPressed)
	{
		SettingVar* list = gMenu.mSelectedList;

		if (list)
		{
			int* value = &list->value;

			if (list->group)
				value = &list->group_vars[gVars.current_group.value].value;

			if (key == MOUSE_WHEEL_UP)
				*value -= 1;
			else if (key == MOUSE_WHEEL_DOWN)
				*value += 1;

			if (*value < list->min)
				*value = list->min;

			if (*value > list->max)
				*value = list->max;
		}
	}

	if (type == IE_ButtonPressed)
	{
		SettingVar* var = gMenu.mCurrentVar;

		if (var)
		{
			if (var->type == VARTYPE_KEYLIST)
			{
				int* value = &var->value;

				if (var->group)
					value = &var->group_vars[gVars.current_group.value].value;

				bool valid = true;

				if (key == KEY_ESCAPE || key == KEY_INSERT)
					valid = false;

				if (var == &gVars.misc_menukey && key == MOUSE_LEFT)
					valid = false;

				if (valid)
				{
					*value = key;

					if (*value < var->min)
						*value = var->min;

					if (*value > var->max)
						*value = var->max;
				}
				else
				{
					*value = 0;
				}

				gMenu.mCurrentVar = nullptr;
			}
		}
	}

	if (type == IE_ButtonRepeat)
	{
		MenuPopup* popup = &gMenu.mPopupWindow;

		if (popup->type == POPUP_STRING)
		{
			if (key >= KEY_1 && key <= KEY_Z && popup->string_pos < popup->string_max)
				popup->string[popup->string_pos++] = key_names[key][0];

			if (key == KEY_BACKSPACE && popup->string_pos > 0)
				popup->string[--popup->string_pos] = '\0';
		}
	}

	if (key == MOUSE_LEFT)
	{
		if (type == IE_ButtonPressed || type == IE_ButtonDoubleClicked)
		{
			gMenu.mMouseHeld = true;
			gMenu.mMouseClicked = true;
			gMenu.mMouseReleased = false;
		}
		else if (type == IE_ButtonReleased)
		{
			gMenu.mMouseHeld = false;
			gMenu.mMouseClicked = false;
			gMenu.mMouseReleased = true;
		}
	}

	if (key == MOUSE_RIGHT)
	{
		if (type == IE_ButtonPressed)
		{
			gMenu.mMouse2Held = true;
			gMenu.mMouse2Clicked = true;
			gMenu.mMouse2Released = false;
		}
		else if (type == IE_ButtonReleased)
		{
			gMenu.mMouse2Held = false;
			gMenu.mMouse2Clicked = false;
			gMenu.mMouse2Released = true;
		}
	}

	((dfn)(base->mUpdateButton))(ecx, k_event);
}

void __fastcall HookOverrideMouse(void* ecx, void* edx, float* x, float* y)
{
	if (!base)
		return;

	typedef void (__thiscall* dfn)(void* thisptr, float* x, float* y);
	((dfn)(base->mOverrideMouse))(ecx, x, y);
}

void __cdecl HookEyePitch(ProxyData* data, Entity* entity, float* out)
{
	float pitch = data->var.fl;

	*out = pitch;
}

void __cdecl HookEyeYaw(ProxyData* data, Entity* entity, float* out)
{
	float yaw = data->var.fl;

	*out = yaw;
}

void __cdecl HookBodyYaw(ProxyData* data, Entity* entity, float* out)
{
	float yaw = data->var.fl;

	*out = yaw;
}

void __cdecl HookFlash(ProxyData* data, Entity* entity, float* out)
{
	if (!base)
		return;

	typedef void (__cdecl* dfn)(ProxyData* data, Entity* entity, float* out);
	((dfn)(base->mFlashFuncOld))(data, entity, out);

	if (gVars.aim_active.value && gVars.aim_noeffects.value)
		*out = 0.0f;
}

void __cdecl HookSmokeEffect(ProxyData* data, Entity* entity, long long* out)
{
	if (!base)
		return;

	typedef void (__cdecl* dfn)(ProxyData* data, Entity* entity, long long* out);
	((dfn)(base->mSmokeEffectFuncOld))(data, entity, out);

	if (IsEntityValid(entity))
		gESP.mSmokeEffectTickBegin[entity->GetIndex()] = (int)(data->var.int64);

	if (gVars.aim_active.value && gVars.aim_noeffects.value)
		*out = -2147483647 + 1;
}