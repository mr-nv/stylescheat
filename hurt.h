class CHurtEvent
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
		int attacker = evt->GetInt("attacker");
		int hitgroup = evt->GetInt("hitgroup");
		int damage = evt->GetInt("dmg_health");

		PlayerInfo info1, info2;

		base->GetPlayerInfo(base->mBaseEngine, local->GetIndex(), &info1);

		for (int i = 1; i <= base->mGlobalVars->clients; ++i)
		{
			Entity* player = base->GetClientEntity(base->mEntList, i);
			if (!IsEntityValid(player))
				continue;

			base->GetPlayerInfo(base->mBaseEngine, i, &info2);

			if (info2.userid != userid)
				continue;

			gAim->PlayerHurt(local, player, hitgroup, damage, info1.userid == attacker);
		}
	}
	virtual int GetEventDebugID()
	{
		return 42;
	}
};

extern CHurtEvent player_hurt;