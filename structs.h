class Entity;

struct DVariant
{
	union
	{
		float fl;
		char pad[12];
		float vec[3];
		__int64 int64;
	};

	int type;
};

struct ProxyData
{
	int pad;
	DVariant var;
};

class GameEvent
{
public:
	bool GetBool(char* name)
	{
		typedef bool (__thiscall* dfn)(void* thisptr, char* name, int default_value);
		return ((dfn)((*(DWORD**)(this))[5]))(this, name, 0);
	}
	int GetInt(char* name)
	{
		typedef int (__thiscall* dfn)(void* thisptr, char* name, int default_value);
		return ((dfn)((*(DWORD**)(this))[6]))(this, name, 0);
	}
	float GetFloat(char* name)
	{
		typedef float (__thiscall* dfn)(void* thisptr, char* name, float default_value);
		return ((dfn)((*(DWORD**)(this))[7]))(this, name, 0.0f);
	}
	char* GetString(char* name)
	{
		typedef char* (__thiscall* dfn)(void* thisptr, char* name, int default_value);
		return ((dfn)((*(DWORD**)(this))[8]))(this, name, 0);
	}
};

class NetMsg
{
public:
	int GetType()
	{
		typedef int (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(this))[7]))(this);
	}
};

struct GlobalVars
{
	float realtime;
	int framecount;
	float absoluteframetime;
	char pad[4];
	float curtime;
	float frametime;
	int clients;
	int ticks;
	float interval;
	float interp;
};

#define USERCMD_DELTA_MAX 8

struct UserCmd
{
	int vmt;
	int index;
	int ticks;
	Vector angles;
	Vector dir;
	Vector move;
	int	buttons;
	char pad1;
	int select;
	int pad2;
	int seed;
	short mousedx;
	short mousedy;
	char predicted;
	char pad3[24];
};

struct ViewSetup
{
	char pad[176];
	float fov;
	float vm_fov;
};

struct VerifiedCmd
{
	UserCmd cmd;
	DWORD crc;
};

struct PlayerInfo
{
	char pad1[16];
	char name[128];
	int userid;
	char guid[33];
	char pad3[256];
};

struct HardwareData
{
	char pad1[4];
	int num_lods;
};

struct DrawModelInfo
{
	char pad1[4];
	HardwareData* hwdata;
	char pad2[16];
	void* entity;
};

struct RayData
{
	RayData()
	{
		SecureZeroMemory(this, sizeof(*this));
		isray = true;
		hasdelta = true;
	}
	inline void Init(Vector start, Vector end)
	{
		base = start;
		delta = (end - start);
	}
	inline void InitHull(Vector start, Vector end, Vector min, Vector max)
	{
		base = start;
		delta = (end - start);

		extents = max - min;
		extents *= 0.5f;
		isray = extents.LengthSqr() < 1e-6;

		offset = min + max;
		offset *= 0.5f;
		base = (start + offset);
		offset *= -1.0f;
	}

	Vector base;
	int pad1;
	Vector delta;
	int pad2;
	Vector offset;
	int pad3;
	Vector extents;
	int pad4;
	int pad5;
	bool isray;
	bool hasdelta;
};

struct TraceResult
{
	TraceResult()
	{
		SecureZeroMemory(this, sizeof(*this));
	}

	char pad1[12];
	Vector hitpos;
	Vector normal;
	char pad2[8];
	float fraction;
	int contents;
	short dispflags;
	bool allsolid;
	bool startsolid;
	float left;
	char pad3[4];
	short props;
	unsigned short surfaceflags;
	char pad4[8];
	Entity* ent;
	int hitbox;
};

struct TraceFilter;

struct HitBox
{
	inline Vector GetMins()
	{
		return min;
	}
	inline Vector GetMaxs()
	{
		return max;
	}

	int bone;
	int group;
	Vector min;
	Vector max;
	int nameindex;
	int pad[3];
	float radius;
	int pad2[4];
};

struct StudioModel
{
	HitBox* GetHitBox(int index)
	{
		void* set = (void*)((DWORD)(this) + setindex);
		if (!set)
			return 0;

		int boxindex = *(int*)((DWORD)(set) + 0x8);
		
		return (HitBox*)((DWORD)(set) + boxindex);
	}

	char pad[176];
	int setindex;
};

#define MAX_POSE_PARAMETERS 24
#define MAX_ANIM_LAYERS 13

struct AnimLayer
{
	char pad1[20]; // 20
	int order; // 24
	int sequence; // 28
	char pad2[4]; // 32
	float cycle2; // 36
	float weight;//char pad5[4]; // 40, m_flWeight
	char pad7[4]; // 44, m_flWeightDeltaRate
	float cycle; // 48
	char pad9[4]; // 52
	char pad10[4]; // 56
}; // 56

struct ConCmdBase
{
	ConCmdBase* GetNext()
	{
		return (ConCmdBase*)(this + 0x4);
	}
	char** GetName()
	{
		return (char**)(this + 0xC);
	}
	int& GetFlags()
	{
		return *(int*)(this + 0x14);
	}
	char** GetString()
	{
		return (char**)(this + 0x24);
	}
	int& GetInt()
	{
		return *(int*)(this + 0x30);
	}
	float& GetFloat()
	{
		return *(float*)(this + 0x2C);
	}
	int* GetCallback()
	{
		return (int*)(this + 0x3A);
	}
};

struct ConVar
{
	int GetValue()
	{
		typedef int (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(this))[13]))(this);
	}
	void SetValueFloat(float value)
	{
		typedef void (__thiscall* dfn)(void* thisptr, float value);
		((dfn)((*(DWORD**)(this))[15]))(this, value);
	}
	void SetValueInt(int value)
	{
		typedef void (__thiscall* dfn)(void* thisptr, int value);
		((dfn)((*(DWORD**)(this))[16]))(this, value);
	}
	ConCmdBase* GetCmdBase()
	{
		return *(ConCmdBase**)(this + 0x1C);
	}
	char** GetName()
	{
		return (char**)(GetCmdBase() + 0xC);
	}
	int& GetFlags()
	{
		return *(int*)(GetCmdBase() + 0x14);
	}
	char** GetString()
	{
		return (char**)(GetCmdBase() + 0x24);
	}
	int GetInt()
	{
		return (int)(*(DWORD*)(this + 0x30) ^ (DWORD)(this));
	}
	float GetFloat()
	{
		DWORD temp = *(DWORD*)(this + 0x2c) ^ (DWORD)(this);
		return *(float*)(&temp);
	}
	void SetFloat(float x)
	{
		DWORD temp = *(DWORD*)(&x) ^ (DWORD)(this);
		*(DWORD*)(this + 0x2c) = temp;
	}
	float* GetMaxValue()
	{
		return (float*)(this + 0x3c);
	}
	int* GetCallbackSize()
	{
		return (int*)((DWORD)(&*(DWORD*)(this + 0x44)) + 0xC);
	}
};

struct CvarIteratorInternal
{
	virtual void SetFirst( void ) = 0;
	virtual void Next( void ) = 0;
	virtual	bool IsValid( void ) = 0;
	virtual ConCmdBase* Get( void ) = 0;
};

struct Material
{
	char* GetName()
	{
		typedef char* (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(this))[0]))(this);
	}
	char* GetTextureGroup()
	{
		typedef char* (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(this))[1]))(this);
	}
	void IncrementReferenceCount()
	{
		typedef void (__thiscall* dfn)(void* thisptr);
		((dfn)((*(DWORD**)(this))[12]))(this);
	}
	void SetAlpha(float a)
	{
		typedef void (__thiscall* dfn)(void* thisptr, float a);
		((dfn)((*(DWORD**)(this))[27]))(this, a);
	}
	void SetColor(float r, float g, float b)
	{
		typedef void (__thiscall* dfn)(void* thisptr, float r, float g, float b);
		((dfn)((*(DWORD**)(this))[28]))(this, r, g, b);
	}
	void SetMaterialVarFlag(int flag, bool on)
	{
		typedef void (__thiscall* dfn)(void* thisptr, int flag, bool on);
		((dfn)((*(DWORD**)(this))[29]))(this, flag, on);
	}
	char* GetShaderName()
	{
		typedef char* (__thiscall* dfn)(void* thisptr);
		return ((dfn)((*(DWORD**)(this))[49]))(this);
	}
};