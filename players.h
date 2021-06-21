struct PlayerObject
{
	int id;

	int filter;
	int hitbox;
	int fixpitch;
	int fixyaw;
	int resolve_pitch;
	int ignore_resolve;
};

class CPlayers
{
public:
	CPlayers()
	{
		SecureZeroMemory(this, sizeof(*this));
	}
	
	void Think();
	PlayerObject* GetObject(int);
	inline int GetListPos(DWORD id)
	{
		for (int i = 0; i < mIndex; ++i)
		{
			if (mList[i].id == id)
				return i;
		}

		return -1;
	}

	PlayerObject mList[MAX_PLAYERS];
	int mIndex;
};

extern CPlayers gPlayers;