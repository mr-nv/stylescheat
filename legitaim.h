struct CSmokeEffect
{
	bool mActive;
	bool mEffect;
	int mIndex;
	bool mExpiring;
	float mExpireTime;
	Vector mOrigin;
};

class CLegitAim
{
public:
	bool Insight(Entity* player, Vector pos);
	bool IsValidTarget(Entity* local, Entity* player);
	bool FindFrameIntersection(Entity* player, Vector forward, int* tick, Matrix* matrix);
	bool GetRefinePosition(Entity* player, Vector* aimpos, Vector forward);
	bool GetAimPosition(Entity* player, Vector* aimpos, Vector forward);
	bool FindTriggerIntersection(Entity* player, Vector forward, int* tick);
	void Think(Entity* local, UserCmd* cmd);
	bool AimbotThink(Entity* local, UserCmd* cmd);
	void FixAngles(Vector* angles);
	void NormalizeAngles(Vector* angles);
	void test(Entity* local, UserCmd* cmd);
	bool test2(Entity* player, Vector angles, Vector delta, float* fvalue);
	void RcsSmoothBack(UserCmd* cmd);
	bool TriggerThink(Entity* local, UserCmd* cmd);
	void AccuracyThink(Entity* local, UserCmd* cmd);
	void HistoryAim(Entity* local, UserCmd* cmd);
	void RefineAim(Entity* local, UserCmd* cmd);
	void SmokeThink();
	bool IntersectWithSmoke(Vector pos);
	inline void AddSmoke(Entity* entity)
	{
		for (int i = 0; i < 32; ++i)
		{
			CSmokeEffect* smoke = &mSmokeEffects[i];
			if (!smoke)
				continue;

			if (entity->GetIndex() == smoke->mIndex)
				break;

			if (smoke->mActive)
				continue;

			smoke->mActive = true;
			smoke->mIndex = entity->GetIndex();

			break;
		}
	}

	// general
	Vector mTargetViewangles;

	// accuracy
	Entity* mAccuratePlayer;
	Matrix mTargetMatrix[20];
	float mRefineFOV;

	// aim
	bool mAimKey;
	float mAimTime;
	float mStartAimTime;
	bool mStartedAiming;
	float mAimVelocityMag;
	Entity* mLastTarget;
	Entity* mPrimaryTarget;

	// rcs
	Vector mPunchAngles;
	Vector mPunchAnglesVel;

	// trigger
	bool mTriggerKey;
	float mLastAttackTime;
	float mLastTriggerTime;
	float mLastFailedTriggerTime;
	float mDelayToAdd;
	float mPlayersInFovTime[MAX_PLAYERS];

	// smoke
	CSmokeEffect mSmokeEffects[32];
};

extern CLegitAim gLegit;