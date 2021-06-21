#include "dll.h"
//#include "float.h"

CCSGOClient* gClient = nullptr;

static int sLastFrameChecked = 0;

void CCSGOClient::HookPreThink()
{
	CBaseClass* b = (CBaseClass*)(GetBaseClassAddress());
	if (!b)
		return;

	if (!gClient)
		gClient = (CCSGOClient*)(b->VirtualAlloc(nullptr, sizeof(CCSGOClient), MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE));

	if (!gAim)
		gAim = (CCSAimbot*)(b->VirtualAlloc(nullptr, sizeof(CCSAimbot), MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE));

	if (sLastFrameChecked == 0)
		sLastFrameChecked = b->mGlobalVars->framecount;

	if ((b->mGlobalVars->framecount - sLastFrameChecked) < 10)
		return;
	
	base = b;

	if (base->mNoInsecureLoad && base->GetParm(base->mCommandLine, "-insecure") == 0 && !gMenu.mIssueUnload)
	{
		base->Warning("-insecure not detected: unloading cheat\n");
		gMenu.mIssueUnload = true;
	}
}

bool CCSGOClient::PreThink(Entity* local, UserCmd* cmd)
{
	if (!mInit)
		mInit = true;

	// the purpose of all this code is to compensate for the frames that RunCommand hasn't got called in,
	// the solution is to set all of the predicted data to the last RunCommand frame,
	// and continue to predict it in CreateMove later
	{
		// store old data
		base->SetupMove(base->mPrediction, local, cmd, nullptr, mOldData.mMoveData);

		mOldData.mFlags = *local->m_fFlags();
		mOldData.mGroundEntity = *local->m_hGroundEntity();
		mOldData.mTickBase = *local->m_nTickBase();
		mOldData.mAimPunch = *local->m_aimPunchAngle();
		mOldData.mDuckAmount = *local->m_flDuckAmount();
		mOldData.mDuckSpeed = *local->m_flDuckSpeed();

		// get last data from RunCommand
		int target_index = cmd->index - 1;

		if (target_index >= 0)
		{
			CPredFrame* frame = &mPredFrames[target_index % 150];

			if (frame->mIndex == target_index)
			{
				memcpy(mPredData.mMoveData, frame->mMoveData, 200);

				mPredData.mFlags = frame->mFlags;
				mPredData.mGroundEntity = frame->mGroundEntity;
				mPredData.mTickBase = frame->mTickBase;
				mPredData.mAimPunch = frame->mAimPunch;
				mPredData.mDuckAmount = frame->mDuckAmount;
				mPredData.mDuckSpeed = frame->mDuckSpeed;
			}
		}

		// restore our predicted data if we can
		if (mPredictionPlayer && mPredictionPlayer == local)
		{
			base->FinishMove(base->mPrediction, local, cmd, mPredData.mMoveData);

			*local->m_fFlags() = mPredData.mFlags;
			*local->m_hGroundEntity() = mPredData.mGroundEntity;
			*local->m_nTickBase() = mPredData.mTickBase;
			*local->m_aimPunchAngle() = mPredData.mAimPunch;
			*local->m_flDuckAmount() = mPredData.mDuckAmount;
			*local->m_flDuckSpeed() = mPredData.mDuckSpeed;
		}
	}

	mMoveYawOverride = cmd->angles.y;

	if ((mCurTime - mFrozenTime) == base->mGlobalVars->interval)
	{
		if (gVars.misc_exec_buy.value)
		{
			char cfg[64];
			memset(cfg, 0, sizeof(cfg));

			char cmd[128];

			if (gVars.misc_exec_buy.value == 2)
				base->wsprintfA(cfg, "%s_buy", gMenu.GetConfigName());
			else
				string_cpy(cfg, "_buy");

			base->wsprintfA(cmd, "exec %s", cfg);

			base->ConCommand(base->mBaseEngine, cmd);
		}
	}

	if (gVars.misc_control_bot.value)
	{
		int entindex = -1;

		for (int i = 1; i <= base->mGlobalVars->clients; ++i)
		{
			Entity* player = base->GetClientEntity(base->mEntList, i);
			if (!IsEntityValid(player))
				continue;

			if (player == local)
				continue;

			if (*player->m_iTeamNum() != *local->m_iTeamNum())
				continue;

			if (*player->m_lifeState() != 0)
				continue;

			PlayerInfo info;
			base->GetPlayerInfo(base->mBaseEngine, i, &info);

			if (CRC32_Get(info.guid, string_len(info.guid)) != 0x97B8469B)
				continue;

			entindex = i;

			break;
		}

		Entity* obv = base->GetClientEntityByHandle(base->mEntList, *local->m_hObserverTarget());
		if (entindex != -1 && IsEntityValid(obv) && *obv->m_lifeState() == 0)
		{
			PlayerInfo info;
			base->GetPlayerInfo(base->mBaseEngine, obv->GetIndex(), &info);

			if (*obv->m_iTeamNum() != *local->m_iTeamNum() || CRC32_Get(info.guid, string_len(info.guid)) != 0x97B8469B)
			{
				if ((cmd->index % 20) == 0)
				{
					char str[128];
					base->wsprintfA(str, "spec_player \"%i\"", entindex);
					base->ConCommand(base->mBaseEngine, str);
				}
			}
			else
			{
				cmd->buttons |= (cmd->index & 1) ? IN_USE : 0;
			}
		}
	}

	if (gVars.misc_antiafk.value)
	{
		if ((cmd->ticks % 128) != base->RandomInt(0, 2))
			cmd->buttons |= IN_JUMP;

		Vector angles = cmd->angles;
		base->GetViewAngles(base->mBaseEngine, &angles);
		if ((cmd->ticks % 90) > 45)
			angles.y += 1.0f;
		else
			angles.y -= 1.0f;

		base->SetViewAngles(base->mBaseEngine, &angles);
	}

	cmd->buttons &= ~(IN_AIRSTUCK);

	if (gVars.move_airstuck.value)
	{
		if (mAirstuckKey)
		{
			if (cmd->select == 0)
				cmd->buttons |= IN_AIRSTUCK;
		}
	}

	gAim->PreThink(local);

	UpdateAnimations();

	StraferThink(local, cmd);

	if (gVars.misc_autohop.value)
	{
		bool autohop = false;
		ConVar* sv_autobunnyhopping = base->FindVar(base->mEngineCvar, "sv_autobunnyhopping");
		if (sv_autobunnyhopping)
		{
			if (sv_autobunnyhopping->GetValue() != 0)
				autohop = true;
		}

		if (*local->m_MoveType() == MOVETYPE_WALK && *local->m_nWaterLevel() < 1 && !autohop)
		{
			if (!(*local->m_fFlags() & FL_ONGROUND))
				cmd->buttons &= ~IN_JUMP;
		}

		if (gVars.move_duckjump.value && *local->m_fFlags() & FL_ONGROUND)
			cmd->buttons &= ~IN_DUCK;
	}

	Vector pred = Vector(0.0f, 0.0f, 0.0f);

	if (gVars.aim_active.value && gVars.aa_fakewalk.value && gHvh.mFakewalkKey)
	{
		CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));

		static int result_k = -1;
		static int result_i = -1;

		if (weapon && mHeldLag == 0)
		{
			result_k = -1;
			result_i = -1;

			int lag = gVars.aa_stand_lag.value;
			if (gVars.aa_valvemm.value)
			{
				if (lag > 11)
					lag = 11;
			}

			float sv_accelerate = base->FindVar(base->mEngineCvar, "sv_accelerate")->GetFloat();
			float sv_friction = base->FindVar(base->mEngineCvar, "sv_friction")->GetFloat();
			float sv_stopspeed = base->FindVar(base->mEngineCvar, "sv_stopspeed")->GetFloat();

			float max_speed = weapon->GetWeaponInfo()->GetMaxSpeed();

			if (*local->m_bIsScoped())
				max_speed *= 0.55f;

			if (mLastFlags & FL_DUCKING)
				max_speed /= 2.95f;

			for (int k = lag; k > 1; --k)
			{
				bool finished = false;

				for (int i = lag; i > 1; --i)
				{
					if (cmd->move.Length() == 0.0f)
						break;

					Vector move = cmd->move;
					pred = Vector(0.0f, 0.0f, 0.0f);

					for (int n = 1; n <= lag + 1; ++n)
					{
						if (n >= k)
							move = Vector(0.0f, 0.0f, 0.0f);

						float move_yaw = cmd->angles.y;
						if (gVars.aa_fakewalk.value == 2 && n >= i)
							move_yaw += 180.0f;

						float speed = pred.Length();

						if (speed >= 0.1f)
						{
							float control = speed < sv_stopspeed ? sv_stopspeed : speed;
							float drop = control * sv_friction * base->mGlobalVars->interval;

							float newspeed = speed - drop;
							if (newspeed < 0.0f)
								newspeed = 0.0f;

							pred *= newspeed / speed;
						}

						Vector fwd = Vector(0.0f, move_yaw, 0.0f).Forward();
						fwd.z = 0.0f;

						Vector right = Vector(0.0f, move_yaw, 0.0f).Right();
						right.z = 0.0f;

						float len = move.Length();
						if (len > 1.0f)
							len = 1.0f;

						float wishspeed = max_speed * len;

						Vector wishdir = fwd * move.x + right * move.y;
						wishdir = wishdir.Normal();

						float addspeed = wishspeed - pred.Dot(wishdir);
		
						if (addspeed > 0.0f)
						{
							float accel = sv_accelerate * max_speed * base->mGlobalVars->interval;

							if (accel > addspeed)
								accel = addspeed;

							pred += wishdir * accel;
						}

						if (pred.Length() < 1.0f)
							pred = Vector(0.0f, 0.0f, 0.0f);
					}

					if (pred.LengthSqr() == 0.0f)
					{
						result_k = k;
						result_i = i;

						finished = true;
						break;
					}

					if (gVars.aa_fakewalk.value != 2)
						break;
				}

				if (finished)
					break;
			}
		}

		if (result_k != -1)
		{
			if (mHeldLag == 0 || mHeldLag >= result_k)
				cmd->move = Vector(0.0f, 0.0f, 0.0f);
			else if (gVars.aa_fakewalk.value == 2 && mHeldLag >= result_i)
				mMoveYawOverride += 180.0f;
		}
	}

	if (gVars.aim_active.value && gVars.aa_stand_lag.value && gVars.aa_fakelag_speed.value)
	{
		if (local->m_vecVelocity()->LengthSqr() < 1.0f && mHeldLag != 1)
			cmd->move = Vector(0.0f, 0.0f, 0.0f);
	}

	if (gVars.misc_getout.value && IsEntityValid(local) && base->IsInGame(base->mBaseEngine))
	{
		if (*local->m_lifeState() == 0 && (cmd->index % 20) == 0)
			base->ConCommand(base->mBaseEngine, "getout");
	}

/*	if (gVars.misc_reconnect.value && !base->IsInGame(base->mBaseEngine))
	{
		if ((base->mGlobalVars->framecount % 240) == 0)
			base->ConCommand(base->mBaseEngine, "retry");
	}*/

	if (gVars.misc_1488.value && (cmd->index % 120) == 0)
		base->ConCommand(base->mBaseEngine, "wlj");

	mTickCount = cmd->ticks;
	mViewAngles = cmd->angles;
	mMoveYaw = mMoveYawOverride;
	mMoveDirection = cmd->move;
	mButtons = cmd->buttons;

	mCurTime = (float)(*local->m_nTickBase()) * base->mGlobalVars->interval;
	mViewOrigin = *local->m_vecOrigin() + *local->m_vecViewOffset();
	mViewOriginPredicted = mViewOrigin;

	if (*local->m_fFlags() & FL_ONGROUND)
		mViewOrigin.z += mViewZAddDelta;
	
	mLastVelocity = mLocalVelocity;
	mLocalVelocity = *local->m_vecVelocity();
	mAimPunchVel = *local->m_aimPunchAngle() - mAimPunch;
	mAimPunch = *local->m_aimPunchAngle();
	mFiring = false;
	mFireReady = false;

	if (gVars.aim_active.value)
	{
		if (gVars.aim_autoaction.value & (1<<0))
		{
			bool duck = true;

			if (!(mLastButtons & IN_DUCK) && *local->m_flDuckSpeed() < 8.0f)
				duck = false;

			float speed = mMaxSpeed / 3.0f;
			float length = mLocalVelocity.Length();

			if (length > speed)
				duck = false;

			if (duck)
				cmd->buttons |= IN_DUCK;
		}
	
		if (gVars.aim_autostop.value)
		{
			float speed = mMaxSpeed / 3.0f;
			float length = mLocalVelocity.Length();

			if (gVars.aim_autostop.value == 2 && length > speed)
			{
				float angle = mLocalVelocity.Yaw() + 180.0f;
				if (angle > 180.0f)
					angle -= 360.0f;

				mMoveYaw = angle;
			}
			else
			{
				if (gVars.aim_autoaction.value & (1 << 3) && length <= speed)
					cmd->move = cmd->move.Normal() * speed;
				else
					cmd->move = Vector(0.0f, 0.0f, 0.0f);
			}
		}
	}

	if (gVars.misc_revolver.value == 2)
	{
		CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
		if (weapon && *weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		{
			if (mLocalVelocity.LengthSqr() == 0.0f && mMoveDirection.LengthSqr() == 0.0f && cmd->select == 0 && !(cmd->buttons & IN_DUCK))
				cmd->buttons |= IN_AIRSTUCK;
		}
	}

	RunPrediction(local, cmd);

	//base->Warning("test %f %f\n",pred.Length(),mLocalVelocity.Length());

	gHvh.mUsingPitch = false;
	gHvh.mUsingYaw = false;

	mAliveAndProcessing = false;

	return true;
}

#ifdef TEST_IMPACTS
void gay(Entity* local, UserCmd* cmd)
{
	if (!IsEntityValid(gAim->mTarget.mPlayer))
		return;

	Matrix matrix[20];

	memcpy(matrix, gAim->mTarget.mBones, sizeof(Matrix) * 20);

	HitBox* box = gAim->GetBaseHitbox(gAim->mTarget.mPlayer);
	if (!box)
		return;

	for (int n = 0; n < 20; n++)
	{
		HitBox* bb = box + n;

		if (!bb)
			break;

		if (bb->radius == -1.0f)
			continue;

		Matrix& m = matrix[n];

		Vector min = matrix[n].Transform(bb->min);
		Vector max = matrix[n].Transform(bb->max);

	//	base->Warning("%x\n",(DWORD)(base->DrawCapsule));

		base->DrawCapsule(base->mDebugOverlay, &min, &max, &bb->radius, 255, 0, 0, 255, 4.0f);
	}
}
#endif

void CCSGOClient::Think(UserCmd* cmd)
{
	/*
		CCSGOClient::Think -> CCSGOClient::PreThink -> CCSAimbot::PreThink -> CAimPlayer::Setup
		CCSGOClient::Think -> CCSAimbot::Think
		CCSGOClient::Think -> CCSWeapon::Think
		CCSGOClient::Think -> CCSGOClient::WeaponThink
	*/

	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	if (!PreThink(local, cmd))
		return;

	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
	if (!weapon)
		return;

	mCurrentWeaponCfg = weapon->GetCurrentConfig();

	mAliveAndProcessing = true;

	if (!weapon->IsHitscan())
		cmd->move = mMoveDirection;

	mUsingAWP = *weapon->m_iItemDefinitionIndex() == WEAPON_AWP;

	if (mSwitchWeapons != 0)
	{
		bool switch_weps = false;

		if (mSwitchWeaponZeus)
		{
			if (mSwitchWeapons == 7)
				switch_weps = true;
		}
		else
		{
			if (mSwitchWeapons == 2)
				switch_weps = true;
		}

		if (switch_weps)
			cmd->select = mTargetWeaponIndex;
		else if (mSwitchWeapons == 1)
			cmd->select = mLastWeaponIndex;

		mSwitchWeapons -= 1;
	}

	CSWeaponInfo* info = weapon->GetWeaponInfo();
	
	///if (base->GetAsyncKeyState('C'))
	{
		//for (int i = 0; i < 0x868; i += 4)
		//	base->Warning("test %i %f\n",i, *(float*)((DWORD)(info) + i));

		//base->Warning("test %i %f %f %f\n",info->GetDamage(),info->GetPenetration(), info->GetRangeModifier(), info->GetMaxSpeed());
	}

	//base->Warning("test %f %f\n", weapon->GetSpread(), weapon->GetInaccuracy());

	int old_lag = mLag;

	if (gVars.aim_active.value)
	{
		gHvh.Think(local, cmd);

		Vector old_angles = cmd->angles;
		bool attack = false;

		if (weapon->PreThink(local, cmd))
		{
			cmd->angles = mViewAngles;

			weapon->AccuracyPreThink(local, cmd);

			gAim->Think(local, cmd);

			if (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2)
				attack = true;

			weapon->AccuracyThink(local, cmd);

			if ((cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2))
				cmd->buttons &= ~IN_AIRSTUCK;

			WeaponThink(local, weapon, cmd);
		}

		if (weapon->KnifePreThink(local, cmd))
		{
			cmd->angles = mViewAngles;

			if (cmd->buttons & IN_ATTACK && (mCurTime >= *weapon->m_flNextPrimaryAttack()))
				mFiring = true;
			else if (cmd->buttons & IN_ATTACK2 && (mCurTime >= *weapon->m_flNextSecondaryAttack()))
				mFiring = true;
		}

		if (weapon->GrenadePreThink(local, cmd))
		{
			cmd->angles = mViewAngles;

			if (gVars.misc_quicktoss.value && *weapon->m_bPinPulled() && !(cmd->buttons & IN_ATTACK))
				cmd->buttons &= ~(IN_ATTACK2);

			if (*weapon->m_fThrowTime() != 0.0f)
			{
				if (mCurTime >= *weapon->m_fThrowTime())
					mFiring = true;
			}
		}

		if ((cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2))
			cmd->buttons &= ~IN_AIRSTUCK;

		if (!gAim->mTarget.mPlayer || (mCopyCmd.buttons & IN_JUMP))
		{
			if (gVars.aim_autoaction.value & (1<<0))
			{
				if (mButtons & IN_DUCK)
					cmd->buttons |= IN_DUCK;
				else
					cmd->buttons &= ~IN_DUCK;
			}

			if (gVars.aim_autostop.value)
			{
				mMoveYaw = mMoveYawOverride;
				cmd->move = mMoveDirection;
			}
		}

		if (!mFiring)
		{
			if (!(gVars.aim_vac_kick.value && attack))
				cmd->angles = old_angles;

			if (!(gVars.aim_autostop.value && gAim->mTarget.mPlayer))
			{
				mMoveYaw = mMoveYawOverride;
				cmd->move = mMoveDirection;
			}
		}
		else
		{
			if (weapon->IsGrenade())
			{
				gHvh.mUsingPitch = false;
				gHvh.mUsingYaw = false;
				mLag = -1;
			}
			else if ((mLag == 0 && mHeldLag == 14) || (mLag > 0 && mHeldLag == 0))
			{
				if (!(*weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER))
				{
					if (!gVars.aim_vac_kick.value)
						cmd->angles = old_angles;

					cmd->move = mMoveDirection;
					cmd->buttons &= ~(IN_ATTACK|IN_ATTACK2);

					mFiring = false;
				}
			}
			else
			{
				if (gVars.aa_fakewalk.value && gVars.aa_fakewalk_shoot.value && gHvh.mFakewalkKey)
				{

				}
				else if (gVars.misc_psilent.value)
				{
					mLag = 1;
				}
				else
				{
					gHvh.mUsingPitch = false;
					gHvh.mUsingYaw = false;
					mLag = -1;
				}
			}

			if (mFiring)
			{
				if ((mCurTime - mShotTime) > (base->mGlobalVars->interval * 2.0f))
				{
					gAim->HandleFiring(local, cmd, old_angles);

					if (!gAim->mTarget.mPlayer && gVars.aim_resolver.value)
					{
						for (int i = 1; i <= base->mGlobalVars->clients; ++i)
						{
							Entity* player = base->GetClientEntity(base->mEntList, i);
							if (!IsEntityValid(player))
								continue;

							if (player == local)
								continue;

							CAimPlayer* aim_player = &gAim->mAimPlayers[i];
							aim_player->ResolveShot(player);
						}
					}
				}

				mFiredThisInterval = true;
				mShotTime = mCurTime;
#ifdef TEST_IMPACTS
				gay(local, cmd);
#endif
			}
		}

		if (gVars.aim_vac_kick.value && cmd->angles != mViewAngles)
		{
			Vector dt = cmd->angles - mOldAngles;

			float dot = cmd->angles.Forward().Dot(mOldAngles.Forward());

			if (dot < float_cos(DEG2RAD(30.0f)))
			{
				cmd->angles = mOldAngles + (dt.Normal() * 30.0f);
				cmd->buttons &= ~(IN_ATTACK|IN_ATTACK2);
				mFiring = false;
			}
		}

		mOldAngles = cmd->angles;
	}
	else
	{
		if (weapon->IsHitscan())
		{
			gLegit.Think(local, cmd);

			if (weapon->PreThink(local, cmd))
				WeaponThink(local, weapon, cmd);

			if (!mFiring)
				cmd->angles = gLegit.mTargetViewangles;

			if (gVars.l_antiaim.value && !(cmd->buttons & IN_ATTACK))
			{
				if (mLag == -1)
					mLag = 1;

				if (mHeldLag == 0 && mLag != -1)
				{
					Entity* best = nullptr;
					float dist = 8192.0f * 8192.0f;
					Vector forward = gClient->mViewAngles.Forward();

					for (int i = 1; i <= base->mGlobalVars->clients; ++i)
					{
						Entity* player = base->GetClientEntity(base->mEntList, i);
						if (!IsEntityValid(player))
							continue;

						if (!gLegit.IsValidTarget(local, player))
							continue;

						Vector org = *player->m_vecOrigin() + Vector(0.0f, 0.0f, 32.0f);
						float len = (org - gClient->mViewOrigin).LengthSqr();
						
						if (len > dist)
							continue;

						best = player;
						dist = len;
					}

					if (gVars.l_antiaim.value == 2 && best)
					{
						float ang = (*best->m_vecOrigin() - mViewOrigin).Yaw();
						ang += 90.0f;

						float dt = cmd->angles.y - ang;

						dt = float_fmod(dt, 360.0f);
						if (dt > 180.0f)
							dt -= 360.0f;

						if (float_abs(dt) < 90.0f)
							ang += 180.0f;

						cmd->angles.y = ang;
					}
					else
					{
						cmd->angles.y += 90.0f;
					}
				}
			}
		}
		else
		{
			gLegit.RcsSmoothBack(cmd);
		}
	}

	if (gAim->mDropPeriod != 0)
	{
		cmd->angles = mViewAngles;
		cmd->buttons &= ~(IN_ATTACK|IN_ATTACK2|IN_AIRSTUCK);
		mFiring = false;
		mLag = -1;

		gHvh.mUsingPitch = false;
		gHvh.mUsingYaw = false;

		if (gAim->mDropPeriod == 1)
			base->ConCommand(base->mBaseEngine, "drop");

		if (gAim->mDropPeriod > 0)
			gAim->mDropPeriod -= 1;
	}

	if (mFiring)
	{
		mShootAngles = cmd->angles;

		CAimbotFrame* aimframe = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];
		aimframe->mFiring = true;
	}

	if (mFiring && gVars.misc_accuracy_switch.value)
	{
		bool switch_weapons = false;
		int idx = *weapon->m_iItemDefinitionIndex();

		if (gVars.misc_accuracy_switch.value & (1<<0))
		{
			if (idx == WEAPON_DEAGLE)
				switch_weapons = true;
		}

		if (gVars.misc_accuracy_switch.value & (1<<1))
		{
			if (idx == WEAPON_REVOLVER)
				switch_weapons = true;
		}

		if (gVars.misc_accuracy_switch.value & (1<<2))
		{
			if (idx == WEAPON_AWP)
				switch_weapons = true;
		}

		if (gVars.misc_accuracy_switch.value & (1<<3))
		{
			if (idx == WEAPON_SSG08)
				switch_weapons = true;
		}

		if (switch_weapons)
		{
			for (int i = 0; i < 20; ++i)
			{
				DWORD wep = local->m_hMyWeapons()[i];
				if (wep == -1)
					continue;

				Entity* gun = base->GetClientEntityByHandle(base->mEntList, wep);
				if (!gun)
					continue;

				if (gun == weapon)
					continue;

				if (*gun->m_iItemDefinitionIndex() != WEAPON_TASER)
					continue;

				mSwitchWeapons = 7;
				mSwitchWeaponZeus = true;
				mTargetWeaponIndex = wep & 0xFFF;
				mLastWeaponIndex = (*local->m_hActiveWeapon()) & 0xFFF;

				break;
			}

			if (mSwitchWeapons == 0)
			{
				Entity* gun = base->GetClientEntityByHandle(base->mEntList, *local->m_hLastWeapon());

				if (gun)
				{
					mSwitchWeapons = 2;
					mSwitchWeaponZeus = false;
					mTargetWeaponIndex = (*local->m_hLastWeapon()) & 0xFFF;
					mLastWeaponIndex = (*local->m_hActiveWeapon()) & 0xFFF;
				}
			}
		}
	}

	mLastButtons = cmd->buttons;
	mLastFlags = mFlags;

	float movelen =	cmd->move.Length();

	if (movelen != 0.0f)
	{
		if (cmd->angles.y != mMoveYaw)
		{
			float angle = mMoveDirection.Yaw();
			if (angle > 180.0f)
				angle -= 360.0f;

			float view_yaw = mMoveYaw;
			float yaw = float_fmod(cmd->angles.y, 360.0f);

			angle += yaw - view_yaw;

			float_sincos(DEG2RAD(angle), &cmd->move.y, &cmd->move.x);
			cmd->move *= movelen;
		}
	}

	if (cmd->buttons & IN_AIRSTUCK)
	{
		cmd->ticks = 2147483647;
		mLag = -1;
	}

	if (mHeldLag == 0)
	{
		mAnimatedFrame = gAim->mFrameCount;
		mAnimatedFrameTime = mCurTime - mAnimatedCurTime;
		mAnimatedCurTime = mCurTime;
		mAnimatedVelocity = mOrigin - mAnimatedOrigin;
		mAnimatedOrigin = mOrigin;
		mAnimatedPitch = gHvh.mUsingPitch ? gHvh.mRotation.x : cmd->angles.x;
		mAnimatedYaw = gHvh.mUsingPitch ? gHvh.mRotation.y : cmd->angles.y;
		mAnimatedBodyYaw = *local->m_flLowerBodyYawTarget();
		mAnimatedViewangles = mViewAngles;
		mAnimatedFlags = mFlags;
		mAnimatedDuckAmount = mDuckAmount;
	}

	if (mLag != -1)
	{
		if (mLag < -1 || mLag > 14)
			mLag = 0;

		if (mLag > 0)
		{
			*mSendMove = false;
			mHeldLag += 1;

			mLag -= 1;
		}
		else
		{
			*mSendMove = true;
			mLastHeldLag = mHeldLag;
			mHeldLag = 0;

			if (gVars.aa_shoot.value && mFiredThisInterval)
			{
				mLag = gVars.aa_valvemm.value ? 10 : 14;
				gHvh.ShootAntiAimThink(local, cmd, mAnimatedFrame);
			}
			else
			{
				mLag -= 1;
			}

			mLastTickBase = mTickBase;
			mFiredThisInterval = false;
		}
	}
	else
	{
		*mSendMove = true;
		mHeldLag = 0;

		mLastTickBase = mTickBase;
		mFiredThisInterval = false;
	}

	gESP.GrenadeSimulationThink();
}

void CCSGOClient::RestrictCmd(UserCmd* cmd)
{
	// tickcount

	if (cmd->ticks < 0)
		cmd->ticks = 0;

	// pitch

	if (cmd->angles.x != cmd->angles.x)
		cmd->angles.x = 0.0f;

	if (cmd->angles.x > 89.0f)
		cmd->angles.x = 89.0f;

	if (cmd->angles.x < -89.0f)
		cmd->angles.x = -89.0f;

	// yaw

	if (cmd->angles.y != cmd->angles.y)
		cmd->angles.y = 0.0f;

	if (cmd->angles.y > 180.0f)
		cmd->angles.y -= 360.0f;

	if (cmd->angles.y < -180.0f)
		cmd->angles.y += 360.0f;

	if (cmd->angles.y > 180.0f)
		cmd->angles.y = 180.0f;

	if (cmd->angles.y < -180.0f)
		cmd->angles.y = -180.0f;

	// roll

	if (cmd->angles.z != cmd->angles.z)
		cmd->angles.z = 0.0f;

	if (cmd->angles.z > 50.0f)
		cmd->angles.z = 50.0f;

	if (cmd->angles.z < -50.0f)
		cmd->angles.z = -50.0f;

	// aimdir

	cmd->dir = Vector(0.0f, 0.0f, 0.0f);

	// move

	if (cmd->move.x != cmd->move.x)
		cmd->move.x = 0.0f;

	if (cmd->move.y != cmd->move.y)
		cmd->move.y = 0.0f;

	if (cmd->move.x > 450.0f)
		cmd->move.x = 450.0f;

	if (cmd->move.x < -450.0f)
		cmd->move.x = -450.0f;

	if (cmd->move.y > 450.0f)
		cmd->move.y = 450.0f;

	if (cmd->move.y < -450.0f)
		cmd->move.y = -450.0f;

	cmd->move.z = 0.0f;
}

void CCSGOClient::WeaponThink(Entity* local, CCSWeapon* weapon, UserCmd* cmd)
{
	if (*weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
	{
		if (mCurTime >= *weapon->m_flNextSecondaryAttack())
			mFireReady = true;

		if (gVars.aim_active.value && gVars.misc_autopistol.value)
		{
			if (!(mCurTime >= *weapon->m_flNextSecondaryAttack()))
				cmd->buttons &= ~IN_ATTACK2;
		}

		if (gVars.misc_revolver.value)
		{
			bool visible = true;

			if (gVars.misc_revolver_visible.value)
			{
				visible = false;

				for (int i = 1; i <= base->mGlobalVars->clients; ++i)
				{
					Entity* player = base->GetClientEntity(base->mEntList, i);
					if (!player)
						continue;

					if (player == local)
						continue;

					if (!gAim->IsValidTarget(local, player))
						continue;

					visible = true;
					break;
				}
			}

			if (visible && mCurTime < mNextRevolverFire)
				cmd->buttons |= IN_ATTACK;
		}

		if (!(cmd->buttons & IN_AIRSTUCK) && !(cmd->buttons & IN_ATTACK))
			mNextRevolverFire = 2147483647.0f;

		if (cmd->buttons & IN_ATTACK)
		{
			float post = mNextRevolverFire;

			if (mCurTime >= post)
				mFiring = true;

			if (!mFiring && mCurTime >= *weapon->m_flNextPrimaryAttack() && mCurTime >= *local->m_flNextAttack())
			{
				if (mNextRevolverFire == 2147483647.0f)
					mNextRevolverFire = mCurTime + 0.2f;
			}

			if ((mCurTime - *local->m_flNextAttack()) <= base->mGlobalVars->interval)
				mFiring = false;

			if (mCurTime < *weapon->m_flNextPrimaryAttack())
				mFiring = false;
		}
		else if (cmd->buttons & IN_ATTACK2)
		{
			if (mCurTime >= *weapon->m_flNextPrimaryAttack() && mCurTime >= *weapon->m_flNextSecondaryAttack())
				mFiring = true;
		}

		if (*local->m_iShotsFired() != 0)
		{
			if (cmd->buttons & IN_ATTACK2)
			{
				mFiring = false;
				mFireReady = false;
			}
		}
	}
	else
	{
		if (cmd->buttons & IN_ATTACK2)
		{
			if (weapon->HasScope())
				cmd->buttons &= ~IN_ATTACK;
			else
				cmd->buttons &= ~IN_ATTACK2;
		}

		if (mCurTime >= *weapon->m_flNextPrimaryAttack())
			mFireReady = true;

		if (gVars.aim_active.value && gVars.misc_autopistol.value)
		{
			if (weapon->IsSemiAuto() && !(mCurTime >= *weapon->m_flNextPrimaryAttack()))
				cmd->buttons &= ~IN_ATTACK;
		}

		if ((cmd->buttons & IN_ATTACK) && (mCurTime >= *weapon->m_flNextPrimaryAttack()))
			mFiring = true;

		if (*weapon->m_iItemDefinitionIndex() == WEAPON_GLOCK || *weapon->m_iItemDefinitionIndex() == WEAPON_FAMAS)
		{
			if (*weapon->m_bBurstMode())
			{
				float burst_time = *weapon->m_flNextBurstFire();

				if (burst_time != 0.0f && mCurTime >= burst_time)
					mFiring = true;
			}
		}

		if (weapon->IsSemiAuto())
		{
			if (*local->m_iShotsFired() != 0)
			{
				mFiring = false;
				mFireReady = false;
			}
		}
	}
}

void CCSGOClient::RunPrediction(Entity* local, UserCmd* cmd)
{
	bool frozen = mFrozen;

	mFrozen = false;
	mMaxSpeed = 250.0f;

	if (!base->mMoveHelper)
		return;

	float z = local->m_vecVelocity()->z;

	mTickBase = *local->m_nTickBase();
	mCurTime = (float)(*local->m_nTickBase()) * base->mGlobalVars->interval;

	int old_buttons = cmd->buttons;

	if (cmd->buttons & IN_DUCK)
	{
		if (gVars.misc_server.value & (1<<1))
			cmd->buttons &= ~IN_DUCK;
	}

	base->SetupMove(base->mPrediction, local, cmd, nullptr, mPredData.mMoveData);

	{
		// set our prerequisites
		float old_curtime = base->mGlobalVars->curtime;
		base->mGlobalVars->curtime = mCurTime;

		float old_frametime = base->mGlobalVars->frametime;
		base->mGlobalVars->frametime = base->mGlobalVars->interval;

		base->SetHost(base->mMoveHelper, local);

		*(float*)((DWORD)(mPredData.mMoveData) + 52) = 1.0f;

		// run the prediction
		base->ProcessMovement(base->mGameMovement, local, mPredData.mMoveData);

		if (*(float*)((DWORD)(mPredData.mMoveData) + 52) == 0.0f) // mv->m_flUpMove
			mFrozen = true;

		if (mFrozen && mFrozen != frozen)
			mFrozenTime = mCurTime;

		mMaxSpeed = *(float*)((DWORD)(mPredData.mMoveData) + 56);

		base->SetHost(base->mMoveHelper, nullptr);

		base->mGlobalVars->curtime = old_curtime;
		base->mGlobalVars->frametime = old_frametime;
	}

	base->FinishMove(base->mPrediction, local, cmd, mPredData.mMoveData);

	mOldViewOrigin = mViewOrigin;
	mViewOrigin = *local->m_vecOrigin() + *local->m_vecViewOffset();
	mViewOriginPredicted = mViewOrigin;
	mOrigin = *local->m_vecOrigin();

	if (*local->m_fFlags() & FL_ONGROUND)
		mViewOrigin.z += mViewZAddDelta;

	mLocalVelocity = *local->m_vecVelocity();
	mAimPunch = *local->m_aimPunchAngle();
	
	mFlags = *local->m_fFlags();

	mDuckAmount = *local->m_flDuckAmount();
	mDuckSpeed = *local->m_flDuckSpeed();

	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
	if (weapon)
	{
		if (weapon->IsHitscan())
			weapon->UpdateAccuracyPenalty(); // update m_fAccuracyPenalty

		mSpread = weapon->GetSpread();
		mInaccuracy = weapon->GetInaccuracy();
	}

	*local->m_nTickBase() += 1;

	if (mLocalVelocity.Length() >= 1.0f)
		mLastTimeMoved = mCurTime;

	if (gVars.misc_scout.value)
	{
		CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
		if (weapon && *weapon->m_iItemDefinitionIndex() == WEAPON_SSG08)
		{
			if (mCopyCmd.buttons & IN_JUMP && mLocalVelocity.z < 0.0f && z > 0.0f)
				cmd->buttons |= IN_AIRSTUCK;
		}
	}

	// store our predicted data for next execution
	if (!(cmd->buttons & IN_AIRSTUCK))
	{
		base->SetupMove(base->mPrediction, local, cmd, nullptr, mPredData.mMoveData);

		mPredData.mFlags = *local->m_fFlags();
		mPredData.mGroundEntity = *local->m_hGroundEntity();
		mPredData.mTickBase = *local->m_nTickBase();
		mPredData.mAimPunch = *local->m_aimPunchAngle();
		mPredData.mDuckAmount = *local->m_flDuckAmount();
		mPredData.mDuckSpeed = *local->m_flDuckSpeed();
	}

	cmd->buttons = old_buttons;

	// restore the old move data
	base->FinishMove(base->mPrediction, local, cmd, mOldData.mMoveData);

	*local->m_fFlags() = mOldData.mFlags;
	*local->m_hGroundEntity() = mOldData.mGroundEntity;
	*local->m_nTickBase() = mOldData.mTickBase;
	*local->m_aimPunchAngle() = mOldData.mAimPunch;
	*local->m_flDuckAmount() = mOldData.mDuckAmount;
	*local->m_flDuckSpeed() = mOldData.mDuckSpeed;

	/*if (mHeldLag == 0)
	{
		mAnimatedFrame = gAim->mFrameCount;
		mAnimatedFrameTime = mCurTime - mAnimatedCurTime;
		mAnimatedCurTime = mCurTime;
		mAnimatedVelocity = mOrigin - mAnimatedOrigin;
		mAnimatedOrigin = mOrigin;
		mAnimatedBodyYaw = *local->m_flLowerBodyYawTarget();
		mAnimatedViewangles = mViewAngles;
		mAnimatedFlags = mFlags;
		mAnimatedDuckAmount = mDuckAmount;

		//UpdateLocalHitboxesData();
	}*/

	CAimbotFrame* frame = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];
	frame->mTickCount = mTickCount;
	frame->mViewOrigin = mViewOrigin;
	frame->mMoveDirection = mMoveDirection;
	frame->mMoveYaw = mMoveYaw;
	frame->mUserCmd = cmd;
	frame->mCopyCmd = mCopyCmd;
	frame->mFiring = false;
}

void CCSGOClient::StraferThink(Entity* local, UserCmd* cmd)
{
	if (!gVars.move_strafer.value)
		return;

	if (*local->m_MoveType() != MOVETYPE_WALK)
		return;

	if (cmd->buttons & 0x40000000)
		return;

	mStrafeIndex += 1;

	Vector velocity = *local->m_vecVelocity();
	velocity.z = 0.0f;

	float len = velocity.Length();
	float strafe = 0.0f;

	float ang1 = 90.0f;
	float ang2 = 90.0f;

	if (len > 0.0f)
	{
		float result = RAD2DEG(float_sin(14.999999f / len));
		if (result < 90.0f)
			ang1 = result;
	}
	
	if (len > 0.0f)
	{
		float result = RAD2DEG(float_sin(30.0f / len));
		if (result < 90.0f)
			ang2 = result;
	}

	static int rotate = 0;

	if (gVars.move_leftright.value)
	{
		float angle = ang2;
		float base = angle;

		static float offset = 0.0f;

		if (gVars.move_leftright.value)
		{
			if (mLeftStrafe)
				rotate = 1;
			else if (mRightStrafe)
				rotate = 2;
		}

		if (!(gClient->mCopyCmd.buttons & IN_JUMP))
		{
			rotate = 0;
			offset = 0.0f;
		}

		if (rotate == 1)
		{
			float minang = (float)(gVars.move_left_speed.value) * 0.1f;

			if (minang != 0.0f)
				angle = max(angle, minang);
		}
		else if (rotate == 2)
		{
			float minang = (float)(gVars.move_right_speed.value) * 0.1f;

			if (minang != 0.0f)
				angle = max(angle, minang);
		}
		
		if (angle > base)
		{
			if (gVars.move_leftright_accel.value)
			{
				int index = gVars.move_leftright_accel.value;
				if (index < 1)
					index = 1;

				if ((mStrafeIndex % index) == 0)
					angle = base;
			}
		}

		if (rotate == 1)
			offset += angle;
		else if (rotate == 2)
			offset -= angle;

		if (offset > 360.0f || offset < -360.0f)
		{
			rotate = 0;

			if (mLeftStrafe || mRightStrafe)
			{
				if (offset > 360.0f)
					offset -= 360.0f;

				if (offset < -360.0f)
					offset += 360.0f;
			}
			else
			{
				offset = 0.0f;
			}
		}

		float yaw = offset;

		if (yaw > 180.0f)
			yaw -= 360.0f;

		if (yaw < -180.0f)
			yaw += 360.0f;

		strafe = yaw;

		if (rotate == 2)
			cmd->move.y = 1.0f;
		else if (rotate == 1)
			cmd->move.y = -1.0f;
	}
	else
	{
		rotate = 0;
	}

	float mouse_delta = mMouseYawDelta;
	mouse_delta /= max(mFrameMult, 1.0f);

	if (cmd->buttons & IN_JUMP && rotate == 0)
	{
		float delta_abs = mouse_delta;
		if (delta_abs < 0.0f)
			delta_abs -= delta_abs * 2.0f;

		if (delta_abs < ang1 && gVars.move_speed.value)
		{
			strafe = ang1 * ((mStrafeIndex & 1) ? -1.0f : 1.0f);

			if (strafe > 0.0f)
				cmd->move.y = 1.0f;
			else if (strafe < 0.0f)
				cmd->move.y = -1.0f;
		}
		else
		{
			if (gVars.move_speed.value)
			{
				if (delta_abs > ang2)
				{
					float add = (delta_abs - ang2) * ((mouse_delta > 0.0f) ? 1.0f : -1.0f);

					strafe = add * -1.0f;
				}
				else
				{
					strafe = 0.0f;
				}
			}

			if (mouse_delta < 0.0f)
				cmd->move.y = 450.0f;
			else if (mouse_delta > 0.0f)
				cmd->move.y = -450.0f;
		}
	}

	if (strafe != 0.0f)
	{
		Vector move = cmd->move;
		cmd->move = Vector(0.0f, 0.0f, 0.0f);

		float angle = move.Yaw();
		if (angle > 180.0f)
			angle -= 360.0f;

		angle += strafe * -1.0f;

		if (angle > 180.0f)
			angle -= 360.0f;

		if (angle < -180.0f)
			angle += 360.0f;

		float_sincos(DEG2RAD(angle), &cmd->move.y, &cmd->move.x);
		cmd->move *= 450.0f;
	}
}

void CCSGOClient::CalculateMouse(UserCmd* cmd)
{
	static float last_yaw = 0.0f;
	static float last_pitch = 0.0f;

	static float last_yaw2 = 0.0f;
	static float last_pitch2 = 0.0f;

	mMouseLastPitch = last_pitch2;
	mMouseLastYaw = last_yaw2;

	last_pitch2 = cmd->angles.x;
	last_yaw2 = cmd->angles.y;

	float pitch = cmd->angles.x;
	if (pitch < 0.0f)
		pitch += 360.0f;

	float yaw = cmd->angles.y;
	if (yaw < 0.0f)
		yaw += 360.0f;

	{
		float x = pitch - last_pitch;
		if (x < 0.0f)
			x += 360.0f;

		x = float_fmod(x, 360.0f);
		if (x > 180.0f)
			x -= 360.0f;

		last_pitch = pitch;

		mMousePitchDelta = x;
	}

	{
		float y = yaw - last_yaw;
		if (y < 0.0f)
			y += 360.0f;

		y = float_fmod(y, 360.0f);
		if (y > 180.0f)
			y -= 360.0f;

		last_yaw = yaw;

		mMouseYawDelta = y;
	}
}

void CCSGOClient::SolvePitchYaw(UserCmd* cmd, Vector spread)
{
	float sin = 0.0f;
	float cos = 0.0f;
	float_sincos(DEG2RAD(cmd->angles.x), &sin, &cos);

	Vector dir = Vector(cos, 0.0f, 0.0f);

	dir += Vector(0.0f, -1.0f, 0.0f) * spread.x;
	dir += Vector(sin, 0.0f, 0.0f) * spread.y;

	float yaw = dir.Yaw();
	float pitch = RAD2DEG(float_atan2(-spread.y, 1.0f));

	gClient->mSpreadAngles.x = pitch;
	gClient->mSpreadAngles.y = yaw;
}

int CCSGOClient::Cfg(SettingVar item)
{
	return item.group_vars[mCurrentWeaponCfg].value;
}

void CCSGOClient::UpdateAnimations()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!local)
		return;

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* player = base->GetClientEntity(base->mEntList, i);
		if (!IsEntityValid(player))
		{
			CAimPlayer* a = &gAim->mAimPlayers[i];
			memset(a, 0, sizeof(CAimPlayer));

			continue;
		}

		if (player == local)
			continue;

		CAimPlayer* aim_player = &gAim->mAimPlayers[i];

		if (!gAim->IsValidTarget(local, player))
		{
			aim_player->mAnimating = false;
			memset(aim_player->mAnimStateRegion, 0, SIZEOF_ANIMSTATE_REGION);

			continue;
		}

		aim_player->Setup(player);
	}
}

void CCSGOClient::NetUpdateThink()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	{
		float sim = *local->m_flSimulationTime();

		if ((sim - mLastLocalSimulation) < 0.0f)
			mLastLocalSimulation = sim;

		if (sim > mLastLocalSimulation)
		{
			mLastLocalSimulation = sim;

			memcpy(mAnimatedAnimLayers, local->m_AnimOverlay(), sizeof(AnimLayer) * MAX_ANIM_LAYERS);
		}
	}

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* player = base->GetClientEntity(base->mEntList, i);
		if (!IsEntityValid(player))
		{
			CAimPlayer* a = &gAim->mAimPlayers[i];
			memset(a, 0, sizeof(CAimPlayer));

			continue;
		}

		if (player == local)
			continue;

		CAimPlayer* aim_player = &(gAim->mAimPlayers[i]);
		aim_player->DataThink(player);

		if (gAim->IsValidTarget(local, player))
		{
			if (aim_player->mAnimating)
				*player->m_bClientSideAnimation() = false;

			aim_player->mAnimating = *player->m_bClientSideAnimation();
		}
		else
		{
			*player->m_bClientSideAnimation() = true;
			aim_player->mAnimating = true;

			memset(aim_player->mAnimStateRegion, 0, SIZEOF_ANIMSTATE_REGION);
			aim_player->mUseBodyReference = false;
		}

		*player->GetRenderOrigin() = *(Vector*)((DWORD)(player) + 0x39c + 0xC);
		//m_flShadowCastDistance
	}
}

void CCSGOClient::UpdateLocalAnimation(Entity* local, float pitch, float yaw)
{
	float old_curtime = base->mGlobalVars->curtime;
	float old_frametime = base->mGlobalVars->frametime;
	int old_framecount = base->mGlobalVars->framecount;
	base->mGlobalVars->curtime = mAnimatedCurTime;
	base->mGlobalVars->frametime = mAnimatedFrameTime;
	base->mGlobalVars->framecount = (int)((base->mGlobalVars->curtime / base->mGlobalVars->frametime) + 0.5f);

	Vector old_origin = *local->m_vecOrigin();
	float old_body = *local->m_flLowerBodyYawTarget();
	Vector old_vel = *local->m_vecVelocity();
	int old_flags = *local->m_fFlags();
	float old_duckamount = *local->m_flDuckAmount();

	*local->m_vecOrigin() = mOrigin;
	*local->m_flLowerBodyYawTarget() = mAnimatedBodyYaw;
	*local->m_vecVelocity() = mAnimatedVelocity;
	*local->m_fFlags() = mAnimatedFlags;
	*local->m_flDuckAmount() = mAnimatedDuckAmount;

	Vector angles = Vector(pitch, yaw, 0.0f);
	base->SetLocalViewAngles(base->mPrediction, &angles);

	local->GetAngRender()->y = yaw;

	local->UpdateClientSideAnimation();

	*local->m_vecOrigin() = old_origin;
	*local->m_flLowerBodyYawTarget() = old_body;
	*local->m_vecVelocity() = old_vel;
	*local->m_fFlags() = old_flags;
	*local->m_flDuckAmount() = old_duckamount;

	base->mGlobalVars->curtime = old_curtime;
	base->mGlobalVars->frametime = old_frametime;
	base->mGlobalVars->framecount = old_framecount;

	memcpy(local->m_iv_flPoseParameter(), local->m_flPoseParameter(), sizeof(float) * MAX_POSE_PARAMETERS);
}

/*void CCSGOClient::UpdateLocalHitboxesData()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	if (gVars.esp_hitbox.value & (1<<0))
	{
		UpdateLocalAnimation(local, mAnimatedPitch, mAnimatedYaw);

	//	memcpy(mHitboxParams[0], local->m_flPoseParameter(), sizeof(float) * MAX_POSE_PARAMETERS);
		//mHitboxYaw[0] = local->GetAngRender()->y;
	}

	if (gVars.esp_hitbox.value & (1<<1))
	{
		UpdateLocalAnimation(local, mAnimatedPitch, local->m_angEyeAngles()->y);

	//	memcpy(mHitboxParams[1], local->m_flPoseParameter(), sizeof(float) * MAX_POSE_PARAMETERS);
		//mHitboxYaw[1] = local->GetAngRender()->y;
	}

	if (gVars.esp_hitbox.value & (1<<2))
	{
		UpdateLocalAnimation(local, mAnimatedPitch, *local->m_flLowerBodyYawTarget());

		//memcpy(mHitboxParams[2], local->m_flPoseParameter(), sizeof(float) * MAX_POSE_PARAMETERS);
	//	mHitboxYaw[2] = local->GetAngRender()->y;
	}

	{
		float pitch = mAnimatedViewangles.x;
		float yaw = mAnimatedViewangles.y;

		if (gVars.esp_rotation.value)
		{
			pitch = mAnimatedPitch;

			if (gVars.esp_rotation.value == 1)
				yaw = mAnimatedYaw;
			else if (gVars.esp_rotation.value == 2)
				yaw = local->m_angEyeAngles()->y;
			else if (gVars.esp_rotation.value == 3)
				yaw = *local->m_flLowerBodyYawTarget();
		}

	//	UpdateLocalAnimation(local, pitch, yaw);
	}
}*/

void CCSGOClient::UpdateLocalPlayerAnimations()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	AnimLayer old_layers[MAX_ANIM_LAYERS];
	memcpy(old_layers, local->m_AnimOverlay(), sizeof(AnimLayer) * MAX_ANIM_LAYERS);

	memcpy(local->m_AnimOverlay(), mAnimatedAnimLayers, sizeof(AnimLayer) * MAX_ANIM_LAYERS);

	Vector org = *local->GetRenderOrigin();
	Vector ang = *local->GetRenderAngles();

	*local->GetRenderOrigin() = mOrigin;

	float old_time = base->mGlobalVars->curtime;
	float old_frametime = base->mGlobalVars->frametime;

	base->mGlobalVars->curtime = 0.0f;
	base->mGlobalVars->frametime = base->mGlobalVars->interval;

	BYTE old = *(BYTE*)((DWORD)(local)+0x39e1);

	if (old != 0)
		*(BYTE*)((DWORD)(local) + 0x39e1) = 0;

	if (gVars.esp_hitbox.value & (1<<0))
	{
		*local->GetRenderAngles() = Vector(0.0f, mAnimatedYaw, 0.0f);

		local->m_flPoseParameter()[12] = (mAnimatedPitch - -90.0f) / (90.0f - -90.0f);
		local->m_flPoseParameter()[11] = 0.5f;

		{
			*local->m_iMostRecentModelBoneCounter() = -1;
			*local->m_flLastBoneSetupTime() = -1.0f;
			
			local->SetupBones(mLocalBoneMatrix, 128, 0x100, 0.0f);
		}
	}

	if (gVars.esp_hitbox.value & (1<<1))
	{
		*local->GetRenderAngles() = Vector(0.0f, local->m_angEyeAngles()->y, 0.0f);

		local->m_flPoseParameter()[12] = (local->m_angEyeAngles()->x - -90.0f) / (90.0f - -90.0f);
		local->m_flPoseParameter()[11] = 0.5f;

		{
			*local->m_iMostRecentModelBoneCounter() = -1;
			*local->m_flLastBoneSetupTime() = -1.0f;

			local->SetupBones(mLocalBoneMatrixServer, 128, 0x100, 0.0f);
		}
	}

	if (gVars.esp_hitbox.value & (1<<2))
	{
		*local->GetRenderAngles() = Vector(0.0f, *local->m_flLowerBodyYawTarget(), 0.0f);

		local->m_flPoseParameter()[12] = (mAnimatedPitch - -90.0f) / (90.0f - -90.0f);
		local->m_flPoseParameter()[11] = 0.5f;

		{
			*local->m_iMostRecentModelBoneCounter() = -1;
			*local->m_flLastBoneSetupTime() = -1.0f;

			local->SetupBones(mLocalBoneMatrixBody, 128, 0x100, 0.0f);
		}
	}

	{
		float pitch = mAnimatedViewangles.x;
		float yaw = mAnimatedViewangles.y;

		if (gVars.esp_rotation.value)
		{
			pitch = mAnimatedPitch;

			if (gVars.esp_rotation.value == 1)
				yaw = mAnimatedYaw;
			else if (gVars.esp_rotation.value == 2)
				pitch = local->m_angEyeAngles()->x, yaw = local->m_angEyeAngles()->y;
			else if (gVars.esp_rotation.value == 3)
				yaw = *local->m_flLowerBodyYawTarget();
		}

		*local->GetRenderAngles() = Vector(0.0f, yaw, 0.0f);

		local->m_flPoseParameter()[12] = (pitch - -90.0f) / (90.0f - -90.0f);
		local->m_flPoseParameter()[11] = 0.5f;

		{
			*local->m_iMostRecentModelBoneCounter() = -1;
			*local->m_flLastBoneSetupTime() = -1.0f;

			local->SetupBones(nullptr, 128, 0x100, 0.0f);
		}

		Vector angles = Vector(pitch, yaw, 0.0f);
		base->SetLocalViewAngles(base->mPrediction, &angles);
	}
	
	if (old != 0)
		*(BYTE*)((DWORD)(local) + 0x39e1) = old;

	base->mGlobalVars->curtime = old_time;
	base->mGlobalVars->frametime = old_frametime;

	*local->GetRenderOrigin() = org;
	*local->GetRenderAngles() = ang;

	memcpy(local->m_AnimOverlay(), old_layers, sizeof(AnimLayer) * MAX_ANIM_LAYERS);
}

void CCSGOClient::FixEngineSetupBones()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (IsEntityValid(local))
	{
		for (int i = 1; i <= base->mGlobalVars->clients; ++i)
		{
			Entity* player = base->GetClientEntity(base->mEntList, i);
			if (!IsEntityValid(player))
				continue;

			if (player == local)
				continue;

			if (player != local)
			{
				if (!gVars.render_noteam.value)
				{
					if (*player->m_iTeamNum() == *local->m_iTeamNum())
						continue;
				}
			}
			
			*player->m_flLastBoneSetupTime() = float_sqrt(-1.0f);
		}
	}
}