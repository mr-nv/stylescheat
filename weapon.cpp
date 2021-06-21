#include "dll.h"

bool CCSWeapon::PreThink(Entity* local, UserCmd* cmd)
{
	if (gClient->mFrozen)
		return false;

	if (!IsHitscan())
		return false;

	if (*m_iClip1() < 1)
	{
		cmd->buttons &= ~(IN_ATTACK|IN_ATTACK2);
		return false;
	}

	if (!(gClient->mCurTime >= *local->m_flNextAttack()))
	{
		cmd->buttons &= ~(IN_ATTACK|IN_ATTACK2);
		return false;
	}

	return true;
}

bool CCSWeapon::KnifePreThink(Entity* local, UserCmd* cmd)
{
	if (gClient->mFrozen)
		return false;

	if (!IsKnife())
		return false;

	if (!(gClient->mCurTime >= *local->m_flNextAttack()))
	{
		cmd->buttons &= ~(IN_ATTACK|IN_ATTACK2);
		return false;
	}

	return true;
}

bool CCSWeapon::GrenadePreThink(Entity* local, UserCmd* cmd)
{
	if (gClient->mFrozen)
		return false;

	if (!IsGrenade())
		return false;

	if (!(gClient->mCurTime >= *local->m_flNextAttack()))
	{
		cmd->buttons &= ~(IN_ATTACK|IN_ATTACK2);
		return false;
	}

	return true;
}

void CCSWeapon::AccuracyPreThink(Entity* local, UserCmd* cmd)
{
	gClient->mSpreadAngles = Vector(0.0f, 0.0f, 0.0f);

	if (!gVars.aim_active.value)
		return;

	if (!gVars.aim_antispread.value)
		return;

	int seed = gVars.aim_antispread.value == 2 ? 62 : (MD5_PseudoRandom(cmd->index) & 0x7FFFFFFF);

	Vector spread = GetSpreadXY(seed & 255, (cmd->buttons & IN_ATTACK2) == IN_ATTACK2);
	Vector old_angles = cmd->angles;

	gClient->SolvePitchYaw(cmd, spread * -1.0f);
}

void CCSWeapon::AccuracyThink(Entity* local, UserCmd* cmd)
{
	if (!gVars.aim_active.value)
		return;

	if (gVars.aim_antispread.value)
		cmd->angles += gClient->mSpreadAngles;

	if (gVars.aim_norecoil.value)
	{
		Vector aim_punch = gClient->mAimPunch;

		ConVar* weapon_recoil_scale = base->FindVar(base->mEngineCvar, "weapon_recoil_scale");
		if (weapon_recoil_scale)
			aim_punch *= (float)(weapon_recoil_scale->GetValue()) / 2.0f;

		cmd->angles -= aim_punch * 2.0f;
	}
}

int CCSWeapon::GetPenetrationCount()
{
	int count = 3;

	switch (*m_iItemDefinitionIndex())
	{
		case WEAPON_GLOCK:
		case WEAPON_FIVESEVEN:
		case WEAPON_MP7:
		case WEAPON_MP9:
		case WEAPON_P90:
		case WEAPON_BIZON:
		case WEAPON_XM1014:
		case WEAPON_MAG7:
		case WEAPON_SAWEDOFF:
		{
			count = 1;
			break;
		}
		case WEAPON_HKP2000:
		case WEAPON_P250:
		case WEAPON_ELITE:
		case WEAPON_UMP45:
		{
			count = 2;
			break;
		}
		case WEAPON_DEAGLE:
		case WEAPON_SSG08:
		case WEAPON_AWP:
		case WEAPON_SCAR20:
		case WEAPON_G3SG1:
		{
			count = 4;
			break;
		}
		case WEAPON_NOVA:
		{
			count = 0;
			break;
		}
	}

	return count;
}

bool CCSWeapon::IsSniper()
{
	switch (*m_iItemDefinitionIndex())
	{
		case WEAPON_SSG08:
		case WEAPON_AWP:
		case WEAPON_SCAR20:
		case WEAPON_G3SG1:
				return true;
	}

	return false;
}

bool CCSWeapon::HasScope()
{
	switch (*m_iItemDefinitionIndex())
	{
		case WEAPON_SSG08:
		case WEAPON_AWP:
		case WEAPON_SCAR20:
		case WEAPON_G3SG1:
		case WEAPON_SG556:
		case WEAPON_AUG:
			return true;
	}

	return false;
}

int CCSWeapon::GetCurrentConfig()
{
	int index = *m_iItemDefinitionIndex();

	if (index == WEAPON_DEAGLE)
	{
		if (gVars.groups_active.value & GROUP_DEAGLE)
			return 3;
	}
	else if (index == WEAPON_REVOLVER)
	{
		if (gVars.groups_active.value & GROUP_REVOLVER)
			return 4;
	}
	else if (index == WEAPON_SSG08)
	{
		if (gVars.groups_active.value & GROUP_SCOUT)
			return 6;
	}
	else if (index == WEAPON_AWP)
	{
		if (gVars.groups_active.value & GROUP_AWP)
			return 7;
	}
	else if (index == WEAPON_TASER)
	{
		if (gVars.groups_active.value & GROUP_ZEUS)
			return 8;
	}

	switch (index)
	{
		case WEAPON_AK47:
		case WEAPON_M4A1:
		case WEAPON_M4A1_SILENCER:
		case WEAPON_GALILAR:
		case WEAPON_FAMAS:
		case WEAPON_SG556:
		case WEAPON_AUG:
		{
			if (gVars.groups_active.value & GROUP_RIFLES)
				return 1;

			break;
		}
		case WEAPON_HKP2000:
		case WEAPON_GLOCK:
		case WEAPON_USP_SILENCER:
		case WEAPON_P250:
		case WEAPON_FIVESEVEN:
		case WEAPON_DEAGLE:
		case WEAPON_TEC9:
		case WEAPON_ELITE:
		case WEAPON_REVOLVER:
		case WEAPON_CZ75A:
		{
			if (gVars.groups_active.value & GROUP_PISTOLS)
				return 2;

			break;
		}
		case WEAPON_G3SG1:
		case WEAPON_SCAR20:
		{
			if (gVars.groups_active.value & GROUP_AUTOS)
				return 5;

			break;
		}
	}

	return 0;
}

bool CCSWeapon::TraceBullet(Entity* player, Vector pos, float* damage, bool* hit, float* length, int max_walls)
{
	TraceResult tr, exit, pc;

	RayData ray;

	TraceFilter filter;
	filter.noplayers = true;

	CSWeaponInfo* info = (CSWeaponInfo*)(GetWeaponInfo());

	float dmg = (float)(info->GetDamage());
	float range = info->GetRange();
	float dist = 0.0f;
	int count = GetPenetrationCount();
	int pen = count;

	Vector start = gClient->mViewOrigin;

	Vector ray_delta = pos - start;
	Vector dir = ray_delta.Normal();

	float pos_dist = ray_delta.LengthSqr();

	while (true)
	{
		range = (float)(info->GetRange()) - dist;

		ray.base = start;
		ray.delta = (dir * range);

		base->TraceRay(base->mEngineTrace, &ray, 0x4600400B, &filter, &tr);

		float add = tr.fraction * range;
		float sq = (dist + add);
		sq *= sq;

		if (sq >= pos_dist)
		{
			Vector delta = pos - start;
			Vector test;

			dist += sse_sqrtfast(delta.LengthSqr());

			if (*m_iItemDefinitionIndex() == WEAPON_TASER)
			{
				if (gVars.aim_autozeus.value)
					dist += 4.0f;
			}

			if (dist > info->GetRange())
				return false;

			dmg *= float_pow(info->GetRangeModifier(), dist / 500.0f);
			if (dmg < 1.0f)
				return false; // just safety check

			if (damage)
				*damage = dmg;

			if (length)
				*length = dist;

			return true;
		}

		if (!gVars.aim_autowall.value || *m_iItemDefinitionIndex() == WEAPON_TASER)
			return false;

		if ((count - pen) >= max_walls)
			return false;

		dist += add;
		dmg *= float_pow(info->GetRangeModifier(), dist / 500.0f);

		if (dist > 3000.0f || pen == 0)
			return false;

		// iterate to unsolid point

		float dist = 0.0f;
		bool exited = false;
		bool tr_flags = ((tr.surfaceflags >> 7) & 1) ? 1 : 0;

		while (dist <= 90.0f)
		{
			dist += 4.0f;
			Vector point = tr.hitpos + (dir * dist);

			int co = base->GetPointContents(base->mEngineTrace, &point, 0x4600400B, nullptr);

			if (co != 0)
				continue;

			ray.Init(point, (point - (dir * 4.0f)));

			base->TraceRay(base->mEngineTrace, &ray, 0x4600400B, &filter, &exit);

			if ((exit.surfaceflags >> 7) & 1 && !tr_flags)
				continue;

			if (exit.fraction == 1.0f || exit.startsolid)
			{
				if (tr.ent)
				{
					char* name = tr.ent->GetClientClass()->name;
					DWORD crc = CRC32_Get(name, string_len(name));

					if (crc == 0xca6864a6) // CBreakableSurface
					{
						exited = true;
						break;
					}
				}

				continue;
			}

			exited = true;
			exit.hitpos = point - (dir * (exit.fraction * 4.0f));

			break;
		}

		if (!exited)
			return false;

		void* trace_data = base->GetSurfaceData(base->mVPhysics, tr.props);
		void* exit_data = base->GetSurfaceData(base->mVPhysics, exit.props);

		short in_mat = *(short*)((DWORD)(trace_data) + 92);
		short out_mat = *(short*)((DWORD)(exit_data) + 92);
		float in_mod = *(float*)((DWORD)(trace_data) + 88);
		float out_mod = *(float*)((DWORD)(exit_data) + 88);

		if (in_mod < 0.1f)
			return false;

		// calculate damage from how many units we penetrated
		float len = (exit.hitpos - tr.hitpos).LengthSqr();

		bool solid = !(tr.contents & CONTENTS_DEBRIS);

		if (tr.ent)
		{
			char* name = tr.ent->GetClientClass()->name;
			DWORD crc = CRC32_Get(name, string_len(name));

			if (crc == 0xca6864a6) // CBreakableSurface
				solid = false;
		}

		if (gVars.aim_grate.value)
		{
			if (!(tr.contents & CONTENTS_LADDER) && !(exit.contents & CONTENTS_LADDER))
			{
				if ((tr.contents & CONTENTS_GRATE) && (exit.contents & CONTENTS_GRATE))
					solid = false;
			}
		}

		if (solid)
		{
			if (hit)
				*hit = true;

			float total_mod = (in_mod + out_mod) * 0.5f;
			float total_mult = 0.16f;

			if (tr.contents & CONTENTS_GRATE || in_mat == 'G')
				total_mult = 0.05f;

			if (in_mat == out_mat)
			{
				if (out_mat == 'W' || out_mat == 'U')
					total_mod = 3.0f;
				else if (out_mat == 'L')
					total_mod = 2.0f;
			}

			float mod = 1.0f / total_mod;
			float extra = (dmg * total_mult) + mod * 3.0f * ((3.0f / info->GetPenetration()) * 1.25f);

			len *= mod;
			len /= 24.0f;

			float lost = max(0.0f, len + extra);
			
			if (lost > dmg)
			{
				dmg = 0.0f;
				return false;
			}

			if (lost > 0.0f)
				dmg -= lost;

			if (dmg < 1.0f)
				return false;
		}

		start = exit.hitpos;

		--pen;
	}

	return false;
}

void CCSWeapon::TraceBulletImpacts(float* damage, Vector* hits, bool* thru, int* hitindex, int* walls)
{
	TraceResult tr, exit;

	RayData ray;

	TraceFilter filter;
	filter.noplayers = true;

	CSWeaponInfo* info = (CSWeaponInfo*)(GetWeaponInfo());

	float dmg = (float)(info->GetDamage());
	float range = info->GetRange();
	float dist = 0.0f;
	int pen = GetPenetrationCount();

	Vector start = gClient->mViewOrigin;
	Vector pos = start + (gClient->mViewAngles.Forward() * range);

	Vector ray_delta = pos - start;
	Vector dir = ray_delta.Normal();

	float pos_dist = ray_delta.Length();

	while (true)
	{
		if (damage)
			*damage = dmg;

		range = (float)(info->GetRange()) - dist;

		ray.base = start;
		ray.delta = (dir * range);

		base->TraceRay(base->mEngineTrace, &ray, 0x4600400B, &filter, &tr);

		if (hits && hitindex)
		{
			hits[*hitindex] = tr.hitpos;
			thru[*hitindex] = false;
			*hitindex += 1;
		}

		float add = tr.fraction * range;

		if ((dist + add) > info->GetRange())
			return;

		dist += add;
		dmg *= float_pow(info->GetRangeModifier(), dist / 500.0f);

		if (dist > 3000.0f || pen == 0)
			return;

		// iterate to unsolid point

		float dist = 0.0f;
		bool exited = false;

		while (dist <= 90.0f)
		{
			dist += 4.0f;
			Vector point = tr.hitpos + (dir * dist);

			int co = base->GetPointContents(base->mEngineTrace, &point, 0x4600400B, nullptr);

			if (co != 0)
				continue;

			ray.Init(point, (point - (dir * 4.0f)));

			base->TraceRay(base->mEngineTrace, &ray, 0x4600400B, &filter, &exit);

			if (exit.startsolid)
				continue;

			if (exit.fraction == 1.0f && !exit.allsolid && !exit.startsolid)
				continue;

			if (!(exit.contents & CONTENTS_WINDOW))
			{
				if (exit.props == 0)
					continue;
			}

			exited = true;

			break;
		}

		if (!exited)
			return;

		void* trace_data = base->GetSurfaceData(base->mVPhysics, tr.props);
		void* exit_data = base->GetSurfaceData(base->mVPhysics, exit.props);

		short in_mat = *(short*)((DWORD)(trace_data) + 92);
		short out_mat = *(short*)((DWORD)(exit_data) + 92);
		float in_mod = *(float*)((DWORD)(trace_data) + 88);
		float out_mod = *(float*)((DWORD)(exit_data) + 88);

		if (in_mod < 0.1f)
			return;

		// calculate damage from how many units we penetrated

		float len = (exit.hitpos - tr.hitpos).Length();

		bool solid = !(tr.contents & CONTENTS_DEBRIS);

		if (tr.ent)
		{
			char* name = tr.ent->GetClientClass()->name;
			DWORD crc = CRC32_Get(name, string_len(name));

			if (crc == 0xca6864a6) // CBreakableSurface
				solid = false;
		}

		if (gVars.aim_grate.value)
		{
			if (!(tr.contents & CONTENTS_LADDER) && !(exit.contents & CONTENTS_LADDER))
			{
				if ((tr.contents & CONTENTS_GRATE) && (exit.contents & CONTENTS_GRATE))
					solid = false;
			}
		}

		if (solid)
		{
			float total_mod = (in_mod + out_mod) * 0.5f;
			float total_mult = 0.16f;

			if (tr.contents & CONTENTS_GRATE || in_mat == 'G')
				total_mult = 0.05f;

			if (in_mat == out_mat)
			{
				if (out_mat == 'W' || out_mat == 'U')
					total_mod = 3.0f;
				else if (out_mat == 'L')
					total_mod = 2.0f;
			}

			float mod = 1.0f / total_mod;
			float extra = (dmg * total_mult) + mod * 3.0f * ((3.0f / info->GetPenetration()) * 1.25f);
			
			len *= len;
			len *= mod;
			len /= 24.0f;

			float lost = max(0, len + extra);

			if (lost > dmg)
			{
				dmg = 0.0f;
				return;
			}

			if (lost > 0.0f)
				dmg -= lost;

			if (dmg < 1.0f)
				return;
		}

		start = exit.hitpos;

		if (hits && hitindex && *hitindex > 0)
		{
			thru[*hitindex - 1] = true;
			hits[*hitindex] = start;
			thru[*hitindex] = true;
			*hitindex += 1;
		}

		*walls += 1;
		--pen;
	}
}

Vector CCSWeapon::GetSpreadXY(int seed, bool secondary)
{
	base->RandomSeed(seed + 1);

	float iacr = base->RandomFloat(0.0f, 1.0f);
	float rad1 = base->RandomFloat(0.0f, MATH_PI * 2.0f);
	float cone = base->RandomFloat(0.0f, 1.0f);
	float rad2 = base->RandomFloat(0.0f, MATH_PI * 2.0f);

	if (*m_iItemDefinitionIndex() == WEAPON_REVOLVER && secondary)
	{
		iacr = 1.0f - (iacr * iacr);
		cone = 1.0f - (cone * cone);
	}

	iacr *= gClient->mInaccuracy;
	cone *= gClient->mSpread;

	float x = 0.0f;
	float y = 0.0f;

	{
		float sin = 0.0f;
		float cos = 0.0f;

		float_sincos(rad1, &sin, &cos);

		x += cos * iacr;
		y += sin * iacr;
	}

	{
		float sin = 0.0f;
		float cos = 0.0f;

		float_sincos(rad2, &sin, &cos);

		x += cos * cone;
		y += sin * cone;
	}

	return Vector(x, y, 0.0f);
}

char* CCSWeapon::GetPrintName()
{
	int i = *m_iItemDefinitionIndex();

	switch (i)
	{
		case WEAPON_DEAGLE:
			return "deagle";
		case WEAPON_ELITE:
			return "elites";
		case WEAPON_FIVESEVEN:
			return "fiveseven";
		case WEAPON_GLOCK:
			return "glock";
		case WEAPON_AK47:
			return "ak47";
		case WEAPON_AUG:
			return "aug";
		case WEAPON_AWP:
			return "awp";
		case WEAPON_FAMAS:
			return "famas";
		case WEAPON_G3SG1:
			return "g3sg1";
		case WEAPON_GALILAR:
			return "galil";
		case WEAPON_M249:
			return "m249";
		case WEAPON_M4A1:
			return "m4a1";
		case WEAPON_MAC10:
			return "mac10";
		case WEAPON_P90:
			return "p90";
		case WEAPON_UMP45:
			return "ump45";
		case WEAPON_XM1014:
			return "xm1014";
		case WEAPON_BIZON:
			return "bizon";
		case WEAPON_MAG7:
			return "mag7";
		case WEAPON_NEGEV:
			return "negev";
		case WEAPON_SAWEDOFF:
			return "sawed off";
		case WEAPON_TEC9:
			return "tec9";
		case WEAPON_TASER:
			return "zeus";
		case WEAPON_HKP2000:
			return "p2000";
		case WEAPON_MP7:
			return "mp7";
		case WEAPON_MP9:
			return "mp9";
		case WEAPON_NOVA:
			return "nova";
		case WEAPON_P250:
			return "p250";
		case WEAPON_SCAR20:
			return "scar 20";
		case WEAPON_SG556:
			return "sg 556";
		case WEAPON_SSG08:
			return "scout";
		case WEAPON_KNIFE:
		case WEAPON_KNIFE_T:
			return "knife";
		case WEAPON_FLASHBANG:
			return "flashbang";
		case WEAPON_HEGRENADE:
			return "grenade";
		case WEAPON_SMOKEGRENADE:
			return "smoke";
		case WEAPON_MOLOTOV:
			return "molotov";
		case WEAPON_INCGRENADE:
			return "incendiary";
		case WEAPON_DECOY:
			return "decoy";
		case WEAPON_C4:
			return "c4";
		case WEAPON_M4A1_SILENCER:
			return "m4a1-s";
		case WEAPON_USP_SILENCER:
			return "usp";
		case WEAPON_CZ75A:
			return "cz75";
		case WEAPON_REVOLVER:
			return "revolver";
	}

	return "knife";
}

bool CCSWeapon::IsSemiAuto()
{
	int i = *m_iItemDefinitionIndex();

	switch (i)
	{
		case WEAPON_HKP2000:
		case WEAPON_GLOCK:
		case WEAPON_USP_SILENCER:
		case WEAPON_P250:
		case WEAPON_FIVESEVEN:
		case WEAPON_DEAGLE:
		case WEAPON_TEC9:
		case WEAPON_ELITE:
		case WEAPON_SSG08:
		case WEAPON_AWP:
		case WEAPON_SAWEDOFF:
		case WEAPON_TASER:
		case WEAPON_REVOLVER:
			return true;
	}

	return false;
}