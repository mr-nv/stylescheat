enum
{
	HITBOX_HEAD,
	HITBOX_NECK,
	//HITBOX_LOWER_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_LOWER_CHEST,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	HITBOX_MAX
};

//#define IGNORE_RIGHT_ARM

#define AIMGROUP_HEAD (1<<0)
#define AIMGROUP_NECK (1<<1)
#define AIMGROUP_LOWER_BODY (1<<2)
#define AIMGROUP_UPPER_BODY (1<<3)
#define AIMGROUP_ARMS (1<<4)
#define AIMGROUP_LEGS (1<<5)
#define AIMGROUP_FOREARMS (1<<6)
#define AIMGROUP_CALVES (1<<7)
#define AIMGROUP_HANDS (1<<8)
#define AIMGROUP_FEET (1<<9)

#define MAX_FRAMES 128

static Vector sphere_offsets[] = {
Vector(1.0f, 0.0f, 0.0f),
Vector(-1.0f, 0.0f, 0.0f),
Vector(0.0f, 1.0f, 0.0f),
Vector(0.0f, -1.0f, 0.0f),
Vector(0.0f, 0.0f, 1.0f),
Vector(0.0f, 0.0f, -1.0f),
};

struct ResolveAntiAim
{
	float pitch;
	float yaw;
	bool override_param;
	float param;
};

class CAimbotFrame
{
public:
	bool IsValid(int index)
	{
		return mIndex == index;
	}

	int mIndex;

	// all players
	Matrix mBones[MAX_PLAYERS][HITBOX_MAX];
	float mSimulationTime[MAX_PLAYERS];
	float mAnimCurTime[MAX_PLAYERS];
	float mAnimFrameTime[MAX_PLAYERS];
	Vector mOrigins[MAX_PLAYERS];
	Vector mAngles[MAX_PLAYERS];

	float mPoseParameters[MAX_PLAYERS][MAX_POSE_PARAMETERS];
	AnimLayer mAnimLayers[MAX_PLAYERS][MAX_ANIM_LAYERS];

	float mPitch[MAX_PLAYERS];
	float mYaw[MAX_PLAYERS];
	float mBodyYaw[MAX_PLAYERS];
	Vector mVelocity[MAX_PLAYERS];
	int mFlags[MAX_PLAYERS];
	float mBodyUpdateTime[MAX_PLAYERS];
	float mBodyUpdateFrame[MAX_PLAYERS];
	bool mBodyHistoryAim[MAX_PLAYERS];

	Vector mAngRender[MAX_PLAYERS];

	char mAnimStateRegion[MAX_PLAYERS][SIZEOF_ANIMSTATE_REGION];
	
	// localplayer
	Vector mViewOrigin;
	Vector mMoveDirection;
	float mMoveYaw;
	Vector mSpreadAngles;
	Vector mPunchAngles;
	int mAimHitbox;
	Vector mAimPos;
	Entity* mAimTarget;
	UserCmd* mUserCmd;
	int mTickCount;
	UserCmd mCopyCmd;
	bool mFiring;
};

enum
{
	RESOLVE_NONE = 0,
	RESOLVE_STAND,
	RESOLVE_WALK,
	RESOLVE_MOVE,
	RESOLVE_BODYAIM,
	RESOLVE_STATIC_BODY,
	RESOLVE_UPDATE,
	RESOLVE_UPDATE2
};

#define ROPTION_SPREADFIX (1<<0)
#define ROPTION_UPDATE (1<<1)
#define ROPTION_FIXLEGET (1<<2)
//#define ROPTION_ALT5 (1<<5)
#define ROPTION_ALTLIMIT (1<<3)
#define ROPTION_STOPUPDATE (1<<4)
#define ROPTION_BODY (1<<5)
//#define ROPTION_BODYAIM (1<<5)

#define MAX_ANTI_AIMS 16

//2147483647.0f
#define BODY_UPDATE_TIME 1.109375f

class CAimPlayer
{
public:
	void Setup(Entity* player);
	void SetupResolve(Entity* player, float yaw, CAimbotFrame* frame, Matrix* bones);
	void UpdateAnimations(Entity* player);
	void DataThink(Entity* player);
	void BodyYawThink(Entity* player, float body_yaw);

	void UpdateTime(float delta);

	void ResolverThink(Entity* player);
	void ResolverAngleThink(Entity* player);
	void MoveResolveThink(Entity* player);
	void WalkResolveThink(Entity* player);
	void BodyaimResolveThink(Entity* player);
	void StaticBodyResolveThink(Entity* player);
	void StandResolveThink(Entity* player);
	void UpdateResolveThink(Entity* player);
	void Update2ResolveThink(Entity* player);

	void ResolveShot(Entity* player);
	void ResolveHit(Entity* player);

	void RotateBonePosition(Entity* player, Matrix& mtx, float angle, bool add);

	inline bool AnimStateRegionValid(Entity* player, char* region)
	{
		if (*(DWORD*)((DWORD)(region)) == 0)
			return false;

		if (*(DWORD*)((DWORD)(region + 8)) == 0)
			return false;

		if (!*(DWORD*)((DWORD)(region + 96)))
			return false;

		if (*(Entity**)((DWORD)(region + 96)) != player)
			return false;

		return true;
	}
	inline void AddAntiAim(float pitch, float yaw, bool override_param, float param)
	{
		ResolveAntiAim* aa = &mMoveAntiAims[mMoveAntiAimIndex++ % MAX_ANTI_AIMS];
		aa->pitch = pitch;
		aa->yaw = yaw;
		aa->override_param = override_param;
		aa->param = param;
	}

	int mLastShotFrame;

	float mLastSimulation;
	float mLastSimulationDelta;
	float mLastAliveTime;
	bool mLastAlive;
	Vector mOrigin;
	
	float mPoseParameters[MAX_POSE_PARAMETERS];
	float mLastDirection;

	// animations

	bool mAnimating;
	float mAnimCurTime;
	float mAnimFrameTime;
	AnimLayer mAnimLayers[MAX_ANIM_LAYERS];
	bool mMultipleProcessing;

	char mAnimStateRegion[SIZEOF_ANIMSTATE_REGION];
	char mAnimStateRegionProcessed[SIZEOF_ANIMSTATE_REGION];
	Vector mAngles;

	float mLastJumpCycle;
	bool mOnGround;
	float mDuckAmount;
	bool mYawParamOverride;
	float mYawParamValue;

	bool mMoving;
	Vector mAnimVelocity;
	Vector mLastOrigin;
	Vector mVelocity;
	Vector mAcceleration;
	Vector mLastVelocity;
	int mLastFlags;
	float mLastDuckAmount;
	Matrix mLastBones[HITBOX_MAX];

	int mTargetHitbox;
	int mAimingHitbox;

	float mEyePitch;
	float mLastPitch;
	float mEyeYaw;
	float mChronicPitch;
	float mChronicYaw;
	float mPitch;
	float mYaw;
	float mBodyYaw;
	float mLastBodyUpdate;
	float mLastBodyUpdateDelta;
	bool mBodyHistoryAim;
	float mLastBodyCycle;
	float mLastBodyFrame;
	float mLastBodyFrameDelta;
	float mLastStandBodyUpdate;

	bool mPointScan;
	bool mPointModeHead;
	bool mBodyaim;

	// general resolver
	int mResolveMode;
	int mShootResolveMode;
	float mThruAngle;
	bool mResolvePlayer;
	bool mResolveShot;
	bool mResolveHeadShot;
	int mDamageFrame;
	int mAvoidAngle;
	float mAvoidBaseAngle;
	
	Matrix mRenderBones[HITBOX_MAX];
	bool mAvoidResolve;
	int mGeneralShootIndex;

	// move
	int mMoveYawIndex;
	ResolveAntiAim mMoveAntiAims[MAX_ANTI_AIMS];
	int mMoveAntiAimIndex;

	// stand
	int mStandYawIndex;
	float mBodyYawReference;
	float mBodyReferences[8];
	int mBodyReferenceCount;
	bool mUseBodyReference;
	int mBaseIndex;

	// bodyaim
	int mBodyaimIndex;

	// static body
	int mStaticBodyYawIndex;

	// update
	int mUpdateYawIndex;
	bool mUpdateShot;

	// update2
	int mUpdate2YawIndex;
	bool mUpdatingBody0;
	bool mUpdatingBody1;
	bool mUpdatingBody2;
	int mUpdateShots2;
	float mLastMove;

	// shot register
	int mLastImpactTime;
	Vector mLastImpactPos;

	bool mFrozen;

	int mLagConsistency;
	int mTicksBehind;
	int mLagTime;

	int mLastClip;
	float mLastShotTime;
	float mLastEyePitch;
	float mLastEyeYaw;
};

//#define TEST_IMPACTS
//#define TEST_BOTAA

class CAimTarget
{
public:
	CAimTarget()
	{
		Init();
	}

	void Init()
	{
		SecureZeroMemory(this, sizeof(*this));
	}

	Entity* mPlayer;
	Vector mAimPos;
	int mPoints;
	int mTargetFrame;
	bool mNormalFrame;
	int mHitbox;
	bool mPointScan;
#ifdef TEST_IMPACTS
	Matrix mBones[20];
#endif
};

#define COS_45 0.70710678118654752440084436210485

#define HITGROUP_HEAD 0
#define HITGROUP_LOWER_BODY 1
#define HITGROUP_UPPER_BODY 2
#define HITGROUP_LEGS 3

#define MAX_SHOOT_INSTANCES 8

#define MAXDIST_SQR 67108864 // 8192^2

class CCSAimbot
{
public:
	CCSAimbot()
	{
		memset(this, 0, sizeof(*this));
	}

	inline HitBox* GetBaseHitbox(Entity* player)
	{
		void* model = player->GetModel();
		if (!model)
			return nullptr;

		StudioModel* studio = base->GetStudioModel(base->mModelInfo, model);
		return studio->GetHitBox(0);
	}
	inline int GetHitgroup(int hitbox)
	{
		//switch (hitbox)
		{
			if (hitbox == HITBOX_HEAD)
				return HITGROUP_HEAD;
			if (hitbox == HITBOX_PELVIS ||
			hitbox == HITBOX_STOMACH)
				return HITGROUP_LOWER_BODY;
			if (hitbox == HITBOX_NECK ||
			hitbox == HITBOX_LOWER_CHEST ||
			hitbox == HITBOX_CHEST ||
			hitbox == HITBOX_UPPER_CHEST ||
			hitbox == HITBOX_RIGHT_HAND ||
			hitbox == HITBOX_LEFT_HAND ||
			hitbox == HITBOX_RIGHT_UPPER_ARM ||
			hitbox == HITBOX_RIGHT_FOREARM ||
			hitbox == HITBOX_LEFT_UPPER_ARM ||
			hitbox == HITBOX_LEFT_FOREARM)
				return HITGROUP_UPPER_BODY;
			if (hitbox == HITBOX_RIGHT_THIGH ||
			hitbox == HITBOX_LEFT_THIGH ||
			hitbox == HITBOX_RIGHT_CALF ||
			hitbox == HITBOX_LEFT_CALF ||
			hitbox == HITBOX_RIGHT_FOOT ||
			hitbox == HITBOX_LEFT_FOOT)
				return HITGROUP_LEGS;
		}

		return 0;
	}
	inline int GetHitgroupRational(int hitbox)
	{
		if (hitbox == -1)
			return -1;

		int hitgroup = GetHitgroup(hitbox);

		switch (hitgroup)
		{
			case HITGROUP_HEAD:
				return HITGROUP_HEAD;
			case HITGROUP_LOWER_BODY:
			case HITGROUP_UPPER_BODY:
				return HITGROUP_LOWER_BODY;
			case HITGROUP_LEGS:
				return HITGROUP_LEGS;
		}

		return -1;
	}
	inline float GetHitgroupDamage(Entity* player, float damage, int group)
	{
		Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
		CCSWeapon* weapon = (CCSWeapon*)(base->GetClientEntityByHandle(base->mEntList, *local->m_hActiveWeapon()));

		if (group == HITGROUP_HEAD)
			damage *= 4.0f;
		else if (group == HITGROUP_LOWER_BODY)
			damage *= 1.25f;
		else if (group == HITGROUP_UPPER_BODY)
			damage *= 1.0f;
		else if (group == HITGROUP_LEGS)
			damage *= 0.75f;

		if (*player->m_ArmorValue() > 0)
			damage *= weapon->GetWeaponInfo()->GetArmorRatio() * 0.5f;

		return damage;
	}
	inline bool IsMainHitbox(int hitbox, int box)
	{
		if (hitbox == 0 && box == HITBOX_HEAD)
			return true;

		if ((hitbox != 0) && box == mBodyHitbox)
			return true;

		return false;
	}
	int GetBodyPriority(int hitbox)
	{
		//switch (hitbox)
		{
			if (hitbox == HITBOX_NECK)
				return 1;

			if (hitbox == HITBOX_UPPER_CHEST)
				return 2;

			if (hitbox == HITBOX_CHEST)
				return 3;

			if (hitbox == HITBOX_LOWER_CHEST)
				return 4;
		}

		return 5;
	}

	void PlayerHurt(Entity* local, Entity* player, int hitgroup, int damage, bool local_attacker);
	void BulletImpact(Entity* player, float x, float y, float z);

	int GetMaxShots(Entity* player, CCSWeapon* weapon, Vector pos, int max_shots, int max_walls, int hitbox, bool* hit_wall);
	bool IsValidHitbox(int hitbox, int box, int hitscan);
	bool IsPointscanHitbox(int box);
	bool GetAimPosition(CAimTarget* target, CAimbotFrame* frame);
	bool FindAimPosition(CAimTarget* target);
	bool IsValidTarget(Entity* local, Entity* player);
	void FindTarget(Entity* local);

	void HandleFiring(Entity* local, UserCmd* cmd, Vector angles);
	void PreThink(Entity* local);
	bool HitchanceThink(Entity* local, CCSWeapon* weapon);
	void Think(Entity* local, UserCmd* cmd);
	int PerformIntersection(Entity* player, Matrix* matrix, Vector vieworigin, Vector delta, Vector* test, float scale = 1.0f);

	int mDropPeriod;
	bool mInit;
	bool mNoSpread;
	bool mOldHitbox;

	bool mAimKey;
	int mLagTicks;
	int mInterpTicks;
	int mNetLag;
	int mRealNetLag;
	float mGravityValue;
	int mBodyHitbox;
	bool mHitLowerHitchance;

	CAimTarget mTarget;
	CAimTarget mLastTarget;
	int mTargetsIndex;

	int mFrameCount;
	CAimbotFrame mFrames[MAX_FRAMES];
	CAimPlayer mAimPlayers[MAX_PLAYERS];
	CAimPlayer* mImpactLastPlayer;

	/* DO NOT DECLARE UNDER THIS LINE */
};

extern CCSAimbot* gAim;