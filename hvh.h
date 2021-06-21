class CHvhManager
{
public:
	CHvhManager()
	{
		SecureZeroMemory(this, sizeof(*this));
	}
	
	inline bool IsAntiAimFake()
	{
		if (mFakingYaw)
			return true;

		return false;
	}

	void Think(Entity* local, UserCmd* cmd);
	bool ShouldUseAntiAim(Entity* local, UserCmd* cmd);
	void AntiAimThink(Entity* local, UserCmd* cmd);
	void FakelagThink(Entity* local, UserCmd* cmd);
	void ShootAntiAimThink(Entity* local, UserCmd* pitch_cmd, int frame);
	float GetDecayedVel(Entity* local, int t)
	{
		Vector pred = gClient->mLocalVelocity;

		float sv_friction = base->FindVar(base->mEngineCvar, "sv_friction")->GetFloat();
		float sv_stopspeed = base->FindVar(base->mEngineCvar, "sv_stopspeed")->GetFloat();

		for (int i = 0; i < t; ++i)
		{
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
		}

		return pred.LengthSqr();
	}

	bool mUsingPitch;
	bool mUsingYaw;
	Vector mRotation;
	float mLastUpdateTime;
	bool mFakingYaw;

	bool mFakewalkKey;

	bool mLagging;
	int mMaxLagTicks;
	int mLagSet;
	int mFreezeSwitch;

	int mIncr;
	bool mJitter;
	int mBodyIncr;
	bool mBodyJitter;

	bool mLeftKey;
	bool mRightKey;
	int mVelAngle;
	float mNextVelCheck;

	float mLastStop;
};

extern CHvhManager gHvh;