#define MAX_PLAYERS 129

#define	FL_ONGROUND				(1<<0)	// At rest / on the ground
#define FL_DUCKING				(1<<1)	// Player flag -- Player is fully crouched

enum
{
	FRAME_UNDEFINED=-1,			// (haven't run any frames yet)
	FRAME_START,

	// A network packet is being recieved
	FRAME_NET_UPDATE_START,
		// Data has been received and we're going to start calling PostDataUpdate
		FRAME_NET_UPDATE_POSTDATAUPDATE_START,
		// Data has been received and we've called PostDataUpdate on all data recipients
		FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// We've received all packets, we can now do interpolation, prediction, etc..
	FRAME_NET_UPDATE_END,		

	// We're about to start rendering the scene
	FRAME_RENDER_START,
	// We've finished rendering the scene.
	FRAME_RENDER_END
};

enum
{
	MOVETYPE_NONE		= 0,	// never moves
	MOVETYPE_ISOMETRIC,			// For players -- in TF2 commander view, etc.
	MOVETYPE_WALK,				// Player only - moving on the ground
	MOVETYPE_STEP,				// gravity, special edge handling -- monsters use this
	MOVETYPE_FLY,				// No gravity, but still collides with stuff
	MOVETYPE_FLYGRAVITY,		// flies through the air + is affected by gravity
	MOVETYPE_VPHYSICS,			// uses VPHYSICS for simulation
	MOVETYPE_PUSH,				// no clip to world, push and crush
	MOVETYPE_NOCLIP,			// No gravity, no collisions, still do velocity/avelocity
	MOVETYPE_LADDER,			// Used by players only when going onto a ladder
	MOVETYPE_OBSERVER,			// Observer movement, depends on player's observer mode
	MOVETYPE_CUSTOM,			// Allows the entity to describe its own physics

	// should always be defined as the last item in the list
	MOVETYPE_LAST		= MOVETYPE_CUSTOM,

	MOVETYPE_MAX_BITS	= 4
};

enum
{
	CONTENTS_SOLID = 0x1,
	CONTENTS_WINDOW = 0x2,
	CONTENTS_GRATE = 0x8,
	CONTENTS_DEBRIS = 0x4000000,
	CONTENTS_DETAIL = 0x8000000,
	CONTENTS_LADDER = 0x20000000,
};

enum
{
	IN_ATTACK = (1 << 0),
	IN_JUMP	= (1 << 1),
	IN_DUCK = (1 << 2),
	IN_FORWARD = (1 << 3),
	IN_BACK = (1 << 4),
	IN_USE = (1 << 5),
	IN_CANCEL = (1 << 6),
	IN_LEFT = (1 << 7),
	IN_RIGHT = (1 << 8),
	IN_MOVELEFT = (1 << 9),
	IN_MOVERIGHT = (1 << 10),
	IN_ATTACK2 = (1 << 11),
	IN_RUN = (1 << 12),
	IN_RELOAD = (1 << 13),
	IN_ALT1 = (1 << 14),
	IN_ALT2 = (1 << 15),
	IN_SCORE = (1 << 16),	// Used by client.dll for when scoreboard is held down
	IN_SPEED = (1 << 17),	// Player is holding the speed key
	IN_WALK = (1 << 18),	// Player holding walk key
	IN_ZOOM	= (1 << 19),	// Zoom key for HUD zoom
	IN_WEAPON1 = (1 << 20),	// weapon defines these bits
	IN_WEAPON2 = (1 << 21),	// weapon defines these bits
	IN_BULLRUSH = (1 << 22),
	IN_AIRSTUCK = 0x40000000 // cheat defined
};

enum
{
	WEAPON_GOLD_KNIFE = 0,
	WEAPON_DEAGLE = 1,
	WEAPON_ELITE = 2,
	WEAPON_FIVESEVEN = 3,
	WEAPON_GLOCK = 4,
	WEAPON_AK47 = 7,
	WEAPON_AUG = 8,
	WEAPON_AWP = 9,
	WEAPON_FAMAS = 10,
	WEAPON_G3SG1 = 11,
	WEAPON_GALILAR = 13,
	WEAPON_M249 = 14,
	WEAPON_M4A1 = 16,
	WEAPON_MAC10 = 17,
	WEAPON_P90 = 19,
	WEAPON_UMP45 = 24,
	WEAPON_XM1014 = 25,
	WEAPON_BIZON = 26,
	WEAPON_MAG7 = 27,
	WEAPON_NEGEV = 28,
	WEAPON_SAWEDOFF = 29,
	WEAPON_TEC9 = 30,
	WEAPON_TASER = 31,
	WEAPON_HKP2000 = 32,
	WEAPON_MP7 = 33,
	WEAPON_MP9 = 34,
	WEAPON_NOVA = 35,
	WEAPON_P250 = 36,
	WEAPON_SCAR20 = 38,
	WEAPON_SG556 = 39,
	WEAPON_SSG08 = 40,
	WEAPON_KNIFE = 42,
	WEAPON_FLASHBANG = 43,
	WEAPON_HEGRENADE = 44,
	WEAPON_SMOKEGRENADE = 45,
	WEAPON_MOLOTOV = 46,
	WEAPON_DECOY = 47,
	WEAPON_INCGRENADE = 48,
	WEAPON_C4 = 49,
	WEAPON_KNIFE_T = 59,
	WEAPON_M4A1_SILENCER = 60,
	WEAPON_USP_SILENCER = 61,
	WEAPON_CZ75A = 63,
	WEAPON_REVOLVER = 64,
};

enum
{
	BUTTON_CODE_INVALID = -1,
	BUTTON_CODE_NONE = 0,

	KEY_FIRST = 0,

	KEY_NONE = KEY_FIRST,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_PAD_0,
	KEY_PAD_1,
	KEY_PAD_2,
	KEY_PAD_3,
	KEY_PAD_4,
	KEY_PAD_5,
	KEY_PAD_6,
	KEY_PAD_7,
	KEY_PAD_8,
	KEY_PAD_9,
	KEY_PAD_DIVIDE,
	KEY_PAD_MULTIPLY,
	KEY_PAD_MINUS,
	KEY_PAD_PLUS,
	KEY_PAD_ENTER,
	KEY_PAD_DECIMAL,
	KEY_LBRACKET,
	KEY_RBRACKET,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_BACKQUOTE,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_BACKSLASH,
	KEY_MINUS,
	KEY_EQUAL,
	KEY_ENTER,
	KEY_SPACE,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_CAPSLOCK,
	KEY_NUMLOCK,
	KEY_ESCAPE,
	KEY_SCROLLLOCK,
	KEY_INSERT,
	KEY_DELETE,
	KEY_HOME,
	KEY_END,
	KEY_PAGEUP,
	KEY_PAGEDOWN,
	KEY_BREAK,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_LCONTROL,
	KEY_RCONTROL,
	KEY_LWIN,
	KEY_RWIN,
	KEY_APP,
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_RIGHT,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_CAPSLOCKTOGGLE,
	KEY_NUMLOCKTOGGLE,
	KEY_SCROLLLOCKTOGGLE,

	KEY_LAST = KEY_SCROLLLOCKTOGGLE,
	KEY_COUNT = KEY_LAST - KEY_FIRST + 1,

	// Mouse
	MOUSE_FIRST = KEY_LAST + 1,

	MOUSE_LEFT = MOUSE_FIRST,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
	MOUSE_4,
	MOUSE_5,
	MOUSE_WHEEL_UP,		// A fake button which is 'pressed' and 'released' when the wheel is moved up 
	MOUSE_WHEEL_DOWN,	// A fake button which is 'pressed' and 'released' when the wheel is moved down

	MOUSE_LAST = MOUSE_WHEEL_DOWN,
	MOUSE_COUNT = MOUSE_LAST - MOUSE_FIRST + 1,

	// Joystick
	JOYSTICK_FIRST = MOUSE_LAST + 1,

	JOYSTICK_FIRST_BUTTON = JOYSTICK_FIRST,
	JOYSTICK_LAST_BUTTON = 115,
	JOYSTICK_FIRST_POV_BUTTON,
	JOYSTICK_LAST_POV_BUTTON = 117,
	JOYSTICK_FIRST_AXIS_BUTTON,
	JOYSTICK_LAST_AXIS_BUTTON = 119,

	JOYSTICK_LAST = JOYSTICK_LAST_AXIS_BUTTON,

	BUTTON_CODE_LAST,
	BUTTON_CODE_COUNT = BUTTON_CODE_LAST - KEY_FIRST + 1,

	// Helpers for XBox 360
	KEY_XBUTTON_UP = JOYSTICK_FIRST_POV_BUTTON,	// POV buttons
	KEY_XBUTTON_RIGHT,
	KEY_XBUTTON_DOWN,
	KEY_XBUTTON_LEFT,

	KEY_XBUTTON_A = JOYSTICK_FIRST_BUTTON,		// Buttons
	KEY_XBUTTON_B,
	KEY_XBUTTON_X,
	KEY_XBUTTON_Y,
	KEY_XBUTTON_LEFT_SHOULDER,
	KEY_XBUTTON_RIGHT_SHOULDER,
	KEY_XBUTTON_BACK,
	KEY_XBUTTON_START,
	KEY_XBUTTON_STICK1,
	KEY_XBUTTON_STICK2,

	KEY_XSTICK1_RIGHT = JOYSTICK_FIRST_AXIS_BUTTON,	// XAXIS POSITIVE
	KEY_XSTICK1_LEFT,							// XAXIS NEGATIVE
	KEY_XSTICK1_DOWN,							// YAXIS POSITIVE
	KEY_XSTICK1_UP,								// YAXIS NEGATIVE
	KEY_XBUTTON_LTRIGGER,						// ZAXIS POSITIVE
	KEY_XBUTTON_RTRIGGER,						// ZAXIS NEGATIVE
	KEY_XSTICK2_RIGHT,							// UAXIS POSITIVE
	KEY_XSTICK2_LEFT,							// UAXIS NEGATIVE
	KEY_XSTICK2_DOWN,							// VAXIS POSITIVE
	KEY_XSTICK2_UP,								// VAXIS NEGATIVE
	KEY_MAX = KEY_XSTICK2_UP
};

enum
{
	MATERIAL_VAR_DEBUG					  = (1 << 0),
	MATERIAL_VAR_NO_DEBUG_OVERRIDE		  = (1 << 1),
	MATERIAL_VAR_NO_DRAW				  = (1 << 2),
	MATERIAL_VAR_USE_IN_FILLRATE_MODE	  = (1 << 3),

	MATERIAL_VAR_VERTEXCOLOR			  = (1 << 4),
	MATERIAL_VAR_VERTEXALPHA			  = (1 << 5),
	MATERIAL_VAR_SELFILLUM				  = (1 << 6),
	MATERIAL_VAR_ADDITIVE				  = (1 << 7),
	MATERIAL_VAR_ALPHATEST				  = (1 << 8),
	MATERIAL_VAR_MULTIPASS				  = (1 << 9),
	MATERIAL_VAR_ZNEARER				  = (1 << 10),
	MATERIAL_VAR_MODEL					  = (1 << 11),
	MATERIAL_VAR_FLAT					  = (1 << 12),
	MATERIAL_VAR_NOCULL					  = (1 << 13),
	MATERIAL_VAR_NOFOG					  = (1 << 14),
	MATERIAL_VAR_IGNOREZ				  = (1 << 15),
	MATERIAL_VAR_DECAL					  = (1 << 16),
	MATERIAL_VAR_ENVMAPSPHERE			  = (1 << 17),
	MATERIAL_VAR_NOALPHAMOD				  = (1 << 18),
	MATERIAL_VAR_ENVMAPCAMERASPACE	      = (1 << 19),
	MATERIAL_VAR_BASEALPHAENVMAPMASK	  = (1 << 20),
	MATERIAL_VAR_TRANSLUCENT              = (1 << 21),
	MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
	MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING  = (1 << 23),
	MATERIAL_VAR_OPAQUETEXTURE			  = (1 << 24),
	MATERIAL_VAR_ENVMAPMODE				  = (1 << 25),
	MATERIAL_VAR_SUPPRESS_DECALS		  = (1 << 26),
	MATERIAL_VAR_HALFLAMBERT			  = (1 << 27),
	MATERIAL_VAR_WIREFRAME                = (1 << 28),
	MATERIAL_VAR_ALLOWALPHATOCOVERAGE     = (1 << 29),
};

enum
{
	IE_ButtonPressed = 0,	// m_nData contains a ButtonCode_t
	IE_ButtonReleased,		// m_nData contains a ButtonCode_t
	IE_ButtonDoubleClicked,	// m_nData contains a ButtonCode_t

	IE_ButtonRepeat = 203,
};

enum
{
	FCVAR_ARCHIVE = (1<<7),
	FCVAR_UNREGISTERED = (1<<3),
	FCVAR_DEVELOPMENTONLY = (1<<1),
	FCVAR_CHEAT = (1<<14),
	FCVAR_CLIENTCMD_CAN_EXECUTE = (1<<30),
	FCVAR_SERVER_CAN_EXECUTE = (1<<28),
	FCVAR_NOT_CONNECTED = (1<<22),
	FCVAR_SPONLY = (1<<6),
	FCVAR_REPLICATED = (1<<13),
};