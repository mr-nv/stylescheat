#include "dll.h"

CLegitAim gLegit;

bool CLegitAim::Insight(Entity* player, Vector pos)
{
	RayData ray;
	ray.Init(gClient->mViewOrigin, pos);

	TraceResult tr;
	TraceFilter filter;
	filter.noplayers = true;
	filter.nogrenades = true;

	base->TraceRay(base->mEngineTrace, &ray, 0x4600400B, &filter, &tr);

	return tr.fraction == 1.0f;
}

bool CLegitAim::IsValidTarget(Entity* local, Entity* player)
{
	if (player == local)
		return false;

	if (!gVars.l_teamaim.value)
	{
		if (*player->m_iTeamNum() == *local->m_iTeamNum())
			return false;
	}

	if (*player->m_lifeState() != 0)
		return false;

	if (player->IsDormant())
		return false;

	if (player->m_clrRender()[3] != 255)
		return false;

	return true;
}

bool CLegitAim::FindFrameIntersection(Entity* player, Vector forward, int* tick, Matrix* matrix)
{
	int index = player->GetIndex();

	CAimPlayer* aim_player = &gAim->mAimPlayers[index];

	int limit = (int)((0.2f / base->mGlobalVars->interval) + 0.5f) - gAim->mInterpTicks - 1;
	limit = (int)(((float)(limit) * ((float)(gClient->Cfg(gVars.l_historyaim_time)) / 100.0f)) + 0.5f);

	Vector origin = gClient->mViewOrigin;
	bool found = false;
	int target_hitbox = gClient->mUsingAWP ? -1 : HITBOX_HEAD;
	Matrix mtx[HITBOX_MAX];
	Vector test;

	float best = -9999.0f;

	int diff = 0;

	if (gClient->Cfg(gVars.l_historyaim_pred))
	{
		CAimbotFrame* target_frame = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];

		int ticks = (int)((target_frame->mSimulationTime[index] / base->mGlobalVars->interval) + 0.5f);
		ticks += gAim->mInterpTicks;

		int delta = (ticks - gClient->mTickCount);

		diff = min(USERCMD_DELTA_MAX - delta, gAim->mRealNetLag - 1) * -1;
		if (diff > 0)
			diff = 0;
	}

	for (int i = diff; i < MAX_FRAMES; ++i)
	{
		if (!gClient->Cfg(gVars.l_historyaim))
		{
			if (i < 0)
				continue;
			else if (i > 0)
				break;
		}

		int target_time = gAim->mFrameCount;

		if (i > 0)
			target_time -= i;

		if (target_time < 0)
			break;

		CAimbotFrame* target_frame = &gAim->mFrames[target_time % MAX_FRAMES];

		int ticks = (int)((target_frame->mSimulationTime[index] / base->mGlobalVars->interval) + 0.5f);
		ticks += gAim->mInterpTicks;

		int delta = (ticks - gClient->mTickCount);

		if (delta > USERCMD_DELTA_MAX)
			continue;

		if (delta < (limit * -1))
			break;

		memcpy(mtx, target_frame->mBones[index], sizeof(Matrix) * HITBOX_MAX);

		if (i < 0)
		{
			for (int n = 0; n < HITBOX_MAX; ++n)
			{
				Vector pos = mtx[n].GetColumn(3);
				pos += (aim_player->mVelocity * base->mGlobalVars->interval) * (i * -1);

				mtx[n].SetColumn(pos, 3);
			}
		}

		int result = gAim->PerformIntersection(player, mtx, origin, forward, &test, (float)(gClient->Cfg(gVars.l_hitbox_scale)) * 0.01f);
		if (result == -1)
			continue;

		HitBox* bb = gAim->GetBaseHitbox(player) + result;

		Vector dir = (mtx[HITBOX_HEAD].Transform((bb->min + bb->max) * 0.5f) - origin).Normal();

		float dot = (test - origin).Normal().Dot(dir);

		if (target_hitbox != -1 && result != target_hitbox)
			dot = -1.0f;

		if (i < 0)
			dot = -1.0f;

		if (dot > best)
		{
			*tick = ticks + ((i < 0) ? (i * -1) : 0);
			memcpy(matrix, mtx, sizeof(Matrix) * HITBOX_MAX);

			best = dot;
			found = true;
		}
	}

	if (found)
		return true;

	return false;
}

bool CLegitAim::GetRefinePosition(Entity* player, Vector* aimpos, Vector forward)
{
	int index = player->GetIndex();

	CAimbotFrame* target_frame = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];

	Matrix matrix[HITBOX_MAX];
	memcpy(matrix, target_frame->mBones[index], sizeof(Matrix) * HITBOX_MAX);

	HitBox* box = gAim->GetBaseHitbox(player);
	if (!box)
		return false;

	bool final_result = false;

	for (int i = 0; i < HITBOX_MAX; ++i)
	{
		bool found = false;

		for (float f = 0.1f; f <= 0.95f; f += (f == 0.9f ? 0.05f : 0.1f))
		{
			if (f > ((float)(gClient->Cfg(gVars.l_refineaim)) * 0.1f))
				break;

			HitBox* bb = box + i;

			int index = 6;

			for (int n = 0; n < (index * 2); ++n)
			{
				Vector base = (n < index) ? bb->min : bb->max;

				float r = bb->radius * f;

				Vector offset = sphere_offsets[n % index];

				Vector pos = matrix[i].Transform(base + (offset * r));

				Vector dir = (pos - gClient->mViewOrigin);
				float len = dir.Length();

				if (len != 0.0f)
				{
					dir /= len;

					if (dir.Dot(forward) < mRefineFOV)
						continue;
				}

				found = true;

				*aimpos = pos;

				if (!(i == HITBOX_HEAD && n < index))
					break;
			}

			if (found)
				break;
		}

		if (found)
		{
			if (Insight(player, *aimpos))
			{
				final_result = true;

				if (i == HITBOX_HEAD)
					break;
			}
		}
	}

	return final_result;
}

void CLegitAim::HistoryAim(Entity* local, UserCmd* cmd)
{
	Entity* best = nullptr;
	int tick = 0;
	Matrix matrix[HITBOX_MAX];

	Vector angles = cmd->angles + (gClient->mAimPunch * 2.0f);
	Vector forward = angles.Forward();

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* pl = base->GetClientEntity(base->mEntList, i);
		if (!pl)
			continue;

		if (pl == local)
			continue;

		if (!IsValidTarget(local, pl))
			continue;

		if (!FindFrameIntersection(pl, forward, &tick, matrix))
			continue;

		best = pl;

		break;
	}

	if (best)
	{
		cmd->ticks = tick;

		mAccuratePlayer = best;
		memcpy(mTargetMatrix, matrix, sizeof(Matrix) * HITBOX_MAX);
	}
}

void CLegitAim::RefineAim(Entity* local, UserCmd* cmd)
{
	if (!gClient->Cfg(gVars.l_refineaim))
		return;

	if (*local->m_bIsScoped())
		mRefineFOV = float_cos(DEG2RAD(0.25f));
	else
		mRefineFOV = float_cos(DEG2RAD(0.55f));

	Entity* best = nullptr;
	Vector pos;

	Vector forward = cmd->angles.Forward();
	float dist = -1.0f;

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* pl = base->GetClientEntity(base->mEntList, i);
		if (!pl)
			continue;

		if (pl == local)
			continue;

		if (!IsValidTarget(local, pl))
			continue;

		CAimbotFrame* target_frame = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];

		Matrix matrix[HITBOX_MAX];
		memcpy(matrix, target_frame->mBones[i], sizeof(Matrix) * HITBOX_MAX);

		HitBox* box = gAim->GetBaseHitbox(pl);
		if (!box)
			continue;

		pos = matrix[HITBOX_HEAD].Transform((box->min + box->max) * 0.5f);

		if (!gClient->Cfg(gVars.l_accuracy_smoke))
		{
			if (IntersectWithSmoke(pos))
				continue;
		}

		float len = (pos - gClient->mViewOrigin).Normal().Dot(forward);

		if (pl == mLastTarget)
			len = 9999.0f;

		if (len < dist)
			continue;

		dist = len;

		best = pl;
	}

	if (best)
	{
		if (GetRefinePosition(best, &pos, forward))
			cmd->angles = (pos - gClient->mViewOrigin).Angles();
	}
}

void CLegitAim::AccuracyThink(Entity* local, UserCmd* cmd)
{
	mAccuratePlayer = nullptr;

	if (!gClient->Cfg(gVars.l_accuracy))
		return;

	RefineAim(local, cmd);
	HistoryAim(local, cmd);
}

bool CLegitAim::GetAimPosition(Entity* player, Vector* aimpos, Vector forward)
{
	int index = player->GetIndex();

	CAimbotFrame* target_frame = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];

	Matrix matrix[HITBOX_MAX];
	memcpy(matrix, target_frame->mBones[index], sizeof(Matrix) * HITBOX_MAX);

	HitBox* box = gAim->GetBaseHitbox(player);
	if (!box)
		return false;

	CAimPlayer* aim_player = &gAim->mAimPlayers[index];

	int hitbox = 0;
	int aimbox = gClient->Cfg(gVars.l_aim_hitbox);

	if (aimbox == 0)
		hitbox = HITBOX_HEAD;
	else if (aimbox == 1)
		hitbox = HITBOX_NECK;
	else if (aimbox == 2)
		hitbox = HITBOX_STOMACH;
	else if (aimbox == 3)
		hitbox = HITBOX_CHEST;
	
	bool aim = false;

	for (int i = 0; i < HITBOX_MAX; ++i)
	{
		if (!gClient->Cfg(gVars.l_aim_hitscan))
		{
			if (i != hitbox)
				continue;
		}

#ifdef IGNORE_RIGHT_ARM
		if (i == HITBOX_RIGHT_UPPER_ARM || i == HITBOX_RIGHT_FOREARM || i == HITBOX_RIGHT_HAND)
			continue;
#endif

		HitBox* bb = box + i;

		Vector min = matrix[i].Transform(bb->min);
		Vector max = matrix[i].Transform(bb->max);

		Vector pos = (min + max) * 0.5f;

		if (!Insight(player, pos))
			continue;

		*aimpos = pos;
		aim = true;

		if (i == hitbox)
			break;
	}

	if (aim)
		return true;

	return false;
}

void CLegitAim::FixAngles(Vector* angles)
{
	float sens = 5.0f;

	ConVar* s = base->FindVar(base->mEngineCvar, "sensitivity");
	if (s)
		sens = s->GetFloat();

	float factor = (sens * 0.022f);

	Vector fixangles;
	angles->x = (float)((int)((angles->x / factor) + 0.5f) * factor);
	angles->y = (float)((int)((angles->y / factor) + 0.5f) * factor);
	angles->z = 0.0f;
}

void CLegitAim::NormalizeAngles(Vector* angles)
{
	if (angles->x < -180.0f)
		angles->x += 360.0f;

	if (angles->x > 180.0f)
		angles->x -= 360.0f;

	if (angles->y < -180.0f)
		angles->y += 360.0f;

	if (angles->y > 180.0f)
		angles->y -= 360.0f;

	angles->z = 0.0f;
}

bool CLegitAim::AimbotThink(Entity* local, UserCmd* cmd)
{
	if (!gClient->Cfg(gVars.l_aim))
		return false;

	if (gClient->Cfg(gVars.l_aim_mode) == 3)
	{
		if (!(gClient->mButtons & IN_SPEED))
			return false;
	}
	else if (gClient->Cfg(gVars.l_aim_mode) == 2)
	{
		if (!(gClient->mButtons & IN_ATTACK))
			return false;
	}
	else if (gClient->Cfg(gVars.l_aim_mode) == 1)
	{
		if (!mAimKey)
			return false;
	}

	bool aiming = true;

	if (gClient->Cfg(gVars.l_aim_time))
	{
		if ((gClient->mCurTime - mAimTime) > ((float)(gClient->Cfg(gVars.l_aim_time)) * 0.01f))
			aiming = false;
	}

	Entity* best = nullptr;
	Vector pos, aimpos;

	float dist = -1.0f;

	Vector cmdangles = cmd->angles;
	if (gClient->Cfg(gVars.l_rcs_active))
		cmdangles += gClient->mAimPunch * 2.0f;

	Vector forward = cmdangles.Forward();

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* pl = base->GetClientEntity(base->mEntList, i);
		if (!pl)
			continue;

		if (pl == local)
			continue;

		if (!IsValidTarget(local, pl))
			continue;

		if (!GetAimPosition(pl, &pos, forward))
			continue;

		if (!gClient->Cfg(gVars.l_accuracy_smoke))
		{
			if (IntersectWithSmoke(pos))
				continue;
		}

		{
			float fov = (float)(gClient->Cfg(gVars.l_aim_fov)) * 0.1f;
			float stop_fov = (float)(gClient->Cfg(gVars.l_aim_stop_fov)) * 0.1f;

			float delta = forward.Dot((pos - gClient->mViewOrigin).Normal());

			if (fov != 0.0f && delta < float_cos(DEG2RAD(fov)))
				continue;

			if (stop_fov != 0.0f && delta > float_cos(DEG2RAD(stop_fov)))
				continue;
		}

		float len = (pos - gClient->mViewOrigin).Normal().Dot(forward);

		if (pl == mLastTarget)
			len = 9999.0f;

		if (len < dist)
			continue;

		dist = len;

		best = pl;
		aimpos = pos;
	}

	float max_speed = 0.0f;
	float sine = 1.0f;

	{
		float smooth = (float)(gClient->Cfg(gVars.l_aim_smooth)) * 0.1f;

		if (gClient->Cfg(gVars.l_aim_sine))
		{
			float sine_length = gClient->Cfg(gVars.l_aim_sine) * 0.01f;

			// 5.0f = 200 MS
			float angle = float_fmod(5.0f * (gClient->mCurTime / base->mGlobalVars->interval), 360.0f);
			sine = (1.5f - (0.5f * (1.0f - sine_length))) - (float_abs(float_sin(DEG2RAD(angle))) * sine_length);
		}

		max_speed = (1.0f / smooth) * sine;
		if (max_speed < 0.001f)
			max_speed = 0.001f;
	}

	if (best && aiming)
	{
		if (gClient->Cfg(gVars.l_aim_single))
		{
			if (mPrimaryTarget && best != mPrimaryTarget)
				return true;
		}

		if (!mPrimaryTarget)
			mPrimaryTarget = best;

		mLastTarget = best;

		Vector aimangles = (aimpos - gClient->mViewOrigin).Angles();

		if (gClient->Cfg(gVars.l_rcs_active))
			aimangles -= gClient->mAimPunch * 2.0f;

		Vector viewangles = mTargetViewangles;

		Vector dt = (aimangles - viewangles);
		NormalizeAngles(&dt);

		{
			Vector dt2 = (aimangles - viewangles);
			NormalizeAngles(&dt2);

			aimangles = viewangles + (dt2 * max_speed);
		}

		if (true)
		{
			FixAngles(&aimangles);

			cmd->angles = aimangles;
			mTargetViewangles = cmd->angles;
		}

		return true;
	}
	else
	{
		Vector aimangles = gClient->mViewAngles;
		Vector viewangles = mTargetViewangles;

		{
			Vector dt2 = (aimangles - viewangles);
			NormalizeAngles(&dt2);

			aimangles = viewangles + (dt2 * max_speed);
		}

		if (true)
		{
			FixAngles(&aimangles);

			cmd->angles = aimangles;
			mTargetViewangles = cmd->angles;
		}
	}

	return true;
}

void CLegitAim::RcsSmoothBack(UserCmd* cmd)
{
	if (mPunchAngles.LengthSqr() > 0.0f)
	{
		cmd->angles -= mPunchAngles;
		mTargetViewangles = cmd->angles;
		base->SetViewAngles(base->mBaseEngine, &cmd->angles);
	}

	mPunchAngles = Vector(0.0f, 0.0f, 0.0f);
	mPunchAnglesVel = Vector(0.0f, 0.0f, 0.0f);
}

bool CLegitAim::FindTriggerIntersection(Entity* player, Vector forward, int* tick)
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));

	int index = player->GetIndex();
	CAimPlayer* aim_player = &gAim->mAimPlayers[index];

	CAimbotFrame* target_frame = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];

	Matrix matrix[HITBOX_MAX];

	if (!gClient->Cfg(gVars.l_historyaim_trigger) || mAccuratePlayer != player)
	{
		for (int i = 0; i < MAX_FRAMES; ++i)
		{
			int target_time = gAim->mFrameCount;

			if (i > 0)
				target_time -= i;

			if (target_time < 0)
				break;

			CAimbotFrame* target_frame = &gAim->mFrames[target_time % MAX_FRAMES];

			int ticks = (int)((target_frame->mSimulationTime[index] / base->mGlobalVars->interval) + 0.5f);
			ticks += gAim->mInterpTicks;

			int delta = (ticks - gClient->mTickCount);

			if (delta > USERCMD_DELTA_MAX)
				continue;

			memcpy(matrix, target_frame->mBones[index], sizeof(Matrix) * HITBOX_MAX);
			*tick = ticks;

			break;
		}
	}
	else
	{
		memcpy(matrix, mTargetMatrix, sizeof(Matrix) * HITBOX_MAX);
		*tick = -1;
	}

	Vector test;
	int result = gAim->PerformIntersection(player, matrix, gClient->mViewOrigin, forward, &test, (float)(gVars.l_hitbox_scale.value) * 0.01f);
	if (result == -1)
		return false;

	if (*weapon->m_iItemDefinitionIndex() == WEAPON_TASER)
	{
		CSWeaponInfo* info = weapon->GetWeaponInfo();

		float dist = (test - gClient->mViewOrigin).Length();
		dist += 4.0f;

		float dmg = info->GetDamage();
		dmg *= float_pow(info->GetRangeModifier(), dist / 500.0f);

		if (dmg < 100.0f)
			return false;
	}

	if (!Insight(player, test))
		return false;

	for (int i = 0; i < HITBOX_MAX; ++i)
	{
		if (gClient->Cfg(gVars.l_trig_hitbox) == 0)
		{
			if (i != HITBOX_HEAD)
				break;
		}

#ifdef IGNORE_RIGHT_ARM
		if (i == HITBOX_RIGHT_UPPER_ARM || i == HITBOX_RIGHT_FOREARM || i == HITBOX_RIGHT_HAND)
			continue;
#endif

		if (result == i)
			return true;
	}

	return false;
}

bool CLegitAim::TriggerThink(Entity* local, UserCmd* cmd)
{
	if (!gClient->Cfg(gVars.l_trig))
		return false;

	{
		Vector fwd = gClient->mViewAngles.Forward();

		for (int i = 1; i <= base->mGlobalVars->clients; ++i)
		{
			Entity* pl = base->GetClientEntity(base->mEntList, i);
			if (!pl)
				continue;

			if (pl == local)
				continue;

			if (*pl->m_lifeState() != 0)
				continue;

			if (pl->IsDormant())
				continue;

			Vector center = *pl->m_vecOrigin() + Vector(0.0f, 0.0f, pl->m_vecMaxs()->z * 0.5f);
			float dot = (center - gClient->mViewOrigin).Dot(fwd);
			if (dot > 0.0f)
				continue;

			mPlayersInFovTime[i] = gClient->mCurTime;
		}
	}

	float attack_delay = ((float)(gClient->Cfg(gVars.l_trig_attack_delay)) * 0.01f);
	float delay = ((float)(gClient->Cfg(gVars.l_trig_delay)) * 0.01f);
	float burst = ((float)(gClient->Cfg(gVars.l_trig_burst)) * 0.01f);

	if (gClient->Cfg(gVars.l_trig_delay_random))
		delay -= base->RandomFloat(0.0f, delay) * ((float)(gClient->Cfg(gVars.l_trig_delay_random)) * 0.01f);

	if (gClient->Cfg(gVars.l_trig_burst_random))
		burst -= base->RandomFloat(0.0f, burst) * ((float)(gClient->Cfg(gVars.l_trig_burst_random)) * 0.01f);

	mDelayToAdd = delay;

	if (gClient->Cfg(gVars.l_trig_burst))
	{
		if ((gClient->mCurTime - mLastTriggerTime) < 0.0f)
			cmd->buttons |= IN_ATTACK;
	}

	if (gClient->Cfg(gVars.l_trig_mode) == 3)
	{
		if (!(gClient->mButtons & IN_SPEED))
			return false;
	}
	else if (gClient->Cfg(gVars.l_trig_mode) == 2)
	{
		if (!(gClient->mButtons & IN_ATTACK))
		{
			mLastAttackTime = gClient->mCurTime;
			return false;
		}

		if ((gClient->mCurTime - mLastAttackTime) < attack_delay)
			cmd->buttons &= ~IN_ATTACK;
	}
	else if (gClient->Cfg(gVars.l_trig_mode) == 1)
	{
		if (!mTriggerKey)
			return false;
	}

	Entity* best = nullptr;
	int tick = -1;

	Vector forward = (cmd->angles + (gClient->mAimPunch * 2.0f)).Forward();

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* pl = base->GetClientEntity(base->mEntList, i);
		if (!pl)
			continue;

		if (pl == local)
			continue;

		if (!IsValidTarget(local, pl))
			continue;

		if (gClient->Cfg(gVars.l_trig_infov))
		{
			if ((gClient->mCurTime - mPlayersInFovTime[i]) < ((float)(gClient->Cfg(gVars.l_trig_infov)) * 0.01f))
				continue;
		}

		if (!FindTriggerIntersection(pl, forward, &tick))
			continue;

		best = pl;

		break;
	}

	if (best)
	{
		bool fire = true;

		if (gClient->Cfg(gVars.l_trig_delay))
		{
			if ((gClient->mCurTime - mLastFailedTriggerTime) < 0.0f)
				fire = false;
		}

		if (fire)
		{
			cmd->buttons |= IN_ATTACK;
			mLastTriggerTime = gClient->mCurTime + burst;
		}

		if (tick != -1)
			cmd->ticks = tick;

		return true;
	}

	return false;
}

bool CLegitAim::test2(Entity* player, Vector angles, Vector delta, float* fvalue)
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));

	int index = player->GetIndex();
	CAimPlayer* aim_player = &gAim->mAimPlayers[index];

	CAimbotFrame* target_frame = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];

	Matrix matrix[HITBOX_MAX];
	memcpy(matrix, target_frame->mBones[index], sizeof(Matrix) * HITBOX_MAX);

	HitBox* bb = gAim->GetBaseHitbox(player);
	float best_dot = -1.0f;
	bool found = false;

	for (float f = 0.0f; f <= 2.0f; f += 0.05f)
	{
		Vector forward = (angles.Forward() + Vector(delta.x * f, delta.y * f, 0.0f)).Normal();

		int result = gAim->PerformIntersection(player, matrix, gClient->mViewOrigin, forward, nullptr, (float)(gVars.l_hitbox_scale.value) * 0.01f);
		if (result != HITBOX_HEAD)
			continue;

		Vector dir = (matrix[HITBOX_HEAD].Transform((bb->min + bb->max) * 0.5f) - gClient->mViewOrigin).Normal();

		float dot = forward.Dot(dir);

		if (dot > best_dot)
		{
			*fvalue = f;
			best_dot = dot;
			found = true;
		}
	}

	return found;
}

void CLegitAim::test(Entity* local, UserCmd* cmd)
{
	if (!gClient->Cfg(gVars.l_deltaaim))
		return;

	Entity* best = nullptr;
	int tick = -1;
	float fvalue = 0.0f;
	Vector base_angles = Vector(gClient->mMouseLastPitch, gClient->mMouseLastYaw, 0.0f);
	Vector angles = base_angles + (gClient->mAimPunch * 2.0f);
	float best_len = -1.0f;

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* pl = base->GetClientEntity(base->mEntList, i);
		if (!pl)
			continue;

		if (pl == local)
			continue;

		if (!IsValidTarget(local, pl))
			continue;

		CAimPlayer* aim_player = &gAim->mAimPlayers[i];
		CAimbotFrame* target_frame = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];
		HitBox* bb = gAim->GetBaseHitbox(pl);
		Vector aimpos = target_frame->mBones[i][HITBOX_HEAD].Transform((bb->min + bb->max) * 0.5f);

		float len = (aimpos - gClient->mViewOrigin).Normal().Dot(angles.Forward());
		if (len > best_len)
		{
			best = pl;
			best_len = len;
		}
	}

	if (best && (gClient->mMousePitchDelta != 0.0f || gClient->mMouseYawDelta != 0.0f))
	{
		if (test2(best, angles, Vector(gClient->mMousePitchDelta, gClient->mMouseYawDelta, 0.0f), &fvalue))
		{
			cmd->angles = base_angles + (Vector(gClient->mMousePitchDelta, gClient->mMouseYawDelta, 0.0f) * fvalue);
			//base->SetViewAngles(base->mBaseEngine, &cmd->angles);
		//	base->Warning("test %f %f %f\n", fvalue, gClient->mMousePitchDelta, gClient->mMouseYawDelta);
		}
	}
}

void CLegitAim::Think(Entity* local, UserCmd* cmd)
{
	if (AimbotThink(local, cmd))
	{
		mStartedAiming = true;
	}
	else
	{
		mTargetViewangles = gClient->mViewAngles;

		if (mStartedAiming)
			mStartAimTime = gClient->mCurTime;
		
		mStartedAiming = false;
		mAimTime = gClient->mCurTime;
		mAimVelocityMag = 0.0f;
		mLastTarget = nullptr;
		mPrimaryTarget = nullptr;
	}

	//if (!(cmd->buttons & IN_ATTACK))
	//	RcsSmoothBack(cmd);

	//if (cmd->buttons & IN_ATTACK)
		test(local, cmd);

	AccuracyThink(local, cmd);

	if (!TriggerThink(local, cmd))
		mLastFailedTriggerTime = gClient->mCurTime + mDelayToAdd;
}

void CLegitAim::SmokeThink()
{
	int limit = base->GetHighestEntityIndex(base->mEntList);

	for (int i = 0; i <= limit; ++i)
	{
		Entity* entity = base->GetClientEntity(base->mEntList, i);
		if (!IsEntityValid(entity))
			continue;

		char* str = entity->GetClientClass()->name;

		if (CRC32_Get(str, string_len(str)) == 0x5b14e777) // CSmokeGrenadeProjectile
			AddSmoke(entity);
	}

	for (int i = 0; i < 32; ++i)
	{
		CSmokeEffect* smoke = &mSmokeEffects[i];
		if (!smoke)
			continue;

		if (!smoke->mActive)
			continue;

		if (smoke->mIndex == -1)
			continue;

		bool invalid = false;

		Entity* entity = base->GetClientEntity(base->mEntList, smoke->mIndex);
		if (entity)
		{
			char* str = entity->GetClientClass()->name;

			if (CRC32_Get(str, string_len(str)) == 0x5b14e777) // CSmokeGrenadeProjectile
				invalid = true;
		}
		else
		{
			invalid = true;
		}

		if (invalid)
		{
			if (!smoke->mExpiring)
			{
				smoke->mExpiring = true;
				smoke->mExpireTime = gClient->mCurTime + 2.5f;
			}
			else
			{
				if (gClient->mCurTime > smoke->mExpireTime)
				{
					smoke->mActive = false;
					smoke->mEffect = false;
					smoke->mIndex = -1;
					smoke->mExpiring = false;
					smoke->mExpireTime = 0.0f;
				}
			}
		}

		if (entity)
		{
			if (*entity->m_bDidSmokeEffect())
			{
				smoke->mOrigin = *entity->m_vecOrigin();
				smoke->mEffect = true;
			}
		}
	}
}

bool CLegitAim::IntersectWithSmoke(Vector pos)
{
	Vector dt = (pos - gClient->mViewOrigin).Normal();

	for (int i = 0; i < 32; ++i)
	{
		CSmokeEffect* smoke = &mSmokeEffects[i];
		if (!smoke)
			continue;

		if (!smoke->mActive)
			continue;

		if (!smoke->mEffect)
			continue;

		if (intersect_sphere(gClient->mViewOrigin, smoke->mOrigin + Vector(0.0f, 0.0f, 64.0f), dt, 135.0f * 135.0f, nullptr))
			return true;
	}

	return false;
}