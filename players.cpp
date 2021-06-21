#include "dll.h"

CPlayers gPlayers;

void CPlayers::Think()
{
	if (mIndex > 1000)
	{
		base->Warning("error: overflow in player list\n");

		memset(mList, 0, sizeof(PlayerObject) * 1024);
		mIndex = 0;
	}

	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* player = player = base->GetClientEntity(base->mEntList, i);

		if (!IsEntityValid(player))
			continue;

		if (player == local)
			continue;

		PlayerInfo info;
		base->GetPlayerInfo(base->mBaseEngine, i, &info);

		DWORD id = CRC32_Get(info.guid,strlen(info.guid));
		if (id == 0x97B8469B) // "BOT"
			continue;

		int pos = GetListPos(id);

		if (pos != -1)
			continue;

		int n = mIndex++;

		mList[n].id = id;
	}
}

PlayerObject* CPlayers::GetObject(int index)
{
	if (index == -1)
		return 0;

	PlayerInfo info;
	base->GetPlayerInfo(base->mBaseEngine, index, &info);

	DWORD crc = CRC32_Get(info.guid, strlen(info.guid));

	for (int i = 0; i < mIndex; ++i)
	{
		if (mList[i].id == crc)
			return &mList[i];
	}

	return 0;
}