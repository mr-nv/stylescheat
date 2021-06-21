#include "dll.h"

CCSAimbot* gAim = nullptr;

void CAimPlayer::Setup(Entity* player)
{
	int index = player->GetIndex();

	CAimbotFrame* frame = &gAim->mFrames[gAim->mFrameCount % MAX_FRAMES];

	{
		frame->mSimulationTime[index] = mLastSimulation;
		frame->mAnimCurTime[index] = mAnimCurTime;
		frame->mAnimFrameTime[index] = mAnimFrameTime;
		frame->mOrigins[index] = mOrigin;
		frame->mAngles[index] = mAngles;

		memcpy(frame->mAnimLayers[index], mAnimLayers, sizeof(AnimLayer) * MAX_ANIM_LAYERS);

		frame->mPitch[index] = mPitch;
		frame->mYaw[index] = mYaw;
		frame->mBodyYaw[index] = mBodyYaw;
		frame->mVelocity[index] = mVelocity;
		frame->mFlags[index] = mLastFlags;
		frame->mBodyUpdateTime[index] = mLastBodyUpdate;
		frame->mBodyHistoryAim[index] = mBodyHistoryAim;
		frame->mBodyUpdateFrame[index] = mLastBodyFrame;

		memcpy(frame->mAnimStateRegion[index], mAnimStateRegion, SIZEOF_ANIMSTATE_REGION);
	}

	DWORD old_effects = *player->m_fEffects();
	//*player->m_fEffects() |= 0x8;

	AnimLayer old_layers[MAX_ANIM_LAYERS];

	memcpy(old_layers, player->m_AnimOverlay(), sizeof(AnimLayer) * MAX_ANIM_LAYERS);

	{
		*player->m_iMostRecentModelBoneCounter() = -1;
		*player->m_flLastBoneSetupTime() = -1.0f;

		*player->m_ModelAnimDistance() = 0.0f;
		*player->m_ModelAnimFlags() = 0;
		*player->m_ModelAnimLastFrame() = 0;
	}

	{
		memcpy(mPoseParameters, player->m_flPoseParameter(), sizeof(float) * MAX_POSE_PARAMETERS);
		memcpy(frame->mPoseParameters[index], mPoseParameters, sizeof(float) * MAX_POSE_PARAMETERS);
	}

	Matrix matrix[128];

	memcpy(player->m_AnimOverlay(), mAnimLayers, sizeof(AnimLayer) * MAX_ANIM_LAYERS);
	//memcpy(player->m_iv_flPoseParameter(), player->m_flPoseParameter(), sizeof(float) * MAX_POSE_PARAMETERS);

	float old_time = base->mGlobalVars->curtime;
	float old_frametime = base->mGlobalVars->frametime;

	base->mGlobalVars->curtime = 0.0f;
	base->mGlobalVars->frametime = base->mGlobalVars->interval;

//	*(BYTE*)((DWORD)(player) + 0x39e1) = 0;

	player->SetupBones(matrix, 128, 0x100, base->mGlobalVars->curtime);

	//*(BYTE*)((DWORD)(player) + 0x39e1) = 1;

	base->mGlobalVars->curtime = old_time;
	base->mGlobalVars->frametime = old_frametime;

	memcpy(player->m_AnimOverlay(), old_layers, sizeof(AnimLayer) * MAX_ANIM_LAYERS);

	*player->m_fEffects() = old_effects;

	HitBox* box = gAim->GetBaseHitbox(player);
	Vector diff = frame->mOrigins[index] - *player->GetRenderOrigin();

	for (int i = 0; i < HITBOX_MAX; ++i)
	{
		memcpy(&frame->mBones[index][i], &matrix[(box + i)->bone], sizeof(Matrix));

		Vector pos = frame->mBones[index][i].GetTransform();
		pos += diff;

		frame->mBones[index][i].SetColumn(pos, 3);

		memcpy(&mLastBones[i], &frame->mBones[index][i], sizeof(Matrix));
	}
}

void __fastcall HookEstimateAbsVelocity(void* ecx, void* edx, Vector* out)
{
	if (ecx && out)
	{
		Entity* player = (Entity*)(ecx);
		CAimPlayer* aim_player = &gAim->mAimPlayers[player->GetIndex()];

		*out = aim_player->mAnimVelocity;
	}
}

void CAimPlayer::UpdateAnimations(Entity* player)
{
	if (mAnimating)
		return;

	if (!player->GetPlayerAnimState())
		return;

//	if (!AnimStateRegionValid(player, mAnimStateRegion))
	//	return;

	//memcpy(player->GetAnimStateRegion(), mAnimStateRegion, SIZEOF_ANIMSTATE_REGION);
	
	//if (mResolveMode == RESOLVE_STAND || mResolveMode == RESOLVE_STATIC_BODY)
		//player->GetAngRender()->y = mYaw;

	float old_curtime = base->mGlobalVars->curtime;
	float old_frametime = base->mGlobalVars->frametime;
	int old_framecount = base->mGlobalVars->framecount;
	base->mGlobalVars->curtime = mAnimCurTime;
	base->mGlobalVars->frametime = mAnimFrameTime;

	//base->mGlobalVars->framecount = (int)((base->mGlobalVars->curtime / base->mGlobalVars->frametime) + 0.5f);

	Vector old_org = *player->GetRenderOrigin();

	Vector old_origin = *player->m_vecOrigin();
	Vector old_angles = *player->m_angEyeAngles();
	float old_body = *player->m_flLowerBodyYawTarget();
	Vector old_vel = *player->m_vecVelocity();
	int old_flags = *player->m_fFlags();
	float old_duckamount = *player->m_flDuckAmount();

	*player->m_vecOrigin() = mOrigin;
	player->m_angEyeAngles()->x = mPitch;
	player->m_angEyeAngles()->y = mYaw;
	*player->m_flLowerBodyYawTarget() = mBodyYaw;
	*player->m_vecVelocity() = mAnimVelocity;
	*player->m_fFlags() = mLastFlags;
	*player->m_flDuckAmount() = mDuckAmount;

	if (gVars.l_resolver.value && !gVars.aim_active.value && mMultipleProcessing)
		player->m_angEyeAngles()->y = mBodyYaw;

	*player->GetRenderAngles() = mAngles;
	
	DWORD restore = 0;
	DWORD prot = 0;

	if (*(DWORD**)(player))
	{
		sizeof(UserCmd);
		base->VirtualProtect((LPVOID)(&(*(DWORD**)(player))[141]), 0x4, PAGE_EXECUTE_READWRITE, &prot);

		DWORD adr = (*(DWORD**)(player))[141];
		(*(DWORD**)(player))[141] = (DWORD)(&HookEstimateAbsVelocity);

		restore = adr;
	}

	if (mOnGround)
		*player->m_fFlags() |= FL_ONGROUND;
	else
		*player->m_fFlags() &= ~FL_ONGROUND;

	*player->m_bClientSideAnimation() = true;

	player->UpdateClientSideAnimation();
	
	*player->m_bClientSideAnimation() = false;

	if (restore != 0)
	{
		(*(DWORD**)(player))[141] = restore;

		base->VirtualProtect((LPVOID)(&(*(DWORD**)(player))[141]), 0x4, prot, 0);
	}

	*player->m_vecOrigin() = old_origin;
	*player->m_angEyeAngles() = old_angles;
	*player->m_flLowerBodyYawTarget() = old_body;
	*player->m_vecVelocity() = old_vel;
	*player->m_fFlags() = old_flags;
	*player->m_flDuckAmount() = old_duckamount;

	*player->GetRenderOrigin() = old_org;

	base->mGlobalVars->curtime = old_curtime;
	base->mGlobalVars->frametime = old_frametime;
	base->mGlobalVars->framecount = old_framecount;
}

void CAimPlayer::DataThink(Entity* player)
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!local)
		return;

	int index = player->GetIndex();

	float time = *player->m_flSimulationTime();

	float dt = (time - mLastSimulation);
	if (dt == 0.0f)
	{
		mTicksBehind += 1;
		return;
	}

	float old_time = mLastSimulation;

	mLastSimulation = time;

	if (dt < 0.0f)
		return;

	mOrigin = *player->m_vecOrigin();

	if (dt != base->mGlobalVars->interval && mLastSimulationDelta != base->mGlobalVars->interval)
		mMultipleProcessing = true;
	else
		mMultipleProcessing = false;

	mLastSimulationDelta = dt;

	UpdateTime(dt);

	if (*player->m_lifeState() == 0 && !mLastAlive)
		mLastAliveTime = mLastSimulation;

	mLastAlive = *player->m_lifeState() == 0;

	mAnimVelocity = Vector(mVelocity.x, mVelocity.y, 0.0f);

	{
		float sv_accelerate = base->FindVar(base->mEngineCvar, "sv_accelerate")->GetFloat();
		float sv_friction = base->FindVar(base->mEngineCvar, "sv_friction")->GetFloat();
		float sv_stopspeed = base->FindVar(base->mEngineCvar, "sv_stopspeed")->GetFloat();

		float max_speed = 260.0f;
		CCSWeapon* gun = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *player->m_hActiveWeapon()));

		if (gun)
			max_speed = gun->GetWeaponInfo()->GetMaxSpeed();

		if (*player->m_bIsScoped())
			max_speed *= 0.55f;

		if (mLastFlags & FL_DUCKING)
			max_speed /= 2.95f;

		float move = mAnimVelocity.Length() > (mLastVelocity.Length() - 1.0f) ? 450.0f : 0.0f;

		float speed = mAnimVelocity.Length();

		if (speed >= 0.1f)
		{
			float control = speed < sv_stopspeed ? sv_stopspeed : speed;
			float drop = control * sv_friction * base->mGlobalVars->interval;

			float newspeed = speed - drop;
			if (newspeed < 0.0f)
				newspeed = 0.0f;

			mAnimVelocity *= newspeed / speed;
		}

		Vector fwd = mAnimVelocity.Normal();
		fwd.z = 0.0f;

		float len = mAnimVelocity.Length();
		if (len > 1.0f)
			len = 1.0f;

		float wishspeed = max_speed * len;

		Vector wishdir = fwd * move;
		wishdir = wishdir.Normal();

		float addspeed = wishspeed - mAnimVelocity.Dot(wishdir);

		if (addspeed > 0.0f)
		{
			float accel = sv_accelerate * max_speed * base->mGlobalVars->interval;

			if (accel > addspeed)
				accel = addspeed;

			mAnimVelocity += wishdir * accel;
		}

		if (mAnimVelocity.Length() < 1.0f)
			mAnimVelocity = Vector(0.0f, 0.0f, 0.0f);

		if (player->m_AnimOverlay()[6].cycle2 == 0.0f)
			mAnimVelocity = Vector(0.0f, 0.0f, 0.0f);
	}

	mVelocity = (mOrigin - mLastOrigin) / dt;
	//mAnimVelocity = mVelocity;
	mAcceleration = (mVelocity - mLastVelocity) * ((dt * dt) / 2.0f);
	mAcceleration.z = 0.0f;

	float yaw = mVelocity.Yaw();
	if (yaw < 0.0f)
		yaw += 360.0f;

	float diff = (yaw - mLastDirection);
	
	if (diff > 180.0f)
		diff -= 360.0f;

	if (diff < -180.0f)
		diff += 360.0f;

	mLastDirection = yaw;

	mLastOrigin = mOrigin;
	mLastVelocity = mVelocity;

	mLastPitch = mEyePitch;
	mEyePitch = player->m_angEyeAngles()->x;
	mEyeYaw = player->m_angEyeAngles()->y;

	//mChronicPitch = *(float*)((DWORD)(player) + 0x38C0);
	//mChronicYaw = float_fmod(*(float*)((DWORD)(player) + 0x38C4), 360.0f);
	//mChronicYaw = float_fmod(*(float*)((DWORD)(player) + 0x38C0), 360.0f);

	if (mVelocity.LengthSqr() > 1.0f)
		mMoving = true;
	else
		mMoving = false;

	{
		bool jumped = false;
		float jump_cycle = player->m_AnimOverlay()[4].cycle2;

		{
			if (jump_cycle != 1.0f && mLastJumpCycle == 1.0f)
				jumped = true;

			mLastJumpCycle = jump_cycle;
		}

		mOnGround = (*player->m_fFlags() & FL_ONGROUND) && (mLastFlags & FL_ONGROUND);

		if (jumped && player->m_AnimOverlay()[5].cycle2 != 0.0f)
			mOnGround = true;

		if ((*player->m_fFlags() & FL_ONGROUND) && !(mLastFlags & FL_ONGROUND))
			mOnGround = false;

		if (dt == base->mGlobalVars->interval && (*player->m_fFlags() & FL_ONGROUND))
			mOnGround = true;
	}

	if (dt != base->mGlobalVars->interval)
		mDuckAmount = mLastDuckAmount;
	else
		mDuckAmount = *player->m_flDuckAmount();

	mLastFlags = *player->m_fFlags();
	mLastDuckAmount = *player->m_flDuckAmount();

	memcpy(mAnimLayers, player->m_AnimOverlay(), sizeof(AnimLayer) * MAX_ANIM_LAYERS);

	if (player->GetPlayerAnimState() != 0)
	{
		if (mAnimating)
			memcpy(mAnimStateRegion, player->GetAnimStateRegion(), SIZEOF_ANIMSTATE_REGION);
	}

	float t = old_time + base->mGlobalVars->interval;
	mAnimFrameTime = t - mAnimCurTime;
	mAnimCurTime = t;

	mAngles = *player->GetRenderAngles();

	BodyYawThink(player, *player->m_flLowerBodyYawTarget());

	{
		CCSWeapon* gun = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *player->m_hActiveWeapon()));
		bool shoot = false;

		if (gun)
		{
			int clip = *gun->m_iClip1();
			if (clip != mLastClip)
				shoot = true;

			mLastClip = clip;

			/*{
			float spd = player->m_AnimOverlay()[6].cycle2;

			float mod = gun->GetWeaponInfo()->GetMaxSpeed() * 1.92f;

			if (spd != 1.0f)
			mAnimVelocity = mAnimVelocity.Normal() * (spd * mod);
			}*/
		}

		if (shoot)
		{
			mEyePitch = mLastEyePitch;
			mEyeYaw = mLastEyeYaw;
			mLastShotTime = mLastSimulation;
		}
		else if (mLastSimulation - mLastShotTime > 0.5f)
		{
			mLastEyePitch = mEyePitch;
			mLastEyeYaw = mEyeYaw;
		}
	}

	ResolverThink(player);
	ResolverAngleThink(player);

	UpdateAnimations(player);
}

void CAimPlayer::BodyYawThink(Entity* player, float body_yaw)
{
	float cycle = player->m_AnimOverlay()[3].cycle;

//	base->Warning("test %f %f %f\n", mEyeYaw, body_yaw, cycle);

	{
		mUpdatingBody0 = false;
		mUpdatingBody1 = false;
		mUpdatingBody2 = false;

		int test1 = (int)(cycle * 1000000.0f);
		int test2 = (int)(mLastBodyCycle * 1000000.0f);

		if (test1 == 0 && test2 != 0)
			mUpdatingBody0 = true;

		if (test1 == 863495 && test2 != 0 && test2 != 863495)
			mUpdatingBody1 = true;

		if (test1 != 0 && test2 == 0)
			mUpdatingBody2 = true;
	}

	if (cycle == 0.0f && mLastBodyCycle != 0.0f)
	{
		float dt = mAnimCurTime - mLastBodyUpdate;

		if (dt > BODY_UPDATE_TIME)
		{
			mLastBodyFrame = mLastSimulation;
			mLastBodyFrameDelta = dt;
		}
	}

	mLastBodyCycle = cycle;

	bool moving = player->m_AnimOverlay()[6].cycle2 != 0.0f && mVelocity.LengthSqr() > 1.0f;

	if (!moving)
		mBodyReferenceCount = 0;

	if (body_yaw != mBodyYaw || moving)
	{
		mBodyYaw = body_yaw;

		float dt = mLastSimulation - mLastBodyUpdate;

		if (moving || dt == mLastSimulationDelta)
			mBodyHistoryAim = true;
		else
			mBodyHistoryAim = false;

		mLastBodyUpdate = mLastSimulation;
		mLastBodyFrame = 0.0f;
		mLastBodyUpdateDelta = dt;

		if (mBodyHistoryAim)
		{
			mUpdateShot = false;
			mUpdateShots2 = 0;

			mBodyReferences[mBodyReferenceCount++ % 8] = body_yaw;

			mBodyYawReference = mBodyReferences[max(mBodyReferenceCount - 7, 0) % 8];
			mUseBodyReference = true;
		}

		mLastStandBodyUpdate = mLastSimulation;
	}

	//base->Warning("test %f %f %f\n",mBodyYaw,mLastSimulation - mLastBodyUpdate,mLastSimulation - mLastBodyFrame);
}

void CAimPlayer::MoveResolveThink(Entity* player)
{
	mMoveAntiAimIndex = 0;

	mYaw = mThruAngle;

	mYaw = float_fmod(mYaw, 360.0f);
	if (mYaw > 180.0f)
		mYaw -= 360.0f;
}

void CAimPlayer::WalkResolveThink(Entity* player)
{
	mYaw = mBodyYaw;
}

void CAimPlayer::StaticBodyResolveThink(Entity* player)
{
	mYaw = float_fmod(mYaw, 360.0f);
	if (mYaw > 180.0f)
		mYaw -= 360.0f;
}

void CAimPlayer::BodyaimResolveThink(Entity* player)
{
	/*if (gVars.aim_resolver_options.value & ROPTION_LIMIT && mAvoidAngle != 0)
	{
		if (mAvoidAngle == 1)
			mYaw = (mThruAngle - 90.0f);
		else if (mAvoidAngle == 2)
			mYaw = (mThruAngle + 90.0f);
	}
	else
	{
		mYaw = mThruAngle;
		if (mBodyaimIndex % 2)
			mYaw += 180.0f;
	}*/

	mYaw = float_fmod(mYaw, 360.0f);
	if (mYaw > 180.0f)
		mYaw -= 360.0f;
}

/*	float dt = 0.0f;

if (mAvoidAngle == 1)
dt = mYaw - (mThruAngle + 90.0f);
else if (mAvoidAngle == 2)
dt = mYaw - (mThruAngle - 90.0f);

dt = float_fmod(dt, 360.0f);
if (dt > 180.0f)
dt -= 360.0f;

if (float_abs(dt) < 90.0f)
mYaw += 180.0f;*/

void CAimPlayer::StandResolveThink(Entity* player)
{
	bool limit = false;

	if (gVars.aim_resolver_limit.value == 2)
		limit = true;
	else if (gVars.aim_resolver_limit.value == 1)
		limit = mBaseIndex % 2 == 1 ? true : false;

	if (limit && mAvoidAngle != 0)
	{
		bool check_wall = gVars.aim_resolver_limit.value == 1 ? ((mBaseIndex % 4) == 3) : (mBaseIndex % 2 == 1);

		if (!(gVars.aim_resolver_options.value & ROPTION_ALTLIMIT))
			check_wall = false;

		if (check_wall)
		{
			if (mAvoidAngle == 1)
				mYaw = mAvoidBaseAngle - 90.0f;
			else if (mAvoidAngle == 2)
				mYaw = mAvoidBaseAngle + 90.0f;
		}
		else
		{
			if (mAvoidAngle == 1)
				mYaw = (mThruAngle - 90.0f);
			else if (mAvoidAngle == 2)
				mYaw = (mThruAngle + 90.0f);
		}
	}
	else
	{
		if (gVars.aim_resolver_options.value & ROPTION_BODY && mUseBodyReference && mBodyYaw != mBodyYawReference)
			mYaw = mBodyYawReference;
		else
			mYaw = mThruAngle;
	}

	float delta = (float)(gVars.aim_stand_delta.value);

	if (gVars.aim_resolver.value == 2)
	{
		int i = mStandYawIndex % 5;
		if (i == 1)
			mYaw += delta / 2.0f;
		else if (i == 2)
			mYaw -= delta / 2.0f;
		else if (i == 3)
			mYaw += delta;
		else if (i == 4)
			mYaw -= delta;
	}
	else
	{
		int i = mStandYawIndex % 3;
		if (i == 1)
			mYaw += delta;
		else if (i == 2)
			mYaw -= delta;
	}

	mYaw = float_fmod(mYaw, 360.0f);
	if (mYaw > 180.0f)
		mYaw -= 360.0f;
}

void CAimPlayer::UpdateResolveThink(Entity* player)
{
	mYaw = mBodyYaw;

	int i = mUpdateYawIndex % 3;

	if (i == 1)
		mYaw += 30.0f;
	else if (i == 2)
		mYaw -= 30.0f;

	mYaw = float_fmod(mYaw, 360.0f);
	if (mYaw > 180.0f)
		mYaw -= 360.0f;
}

void CAimPlayer::Update2ResolveThink(Entity* player)
{
	mYaw = mBodyYaw;

	int i = mUpdate2YawIndex % 3;

	if (i == 1)
		mYaw += 30.0f;
	else if (i == 2)
		mYaw -= 30.0f;

	mYaw = float_fmod(mYaw, 360.0f);
	if (mYaw > 180.0f)
		mYaw -= 360.0f;
}

void CAimPlayer::ResolverAngleThink(Entity* player)
{
	mPitch = mEyePitch;
	mYaw = mEyeYaw;
	mYawParamOverride = false;

	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	PlayerObject* obj = gPlayers.GetObject(player->GetIndex());

#ifdef TEST_BOTAA
	if (!local)
#else
	if (!local || !obj)
#endif
		return;

	if (gVars.esp_body.value)
	{
		if (local && *local->m_lifeState() != 0)
			mYaw = mBodyYaw;
	}

	if (mResolveMode == RESOLVE_NONE)
		return;

	/*{
		float p1 = mEyePitch;
		float p2 = mLastPitch;

		if (p1 > 180.0f)
			p1 -= 360.0f;

		if (p2 > 180.0f)
			p2 -= 360.0f;

		if (!(p1 < 88.0f && p2 < 88.0f))
			mPitch = 89.0f;
	}*/

	switch (mResolveMode)
	{
		case RESOLVE_MOVE:
		{
			MoveResolveThink(player);
			break;
		}
		case RESOLVE_WALK:
		{
			WalkResolveThink(player);
			break;
		}
		case RESOLVE_STAND:
		{
			StandResolveThink(player);
			break;
		}
		case RESOLVE_BODYAIM:
		{
			BodyaimResolveThink(player);
			break;
		}
		case RESOLVE_STATIC_BODY:
		{
			StaticBodyResolveThink(player);
			break;
		}
		case RESOLVE_UPDATE:
		{
			UpdateResolveThink(player);
			break;
		}
		case RESOLVE_UPDATE2:
		{
			Update2ResolveThink(player);
			break;
		}
	}
	
	if (obj)
	{
		if (obj->fixpitch)
		{
			switch (obj->fixpitch)
			{
				case 1:
				{
					mPitch = 89.0f;
					break;
				}
				case 2:
				{
					mPitch = -89.0f;
					break;
				}
			}
		}

		if (obj->fixyaw)
		{
			switch (obj->fixyaw)
			{
				case 1:
				{
					mYaw = 0.0f;
					break;
				}
				case 2:
				{
					mYaw = 180.0f;
					break;
				}
				case 3:
				{
					mYaw = 90.0f;
					break;
				}
				case 4:
				{
					mYaw = -90.0f;
					break;
				}
				case 5:
				{
					mYaw = 0.0f;
					mYaw += mThruAngle;

					break;
				}
				case 6:
				{
					mYaw = 90.0f;
					mYaw += mThruAngle;

					break;
				}
				case 7:
				{
					mYaw = -90.0f;
					mYaw += mThruAngle;

					break;
				}
			}
		}
	}
}

void CAimPlayer::RotateBonePosition(Entity* player, Matrix& mtx, float angle, bool add)
{
	Vector opos = mtx.GetTransform();

	Vector origin = *player->GetRenderOrigin();

	Vector delta = opos - origin;
	delta.z = 0.0f;

	float yaw1 = delta.Yaw();
	float yaw2 = (mtx.GetColumn(0)).Yaw();

	if (add)
	{
		yaw1 += angle;
		yaw2 += angle;
	}
	else
	{
		yaw1 += (angle - yaw1);
		yaw2 += (angle - yaw2);
	}

	Vector rot1 = Vector(0.0f, yaw1, 0.0f);
	Vector rot2 = Vector(0.0f, yaw2, 0.0f);

	float length = delta.Length();

	Vector newpos = origin + (rot1.Forward() * length);

	mtx.SetColumn(rot2.Forward(), 0);
	mtx.SetColumn(rot2.Right(), 1);
	mtx.SetColumn(rot2.Up(), 2);
	mtx.SetColumn(Vector(newpos.x, newpos.y, opos.z), 3);
}

void CAimPlayer::ResolverThink(Entity* player)
{
	int index = player->GetIndex();

	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	PlayerObject* obj = gPlayers.GetObject(player->GetIndex());

	if (!local
#ifndef TEST_BOTAA
		|| !obj
#endif
		|| !gVars.aim_active.value
		|| !gVars.aim_resolver.value
		|| (local && *local->m_lifeState() != 0)
#ifndef TEST_BOTAA
		|| obj->filter == 1
		|| obj->ignore_resolve
#endif
		|| (gVars.aim_resolver_options.value & ROPTION_FIXLEGET && !mMultipleProcessing))
	{
		mResolveMode = RESOLVE_NONE;

		return;
	}

	mThruAngle = (mOrigin - gClient->mViewOrigin).Yaw();

	bool use_update = true;

	{
		float dt = mThruAngle - mBodyYaw;

		if (dt < -180.0f)
			dt += 360.0f;

		if (dt > 180.0f)
			dt -= 360.0f;

		if (float_abs(dt) < 45.0f)
			use_update = false;
	}

	{
		if (gVars.aim_active.value && gVars.aim_resolver.value && gVars.aim_resolver_limit.value)
		{
			Matrix bones[HITBOX_MAX];
			memcpy(bones, mLastBones, sizeof(Matrix) * HITBOX_MAX);

			Vector headpos_90, headpos_270;

			HitBox* box = gAim->GetBaseHitbox(player);
			Vector pos = mLastBones[HITBOX_HEAD].Transform((box->min + box->max) * 0.5f);

			RotateBonePosition(player, bones[HITBOX_HEAD], mThruAngle + 90.0f, false);
			headpos_90 = bones[HITBOX_HEAD].Transform((box->min + box->max) * 0.5f);

			RotateBonePosition(player, bones[HITBOX_HEAD], mThruAngle - 90.0f, false);
			headpos_270 = bones[HITBOX_HEAD].Transform((box->min + box->max) * 0.5f);

			/*Vector scal = Vector(2.0f, 2.0f, 2.0f);
			Vector scal2 = scal * -1.0f;
			Vector ang = Vector(0.0f, 0.0f, 0.0f);

			base->DrawBox(base->mDebugOverlay, &headpos_90, &scal, &scal2, &ang, 0, 255, 0, 255, 0.05f);
			base->DrawBox(base->mDebugOverlay, &headpos_270, &scal, &scal2, &ang, 255, 0, 0, 255, 0.05f);*/

			CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
			if (weapon)
			{
				mAvoidAngle = 0;

				{
					bool hit1 = false;
					bool hit2 = false;
					float len_90 = 0.0f;
					float len_270 = 0.0f;
					float face_angle_90 = 0.0f;
					float face_angle_270 = 0.0f;

					TraceResult tr, exit;

					RayData ray;

					TraceFilter filter;
					filter.noplayers = true;

					{
						ray.base = headpos_90;
						ray.delta = gClient->mViewOrigin - ray.base;

						base->TraceRay(base->mEngineTrace, &ray, 0x4600400B, &filter, &tr);

						if (tr.fraction != 1.0f)
						{
							hit1 = true;
							len_90 = ray.delta.Length() * (1.0f - tr.fraction);
							face_angle_90 = tr.normal.Yaw();
						}
					}

					{
						ray.base = headpos_270;
						ray.delta = gClient->mViewOrigin - ray.base;

						base->TraceRay(base->mEngineTrace, &ray, 0x4600400B, &filter, &tr);

						if (tr.fraction != 1.0f)
						{
							hit2 = true;
							len_270 = ray.delta.Length() * (1.0f - tr.fraction);
							face_angle_270 = tr.normal.Yaw();
						}
					}

					if (!hit1 || !hit2)
					{
						if (!hit1 && hit2)
							mAvoidAngle = 1;
						else if (hit1 && !hit2)
							mAvoidAngle = 2;
					}
					else
					{
						if (len_90 > len_270)
							mAvoidAngle = 2;
						else if (len_270 > len_90)
							mAvoidAngle = 1;
					}

					if (mAvoidAngle == 1)
						mAvoidBaseAngle = face_angle_270;
					else if (mAvoidAngle == 2)
						mAvoidBaseAngle = face_angle_90;
				}
			}
		}
	}

	float speed = mVelocity.LengthSqr();

	bool update_alt = false;

	if (gVars.aim_resolver_options.value & ROPTION_UPDATE && gVars.aim_resolver_update.value)
	{
		bool updates[3];
		int update_index = 0;

		if (gVars.aim_resolver_update.value & (1 << 0))
		{
			updates[update_index++] = mUpdatingBody0;
			updates[update_index++] = mUpdatingBody1;
		}

		if (gVars.aim_resolver_update.value & (1 << 1))
			updates[update_index++] = mUpdatingBody2;

		if (updates[mUpdateShots2 % update_index])
			update_alt = true;
	}

	if (mMoving && !(*player->m_fFlags() & FL_ONGROUND))
		mResolveMode = RESOLVE_MOVE;
	else if ((mBodyHistoryAim && mLastSimulation == mLastBodyUpdate))
		mResolveMode = RESOLVE_WALK;
	else if (update_alt)
		mResolveMode = RESOLVE_UPDATE2;
	else if (gVars.aim_resolver_options.value & ROPTION_UPDATE && mLastSimulation == mLastBodyUpdate && !mUpdateShot && use_update)
		mResolveMode = RESOLVE_UPDATE;
	//else if ((mLastSimulation - mLastBodyUpdate) > BODY_UPDATE_TIME && (mLastSimulation - mLastBodyFrame) > (BODY_UPDATE_TIME))
	//	mResolveMode = RESOLVE_STATIC_BODY;
//	else if (gVars.aim_resolver_options.value & ROPTION_BODYAIM && mBodyaim)
	//	mResolveMode = RESOLVE_BODYAIM;
	else
		mResolveMode = RESOLVE_STAND;

	if (mResolvePlayer)
	{
		bool resolve_shot = mResolveShot;
		bool resolve_hs = mResolveHeadShot;
		bool head_aim = true;

		CAimbotFrame* frame = &(gAim->mFrames[mLastShotFrame % MAX_FRAMES]);

		if (frame && frame->IsValid(mLastShotFrame))
		{
			head_aim = frame->mAimHitbox == HITBOX_HEAD;

			if (gVars.aim_resolver_options.value & ROPTION_SPREADFIX && frame->mAimHitbox == HITBOX_HEAD)
			{
				Vector delta = (mLastImpactPos - frame->mViewOrigin).Normal();
				int result = gAim->PerformIntersection(player, frame->mBones[index], frame->mViewOrigin, delta, nullptr);

				if (result != HITBOX_HEAD)
					resolve_hs = false;
			}
		}

		if (resolve_hs)
		{
			mGeneralShootIndex += 1;

			//if (head_aim)
				ResolveShot(player);
		}

		mResolvePlayer = false;
	}

	mResolveShot = true;
	mResolveHeadShot = true;
}

void CAimPlayer::ResolveShot(Entity* player)
{
	switch (mShootResolveMode)
	{
		case RESOLVE_MOVE:
		{
			mMoveYawIndex += 1;

			break;
		}
		case RESOLVE_WALK:
		{
			break;
		}
		case RESOLVE_STAND:
		{
			mStandYawIndex += 1;
			
			if (mBodyaim)
			{
				mBaseIndex += 1;
			}
			else
			{
				if (gVars.aim_resolver.value == 2)
				{
					if ((mStandYawIndex % 5) == 0)
						mBaseIndex += 1;
				}
				else
				{
					if ((mStandYawIndex % 3) == 0)
						mBaseIndex += 1;
				}
			}

			break;
		}
		case RESOLVE_BODYAIM:
		{
			mBodyaimIndex += 1;

			break;
		}
		case RESOLVE_STATIC_BODY:
		{
			mStaticBodyYawIndex += 1;

			break;
		}
		case RESOLVE_UPDATE:
		{
			mUpdateYawIndex += 1;

			if (gVars.aim_resolver_options.value & ROPTION_STOPUPDATE)
				mUpdateShot = true;

			break;
		}
		case RESOLVE_UPDATE2:
		{
			mUpdate2YawIndex += 1;

			if (gVars.aim_resolver_options.value & ROPTION_STOPUPDATE)
				mUpdateShots2 += 1;

			break;
		}
	}
}

void CAimPlayer::ResolveHit(Entity* player)
{

}

void CCSAimbot::PlayerHurt(Entity* local, Entity* player, int hitgroup, int damage, bool local_attacker)
{
	CAimPlayer* aim_player = &mAimPlayers[player->GetIndex()];

	if ((*player->m_iHealth() - damage) <= 0)
		aim_player->mResolvePlayer = false;

	aim_player->mResolveShot = false;

	if (hitgroup == 1)
	{
		aim_player->mResolveHeadShot = false;	
		aim_player->ResolveHit(player);
		aim_player->mDamageFrame = aim_player->mLastShotFrame;
	}

	if ((*player->m_iHealth() - damage) <= 0)
	{
		if (gVars.misc_killsay.value && *player->m_iTeamNum() != *local->m_iTeamNum() && local_attacker)
		{
			char cfg[64];
			memset(cfg, 0, sizeof(cfg));

			char cmd[128];

			if (gVars.misc_killsay.value == 2)
				base->wsprintfA(cfg, "%s_kill", gMenu.GetConfigName());
			else
				string_cpy(cfg, "_kill");

			base->wsprintfA(cmd, "exec %s", cfg);

			base->ConCommand(base->mBaseEngine, cmd);
		}
	}
}

void CCSAimbot::BulletImpact(Entity* player, float x, float y, float z)
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!local)
		return;

	if (player != local)
		return;

	if (gClient->mShootTime != 0 && mFrameCount < gClient->mShootTime)
	{
		Entity* pl = gClient->mShootPlayer;

		if (IsEntityValid(pl))
		{
			CAimPlayer* aim_player = &mAimPlayers[pl->GetIndex()];
			aim_player->mResolvePlayer = true;
			aim_player->mShootResolveMode = gClient->mShootResolveMode;

			aim_player->mLastShotFrame = gClient->mShootFrame;

			aim_player->mLastImpactTime = mFrameCount;
			aim_player->mLastImpactPos = Vector(x, y, z);

			gClient->mShootTime = 0;
		}
	}
}

void CAimPlayer::UpdateTime(float delta)
{
	mTicksBehind = 0;
	
	mLagTime = (int)((delta / base->mGlobalVars->interval) + 0.5f);

	if (mLagTime > 1)
		mLagConsistency += 1;
	else
		mLagConsistency = 0;
}

bool CCSAimbot::IsValidHitbox(int hitbox, int box, int hitscan)
{
	if (hitbox == 2)
	{
		if (box == mBodyHitbox)
			return true;

		return false;
	}

	if (hitbox == 0 && box == HITBOX_HEAD)
		return true;

	if (hitbox == 1 && box == mBodyHitbox)
		return true;

	if (!gVars.aim_hitscan.value)
		return false;

	if (!hitscan)
		return false;

	if (hitscan & AIMGROUP_HEAD)
	{
		if (box == HITBOX_HEAD)
			return true;
	}

	if (hitscan & AIMGROUP_NECK)
	{
		switch (box)
		{
			case HITBOX_NECK:
				return true;
		}
	}

	if (hitscan & AIMGROUP_LOWER_BODY)
	{
		switch (box)
		{
			case HITBOX_PELVIS:
			case HITBOX_STOMACH:
				return true;
		}
	}

	if (hitscan & AIMGROUP_UPPER_BODY)
	{
		switch (box)
		{
			case HITBOX_LOWER_CHEST:
			case HITBOX_CHEST:
			case HITBOX_UPPER_CHEST:
				return true;
		}
	}

	if (hitscan & AIMGROUP_ARMS)
	{
		switch (box)
		{
			case HITBOX_LEFT_UPPER_ARM:
#ifndef IGNORE_RIGHT_ARM
			case HITBOX_RIGHT_UPPER_ARM:
#endif
				return true;
		}
	}

	if (hitscan & AIMGROUP_LEGS)
	{
		switch (box)
		{
			case HITBOX_LEFT_THIGH:
			case HITBOX_RIGHT_THIGH:
				return true;
		}
	}

	if (hitscan & AIMGROUP_FOREARMS)
	{
		switch (box)
		{
			case HITBOX_LEFT_FOREARM:
#ifndef IGNORE_RIGHT_ARM
			case HITBOX_RIGHT_FOREARM:
#endif
				return true;
		}
	}

	if (hitscan & AIMGROUP_CALVES)
	{
		switch (box)
		{
			case HITBOX_LEFT_CALF:
			case HITBOX_RIGHT_CALF:
				return true;
		}
	}

	if (hitscan & AIMGROUP_HANDS)
	{
		switch (box)
		{
			case HITBOX_LEFT_HAND:
#ifndef IGNORE_RIGHT_ARM
			case HITBOX_RIGHT_HAND:
#endif
				return true;
		}
	}

	if (hitscan & AIMGROUP_FEET)
	{
		switch (box)
		{
			case HITBOX_LEFT_FOOT:
			case HITBOX_RIGHT_FOOT:
				return true;
		}
	}

	return false;
}

bool CCSAimbot::IsPointscanHitbox(int hitbox)
{
	int pointscan = gVars.aim_point_groups.value;

	if (pointscan & AIMGROUP_HEAD)
	{
		if (hitbox == HITBOX_HEAD)
			return true;
	}

	if (pointscan & AIMGROUP_NECK)
	{
		switch (hitbox)
		{
			case HITBOX_NECK:
				return true;
		}
	}

	if (pointscan & AIMGROUP_LOWER_BODY)
	{
		switch (hitbox)
		{
			case HITBOX_PELVIS:
			case HITBOX_STOMACH:
				return true;
		}
	}

	if (pointscan & AIMGROUP_UPPER_BODY)
	{
		switch (hitbox)
		{
			case HITBOX_LOWER_CHEST:
			case HITBOX_CHEST:
			case HITBOX_UPPER_CHEST:
				return true;
		}
	}

	if (pointscan & AIMGROUP_ARMS)
	{
		switch (hitbox)
		{
			case HITBOX_LEFT_UPPER_ARM:
			case HITBOX_RIGHT_UPPER_ARM:
				return true;
		}
	}

	if (pointscan & AIMGROUP_LEGS)
	{
		switch (hitbox)
		{
			case HITBOX_LEFT_THIGH:
			case HITBOX_RIGHT_THIGH:
				return true;
		}
	}

	if (pointscan & AIMGROUP_FOREARMS)
	{
		switch (hitbox)
		{
			case HITBOX_LEFT_FOREARM:
			case HITBOX_RIGHT_FOREARM:
				return true;
		}
	}

	if (pointscan & AIMGROUP_CALVES)
	{
		switch (hitbox)
		{
			case HITBOX_LEFT_CALF:
			case HITBOX_RIGHT_CALF:
				return true;
		}
	}

	return false;
}

int CCSAimbot::GetMaxShots(Entity* player, CCSWeapon* weapon, Vector pos, int max_shots, int max_walls, int hitbox, bool* hit_wall)
{
	float damage = 0.0f;
	bool hit = false;
	float length = 0.0f;

	if (!weapon->TraceBullet(player, pos, &damage, &hit, &length, max_walls))
		return -1;

	damage = GetHitgroupDamage(player, damage, (hitbox == -1) ? GetHitgroup(HITGROUP_UPPER_BODY) : GetHitgroup(hitbox));
	*hit_wall = hit;

	float hp = (float)(*player->m_iHealth());
	int shots = max((int)(hp / damage) + 1, 1);

	if (shots != 1)
	{
		if (max_shots != 1)
			shots = max((int)(100.0f / damage) + 1, 1);
	}

	if (hitbox == -1)
	{
		if (shots > 1)
			return -1;
	}
	else if (max_shots != 0 && hit)
	{
		bool check = true;

		if (shots != 1)
		{
			CSWeaponInfo* info = weapon->GetWeaponInfo();

			float wep_damage = (float)(info->GetDamage());
			wep_damage *= float_pow(info->GetRangeModifier(), length / 500.0f);

			float damage2 = GetHitgroupDamage(player, wep_damage, GetHitgroup(HITBOX_HEAD));

			int shots2 = max((int)(hp / damage2) + 1, 1);

			if (shots2 != 1)
			{
				float hp2 = hp - damage;
				float damage3 = GetHitgroupDamage(player, wep_damage, GetHitgroup(HITBOX_HEAD));

				int shots3 = max((int)(hp2 / damage3) + 1, 1);

				if (shots3 == 1)
					check = false;
			}
		}

		if (check)
		{
			if (shots > max_shots)
				return -1;
		}
	}

	return shots;
}

bool CCSAimbot::GetAimPosition(CAimTarget* target, CAimbotFrame* frame)
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));

	int index = target->mPlayer->GetIndex();

	Entity* player = target->mPlayer;
	CAimPlayer* aim_player = &mAimPlayers[index];

	HitBox* box = GetBaseHitbox(player);
	if (!box)
		return false;

	Matrix matrix[HITBOX_MAX];
	memcpy(matrix, frame->mBones[player->GetIndex()], sizeof(Matrix) * HITBOX_MAX);

	int best = -9999;
	int aim_hitbox = 0;
	int aim_point = 0;
	bool point_scan = false;
	bool point_head = false;
	bool wall_hit = false;
	bool body_aim = false;
	Vector aimpos;

	int hitbox = gVars.aim_hitbox.value;
	int hitscan = gVars.aim_hitscan_groups.value;
	int pointscan = gVars.aim_pointscan.value;
	int max_shots = gVars.aim_maxshots.value;
	int pointscale_head = gVars.aim_pointscale_head.value;
	bool onhp = gVars.aim_hp.value == 1;

	bool awp = false;
	bool zeus = false;

	if (aim_player->mBodyHistoryAim && aim_player->mLastSimulation == aim_player->mLastBodyUpdate)
	{
		if (gVars.aim_headaim.value)
			hitbox = 0;
		
		if (gVars.aim_headaim.value == 2)
			hitscan = 0;
	}

	if (gVars.aim_inair.value && !(*player->m_fFlags() & FL_ONGROUND))
		hitbox = 4;

	if (gVars.aim_hps_move.value && aim_player->mBodyHistoryAim && (*player->m_fFlags() & FL_ONGROUND))
		pointscale_head = 0;

	if (gVars.aim_fast_history.value && !target->mNormalFrame)
		hitscan = 0;

	if (gVars.aim_autozeus.value && *weapon->m_iItemDefinitionIndex() == WEAPON_TASER)
	{
		hitbox = 4;
		zeus = true;
	}

	PlayerObject* obj = gPlayers.GetObject(player->GetIndex());
	if (obj)
	{
		if (obj->hitbox != 0)
		{
			if (obj->hitbox == 1)
				hitbox = 0;
			else if (obj->hitbox == 2)
				hitbox = 2;
			else if (obj->hitbox == 3)
				hitbox = 4;
			else if (obj->hitbox == 4)
				hitbox = 0; hitscan = 0;
		}
	}

	aim_player->mTargetHitbox = hitbox;

	for (int i = 0; i < HITBOX_MAX; ++i)
	{
		if (!IsValidHitbox(hitbox, i, hitscan))
			continue;

		HitBox* bb = box + i;

		for (int s = 0; s < 2; ++s)
		{
			if (s == 1)
			{
				if (pointscale_head == 0 || i != HITBOX_HEAD)
					break;
			}

			float point_scale = (float)(gVars.aim_pointscale.value) * 0.01f;

			if (s == 1)
				point_scale = (float)(pointscale_head) * 0.01f; 

			int index = 6;
			int best_n = -1;

			if (pointscan && IsPointscanHitbox(i))
			{
				Vector center = matrix[i].Transform((bb->min + bb->max) * 0.5f);
				Vector dt1 = (center - gClient->mViewOrigin).Normal();
				float best = 1.0f;

				for (int n = 0; n < index; ++n)
				{
					Vector base = bb->min;
					if (i == HITBOX_HEAD || i > HITBOX_UPPER_CHEST)
						base = bb->max;

					float r = bb->radius * point_scale;
					Vector offset = sphere_offsets[n % index];

					Vector pos = matrix[i].Transform(base + (offset * r));
					Vector dt2 = (pos - gClient->mViewOrigin).Normal();
					
					float dot = dt1.Dot(dt2);

					if (dot < best)
					{
						best_n = n;
						best = dot;
					}
				}
			}

			for (int n = -1; n < index; ++n)
			{
				if (n != -1)
				{
					if (!pointscan)
						break;

					if (hitbox == 3)
						break;

					if (!IsPointscanHitbox(i))
						break;

					if (bb->radius == -1.0f)
						break;
				}

				Vector pos, offset;

				if (n != -1)
				{
					if (n != best_n)
						continue;

					Vector base = bb->min;
					if (i == HITBOX_HEAD || i > HITBOX_UPPER_CHEST)
						base = bb->max;

					float r = bb->radius * point_scale;
					offset = sphere_offsets[n % index];

					pos = matrix[i].Transform(base + (offset * r));
				}
				else
				{
					pos = matrix[i].Transform((bb->min + bb->max) * 0.5f);
				}

				int max_walls = 4;// gVars.aim_maxwalls.value;
				if (gVars.aim_fast_trace.value && !IsMainHitbox(hitbox, i))
					max_walls = 0;

				int max_shots_alt = max_shots;
				if (gVars.aim_maxshots_head.value != -1 && i == HITBOX_HEAD)
					max_shots_alt = gVars.aim_maxshots_head.value;

				bool hit = false;
				int shots = GetMaxShots(player, weapon, pos, max_shots_alt, max_walls, (awp || zeus) ? -1 : i, &hit);

				if (shots == -1)
					continue;

				if (onhp && shots != 1 && i != HITBOX_HEAD && !IsMainHitbox(hitbox, i))
					continue;

				if (pointscan && i == HITBOX_HEAD)
				{
					int result = PerformIntersection(player, matrix, gClient->mViewOrigin, (pos - gClient->mViewOrigin).Normal(), nullptr);

					if (result != i)
						continue;
				}
				
				if (s == 1)
					shots += 1;

				if (i == mBodyHitbox && shots == 1)
					shots = -11;
				else if (IsMainHitbox(hitbox, i) && n == -1)
					shots = -10;
				else if (hitbox >= 1 && (GetHitgroup(i) == HITGROUP_UPPER_BODY || GetHitgroup(i) == HITGROUP_LOWER_BODY))
					shots = -1 - (GetBodyPriority(i) + 1);

				if (best == -9999 || shots < best)
				{
					aimpos = pos;
					aim_hitbox = i;
					aim_point = n;
					point_scan = n != -1;
					point_head = s != 0;
					wall_hit = hit;
					body_aim = GetHitgroup(i) == HITGROUP_UPPER_BODY || GetHitgroup(i) == HITGROUP_LOWER_BODY;

					best = shots;
				}
			}
		}
	}

	if (best == -9999)
		return false;
	
	if (aim_player->mBodyHistoryAim && aim_player->mLastSimulation == aim_player->mLastBodyUpdate)
		best = -12;

	target->mPoints = 0;
	target->mPoints -= (int)((aimpos - gClient->mViewOrigin).LengthSqr() + 0.5f);
	target->mPoints -= (MAXDIST_SQR * best);

	target->mAimPos = aimpos;
	target->mHitbox = aim_hitbox;
	target->mPointScan = point_scan;

	aim_player->mAimingHitbox = aim_hitbox;
	aim_player->mPointScan = point_scan;
	aim_player->mPointModeHead = point_head;
	aim_player->mBodyaim = body_aim;

	return true;
}

bool CCSAimbot::FindAimPosition(CAimTarget* target)
{
	//mOldHitbox = *(BYTE*)((DWORD)(target->mPlayer)+0x39e1) == 0;

	int index = target->mPlayer->GetIndex();

	CAimPlayer* aim_player = &mAimPlayers[index];
	CAimbotFrame* frame = nullptr;
	CAimbotFrame* last_frame = nullptr;
	CAimbotFrame* body_frame = nullptr;
	CAimbotFrame* body_frame_last = nullptr;

	int limit = (int)((0.2f / base->mGlobalVars->interval) + 0.5f) - mInterpTicks - 1;

	{
		for (int n = 0; n < MAX_FRAMES; ++n)
		{
			int target_time = mFrameCount - n;
			if (target_time < 0)
				break;

			CAimbotFrame* target_frame = &mFrames[target_time % MAX_FRAMES];

			int ticks = (int)((target_frame->mSimulationTime[index] / base->mGlobalVars->interval) + 0.5f);
			ticks += mInterpTicks;

			int delta = (ticks - gClient->mTickCount);

			if (delta > USERCMD_DELTA_MAX)
				continue;

			if (delta < (limit * -1))
				break;

			if (!frame)
				frame = target_frame;

			//if (gVars.aim_history.value == 2 && target_time == aim_player->mDamageFrame)
				//frame = target_frame;

			last_frame = target_frame;

			if (gVars.aim_resolver.value &&
				((target_frame->mBodyHistoryAim[index] && target_frame->mSimulationTime[index] == target_frame->mBodyUpdateTime[index])))
			{
				if (!body_frame)
					body_frame = target_frame;

				body_frame_last = target_frame;
			}
		}
	}
	/*else
	{
		frame = &mFrames[mFrameCount % MAX_FRAMES];
	}*/

	if (!frame)
		return false;

	//if (gVars.aim_resolver.value && !frame->mBodyHistoryAim[index] && frame->mSimulationTime[index] == frame->mBodyUpdateTime[index])
	//	return false;

	if (body_frame_last)
	{
		target->mNormalFrame = false;

		if (gVars.aim_history.value >= 3 && GetAimPosition(target, body_frame_last))
		{
			target->mTargetFrame = body_frame_last->mIndex;

			return true;
		}
	}

	target->mNormalFrame = true;

	if (GetAimPosition(target, frame))
	{
		target->mTargetFrame = frame->mIndex;

#ifdef TEST_IMPACTS
		memcpy(target->mBones, frame->mBones[target->mPlayer->GetIndex()], sizeof(Matrix) * 20);
#endif

		return true;
	}

	if (last_frame)
	{
		target->mNormalFrame = false;

		if (gVars.aim_history.value >= 1 && GetAimPosition(target, last_frame))
		{
			target->mTargetFrame = last_frame->mIndex;

			return true;
		}
	}

	return false;
}

bool CCSAimbot::IsValidTarget(Entity* local, Entity* player)
{
	if (player == local)
		return false;

	if (!gVars.aim_team.value)
	{
		if (*player-> m_iTeamNum() == *local->m_iTeamNum())
			return false;
	}

	if (*player->m_lifeState() != 0)
		return false;

	if (player->IsDormant())
		return false;

	return true;
}

void CCSAimbot::FindTarget(Entity* local)
{
	CAimTarget target;
	int best = (int)(~0x7FFFFFFF);

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		target.Init();

		target.mPlayer = base->GetClientEntity(base->mEntList, i);
		if (!IsEntityValid(target.mPlayer))
			continue;

		if (!IsValidTarget(local, target.mPlayer))
			continue;

		if (*target.mPlayer->m_bGunGameImmunity())
			continue;

		if (!FindAimPosition(&target))
			continue;

		if (gVars.aim_sticky.value)
		{
			if (target.mPlayer == mLastTarget.mPlayer)
				target.mPoints = 0x7FFFFFFF;
		}

		PlayerObject* obj = gPlayers.GetObject(target.mPlayer->GetIndex());

		if (obj)
		{
			if (obj->filter == 1)
				continue;

			if (obj->filter == 2)
			{
				target.mPoints = 0x7FFFFFFF;
				best = 0;
			}
		}

		if (target.mPoints > best)
		{
			best = target.mPoints;

			memcpy(&mTarget, &target, sizeof(CAimTarget));
		}
	}
}

void CCSAimbot::HandleFiring(Entity* local, UserCmd* cmd, Vector old_angles)
{
	Entity* player = mTarget.mPlayer;
	if (!IsEntityValid(player))
		return;

	CAimPlayer* aim_player = &mAimPlayers[player->GetIndex()];
	CAimbotFrame* frame = &mFrames[mTarget.mTargetFrame % MAX_FRAMES];

	gClient->mShootTime = mFrameCount + (int)(1.0f / base->mGlobalVars->interval);
	gClient->mShootPlayer = player;
	memcpy(gClient->mShootLastBones, aim_player->mLastBones, sizeof(Matrix) * HITBOX_MAX);

	gClient->mShootFrame = mTarget.mTargetFrame;
	gClient->mShootResolveMode = aim_player->mResolveMode;

	frame->mAimHitbox = mTarget.mHitbox;
	frame->mAimPos = mTarget.mAimPos;
	frame->mAimTarget = player;
	frame->mUserCmd = cmd;
}

void CCSAimbot::PreThink(Entity* local)
{
	memcpy(&mLastTarget, &mTarget, sizeof(CAimTarget));
	mTarget.Init();

	if (!mInit)
		mInit = true;

	mFrameCount += 1;

	CAimbotFrame* frame = &mFrames[mFrameCount % MAX_FRAMES];
	memset(frame, 0, sizeof(CAimbotFrame));

	frame->mIndex = mFrameCount;
	frame->mAimTarget = nullptr;

	if (gVars.misc_force_interp.value == 2)
		mInterpTicks = (int)((0.19f / base->mGlobalVars->interval) + 0.5f);
	else if (gVars.misc_force_interp.value == 1)
		mInterpTicks = (int)((0.063f / base->mGlobalVars->interval) + 0.5f);
	else
		mInterpTicks = (int)((0.031f / base->mGlobalVars->interval) + 0.5f);

	void* net_channel = base->GetNetChannel(base->mBaseEngine);
	if (net_channel)
	{
		base->GetLatency = (CBaseClass::GetLatencyFn)((*(DWORD**)(net_channel))[10]);

		float lag = ((base->GetLatency(net_channel, 0) / 2.0f) / base->mGlobalVars->interval) + 0.5f;

		mNetLag = (int)(lag);
		mNetLag *= 2;

		mRealNetLag = mNetLag - 1;
		
		if (gVars.misc_psilent.value)
			mRealNetLag += 1;
	}

	ConVar* sv_gravity = base->FindVar(base->mEngineCvar, "sv_gravity");
	if (sv_gravity)
		mGravityValue = (float)(sv_gravity->GetValue());

	ConVar* sv_enablebunnyhopping = base->FindVar(base->mEngineCvar, "sv_enablebunnyhopping");
	if (sv_enablebunnyhopping)
		mNoSpread = sv_enablebunnyhopping->GetInt() != 0;

	{
		if (gVars.aim_body_hitbox.value == 0)
			mBodyHitbox = HITBOX_PELVIS;
		else if (gVars.aim_body_hitbox.value == 1)
			mBodyHitbox = HITBOX_STOMACH;
		else if (gVars.aim_body_hitbox.value == 2)
			mBodyHitbox = HITBOX_LOWER_CHEST;
		else if (gVars.aim_body_hitbox.value == 3)
			mBodyHitbox = HITBOX_CHEST;
		else if (gVars.aim_body_hitbox.value == 4)
			mBodyHitbox = HITBOX_UPPER_CHEST;
	}
}

bool CCSAimbot::HitchanceThink(Entity* local, CCSWeapon* weapon)
{
	if (gVars.aim_autoaction.value & (1<<2))
	{
		float speed = (weapon->GetWeaponInfo()->GetMaxSpeed() / 3.0f);

		if (*local->m_bIsScoped())
		{
			if (*weapon->m_iItemDefinitionIndex() == WEAPON_SCAR20
				|| *weapon->m_iItemDefinitionIndex() == WEAPON_G3SG1)
				speed = 120.0f / 3.0f;
			else if (*weapon->m_iItemDefinitionIndex() == WEAPON_SSG08)
				speed = 230.0f / 3.0f;
			else if (*weapon->m_iItemDefinitionIndex() == WEAPON_AWP)
				speed = 100.0f / 3.0f;
		}

		if (gClient->mUserCmd->buttons & IN_DUCK)
			speed *= 1.025f;

		float length = (float)((int)(gClient->mLocalVelocity.Length() + 0.5f));

		if (length > speed)
			return true;
	}

	bool result = false;

	if (gVars.aim_hitchance_factor.value != 0)
	{
		int factor = gVars.aim_hitchance_factor.value;
		if (gVars.aim_hitchance_min.value && !mHitLowerHitchance)
			factor = gVars.aim_hitchance_min.value;

		//if (gVars.aim_autoawp.value && *weapon->m_iItemDefinitionIndex() == WEAPON_AWP)
		//	factor = 100;

		//if (gVars.aim_autoscout.value && *weapon->m_iItemDefinitionIndex() == WEAPON_SSG08)
		//	factor = 100;

		int index = mTarget.mPlayer->GetIndex();
		Vector aimangles = (mTarget.mAimPos - gClient->mViewOrigin).Angles();
		aimangles += gClient->mSpreadAngles;

		Vector forward = aimangles.Forward();
		Vector right = aimangles.Right();
		Vector up = aimangles.Up();

		CAimPlayer* aim_player = &mAimPlayers[index];

		CAimbotFrame* frame = nullptr;
		frame = &mFrames[mTarget.mTargetFrame % MAX_FRAMES];

		int hits = 0;

		int hitgroup = GetHitgroupRational(mTarget.mHitbox);
		bool revolver = *weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER;

		for (int i = 0; i < 255; ++i)
		{
			Vector spread = weapon->GetSpreadXY(i, revolver);
			Vector spreaddir = forward + (right * spread.x) + (up * spread.y);

			int hitbox = PerformIntersection(mTarget.mPlayer, frame->mBones[index], gClient->mViewOrigin, spreaddir, nullptr);
			if (hitbox == -1)
				continue;

			if (mTarget.mHitbox == HITBOX_NECK)
			{
				hits += 1;
			}
			else if (mTarget.mHitbox != HITBOX_HEAD)
			{
				if (hitgroup == GetHitgroupRational(hitbox))
					hits += 1;
			}
			else
			{
				if (hitbox == HITBOX_HEAD)
					hits += 1;
			}
		}

		float shots = ((float)(hits) / 255.0f);

		if (shots < ((float)(gVars.aim_hitchance_min.value) * 0.01f))
			mHitLowerHitchance = true;
		else if (shots >= ((float)(gVars.aim_hitchance_factor.value) * 0.01f))
			mHitLowerHitchance = false;

		if (shots < ((float)(factor) * 0.01f))
			result = true;
	}

	return result;
}

void CCSAimbot::Think(Entity* local, UserCmd* cmd)
{
	CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));
	if (!weapon)
		return;

	if (!gVars.aim_active.value)
		return;

	if (!gVars.aim_mode.value)
		return;

	if (gVars.aim_mode.value == 2)
	{
		if (!mAimKey)
			return;
	}

	if (gVars.aim_manual.value)
	{
		if ((gClient->mCopyCmd.buttons & IN_ATTACK) || (gClient->mCopyCmd.buttons & IN_ATTACK2))
			return;
	}

	FindTarget(local);

	if (!mTarget.mPlayer)
		return;

	Vector aimdir = (mTarget.mAimPos - gClient->mViewOrigin).Normal();
	Vector aimangles = aimdir.Angles();

	cmd->angles = aimangles;
	
	if (!gVars.aim_silent.value)
		base->SetViewAngles(base->mBaseEngine, &cmd->angles);

	if (gVars.aim_autofire.value)
	{
		if (!HitchanceThink(local, weapon))
			cmd->buttons |= IN_ATTACK;
	}

	if (gVars.aim_autoaction.value & (1<<1))
	{
		bool fire_ready = gClient->mCurTime >= (*weapon->m_flNextPrimaryAttack() + base->mGlobalVars->interval);

		if (gClient->mFlags & FL_ONGROUND && weapon->HasScope() && fire_ready && !*local->m_bIsScoped())
			cmd->buttons |= IN_ATTACK2;
	}

	{
		CAimPlayer* aim_player = &mAimPlayers[mTarget.mPlayer->GetIndex()];

		CAimbotFrame* frame = nullptr;
		frame = &mFrames[mTarget.mTargetFrame % MAX_FRAMES];

		int ticks = (int)((frame->mSimulationTime[mTarget.mPlayer->GetIndex()] / base->mGlobalVars->interval) + 0.5f);
		ticks += mInterpTicks;

		cmd->ticks = ticks;
	}
}

int CCSAimbot::PerformIntersection(Entity* player, Matrix* matrix, Vector vieworigin, Vector delta, Vector* test, float scale)
{
	HitBox* box = GetBaseHitbox(player);

	int index = 0;
	int hitbox = -1;
	float best = 0x7FFFFFFF;

	for (int i = 0; i < HITBOX_MAX; ++i)
	{
		HitBox* bb = box + i;

		if (!bb)
			break;

		if (bb->radius == -1.0f)
			continue;

		Vector min = matrix[i].Transform(bb->min);
		Vector max = matrix[i].Transform(bb->max);

		Vector sect;

		if (intersect_capsule(vieworigin, delta, min, max, bb->radius * scale, &sect))
		{
			float len = (sect - vieworigin).LengthSqr();

			if (len < best)
			{
				if (test)
					*test = sect;

				hitbox = i;
				best = len;
			}
		}
	}

	return hitbox;
}