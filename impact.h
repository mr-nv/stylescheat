class CImpactEvent
{
public:
	virtual void pad1() {};
	virtual void FireGameEvent(GameEvent* evt)
	{
		if (!base)
			return;

		Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
		if (!IsEntityValid(local))
			return;

		int userid = evt->GetInt("userid");
		float x = evt->GetInt("x");
		float y = evt->GetInt("y");
		float z = evt->GetInt("z");

		PlayerInfo info1;

		for (int i = 1; i <= base->mGlobalVars->clients; ++i)
		{
			Entity* player = base->GetClientEntity(base->mEntList, i);
			if (!IsEntityValid(player))
				continue;

			base->GetPlayerInfo(base->mBaseEngine, i, &info1);

			if (info1.userid != userid)
				continue;

			gAim->BulletImpact(player, x, y, z);
		}
	}
	virtual int GetEventDebugID()
	{
		return 42;
	}
};

extern CImpactEvent bullet_impact;