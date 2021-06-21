#define BASE_OFFSET 0xEC

struct CSWeaponInfo
{
	float GetMaxSpeed()
	{
		return *(float*)(this + 0x12C);
	}
	int GetDamage()
	{
		return *(int*)(this + BASE_OFFSET);
	}
	float GetPenetration()
	{
		return *(float*)(this + BASE_OFFSET + 0xC);
	}
	float GetRange()
	{
		return *(float*)(this + BASE_OFFSET + 0x18);
	}
	float GetRangeModifier()
	{
		return *(float*)(this + BASE_OFFSET + 0x1C);
	}
	float GetArmorRatio()
	{
		return *(float*)(this + BASE_OFFSET + 0x4);
	}
	float GetCycleTime()
	{
		return *(float*)(this + BASE_OFFSET - 0x14);
	}
};

class CCSWeapon : public Entity
{
public:
	__forceinline float GetInaccuracy()
	{
		typedef float (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(this))[469]))(this);
	}
	__forceinline float GetSpread()
	{
		typedef float (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(this))[439]))(this);
	}
	__forceinline void UpdateAccuracyPenalty()
	{
		typedef void (__thiscall* dfn)(void* thisptr);
		((dfn)((*(DWORD**)(this))[470]))(this);
	}
	__forceinline CSWeaponInfo* GetWeaponInfo()
	{
		typedef CSWeaponInfo* (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(this))[446]))(this); // 464 457
	}

	bool PreThink(Entity* local, UserCmd* cmd);
	bool KnifePreThink(Entity* local, UserCmd* cmd);
	bool GrenadePreThink(Entity* local, UserCmd* cmd);
	void AccuracyPreThink(Entity* local, UserCmd* cmd);
	void AccuracyThink(Entity* local, UserCmd* cmd);
	bool TraceBullet(Entity* player, Vector pos, float* damage, bool* hit, float* length, int max_walls);
	void TraceBulletImpacts(float* damage, Vector* hits, bool* thru, int* hitindex, int* walls);
	Vector GetSpreadXY(int seed, bool secondary);
	char* GetPrintName();
	bool IsSemiAuto();
	int GetPenetrationCount();
	bool IsSniper();
	bool HasScope();
	int GetCurrentConfig();

	inline bool IsHitscan()
	{
		return *m_iClip1() != -1;
	}
	inline bool IsKnife()
	{
		int idx = *m_iItemDefinitionIndex();

		if (idx == WEAPON_KNIFE || idx == WEAPON_KNIFE_T || idx == WEAPON_GOLD_KNIFE)
			return true;

		if (idx > WEAPON_REVOLVER)
			return true;

		return false;
	}
	inline bool IsGrenade()
	{
		if (IsHitscan())
			return false;

		if (IsKnife())
			return false;

		return true;
	}
};