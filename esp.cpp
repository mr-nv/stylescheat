#include "dll.h"

CESPManager gESP;

int BoxFaceIndices[6][4] =
{
	{ 0, 4, 6, 2 }, // -x
	{ 5, 1, 3, 7 }, // +x
	{ 0, 1, 5, 4 }, // -y
	{ 2, 6, 7, 3 }, // +y
	{ 0, 2, 3, 1 },	// -z
	{ 4, 5, 7, 6 }	// +z
};

void CESPManager::DrawESP()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	if (gVars.esp_only_dead.value)
	{
		if (*local->m_lifeState() == 0)
			return;
	}

	BYTE* t_clr = (BYTE*)(&(gVars.esp_color_t.value));
	BYTE* ct_clr = (BYTE*)(&(gVars.esp_color_ct.value));

	BYTE* tbox_clr = (BYTE*)(&(gVars.esp_color_box_t.value));
	BYTE* ctbox_clr = (BYTE*)(&(gVars.esp_color_box_ct.value));

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* player = base->GetClientEntity(base->mEntList, i);
		if (!IsEntityValid(player))
			continue;

		if (player == local)
			continue;

		if (*player->m_lifeState() != 0)
			continue;

		if (player->IsDormant())
			continue;

		CAimPlayer* aim_player = &gAim->mAimPlayers[i];

		if (!gVars.esp_team.value)
		{
			if (*player->m_iTeamNum() == *local->m_iTeamNum())
				continue;
		}

		mSideYOffset = 0.0f;
		mTopYOffset = -14.0f;
		mBottomYOffset = 0.0f;
		mCenterYOffset = 0.0f;

		if (gVars.esp_health.value == 2)
			mTopYOffset -= 12.0f;

		if (gVars.esp_weapon.value == 2)
			mTopYOffset -= 12.0f;

		float minx = 0.0f;
		float miny = 0.0f;
		float maxx = 0.0f;
		float maxy = 0.0f;

		Vector org = *player->GetRenderOrigin();
		Vector offset = Vector(0.0f, 0.0f, (*player->m_fFlags() & FL_DUCKING) ? player->m_vecMaxs()->z * 0.75f : player->m_vecMaxs()->z);

		if (!ToScreen(org, &minx, &miny) || !ToScreen(org + offset, &maxx, &maxy))
			continue;

		if (gVars.esp_history.value)
		{
			CAimbotFrame* frame = nullptr;
			CAimbotFrame* last_frame = nullptr;
			CAimbotFrame* body_frame = nullptr;
			CAimbotFrame* body_frame_last = nullptr;

			int limit = (int)((0.2f / base->mGlobalVars->interval) + 0.5f) - gAim->mInterpTicks - 1;

			for (int n = 0; n < MAX_FRAMES; ++n)
			{
				int target_time = gAim->mFrameCount - n;
				if (target_time < 0)
					break;

				CAimbotFrame* target_frame = &gAim->mFrames[target_time % MAX_FRAMES];

				int ticks = (int)((target_frame->mSimulationTime[i] / base->mGlobalVars->interval) + 0.5f);
				ticks += gAim->mInterpTicks;

				int delta = (ticks - gClient->mTickCount);

				if (delta > USERCMD_DELTA_MAX)
					continue;

				if (delta < (limit * -1))
					break;

				if (!frame)
					frame = target_frame;

				last_frame = target_frame;

				if (gVars.aim_resolver.value &&
					((target_frame->mBodyHistoryAim[i] && target_frame->mSimulationTime[i] == target_frame->mBodyUpdateTime[i])))
				{
					if (!body_frame)
						body_frame = target_frame;

					body_frame_last = target_frame;
				}
			}

			if (gVars.aim_history.value && last_frame)
				DrawPlayerRotation(player, last_frame->mBones[i], 255, 0, 128, true);

			if (gVars.aim_history.value == 2 && body_frame_last)
				DrawPlayerRotation(player, body_frame_last->mBones[i], 128, 0, 255, true);
		}

		if (gVars.esp_box.value == 2)
		{
			Vector min = *player->m_vecMins();
			Vector max = *player->m_vecMaxs();

			if (*player->m_iTeamNum() == 2)
				DrawBox(org, min, max, 0.0f, gAim->mAimPlayers[i].mAngles.y, 0.0f, (int)(tbox_clr[0]), (int)(tbox_clr[1]), (int)(tbox_clr[2]));
			else
				DrawBox(org, min, max, 0.0f, gAim->mAimPlayers[i].mAngles.y, 0.0f, (int)(ctbox_clr[0]), (int)(ctbox_clr[1]), (int)(ctbox_clr[2]));
		}
		else if (gVars.esp_box.value == 1)
		{
			float x = maxx - ((miny - maxy) * 0.25f);
			float y = maxy;

			if (*player->m_iTeamNum() == 2)
				gDraw.DrawOutlined(x, y, (miny - maxy) * 0.5f, miny - maxy, (int)(tbox_clr[0]), (int)(tbox_clr[1]), (int)(tbox_clr[2]), 255);
			else
				gDraw.DrawOutlined(x, y, (miny - maxy) * 0.5f, miny - maxy, (int)(ctbox_clr[0]), (int)(ctbox_clr[1]), (int)(ctbox_clr[2]), 255);

			gDraw.DrawOutlined(x - 1, y - 1, ((miny - maxy) * 0.5f) + 2, (miny - maxy) + 2, 0, 0, 0, 195);
			gDraw.DrawOutlined(x + 1, y + 1, ((miny - maxy) * 0.5f) - 2, (miny - maxy) - 2, 0, 0, 0, 195);
		}

		if (gVars.esp_healthbar.value)
		{
			float mul = (float)(*player->m_iHealth()) / 100.0f;

			if (mul > 1.0f)
				mul = 1.0f;

			float step = 1.0f;

			if (gVars.esp_healthbar_fraction.value != 0)
				step /= (float)(gVars.esp_healthbar_fraction.value);

			float hp_mul = 0.0f;

			for (hp_mul = step; hp_mul <= 1.0f; hp_mul += step)
			{
				if (hp_mul >= mul)
					break;
			}

			float x = maxx - ((miny - maxy) * 0.25f);
			float y = maxy;
			float dt = (miny - maxy);

			float len = dt * hp_mul;

			gDraw.DrawRect(x - 5, y, 2, dt, CLR_BLACK);
			gDraw.DrawOutlined(x - 6, y, 4, dt + 1, 0, 0, 0, 195);

			int r = (int)(0.0f + (510.0f * (1.0f - mul)));
			int g = (int)(510.0f - (510.0f * (1.0f - mul)));

			if (r > 255)
				r = 255;

			if (g > 255)
				g = 255;

			gDraw.DrawRect(x - 5, y + (dt - len), 2, len, r, g, 0, 255);

			for (float f = 0.0f; f <= 1.0f; f += step)
			{
				float len2 = dt * f;

				gDraw.DrawLine(x - 5, y + (dt - len2), x - 3, y + (dt - len2), CLR_BLACK);
			}
		}

		if (gVars.esp_name.value)
		{
			PlayerInfo info;
			if (base->GetPlayerInfo(base->mBaseEngine, i, &info))
				DrawText(info.name, player, gVars.esp_name.value);
		}

		if (gVars.esp_health.value)
		{
			char result[64];
			base->wsprintfA(result, "%i HP", *player->m_iHealth());

			DrawText(result, player, gVars.esp_health.value);
		}

		if (gVars.esp_weapon.value)
		{
			CCSWeapon* gun = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *player->m_hActiveWeapon()));

			if (gun)
			{
				char result[64];

				if (gVars.esp_ammo.value)
					base->wsprintfA(result, "%s - %i", gun->GetPrintName(), *gun->m_iClip1());
				else
					base->wsprintfA(result, "%s", gun->GetPrintName());

				DrawText(result, player, gVars.esp_weapon.value);
			}
		}

		if (gVars.esp_debug.value)
		{
			char result[64];
			base->wsprintfA(result, "resolvemode %i", aim_player->mResolveMode);

			DrawText(result, player, 1);
		}
	}
}

bool CESPManager::IsWeaponEntity(char* classname)
{
	if (CRC32_Get(classname, 7) == 0x37d2b3e9) // CWeapon
		return true;

	DWORD crc = CRC32_Get(classname, string_len(classname));

	if (crc == 0x8581de05) // CAK47
		return true;

	if (crc == 0xd6c7d366) // CDEagle
		return true;

	return false;
}

bool CESPManager::IsGrenadeEntity(char* classname)
{
	DWORD crc = CRC32_Get(classname, string_len(classname));

	if (crc == 0xfd7e2c06) // CBaseCSGrenadeProjectile
		return true;

	if (crc == 0x5f48f653) // CDecoyProjectile
		return true;

	if (crc == 0x5b14e777) // CSmokeGrenadeProjectile
		return true;

	if (crc == 0xb4f1cddc) // CMolotovProjectile
		return true;

	return false;
}

char* CESPManager::GetGrenadeName(char* modelname)
{
	DWORD crc = CRC32_Get(modelname, string_len(modelname));

	switch (crc)
	{
		case 0xde7b952e:
			return "frag";
		case 0xb533f330:
			return "flashbang";
		case 0xabac3b4a:
			return "incendiary";
		case 0xa4cde557:
			return "smoke";
		case 0x2d60f56e:
			return "decoy";
		case 0x533f7fb6:
			return "molotov";
	}

	return "null";
}

void CESPManager::DrawEntityESP()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	int max_ents = base->GetHighestEntityIndex(base->mEntList);

	for (int i = base->mGlobalVars->clients + 1; i <= max_ents; ++i)
	{
		if (!mSetGrenadeTime[i])
			mGrenadeTime[i] = base->mGlobalVars->curtime;

		mSetGrenadeTime[i] = false;

		Entity* entity = base->GetClientEntity(base->mEntList, i);
		if (!IsEntityValid(entity))
			continue;

		if (entity->IsDormant())
			continue;

		char* classname = entity->GetClientClass()->name;
		CRC32 crc = CRC32_Get(classname, string_len(classname));

		if (gVars.esp_weapons.value && IsWeaponEntity(classname))
		{
			if (*entity->m_hOwner() != -1)
				continue;

			Vector org = *entity->m_vecOrigin();
			float x = 0.0f;
			float y = 0.0f;

			if (!ToScreen(org, &x, &y))
				continue;

			gDraw.DrawTextA(((CCSWeapon*)(entity))->GetPrintName(), x, y, 255, 255, 255, 235, true, false);
		}
		else if (gVars.esp_grenade.value && IsGrenadeEntity(classname))
		{
			Vector org = *entity->m_vecOrigin();
			float x = 0.0f;
			float y = 0.0f;

			if (gVars.esp_effects.value && crc == 0x5b14e777 && mSmokeEffectTickBegin[i] != 0) // CSmokeGrenadeProjectile
			{
				mSetGrenadeTime[i] = true;

				if (!ToScreen(org, &x, &y))
					continue;

				char str[128];
				SecureZeroMemory(str, sizeof(str));

				int time = (int)(16.5f - (base->mGlobalVars->curtime - mGrenadeTime[i]));

				base->wsprintfA(str, "smoke %is", time);

				gDraw.DrawTextA(str, x, y, 255, 255, 255, 235, true, false);
			}
			else
			{
				if (!ToScreen(org, &x, &y))
					continue;

				char* modelname = base->GetModelName(base->mModelInfo, entity->GetModel());

				gDraw.DrawTextA(GetGrenadeName(modelname), x, y, 255, 255, 255, 235, true, false);
			}
		}//CInferno
		else if (gVars.esp_bomb.value && crc == 0x777d02f8) // CC4
		{
			if (*entity->m_hOwner() != -1)
				continue;

			Vector org = *entity->m_vecOrigin();
			float x = 0.0f;
			float y = 0.0f;

			if (!ToScreen(org, &x, &y))
				continue;

			gDraw.DrawTextA("C4", x, y, 255, 255, 255, 235, true, false);
		}
		else if (gVars.esp_bomb.value && crc == 0xc9bac39d) // CPlantedC4
		{
			Vector org = *entity->m_vecOrigin();
			float x = 0.0f;
			float y = 0.0f;

			if (!ToScreen(org, &x, &y))
				continue;

			char str[128];
			SecureZeroMemory(str, sizeof(str));

			float curtime = (float)(gClient->mTickCount) * base->mGlobalVars->interval;
			int time = (int)(*entity->m_flC4Blow() - curtime);
			if (time < 0)
				time = 0;

			base->wsprintfA(str, "C4 - %i", time);

			Entity* obv = base->GetClientEntityByHandle(base->mEntList, *local->m_hObserverTarget());

			if (*local->m_iTeamNum() == 3 || (obv && *obv->m_iTeamNum() == 3))
			{
				bool defuser = false;

				if (obv)
					defuser = *obv->m_bHasDefuser();
				else
					defuser = *local->m_bHasDefuser();

				if (time >= 10 || (defuser && time >= 5))
					gDraw.DrawTextA(str, x, y, 0, 255, 0, 235, true, false);
				else
					gDraw.DrawTextA(str, x, y, 255, 0, 0, 235, true, false);
			}
			else
			{
				gDraw.DrawTextA(str, x, y, 255, 255, 255, 235, true, false);
			}
		}
		else if (gVars.esp_effects.value && crc == 0x4ad147e5) // CInferno
		{
			mSetGrenadeTime[i] = true;

			Vector org = *entity->m_vecOrigin();
			float x = 0.0f;
			float y = 0.0f;

			if (!ToScreen(org, &x, &y))
				continue;

			char str[128];
			SecureZeroMemory(str, sizeof(str));

			int time = (int)(7.0f - (base->mGlobalVars->curtime - mGrenadeTime[i]));

			base->wsprintfA(str, "fire %is", time);

			gDraw.DrawTextA(str, x, y, 255, 160, 0, 235, true, false);
		}
		else if (gVars.esp_defusekit.value && crc == 0x5b26cf98) // CBaseAnimating
		{
			char* modelname = base->GetModelName(base->mModelInfo, entity->GetModel());

			if (CRC32_Get(modelname, string_len(modelname)) != 0x6a1bf0c7) // models/weapons/w_defuser.mdl
				continue;

			Vector org = *entity->m_vecOrigin();
			float x = 0.0f;
			float y = 0.0f;

			if (!ToScreen(org, &x, &y))
				continue;

			gDraw.DrawTextA("defuse kit", x, y, 255, 255, 255, 235, true, false);
		}
	}
}

void CESPManager::DrawSpectatorList()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	if (base->IsPlayingDemo(base->mBaseEngine))
		return;

	gDraw.DrawTextA("spectators", gDraw.mWidth * 0.005f, gDraw.mHeight * 0.25f, CLR_WHITE);

	int add = 0;

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* player = base->GetClientEntity(base->mEntList, i);
		if (!IsEntityValid(player))
			continue;

		if (player == local)
			continue;

		if (*player->m_lifeState() == 0)
			continue;

		if (player->IsDormant())
			continue;

		int mode = *player->m_iObserverMode();

		if (mode == 0 || mode == 6)
			continue;

		Entity* obv = base->GetClientEntityByHandle(base->mEntList, *player->m_hObserverTarget());
		if (!obv)
			continue;

		if (gVars.esp_speclist.value == 1)
		{
			if (obv != local)
				continue;
		}

		if (*obv->m_lifeState() != 0)
			continue;

		PlayerInfo info, info2;
		base->GetPlayerInfo(base->mBaseEngine, i, &info);
		base->GetPlayerInfo(base->mBaseEngine, obv->GetIndex(), &info2);

		if (CRC32_Get(info.guid, string_len(info.guid)) == 0x97B8469B)
			continue;

		int add2 = 0;

		{
			if (*player->m_iTeamNum() == 2)
				gDraw.DrawTextA(info.name, (gDraw.mWidth * 0.005f) + 15, (gDraw.mHeight * 0.25f) + 20 + add, CLR_TERRORIST);
			else if (*player->m_iTeamNum() == 3)
				gDraw.DrawTextA(info.name, (gDraw.mWidth * 0.005f) + 15, (gDraw.mHeight * 0.25f) + 20 + add, CLR_COUNTER_TERRORIST);
			else
				gDraw.DrawTextA(info.name, (gDraw.mWidth * 0.005f) + 15, (gDraw.mHeight * 0.25f) + 20 + add, CLR_WHITE);

			wchar_t result[128];
			SecureZeroMemory(result, sizeof(result));

			int num = base->MultiByteToWideChar(CP_UTF8, 0, info.name, -1, nullptr, 0);
			base->MultiByteToWideChar(CP_UTF8, 0, info.name, -1, result, num);

			int x = 0;
			int y = 0;

			base->GetTextSize(base->mSurface, gDraw.mFont, result, &x, &y);

			add2 += x + 8;
		}

		if (mode == 4)
			gDraw.DrawTextA("->", (gDraw.mWidth * 0.005f) + 15 + add2, (gDraw.mHeight * 0.25f) + 20 + add, 192, 255, 0, 255);
		else
			gDraw.DrawTextA("->", (gDraw.mWidth * 0.005f) + 15 + add2, (gDraw.mHeight * 0.25f) + 20 + add, CLR_WHITE);

		add2 += 20;

		{
			if (obv == local)
				gDraw.DrawTextA(info2.name, (gDraw.mWidth * 0.005f) + 15 + add2, (gDraw.mHeight * 0.25f) + 20 + add, 0, 255, 0, 255);
			else if (*obv->m_iTeamNum() == 2)
				gDraw.DrawTextA(info2.name, (gDraw.mWidth * 0.005f) + 15 + add2, (gDraw.mHeight * 0.25f) + 20 + add, CLR_TERRORIST);
			else if (*obv->m_iTeamNum() == 3)
				gDraw.DrawTextA(info2.name, (gDraw.mWidth * 0.005f) + 15 + add2, (gDraw.mHeight * 0.25f) + 20 + add, CLR_COUNTER_TERRORIST);
			else
				gDraw.DrawTextA(info2.name, (gDraw.mWidth * 0.005f) + 15 + add2, (gDraw.mHeight * 0.25f) + 20 + add, CLR_WHITE);
		}

		add += 20;
	}
}

void CESPManager::DrawGrenadeTracer()
{
	if (!gVars.esp_grenade_tracer.value)
		return;

	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
	if (!weapon)
	{
		SecureZeroMemory(mGrenades, sizeof(CSimulatedGrenade) * MAX_GRENADES);
		return;
	}

	for (int i = 0; i < MAX_GRENADES; ++i)
	{
		CSimulatedGrenade* grenade = &mGrenades[i];
		if (grenade->mLifetime != -1.0f && gClient->mCurTime > grenade->mLifetime)
			continue;

		Vector prev_origin = grenade->mBeginOrigin;
		Vector origin = grenade->mSimulatedOrigins[0];

		for (int n = 0; n < grenade->mSimulationsIndex; ++n)
		{
			origin = grenade->mSimulatedOrigins[n];

			float x1 = 0.0f;
			float y1 = 0.0f;
			float x2 = 0.0f;
			float y2 = 0.0f;

			bool t1 = ToScreen(prev_origin, &x1, &y1);
			bool t2 = ToScreen(origin, &x2, &y2);

			if (t1 && t2)
				gDraw.DrawLine(x1, y1, x2, y2, CLR_WHITE);

			if (n == (grenade->mSimulationsIndex - 1))
			{
				if (t1 && t2)
				{
					if (grenade->mType == GRENADE_FRAG || grenade->mType == GRENADE_FLASH)
					{
						bool green = false;

						for (int k = 0; k < MAX_PLAYERS; ++k)
						{
							if (grenade->mType == GRENADE_FRAG)
							{
								if (grenade->mPredictedDamage[k] != 0.0f)
									green = true;
							}
							else
							{
								if (grenade->mPlayerFlash[k])
									green = true;
							}
						}

						if (green)
						{
							gDraw.DrawLine(x2 - 5, y2 - 5, x2 + 5, y2 + 5, 0, 255, 0, 255);
							gDraw.DrawLine(x2 - 5, y2 + 5, x2 + 5, y2 - 5, 0, 255, 0, 255);
						}
						else
						{
							gDraw.DrawLine(x2 - 5, y2 - 5, x2 + 5, y2 + 5, 255, 0, 0, 255);
							gDraw.DrawLine(x2 - 5, y2 + 5, x2 + 5, y2 - 5, 255, 0, 0, 255);
						}
					}
					else
					{
						gDraw.DrawLine(x2 - 5, y2 - 5, x2 + 5, y2 + 5, 255, 255, 0, 255);
						gDraw.DrawLine(x2 - 5, y2 + 5, x2 + 5, y2 - 5, 255, 255, 0, 255);
					}
				}
			}

			prev_origin = origin;
		}
	}
}

void CESPManager::GrenadeSimulationThink()
{
	for (int i = 0; i < MAX_GRENADES; ++i)
	{
		CSimulatedGrenade* grenade = &mGrenades[i];
		if (grenade->mLifetime != -1.0f && grenade->mLifetime > gClient->mCurTime)
			continue;

		SecureZeroMemory(grenade, sizeof(CSimulatedGrenade));
	}

	mGrenades[0].mLifetime = -1.0f;

	if (!gVars.esp_grenade_tracer.value)
		return;

	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
	if (!weapon)
		return;

	if (!weapon->IsGrenade())
		return;

	if (*weapon->m_iItemDefinitionIndex() == WEAPON_C4)
		return;

	if (gVars.esp_tracer_log.value && gClient->mFiring)
	{
		for (int i = 0; i < MAX_GRENADES; ++i)
		{
			CSimulatedGrenade* grenade = &mGrenades[i];
			if (grenade->mLifetime == -1.0f || grenade->mLifetime > gClient->mCurTime)
				continue;

			SimulateGrenade(grenade);
			grenade->mLifetime = gClient->mCurTime + (float)(gVars.esp_tracer_log.value);

			break;
		}
	}
	else
	{
		SimulateGrenade(&mGrenades[0]);
	}
}

Vector sHullMin = Vector(-2.0f, -2.0f, -2.0f);
Vector sHullMax = Vector(2.0f, 2.0f, 2.0f);
Vector sLastAngles = Vector(0.0f, 0.0f, 0.0f);

void CESPManager::SimulateGrenade(CSimulatedGrenade* grenade)
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
	if (!weapon)
		return;

	if (!weapon->IsGrenade())
		return;

	int def = *weapon->m_iItemDefinitionIndex();

	Vector angles = gClient->mViewAngles;
	angles.x = angles.x - (((90.0f - float_abs(angles.x)) * 10.0f) / 90.0f);

	if (!(gClient->mUserCmd && gClient->mUserCmd->buttons & IN_AIRSTUCK))
		sLastAngles = angles;

	Vector vieworigin = gClient->mViewOrigin;
	Vector viewangles = sLastAngles;

	Vector viewforward = viewangles.Forward();

	float speed = 750.0f * 0.9f;

	float mod = *weapon->m_flThrowStrength();
	speed *= (mod * 0.7f) + 0.3f;

	Vector velocity = ((viewforward * speed) + (gClient->mLocalVelocity * 1.25f));

	Vector origin = vieworigin;
	origin.z += (mod * 12.0f) - 12.0f;

	{
		RayData ray;
		ray.InitHull(origin, origin + (viewforward * 22.0f), sHullMin, sHullMax);

		TraceFilter filter;
		filter.noplayers = true;
		filter.nogrenades = true;

		TraceResult tr;

		base->TraceRay(base->mEngineTrace, &ray, 0x600400B, &filter, &tr);

		origin = tr.hitpos;
		origin -= viewforward * 6.0f;
	}

	if (def == WEAPON_HEGRENADE)
		grenade->mType = GRENADE_FRAG;
	else if (def == WEAPON_FLASHBANG)
		grenade->mType = GRENADE_FLASH;
	else if (def == WEAPON_SMOKEGRENADE)
		grenade->mType = GRENADE_SMOKE;
	else if (def == WEAPON_DECOY)
		grenade->mType = GRENADE_DECOY;
	else if (def == WEAPON_MOLOTOV || def == WEAPON_INCGRENADE)
		grenade->mType = GRENADE_FIRE;

	grenade->mLifetime = -1.0f;
	grenade->mBeginOrigin = origin;
	grenade->mSimulationsIndex = 0;

	do
	{
		Vector prev_origin = origin;

		// velocity
		origin += Vector(velocity.x, velocity.y, 0.0f) * base->mGlobalVars->interval;

		// gravity
		float z = (velocity.z - ((0.4f * 800.0f) * base->mGlobalVars->interval));
		origin.z += ((velocity.z + z) / 2.0f) * base->mGlobalVars->interval;
		velocity.z = z;

		RayData ray;
		ray.InitHull(prev_origin, origin, sHullMin, sHullMax);

		TraceFilter filter;
		filter.noplayers = true;
		filter.nogrenades = true;

		TraceResult tr;

		base->TraceRay(base->mEngineTrace, &ray, 0x600400B, &filter, &tr);

		int i2 = grenade->mSimulationsIndex - 1;
		if (i2 < 0)
			i2 = 0;

		bool done = false;
		bool check = (i2 % (int)((0.2f / base->mGlobalVars->interval) + 0.5f)) == 0;

		if (def == WEAPON_HEGRENADE || def == WEAPON_FLASHBANG)
		{
			if ((float)(i2)* base->mGlobalVars->interval > 1.5f)
			{
				if (check)
					done = true;
			}
		}
		else if (def == WEAPON_SMOKEGRENADE || def == WEAPON_DECOY)
		{
			if (velocity.LengthSqr() < (0.1f * 0.1f))
			{
				if (check)
					done = true;
			}
		}
		else if (def == WEAPON_MOLOTOV || def == WEAPON_INCGRENADE)
		{
			if (tr.fraction != 1.0f && tr.normal.z > 0.7f)
				done = true;
		}

		// physics_main_shared.cpp
		if (tr.fraction != 1.0f)
		{
			origin = tr.hitpos;

			Vector abs_velocity;
			float backoff = velocity.Dot(tr.normal) * 2.0f;

			for (int n = 0; n < 3; ++n)
			{
				float change = tr.normal[n] * backoff;

				abs_velocity[n] = velocity[n] - change;
				if (abs_velocity[n] > -0.1f && abs_velocity[n] < 0.1f)
					abs_velocity[n] = 0.0f;
			}

			abs_velocity *= 0.45f;

			float speed_dot = abs_velocity.Dot(abs_velocity);

			if (speed_dot < (20.0f * 20.0f))
				abs_velocity = Vector(0.0f, 0.0f, 0.0f);

			if (tr.normal.z > 0.7f)
			{
				velocity = abs_velocity;
				abs_velocity *= (1.0f - tr.fraction) * base->mGlobalVars->interval;

				ray.InitHull(origin, origin + abs_velocity, sHullMin, sHullMax);
				base->TraceRay(base->mEngineTrace, &ray, 0x600400B, &filter, &tr);
				origin = tr.hitpos;
			}
			else
			{
				velocity = abs_velocity;
			}
		}

		grenade->mSimulatedOrigins[grenade->mSimulationsIndex] = origin;

		if (done)
		{
			if (def == WEAPON_HEGRENADE || def == WEAPON_FLASHBANG)
			{
				for (int i = 1; i <= base->mGlobalVars->clients; ++i)
				{
					grenade->mPredictedDamage[i] = 0.0f;
					grenade->mPlayerFlash[i] = false;

					Entity* player = base->GetClientEntity(base->mEntList, i);
					if (!IsEntityValid(player))
						continue;

					if (player == local)
						continue;

					if (!gAim->IsValidTarget(local, player))
						continue;

					if (def == WEAPON_HEGRENADE)
					{
						Vector spot = *player->m_vecOrigin() + Vector(0.0f, 0.0f, player->m_vecMaxs()->z * 0.5f);

						ray.Init(origin, spot);
						base->TraceRay(base->mEngineTrace, &ray, 0x600400B, &filter, &tr);

						if (tr.fraction == 1.0f)
						{
							float falloff = 99.0f / (99.0f * 3.5f);
							float damage = ray.delta.Length() * falloff;
							damage = 99.0f - damage;

							if (damage > 1.0f)
								grenade->mPredictedDamage[i] = damage;
						}
					}

					if (def == WEAPON_FLASHBANG)
					{
						Vector spot = *player->m_vecOrigin() + Vector(0.0f, 0.0f, player->m_vecMaxs()->z);

						ray.Init(origin, spot);
						base->TraceRay(base->mEngineTrace, &ray, 0x600400B, &filter, &tr);

						if (tr.fraction == 1.0f)
							grenade->mPlayerFlash[i] = true;
					}
				}
			}

			break;
		}
	} while (++grenade->mSimulationsIndex < MAX_GRENADE_ORIGINS);
}

void CESPManager::Think()
{
	if (!base->IsInGame(base->mBaseEngine))
		return;

	if (gVars.esp_active.value)
		DrawESP();

	if (gVars.esp_entity.value)
		DrawEntityESP();

	if (gVars.esp_speclist.value)
		DrawSpectatorList();

	DrawGrenadeTracer();

	mScreenCenterYOffset = 0.0f;

	{
		if (gHvh.mLeftKey)
			gDraw.DrawTextA("AntiAim: left", gDraw.mWidth * 0.5f, (gDraw.mHeight * 0.5f) + mCenterYOffset, 0, 255, 0, 255);
		else if (gHvh.mRightKey)
			gDraw.DrawTextA("AntiAim: right", gDraw.mWidth * 0.5f, (gDraw.mHeight * 0.5f) + mCenterYOffset, 0, 255, 0, 255);

		if (gHvh.mLeftKey || gHvh.mRightKey)
			mScreenCenterYOffset += 12.0f;
	}
	
	{
		Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
		if (IsEntityValid(local))
		{
			if (gVars.esp_no_scope.value && *local->m_bIsScoped())
			{
				float x = gDraw.mWidth * 0.5f;
				float y = gDraw.mHeight * 0.5f;

				gDraw.DrawLine(x - 20.0f, y, x + 20.0f, y, 0, 255, 0, 255);
				gDraw.DrawLine(x, y - 20.0f, x, y + 20.0f, 0, 255, 0, 255);
				gDraw.DrawLine(x - 5.0f, y - 10.0f, x + 5.0f, y - 10.0f, 0, 255, 0, 255);
				gDraw.DrawLine(x - 5.0f, y + 10.0f, x + 5.0f, y + 10.0f, 0, 255, 0, 255);
				gDraw.DrawLine(x - 10.0f, y - 5.0f, x - 10.0f, y + 5.0f, 0, 255, 0, 255);
				gDraw.DrawLine(x + 10.0f, y - 5.0f, x + 10.0f, y + 5.0f, 0, 255, 0, 255);
			}

			if (gClient->mAliveAndProcessing && gClient->mThirdPerson)
			{
				if (gVars.esp_hitbox.value & (1 << 2))
					DrawPlayerRotation(local, gClient->mLocalBoneMatrixBody, 0, 255, 0, false);

				if (gVars.esp_hitbox.value & (1 << 1))
					DrawPlayerRotation(local, gClient->mLocalBoneMatrixServer, 255, 0, 0, false);

				if (gVars.esp_hitbox.value & (1 << 0))
					DrawPlayerRotation(local, gClient->mLocalBoneMatrix, 0, 0, 255, false);
			}
		}
	}

	if (gVars.esp_impacts.value)
		DrawWeaponImpacts();
}

void CESPManager::DrawPlayerRotation(Entity* player, Matrix* mtx, int r, int g, int b, bool new_array)
{
	HitBox* box = gAim->GetBaseHitbox(player);
	if (!box)
		return;

	for (int i = 0; i < HITBOX_MAX; ++i)
	{
		HitBox* bb = box + i;
		if (bb->radius == -1.0f)
			continue;

		float rad = bb->radius;
		int n = new_array ? i : bb->bone;

		Vector trans = mtx[n].GetTransform();
		Vector mins;
		if (bb->radius != -1.0f)
			mins = bb->min + Vector(-rad, -rad, -rad);
		else
			mins = bb->min;

		Vector maxs;
		if (bb->radius != -1.0f)
			maxs = bb->max + Vector(rad, rad, rad);
		else
			maxs = bb->max;

		Vector angle = mtx[n].GetColumn(0).Angles();
		float roll = RAD2DEG(float_atan2(mtx[n].self[2][1], mtx[n].self[2][2]));

		DrawBox(trans, mins, maxs, angle.x, angle.y, roll, r, g, b);
	}
}

void CESPManager::DrawWeaponImpacts()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (IsEntityValid(local))
	{
		CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
		if (weapon && weapon->IsHitscan())
		{
			float damage = 0.0f;
			Vector hits[20];
			bool thru[20];
			int hitindex = 0;
			int walls = 0;
			int wallbang = 0;

			weapon->TraceBulletImpacts(&damage, hits, thru, &hitindex, &walls);
			wallbang = hitindex - 1;

			if (hitindex != 0)
			{
				for (int i = 0; i < hitindex; ++i)
				{
					Vector org = hits[i];
					Vector min = Vector(-2.0f, -2.0f, -2.0f);
					Vector max = Vector(2.0f, 2.0f, 2.0f);

					DrawBox(org, min, max, 0.0f, 0.0f, 0.0f, thru[i] ? 0 : 255, thru[i] ? 255 : 0, 0);
				}
			}

			char str[64];
			base->wsprintfA(str, "wallbang count: %i", wallbang);

			gDraw.DrawTextA(str, gDraw.mWidth * 0.5f, (gDraw.mHeight * 0.5f) + mScreenCenterYOffset, wallbang != 0 ? 0 : 255, wallbang != 0 ? 255 : 0, 0, 255);
			mScreenCenterYOffset += 12.0f;

			base->wsprintfA(str, "wallbang damage: %i", (int)(damage + 0.5f));

			gDraw.DrawTextA(str, gDraw.mWidth * 0.5f, (gDraw.mHeight * 0.5f) + mScreenCenterYOffset, 0, 255, 0, 255);
			mScreenCenterYOffset += 12.0f;
		}
	}
}

void CESPManager::DrawBox(Vector org, Vector min, Vector max, float pitch, float yaw, float roll, int r, int g, int b)
{
	Vector out[8];

	for (int n = 0; n < 8; ++n)
	{
		Vector pos;

		pos.x = (n & 0x1) ? max.x : min.x;
		pos.y = (n & 0x2) ? max.y : min.y;
		pos.z = (n & 0x4) ? max.z : min.z;

		Matrix matrix;
		matrix.Init(org, Vector(pitch, yaw, roll));

		out[n] = matrix.Transform(pos);
	}

	for (int n = 0; n < 6; ++n)
	{
		int* index = BoxFaceIndices[n];

		for (int j = 0; j < 4; ++j)
		{
			Vector start;
			if (!gESP.ToScreen(out[index[j]], &start.x, &start.y))
				continue;

			Vector end;
			if (!gESP.ToScreen(out[index[(j == 3) ? 0 : j + 1]], &end.x, &end.y))
				continue;

			if (!start || !end)
				continue;

			gDraw.DrawLine(start.x, start.y, end.x, end.y, r, g, b, 255);
		}
	}
}

void CESPManager::DrawText(char* text, Entity* player, int mode)
{
	int team = *player->m_iTeamNum();

	BYTE* t_clr = (BYTE*)(&(gVars.esp_color_t.value));
	BYTE* ct_clr = (BYTE*)(&(gVars.esp_color_ct.value));

	switch (mode)
	{
	case 1:
	{
		float minx = 0.0f;
		float miny = 0.0f;
		float maxx = 0.0f;
		float maxy = 0.0f;

		Vector org = *player->GetRenderOrigin();
		Vector offset = Vector(0.0f, 0.0f, (*player->m_fFlags() & FL_DUCKING) ? player->m_vecMaxs()->z * 0.75f : player->m_vecMaxs()->z);

		if (!ToScreen(org, &minx, &miny) || !ToScreen(org + offset, &maxx, &maxy))
			return;

		if (team == 2)
			gDraw.DrawTextA(text, maxx + ((miny - maxy) * 0.25f) + 4, maxy + mSideYOffset - 2, t_clr[0], t_clr[1], t_clr[2], 235, false, false);
		else
			gDraw.DrawTextA(text, maxx + ((miny - maxy) * 0.25f) + 4, maxy + mSideYOffset - 2, ct_clr[0], ct_clr[1], ct_clr[2], 235, false, false);

		mSideYOffset += TEXT_OFFSET;

		break;
	}
	case 2:
	{
		float x = 0.0f;
		float y = 0.0f;

		Vector org = *player->GetRenderOrigin();
		Vector offset = Vector(0.0f, 0.0f, (*player->m_fFlags() & FL_DUCKING) ? player->m_vecMaxs()->z * 0.75f : player->m_vecMaxs()->z);

		if (!ToScreen(org + offset, &x, &y))
			return;

		if (team == 2)
			gDraw.DrawTextA(text, x, y + mTopYOffset - 2, t_clr[0], t_clr[1], t_clr[2], 235, true, false);
		else
			gDraw.DrawTextA(text, x, y + mTopYOffset - 2, ct_clr[0], ct_clr[1], ct_clr[2], 235, true, false);

		mTopYOffset += TEXT_OFFSET;

		break;
	}
	case 3:
	{
		float minx = 0.0f;
		float miny = 0.0f;
		float maxx = 0.0f;
		float maxy = 0.0f;

		Vector org = *player->GetRenderOrigin();
		Vector offset = Vector(0.0f, 0.0f, (*player->m_fFlags() & FL_DUCKING) ? player->m_vecMaxs()->z * 0.75f : player->m_vecMaxs()->z);

		if (!ToScreen(org, &minx, &miny) || !ToScreen(org + offset, &maxx, &maxy))
			return;

		if (team == 2)
			gDraw.DrawTextA(text, maxx, miny + mBottomYOffset, t_clr[0], t_clr[1], t_clr[2], 235, true, false);
		else
			gDraw.DrawTextA(text, maxx, miny + mBottomYOffset, ct_clr[0], ct_clr[1], ct_clr[2], 235, true, false);

		mBottomYOffset += TEXT_OFFSET;

		break;
	}
	case 4:
	{
		float x = 0.0f;
		float y = 0.0f;

		Vector org = *player->GetRenderOrigin();
		Vector offset = Vector(0.0f, 0.0f, player->m_vecMaxs()->z * 0.5f);

		if (!ToScreen(org + offset, &x, &y))
			return;

		if (team == 2)
			gDraw.DrawTextA(text, x, y + mCenterYOffset, t_clr[0], t_clr[1], t_clr[2], 235, false, false);
		else
			gDraw.DrawTextA(text, x, y + mCenterYOffset, ct_clr[0], ct_clr[1], ct_clr[2], 235, false, false);

		mCenterYOffset += TEXT_OFFSET;

		break;
	}
	case 5:
	{
		float x = 0.0f;
		float y = 0.0f;

		Vector org = *player->GetRenderOrigin();
		Vector offset = Vector(0.0f, 0.0f, player->m_vecMaxs()->z * 0.5f);

		if (!ToScreen(org + offset, &x, &y))
			return;

		if (team == 2)
			gDraw.DrawTextA(text, x, y + mCenterYOffset, t_clr[0], t_clr[1], t_clr[2], 235, true, false);
		else
			gDraw.DrawTextA(text, x, y + mCenterYOffset, ct_clr[0], ct_clr[1], ct_clr[2], 235, true, false);

		mCenterYOffset += TEXT_OFFSET;

		break;
	}
	}
}

bool CESPManager::ToScreen(Vector pos, float* x2, float* y2)
{
	float px = pos.x;
	float py = pos.y;
	float pz = pos.z;

	float mtx[4][4];
	memcpy(&mtx, base->GetScreenMatrix(base->mBaseEngine), sizeof(mtx));

	float z = mtx[3][0] * px + mtx[3][1] * py + mtx[3][2] * pz + mtx[3][3];

	if (z <= 0.001f)
		return false;

	float x = (0.5f * ((mtx[0][0] * px + mtx[0][1] * py + mtx[0][2] * pz + mtx[0][3]) / z) * (gDraw.mWidth + 0.5f));

	*x2 = (gDraw.mWidth * 0.5f) + x;

	float y = (0.5f * ((mtx[1][0] * px + mtx[1][1] * py + mtx[1][2] * pz + mtx[1][3]) / z) * (gDraw.mHeight + 0.5f));

	*y2 = (gDraw.mHeight * 0.5f) - y;

	return true;
}