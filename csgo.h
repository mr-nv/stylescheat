class CPredFrame
{
public:
	int mIndex;
	char mMoveData[200];
	int mFlags;
	DWORD mGroundEntity;
	int mTickBase;
	Vector mAimPunch;
	float mDuckAmount;
	float mDuckSpeed;
};

class CPredData
{
public:
	char mMoveData[200];
	int mFlags;
	DWORD mGroundEntity;
	int mTickBase;
	Vector mAimPunch;
	float mDuckAmount;
	float mDuckSpeed;
};

class CCSGOClient
{
public:
	CCSGOClient()
	{
		memset(this, 0, sizeof(*this));
	}

	void HookPreThink();
	bool PreThink(Entity* local, UserCmd* cmd);
	void Think(UserCmd* cmd);
	void RestrictCmd(UserCmd* cmd);
	void WeaponThink(Entity* local, class CCSWeapon* weapon, UserCmd* cmd);
	void RunPrediction(Entity* local, UserCmd* cmd);
	void StraferThink(Entity* local, UserCmd* cmd);
	void CalculateMouse(UserCmd* cmd);
	void SolvePitchYaw(UserCmd* cmd, Vector spread);
	int Cfg(struct SettingVar var);
	void NetUpdateThink();
	void UpdateAnimations();
	void UpdateLocalPlayerAnimations();
	//void UpdateLocalHitboxesData();
	void UpdateLocalAnimation(Entity* local, float pitch, float yaw);
	void FixEngineSetupBones();

	//int mTimeVar;

	bool mThirdPerson;
	int mSwitchWeapons;
	int mTargetWeaponIndex;
	int mLastWeaponIndex;
	bool mSwitchWeaponZeus;
	bool mUsingAWP;

	Entity* mPredictionPlayer;
	CPredFrame mPredFrames[150];
	CPredData mPredData;
	CPredData mOldData;
	Vector mAimPunch;
	Vector mAimPunchVel;

	bool mInit;
	bool mFrozen;
	float mFrozenTime;
	bool mAliveAndProcessing;

	bool* mSendMove;
	int mLag;
	int mHeldLag;
	int mLastHeldLag;
	bool mSendNextCmd;
	bool mCopyNextCmd;
	UserCmd mNextUserCmd;

	UserCmd* mUserCmd;
	UserCmd mCopyCmd;
	UserCmd mLastCopyCmd;
	int mTickCount;
	Vector mViewAngles;
	Vector mMoveDirection;
	float mMoveYaw;
	float mMoveYawOverride;
	int mButtons;
	int mLastButtons;
	Vector mOldAngles;

	float mCurTime;
	int mTickBase;
	int mLastTickBase;
	Vector mOrigin;
	Vector mViewOrigin;
	Vector mViewOriginPredicted;
	Vector mOldViewOrigin;
	Vector mLocalVelocity;
	Vector mLastVelocity;
	Vector* mViewOriginEngine;
	float mViewZAddDelta;
	float mLastTimeMoved;
	float mShotTime;

	bool mFiring;
	bool mFireReady;
	bool mFiredThisInterval;
	Vector mSpreadAngles;
	Vector mShootAngles;
	float mSpread;
	float mInaccuracy;

	int mStrafeIndex;
	float mFrameMult;
	float mMousePitchDelta;
	float mMouseYawDelta;
	float mMouseLastPitch;
	float mMouseLastYaw;
	bool mLeftStrafe;
	bool mRightStrafe;
	bool mLoopStrafe;

	int mFlags;
	int mLastFlags;
	float mDuckAmount;
	float mDuckSpeed;
	float mMaxSpeed;
	float mNextRevolverFire;

	bool mAirstuckKey;
	bool mSpeedhackKey;

	DWORD mSendNetMsgHook;
	DWORD mSendNetMsgAddress;
	DWORD mSendNetMsgProt;

	bool mCvarsFixed;

	int mCurrentWeaponCfg;

	// for animations
	bool mLocalAnimating;

	int mAnimatedFrame;
	float mAnimatedCurTime;
	float mAnimatedFrameTime;
	Vector mAnimatedOrigin;
	float mAnimatedPitch;
	float mAnimatedYaw;
	float mAnimatedBodyYaw;
	Vector mAnimatedViewangles;
	Vector mAnimatedVelocity;
	int mAnimatedFlags;
	float mAnimatedDuckAmount;

	float mLastLocalSimulation;
	AnimLayer mAnimatedAnimLayers[MAX_ANIM_LAYERS];
	Matrix mLocalBoneMatrix[128];
	Matrix mLocalBoneMatrixBody[128];
	Matrix mLocalBoneMatrixServer[128];

	int mShootTime;
	Entity* mShootPlayer;
	Matrix mShootLastBones[20];
	int mShootFrame;
	int mShootResolveMode;
};

extern CCSGOClient* gClient;