#include "dll.h"

CHvhManager gHvh;

void CHvhManager::Think(Entity* local, UserCmd* cmd)
{
	mLagging = false;

	if (gVars.aim_active.value)
	{
		FakelagThink(local, cmd);
		AntiAimThink(local, cmd);
	}
}

bool CHvhManager::ShouldUseAntiAim(Entity* local, UserCmd* cmd)
{
	if (cmd->buttons & IN_USE)
		return false;

	if (*local->m_MoveType() != MOVETYPE_WALK)
		return false;

	if (*local->m_nWaterLevel() > 1)
		return false;

	return true;
}

void CHvhManager::AntiAimThink(Entity* local, UserCmd* cmd)
{
	mFakingYaw = false;

	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
	if (!weapon)
		return;

//	if (!weapon->IsKnife() && *weapon->m_iClip1() == -1)
	//	return;

	if (!ShouldUseAntiAim(local, cmd))
		return;

	if (!gVars.aa_active.value)
		return;

	if (gClient->mHeldLag == 0)
	{
		mIncr += 1;

		if (gVars.aa_jitter_delay.value == 0)
		{
			if (base->RandomInt(0, 1) == 0)
				mJitter = !mJitter;
		}
		else
		{
			if ((mIncr % (1 + gVars.aa_jitter_delay.value)) == 1)
				mJitter = !mJitter;
		}
	}

	if (gVars.aa_pitch.value)
	{
		if (gVars.aa_pitch.value == 3)
			cmd->angles.x = -89.0f;
		else if (gVars.aa_pitch.value == 2)
			cmd->angles.x = 0.0f;
		else
			cmd->angles.x = 89.0f;
		
		mUsingPitch = true;
	}

	if (gVars.aa_yaw.value)
	{
		Entity* best = nullptr;
		float base_angle = gClient->mViewAngles.y;
		float thru = 0.0f;
		Vector best_origin;

		{
			float dist = (gVars.aa_base.value == 2) ? -1.0f : 8192.0f * 8192.0f;
			Vector forward = gClient->mViewAngles.Forward();

			for (int i = 1; i <= base->mGlobalVars->clients; ++i)
			{
				Entity* player = base->GetClientEntity(base->mEntList, i);
				if (!IsEntityValid(player))
					continue;

				if (!gAim->IsValidTarget(local, player))
					continue;

				float len = 0.0f;
				Vector org = *player->m_vecOrigin() + Vector(0.0f, 0.0f, 32.0f);

				if (gVars.aa_base.value == 2)
					len = (org - gClient->mViewOrigin).Normal().Dot(forward);
				else
					len = (org - gClient->mViewOrigin).LengthSqr();

				PlayerObject* obj = gPlayers.GetObject(player->GetIndex());

				if (obj)
				{
					if (obj->filter == 1)
						continue;

					if (obj->filter == 2)
						len = gVars.aa_base.value == 2 ? 9999.0f : -1.0f;
				}

				if (gVars.aa_base.value == 2)
				{
					if (len < dist)
						continue;
				}
				else
				{
					if (len > dist)
						continue;
				}

				best = player;
				dist = len;
			}

			if (best)
			{
				thru = (*best->m_vecOrigin() - gClient->mViewOrigin).Yaw();
				best_origin = *best->m_vecOrigin() + Vector(0.0f, 0.0f, 64.0f);
			}

			if (gVars.aa_base.value)
			{
				if (gVars.aa_base.value == 3)
				{
					cmd->angles.y = 0.0f;
					base_angle = 0.0f;
				}
				else if (gVars.aa_base.value <= 2 && best)
				{
					cmd->angles.y = thru;
					base_angle = thru;

					if (cmd->angles.y > 180.0f)
						cmd->angles.y -= 360.0f;

					if (cmd->angles.y < -180.0f)
						cmd->angles.y += 360.0f;
				}
			}
		}

		float yaw = 0.0f;
		int angle = 0;

		float len = Vector(gClient->mLocalVelocity.x, gClient->mLocalVelocity.y, 0.0f).LengthSqr();
		float len2 = Vector(gClient->mLastVelocity.x, gClient->mLastVelocity.y, 0.0f).LengthSqr();
		
		{
			int idelta = gVars.aa_yaw_delta.value;
			if (idelta == 0)
				idelta = 180.0f;

			float delta = (float)(idelta);

			switch (gVars.aa_yaw.value)
			{
				case 1:
				{
					yaw = 180.0f;
					break;
				}
				case 2:
				{
					yaw = mJitter ? 90.0f : 90.0f + delta;
					break;
				}
				case 3:
				{
					yaw = float_fmod(30.0f * (float)(mIncr), 360.0f);
					break;
				}
				case 4:
				{
					yaw += 180.0f;
					yaw = float_fmod(30.0f * (float)(mIncr), 360.0f);

					int i = 3;

					if (mIncr % (24 * i) < (12 * i))
						yaw *= -1.0f;

					break;
				}
				case 5:
				{
					yaw = 180.0f + (float)(base->RandomInt(idelta * -1, idelta));
					break;
				}
				case 6:
				{
					yaw = 180.0f + (float)(base->RandomInt(0, idelta));
					break;
				}
				case 7:
				{
					yaw = 180.0f - (float)(base->RandomInt(0, idelta));
					break;
				}
			}

			yaw += (float)(gVars.aa_yaw_base.value);

			if (gVars.aa_yaw_move.value && len > 1.0f)
				yaw = (float)(gVars.aa_yaw_move.value);

			if (gVars.aa_velocity.value)
			{
				if (len2 < 1.0f)
				{
					if (gClient->mCurTime > mNextVelCheck)
					{
						if (gClient->mMoveDirection.y < 0.0f)
							mVelAngle = 2;
						else if (gClient->mMoveDirection.y > 0.0f)
							mVelAngle = 1;
						else
							mVelAngle = 0;
					}
				}
				else
				{
					mNextVelCheck = gClient->mCurTime + 0.1f;
				}

				angle = mVelAngle;
			}

			if (gVars.aa_left_key.value && gHvh.mLeftKey)
				angle = 1;
			else if (gVars.aa_right_key.value && gHvh.mRightKey)
				angle = 2;

			if (angle == 1)
				yaw = 90.0f;
			else if (angle == 2)
				yaw = 270.0f;

			yaw += base_angle;
		}
		
		if (gVars.aa_faceaim.value)
		{
			TraceResult tr;

			RayData ray;

			TraceFilter filter;
			filter.noplayers = true;

			bool hit1 = false;
			bool hit2 = false;

			float len_90 = 0.0f;
			float len_270 = 0.0f;

			float side_angle_90 = 0.0f;
			float side_angle_270 = 0.0f;

			Vector targetpos = best ? best_origin : (gClient->mViewOrigin + (gClient->mViewAngles.Forward() * 512.0f));
			float face_angle = base_angle;

			Vector headpos_90 = gClient->mViewOrigin + (Vector(0.0f, face_angle, 0.0f).Right() * 8.0f * -1.0f);
			Vector headpos_270 = gClient->mViewOrigin + (Vector(0.0f, face_angle, 0.0f).Right() * 8.0f);

			{
				ray.base = headpos_90;
				ray.delta = targetpos - ray.base;

				base->TraceRay(base->mEngineTrace, &ray, 0x4600400B, &filter, &tr);

				if (tr.fraction != 1.0f)
				{
					hit1 = true;
					len_90 = ray.delta.Length() * (1.0f - tr.fraction);
					side_angle_90 = tr.normal.Yaw();
				}
			}

			{
				ray.base = headpos_270;
				ray.delta = targetpos - ray.base;

				base->TraceRay(base->mEngineTrace, &ray, 0x4600400B, &filter, &tr);

				if (tr.fraction != 1.0f)
				{
					hit2 = true;
					len_270 = ray.delta.Length() * (1.0f - tr.fraction);
					side_angle_270 = tr.normal.Yaw();
				}
			}

			int avoid = 0;

			if (!hit1 || !hit2)
			{
				if (!hit1 && hit2)
					avoid = 1;
				else if (hit1 && !hit2)
					avoid = 2;
			}
			else
			{
				if (len_90 > len_270)
					avoid = 2;
				else if (len_270 > len_90)
					avoid = 1;
			}

			if (avoid == 1)
				yaw = side_angle_90 + 90.0f;
			else if (avoid == 2)
				yaw = side_angle_270 - 90.0f;
		}

		static bool updated_this_interval = false;
		static float body_yaw = 0.0f;

		if (gVars.aa_body.value && gClient->mHeldLag == 0)
		{
			updated_this_interval = false;

			bool update = false;
			bool pre_update = false;
			static bool post_update = false;

			float curtime = gClient->mCurTime;
			float frametime = curtime - gClient->mAnimatedCurTime;

			if (len > 1.0f)
			{
				if (gVars.aa_body_last.value && gClient->mMoveDirection.LengthSqr() == 0.0f && GetDecayedVel(local, gVars.aa_body_last.value) < 1.0f)
					update = true;

				if (!(gClient->mFlags & FL_ONGROUND && gClient->mLastFlags & FL_ONGROUND) && gClient->mAnimatedFlags & FL_ONGROUND)
					update = true;
				else
					mLastStop = curtime + 0.22f;
			}
			else
			{
				if (curtime > mLastStop)
				{
					update = true;
					mLastStop = curtime + 1.1f;
				}
				else
				{
					if (curtime > (mLastStop - frametime))
						pre_update = true;
				}
			}

			if ((gVars.aa_body_update.value && post_update) || update || (gVars.aa_body.value != 4 && pre_update))
			{
				if (gVars.aa_body.value == 1)
					yaw = gClient->mViewAngles.y;
				else if (gVars.aa_body.value == 2)
					yaw = base_angle;
				else if (gVars.aa_body.value == 3)
					yaw = 0.0f;

				yaw += (float)(gVars.aa_body_yaw.value);

				if (gVars.aa_body_wrap.value)
				{
					if (angle == 1)
						yaw = base_angle + 90.0f;
					else if (angle == 2)
						yaw = base_angle + 270.0f;
				}

				if (gVars.aa_body.value != 4 && pre_update)
					yaw += 180.0f;

				updated_this_interval = true;
				body_yaw = yaw;

				if (update)
					mBodyIncr += 1;
			}

			post_update = update;

			//base->Warning("Test %f %f %i %f\n", yaw, *local->m_flLowerBodyYawTarget(), update ? 1 : 0,gClient->mLocalVelocity.Length());
		}

		cmd->angles.y = yaw;
			
		if (gClient->mHeldLag == 0)
		{
			mRotation.y = float_fmod(cmd->angles.y, 360.0f);
			if (mRotation.y > 180.0f)
				mRotation.y -= 360.0f;
		}

		if (gVars.aa_yaw_fake.value)
		{
			yaw = base_angle;

			if (gVars.aa_yaw_fake.value == 2)
				yaw += 90.0f;
			else if (gVars.aa_yaw_fake.value == 3)
				yaw += -90.0f;
			else if (gVars.aa_yaw_fake.value == 4)
				yaw += 45.0f;
			else if (gVars.aa_yaw_fake.value == 5)
				yaw = base->RandomFloat(-180.0f, 180.0f);
			else if (gVars.aa_yaw_fake.value == 6)
				yaw = gClient->mViewAngles.y;

			bool hold = false;

			if (gVars.aim_vac_kick.value)
			{
				if (gClient->mHeldLag > 6)
					hold = true;
			}
			else
			{
				if (gClient->mHeldLag != 0)
					hold = true;
			}

			if (updated_this_interval)
				yaw = body_yaw;

			if (hold)
				cmd->angles.y = yaw;

			mFakingYaw = true;
		}

		mUsingYaw = true;
	}

	cmd->angles.y = float_fmod(cmd->angles.y, 360.0f);
	if (cmd->angles.y > 180.0f)
		cmd->angles.y -= 360.0f;

	mRotation.x = cmd->angles.x;
}

void CHvhManager::FakelagThink(Entity* local, UserCmd* cmd)
{
	if (cmd->buttons & 0x40000000)
		return;

	bool active = gVars.aa_jump_lag.value != 0;

	if (!(gClient->mCopyCmd.buttons & IN_JUMP))
		active = false;

	Vector vel = *local->m_vecVelocity();
	vel.z = 0.0f;

	if (!active)
	{
		{
			int factor = gVars.aa_stand_lag.value;

			if (IsAntiAimFake())
			{
				if (gVars.aim_vac_kick.value)
				{
					if (factor < 12)
						factor = 12;
				}
				else
				{
					if (factor < 1)
						factor = 1;
				}
			}

			if (gVars.aa_valvemm.value)
			{
				if (factor > 10)
					factor = 10;
			}

			if (gClient->mLag == -1)
			{
				gClient->mLag = factor;
				mLagSet = factor;
			}
		}

		mFreezeSwitch = false;

		return;
	}

	int max_ticks = 15;
	int highest_ticks = 14;

	{
		Vector vel = *local->m_vecVelocity();
		float len = Vector(vel.x, vel.y, 0.0f).Length();

		if (len < 1.0f)
			len = 1.0f;

		max_ticks = (int)(ceil((64.0f / base->mGlobalVars->interval) / len));

		if (max_ticks < 1)
			max_ticks = 1;

		if (max_ticks > 15)
			mLagging = false;
		else
			mLagging = true;

		if (max_ticks > 15)
			max_ticks = 15;

		max_ticks -= 1;
	}

	if (gVars.aa_valvemm.value)
	{
		if (max_ticks > 10)
			max_ticks = 10;

		highest_ticks = 10;
	}

	if (gClient->mLag == -1)
	{
		static int incr = 0;
		incr += 1;

		if (gVars.aa_jump_lag.value == 1)
			gClient->mLag = max_ticks;
		else if (gVars.aa_jump_lag.value == 2)
			gClient->mLag = (incr % 2) ? (max_ticks - 1) : max_ticks;
		else if (gVars.aa_jump_lag.value == 3)
			gClient->mLag = (incr % 2) ? (incr % max(highest_ticks - max_ticks, 1)) : max_ticks;
	}
}

void CHvhManager::ShootAntiAimThink(Entity* local, UserCmd* pitch_cmd, int frame)
{
	int shoot = gVars.aa_shoot.value;

	if (shoot <= 1)
		return;

	if (frame == 0)
		return;

	CAimbotFrame* aimframe = &gAim->mFrames[frame % MAX_FRAMES];

	UserCmd* cmd = aimframe->mUserCmd;
	if (!cmd)
		return;

	if (aimframe->mFiring)
		return;

	if (shoot == 2)
	{
		pitch_cmd->angles.x = -89.0f;
	}
	else if (shoot == 3)
	{
		pitch_cmd->angles.x = 0.0f;
	}

	float movelen =	cmd->move.Length();

	if (movelen != 0.0f)
	{
		if (cmd->angles.y != aimframe->mMoveYaw)
		{
			float angle = aimframe->mMoveDirection.Yaw();
			if (angle > 180.0f)
				angle -= 360.0f;

			float view_yaw = aimframe->mMoveYaw;
			float yaw = float_fmod(cmd->angles.y, 360.0f);

			angle += yaw - view_yaw;

			float_sincos(DEG2RAD(angle), &cmd->move.y, &cmd->move.x);
			cmd->move *= movelen;
		}
	}

	gClient->RestrictCmd(cmd);

	VerifiedCmd* cmds = *(VerifiedCmd**)((DWORD)(base->mInput) + 0xF0);
	memcpy(&(cmds[cmd->index % 150].cmd), cmd, sizeof(UserCmd));
}