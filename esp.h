#define CLR_TERRORIST 255, 224, 96, 235
#define CLR_COUNTER_TERRORIST 96, 192, 255, 235

#define TEXT_OFFSET 12

#define MAX_GRENADES 8
#define MAX_GRENADE_ORIGINS 256

#define GRENADE_FRAG 0
#define GRENADE_FLASH 1
#define GRENADE_SMOKE 2
#define GRENADE_DECOY 3
#define GRENADE_FIRE 4

class CSimulatedGrenade
{
public:
	float mLifetime;
	int mType;
	Vector mBeginOrigin;
	Vector mSimulatedOrigins[MAX_GRENADE_ORIGINS];
	int mSimulationsIndex;
	float mPredictedDamage[MAX_PLAYERS];
	bool mPlayerFlash[MAX_PLAYERS];
};

class CESPManager
{
public:
	void Think();
	bool IsWeaponEntity(char* classname);
	bool IsGrenadeEntity(char* classname);
	char* GetGrenadeName(char* modelname);
	void DrawESP();
	void DrawEntityESP();
	void DrawSpectatorList();
	void DrawGrenadeTracer();
	void GrenadeSimulationThink();
	void SimulateGrenade(CSimulatedGrenade* grenade);
	void DrawWeaponImpacts();
	void DrawPlayerRotation(Entity* player, Matrix* mtx, int r, int g, int b, bool new_array);
	void DrawText(char* text, Entity* player, int mode);
	bool ToScreen(Vector pos, float* x2, float* y2);
	void DrawBox(Vector org, Vector min, Vector max, float pitch, float yaw, float roll, int r, int g, int b);

	float mSideYOffset;
	float mTopYOffset;
	float mBottomYOffset;
	float mCenterYOffset;

	bool mSetGrenadeTime[2048];
	float mGrenadeTime[2048];
	int mSmokeEffectTickBegin[2048];

	CSimulatedGrenade mGrenades[MAX_GRENADES];
	int mGrenadesIndex;

	float mScreenCenterYOffset;
};

extern CESPManager gESP;