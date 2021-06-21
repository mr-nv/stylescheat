struct ClientClass
{
	char pad1[8];
	char* name;
	void* table;
	ClientClass* next;
	int id;
};

struct AnimOverlay;

#define SIZEOF_ANIMSTATE_REGION 384

static Vector null_vec;

class Entity
{
public:
	__forceinline DWORD GetPlayerAnimState()
	{
		return *(DWORD*)(this + base->m_PlayerAnimState);
	}
	__forceinline void* GetAnimStateRegion()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (void*)(AnimState);
	}
	__forceinline int* GetAnimTick()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (int*)(AnimState + 8);
	}
	__forceinline float* GetClockTime()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (float*)(AnimState + 24);
	}
	__forceinline float* GetClockTime2()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (float*)(AnimState + 344);
	}
	__forceinline float* GetRenderTime()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (float*)(AnimState + 108);
	}
	__forceinline float* GetRenderTimeDelta()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (float*)(AnimState + 116);
	}
	__forceinline float* GetHeightFloat()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (float*)(AnimState + 164);
	}
	__forceinline float* GetMoveSpeed()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (float*)(AnimState + 236);
	}
	__forceinline float* GetMoveSpeedZ()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (float*)(AnimState + 240);
	}
	__forceinline float* GetMoveSpeedX()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (float*)(AnimState + 200);
	}
	__forceinline float* GetMoveSpeedY()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (float*)(AnimState + 204);
	}
	__forceinline Vector* GetMoveVelocity()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (Vector*)(AnimState + 348);
	}
	__forceinline Vector* GetAngRender()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (Vector*)(AnimState + 124);
	}
	__forceinline Vector* GetOrigin1()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (Vector*)(AnimState + 176);
	}
	__forceinline Vector* GetOrigin2()
	{
		DWORD AnimState = GetPlayerAnimState();
		return (Vector*)(AnimState + 188);
	}
	__forceinline void* GetAnimPlayer()
	{
		DWORD AnimState = GetPlayerAnimState();
		void* p = *(void**)(AnimState + 96);

		return p;
	}
	__forceinline void Interpolate(float time)
	{
		typedef void (__thiscall* dfn)(void* thisptr, float time);
		((dfn)((*(DWORD**)(this))[112]))(this, time);
	}
	__forceinline void UpdateClientSideAnimation()
	{
		void* p = GetAnimPlayer();
		if (!p)
			return;

		typedef void (__thiscall* dfn)(void* thisptr);
		((dfn)((*(DWORD**)(p))[218]))(p);
	}
	__forceinline Vector* GetRenderOrigin()
	{
		void* p = this + 0x4;
		if (!p)
			return &null_vec;

		typedef Vector* (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(p))[1]))(p);
	}
	__forceinline Vector* GetRenderAngles()
	{
		void* p = this + 0x4;
		if (!p)
			return &null_vec;

		typedef Vector* (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(p))[2]))(p);
	}
	__forceinline void* GetModel()
	{
		void* p = this + 0x4;
		if (!p)
			return nullptr;

		typedef void* (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(p))[8]))(p);
	}
	__forceinline void DrawModel(int mode, void* a2)
	{
		void* p = this + 0x4;
		if (!p)
			return;

		typedef int (__thiscall* dfn)(void* thisptr, int mode, void* a2);
		((dfn)((*(DWORD**)(p))[9]))(p, mode, a2);
	}
	__forceinline void SetupBones(Matrix* matrix, int max_bones, int bonemask, float curtime)
	{
		void* p = this + 0x4;
		if (!p)
			return;

		typedef int (__thiscall* dfn)(void* thisptr, Matrix* matrix, int max_bones, int bonemask, float curtime);
		((dfn)((*(DWORD**)(p))[13]))(p, matrix, max_bones, bonemask, curtime);
	}
	__forceinline ClientClass* GetClientClass()
	{
		void* p = this + 0x8;
		if (!p)
			return nullptr;

		typedef ClientClass* (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(p))[2]))(p);
	}
	__forceinline bool IsDormant()
	{
		void* p = this + 0x8;
		if (!p)
			return true;

		typedef bool (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(p))[9]))(p);
	}
	__forceinline int GetIndex()
	{
		void* p = this + 0x8;
		if (!p)
			return 1;

		typedef bool (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(p))[10]))(p);
	}

	__forceinline float* m_flSimulationTime() { return (float*)(this + base->m_flSimulationTime); }
	__forceinline BYTE* m_clrRender() { return (BYTE*)(this + base->m_clrRender); }
	__forceinline int* m_iTeamNum() { return (int*)(this + base->m_iTeamNum); }
	__forceinline Vector* m_vecOrigin() { return (Vector*)(this + base->m_vecOrigin); }
	__forceinline Vector* m_vecMins() { return (Vector*)(this + base->m_vecMins); }
	__forceinline Vector* m_vecMaxs() { return (Vector*)(this + base->m_vecMaxs); }

	__forceinline int* m_nTickBase() { return (int*)(this + base->m_nTickBase); }
	__forceinline BYTE* m_lifeState() { return (BYTE*)(this + base->m_lifeState); }
	__forceinline int* m_iHealth() { return (int*)(this + base->m_iHealth); }
	__forceinline DWORD* m_fFlags() { return (DWORD*)(this + base->m_fFlags); }
	__forceinline DWORD* m_fEffects() { return (DWORD*)(this + base->m_fEffects); }
	__forceinline Vector* m_aimPunchAngle() { return (Vector*)(this + base->m_aimPunchAngle); }
	__forceinline Vector* m_viewPunchAngle() { return (Vector*)(this + base->m_viewPunchAngle); }
	__forceinline Vector* m_vecViewOffset() { return (Vector*)(this + base->m_vecViewOffset); }
	__forceinline Vector* m_vecVelocity() { return (Vector*)(this + base->m_vecVelocity); }
	__forceinline DWORD* m_hLastWeapon() { return (DWORD*)(this + base->m_hLastWeapon); }
	__forceinline float* m_flDuckAmount() { return (float*)(this + base->m_flDuckAmount); }
	__forceinline float* m_flDuckSpeed() { return (float*)(this + base->m_flDuckSpeed); }

	__forceinline BYTE* m_MoveType() { return (BYTE*)(this + base->m_MoveType); }
	__forceinline BYTE* m_nWaterLevel() { return (BYTE*)(this + base->m_nWaterLevel); }
	__forceinline DWORD* m_hGroundEntity() { return (DWORD*)(this + base->m_hGroundEntity); }
	__forceinline int* m_ArmorValue() { return (int*)(this + base->m_ArmorValue); }
	__forceinline Vector* m_angEyeAngles() { return (Vector*)(this + base->m_angEyeAngles); }
	__forceinline bool* m_bIsScoped() { return (bool*)(this + base->m_bIsScoped); }
	__forceinline bool* m_iShotsFired() { return (bool*)(this + base->m_iShotsFired); }
	__forceinline int* m_iObserverMode() { return (int*)(this + base->m_iObserverMode); }
	__forceinline DWORD* m_hObserverTarget() { return (DWORD*)(this + base->m_hObserverTarget); }
	__forceinline float* m_flLowerBodyYawTarget() { return (float*)(this + base->m_flLowerBodyYawTarget); }
	__forceinline bool* m_bHasDefuser() { return (bool*)(this + base->m_bHasDefuser); }
	__forceinline bool* m_bGunGameImmunity() { return (bool*)(this + base->m_bGunGameImmunity); }

	__forceinline bool* m_bBurstMode() { return (bool*)(this + base->m_bBurstMode); }
	__forceinline float* m_flPostponeFireReadyTime() { return (float*)(this + base->m_flPostponeFireReadyTime); }
	__forceinline float* m_fAccuracyPenalty() { return (float*)(this + base->m_fAccuracyPenalty); }

	__forceinline float* m_flNextBurstFire() { return (float*)(this + base->m_flNextBurstFire); }

	__forceinline DWORD* m_hActiveWeapon() { return (DWORD*)(this + base->m_hActiveWeapon); }
	__forceinline DWORD* m_hMyWeapons() { return (DWORD*)(this + base->m_hMyWeapons); }
	__forceinline float* m_flNextAttack() { return (float*)(this + base->m_flNextAttack); }

	__forceinline DWORD* m_hOwner() { return (DWORD*)(this + base->m_hOwner); }
	__forceinline int* m_iClip1() { return (int*)(this + base->m_iClip1); }
	__forceinline float* m_flNextPrimaryAttack() { return (float*)(this + base->m_flNextPrimaryAttack); }
	__forceinline float* m_flNextSecondaryAttack() { return (float*)(this + base->m_flNextSecondaryAttack); }

	__forceinline bool* m_bClientSideAnimation() { return (bool*)(this + base->m_bClientSideAnimation); }
	__forceinline float* m_flPoseParameter() { return (float*)(this + base->m_flPoseParameter); }
	__forceinline float* m_iv_flPoseParameter() { return (float*)(this + base->m_iv_flPoseParameter); }
	__forceinline float* m_flLastBoneSetupTime() { return (float*)(this + base->m_flLastBoneSetupTime); }
	__forceinline DWORD* m_iMostRecentModelBoneCounter() { return (DWORD*)(this + base->m_iMostRecentModelBoneCounter); }

	__forceinline bool* m_bPinPulled() { return (bool*)(this + base->m_bPinPulled); }
	__forceinline float* m_fThrowTime() { return (float*)(this + base->m_fThrowTime); }
	__forceinline float* m_flThrowStrength() { return (float*)(this + base->_m_flThrowStrength); }

	__forceinline WORD* m_iItemDefinitionIndex() { return (WORD*)(this + base->m_iItemDefinitionIndex); }

	__forceinline AnimLayer* m_AnimOverlay() { return *(AnimLayer**)(this + base->m_AnimOverlay); }

	__forceinline float* m_ModelAnimDistance() { return (float*)(this + base->m_ModelAnimDistance); }
	__forceinline int* m_ModelAnimFlags() { return (int*)(this + base->m_ModelAnimFlags); }
	__forceinline int* m_ModelAnimLastFrame() { return (int*)(this + base->m_ModelAnimLastFrame); }

	__forceinline int* m_fireXDelta() { return (int*)(this + base->m_fireXDelta); }
	__forceinline int* m_fireYDelta() { return (int*)(this + base->m_fireYDelta); }

	__forceinline int* m_bDidSmokeEffect() { return (int*)(this + base->m_bDidSmokeEffect); }

	__forceinline float* m_flC4Blow() { return (float*)(this + base->m_flC4Blow); }

	__forceinline int* m_iPing() { return (int*)(this + base->m_iPing); }
};

static bool IsEntityValid(Entity* entity)
{
	if (!entity)
		return false;

	if (!(entity + 0x4))
		return false;

	if (!(entity + 0x8))
		return false;

	return true;
}

struct TraceFilter
{
	TraceFilter()
	{
		noplayers = false;
	}
	virtual bool ShouldHitEntity(Entity* entity, int type)
	{
		char* name = entity->GetClientClass()->name;
		CRC32 crc = CRC32_Get(name, string_len(name));

		if (noplayers)
		{
			if (crc == 0x9a4ec3b8) // CCSPlayer
				return false;
		}

		if (nogrenades)
		{
			if (crc == 0xfd7e2c06) // CBaseCSGrenadeProjectile
				return false;

			if (crc == 0x5f48f653) // CDecoyProjectile
				return false;

			if (crc == 0x5b14e777) // CSmokeGrenadeProjectile
				return false;

			if (crc == 0xb4f1cddc) // CMolotovProjectile
				return false;
		}

		{
			if (CRC32_Get(name, 7) == 0x37d2b3e9) // CWeapon
				return false;

			if (crc == 0x8581de05) // CAK47
				return false;

			if (crc == 0xd6c7d366) // CDEagle
				return false;
		}

		return true;
	}
	virtual int GetTraceType() { return 0; }
	//DWORD* vtable;

	char pad[8];
	bool noplayers;
	bool nogrenades;
};
