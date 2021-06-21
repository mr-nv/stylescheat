#define CLR_DARK 22, 22, 22, 255
#define CLR_BACKGROUND ((BYTE*)(&(gVars.menu_color.value)))[0], ((BYTE*)(&(gVars.menu_color.value)))[1], ((BYTE*)(&(gVars.menu_color.value)))[2], 255 // 28, 28, 28, 255
#define CLR_HALF 50, 50, 50, 255
#define CLR_HALF_BRIGHT 70, 70, 70, 255
#define CLR_BRIGHT 150, 150, 150, 255
#define CLR_SELECT 170, 170, 170, 255
#define CLR_SELECT_SLIDER 190, 190, 190, 255
#define CLR_TRANSPARENT 22, 22, 22, 224
#define CLR_TRANSPARENT_SELECT 128, 128, 128, 224

#define CLR_BLACK 0, 0, 0, 255
#define CLR_WHITE 255, 255, 255, 255

#define CLR_SELECT_GREEN 155, 205, 155, 255
#define CLR_SELECT_RED 205, 155, 155, 255
#define CLR_LIGHT_GREEN 105, 205, 105, 255
#define CLR_LIGHT_RED 205, 105, 105, 255

#define CLR_BLUE 0, 144, 255, 255
#define CLR_GREEN 0, 208, 0, 255
#define CLR_YELLOW 224, 224, 0, 255

#define CLR_SLIDER_RED 255, 0, 0, 255
#define CLR_SLIDER_GREEN 0, 255, 0, 255
#define CLR_SLIDER_BLUE 0, 0, 255, 255

#define BASE_WIDTH 400
#define BASE_HEIGHT 300
#define BASE_SPACE 5

#define BAR_WIDTH 9
#define BAR_HEIGHT 20

#define SECTION_WIDTH 80
#define SECTION_HEIGHT 26

#define TAB_HEIGHT 20

#define TICKBOX_SIZE 16
#define LIST_WIDTH 140
#define LIST_HEIGHT 16
#define SLIDER_OFFSET 20
#define SLIDER_WIDTH 140 - SLIDER_OFFSET
#define SLIDER_HEIGHT 8
#define SLIDER_DOT_SIZE 8
#define COLOR_SLIDER_WIDTH 180

#define PCONTENTS_WIDTH 280
#define PCONTENTS_SPACE 30
#define PCONTENTS_MAX_COUNT 18

#define POPUP_WIDTH 300
#define POPUP_HEIGHT 125
#define POPUP_BAR_HEIGHT 20
#define POPUP_BUTTON_WIDTH 80
#define POPUP_BUTTON_HEIGHT 20
#define POPUP_TEXTBAR_WIDTH 240
#define POPUP_TEXTBAR_HEIGHT 20

#define OPTION_ITEM_WIDTH 100
#define OPTION_ITEM_SIZE 15

#define SPECIALTAB_PLAYERLIST 1

struct MenuTab
{
	SettingVar* AddVar(SettingVar* var)
	{
		vars[var_count] = var;

		return vars[var_count++];
	}

	char name[32];
	SettingVar* vars[32];
	int var_count;
	int special;
};

struct MenuSection
{
	MenuTab* AddTab(char* string)
	{
		string_cpy((&tabs[tab_count])->name, string);

		return &tabs[tab_count++];
	}

	char name[32];
	MenuTab tabs[8];
	int tab_count;
	MenuTab* current_tab;
};

#define MAX_CONFIG_COUNT 5

struct MenuConfig
{
	char name[32];
	CRC32 crc;
	Vars vars;
};

#define POPUP_OK 0
#define POPUP_OKCANCEL 1
#define POPUP_STRING 2
#define POPUP_YESNO 3

typedef void (__cdecl* OptionFuncFn)();

struct MenuOption
{
	char name[32];
	OptionFuncFn func;

	// parameters for the popup window if we use it
	bool popup;
	char description[256];
	int type;
	int string_max;
};

typedef void (__cdecl* NoParamFn)();
typedef void (__cdecl* StringParamFn)(char* string);

struct MenuPopup
{
	char name[32];
	char description[256];
	int type;
	bool* variable;

	NoParamFn no_param;
	StringParamFn string_param;

	// string input parameters
	char string[32];
	int string_pos;
	int string_max;
};

class CMenu
{
public:
	CMenu()
	{
		SecureZeroMemory(this, sizeof(*this));
	}

	inline bool ShouldHighlightElements()
	{
		if (mCurrentVar)
			return false;

		if (mOptionMenuDrawing)
			return false;

		if (mPopupDrawing)
			return false;

		return true;
	}
	inline char* GetConfigName()
	{
		for (int i = 0; i < mConfigCount; ++i)
		{
			if (mConfigs[i].crc == mCurrentConfig)
				return mConfigs[i].name;
		}

		return "null";
	}

	void Init();
	void RegistryInit();
	void Think();
	void KeyThink(int type, int key);
	void BindThink(int type, int key);

	void Draw();
	bool CursorWithinRect(float x, float y, float w, float h);
	void DrawSections();
	void DrawConfigs();
	void DrawTabs();
	void DrawItems();
	void DrawPlayerList();
	void DrawCurrentVar();
	void DrawOptionMenu();
	void DrawPopupWindow();

	void DrawCheckbox(SettingVar* var, int i);
	void DrawList(SettingVar* var, int i);
	void DrawKeylist(SettingVar* var, int i);
	void DrawFlaglist(SettingVar* var, int i);
	void DrawSlider(SettingVar* var, int i);
	void DrawColorSlider(SettingVar* var, int i);
	void DrawCheckboxPlayer(SettingVar* var, int i);
	void DrawListPlayer(SettingVar* var, int i);

	CRC32 ReadConfig(HKEY base_key, char* name, MenuConfig* target);
	CRC32 WriteConfig(HKEY base_key, char* name, CRC32 copy);
	void WriteAll();

	void InitElements();
	MenuSection* AddSection(char* string);
	MenuOption* AddOption(char* name, OptionFuncFn func, bool popup, char* description, int type, int string_max);

	bool mInit;

	MenuSection mSections[8];
	int mSectionCount;

	MenuSection* mCurrentSection;

	MenuConfig mConfigs[MAX_CONFIG_COUNT];
	MenuConfig mCopyConfigs[MAX_CONFIG_COUNT];
	int mConfigCount;
	CRC32 mCurrentConfig;

	bool mDrawing;
	bool mOptionMenuDrawing;
	bool mPopupDrawing;
	bool mCopyDecided;
	bool mCopyCfg;

	SettingVar* mCurrentVar;
	SettingVar* mSelectedList;
	float mListPosX;
	float mListPosY;
	int mColorSelected;
	bool mVarAcceptClicks;

	MenuOption mOptions[6];
	int mOptionCount;

	MenuPopup mPopupWindow;

	int mMouseX;
	int mMouseY;
	int mMouseDeltaX;
	int mMouseDeltaY;
	bool mMouseHeld;
	bool mMouseClicked;
	bool mMouseReleased;
	bool mMouse2Held;
	bool mMouse2Clicked;
	bool mMouse2Released;

	float mMenuPosX;
	float mMenuPosY;
	bool mMenuPosInit;
	float mOptionMenuPosX;
	float mOptionMenuPosY;

	bool mIssueUnload;
	bool mCopyConfigActive;
	int mPasteConfigTime;

	int mPlayerSelected;
};

extern CMenu gMenu;

static char* key_names[] =
{
	"none",				// KEY_NONE
	"0",			// KEY_0,
	"1",			// KEY_1,
	"2",			// KEY_2,
	"3",			// KEY_3,
	"4",			// KEY_4,
	"5",			// KEY_5,
	"6",			// KEY_6,
	"7",			// KEY_7,
	"8",			// KEY_8,
	"9",			// KEY_9,
	"a",			// KEY_A,
	"b",			// KEY_B,
	"c",			// KEY_C,
	"d",			// KEY_D,
	"e",			// KEY_E,
	"f",			// KEY_F,
	"g",			// KEY_G,
	"h",			// KEY_H,
	"i",			// KEY_I,
	"j",			// KEY_J,
	"k",			// KEY_K,
	"l",			// KEY_L,
	"m",			// KEY_M,
	"n",			// KEY_N,
	"o",			// KEY_O,
	"p",			// KEY_P,
	"q",			// KEY_Q,
	"r",			// KEY_R,
	"s",			// KEY_S,
	"t",			// KEY_T,
	"u",			// KEY_U,
	"v",			// KEY_V,
	"w",			// KEY_W,
	"x",			// KEY_X,
	"y",			// KEY_Y,
	"z",			// KEY_Z,
	"KP_INS",		// KEY_PAD_0,
	"KP_END",		// KEY_PAD_1,
	"KP_DOWNARROW",	// KEY_PAD_2,
	"KP_PGDN",		// KEY_PAD_3,
	"KP_LEFTARROW",	// KEY_PAD_4,
	"KP_5",			// KEY_PAD_5,
	"KP_RIGHTARROW",// KEY_PAD_6,
	"KP_HOME",		// KEY_PAD_7,
	"KP_UPARROW",	// KEY_PAD_8,
	"KP_PGUP",		// KEY_PAD_9,
	"KP_SLASH",		// KEY_PAD_DIVIDE,
	"KP_MULTIPLY",	// KEY_PAD_MULTIPLY,
	"KP_MINUS",		// KEY_PAD_MINUS,
	"KP_PLUS",		// KEY_PAD_PLUS,
	"KP_ENTER",		// KEY_PAD_ENTER,
	"KP_DEL",		// KEY_PAD_DECIMAL,
	"[",			// KEY_LBRACKET,
	"]",			// KEY_RBRACKET,
	"SEMICOLON",	// KEY_SEMICOLON,
	"'",			// KEY_APOSTROPHE,
	"`",			// KEY_BACKQUOTE,
	",",			// KEY_COMMA,
	".",			// KEY_PERIOD,
	"/",			// KEY_SLASH,
	"\\",			// KEY_BACKSLASH,
	"-",			// KEY_MINUS,
	"=",			// KEY_EQUAL,
	"ENTER",		// KEY_ENTER,
	"SPACE",		// KEY_SPACE,
	"BACKSPACE",	// KEY_BACKSPACE,
	"TAB",			// KEY_TAB,
	"CAPSLOCK",		// KEY_CAPSLOCK,
	"NUMLOCK",		// KEY_NUMLOCK,
	"ESCAPE",		// KEY_ESCAPE,
	"SCROLLLOCK",	// KEY_SCROLLLOCK,
	"INS",			// KEY_INSERT,
	"DEL",			// KEY_DELETE,
	"HOME",			// KEY_HOME,
	"END",			// KEY_END,
	"PGUP",			// KEY_PAGEUP,
	"PGDN",			// KEY_PAGEDOWN,
	"PAUSE",		// KEY_BREAK,
	"SHIFT",		// KEY_LSHIFT,
	"RSHIFT",		// KEY_RSHIFT,
	"ALT",			// KEY_LALT,
	"RALT",			// KEY_RALT,
	"CTRL",			// KEY_LCONTROL,
	"RCTRL",		// KEY_RCONTROL,
	"LWIN",			// KEY_LWIN,
	"RWIN",			// KEY_RWIN,
	"APP",			// KEY_APP,
	"UPARROW",		// KEY_UP,
	"LEFTARROW",	// KEY_LEFT,
	"DOWNARROW",	// KEY_DOWN,
	"RIGHTARROW",	// KEY_RIGHT,
	"F1",			// KEY_F1,
	"F2",			// KEY_F2,
	"F3",			// KEY_F3,
	"F4",			// KEY_F4,
	"F5",			// KEY_F5,
	"F6",			// KEY_F6,
	"F7",			// KEY_F7,
	"F8",			// KEY_F8,
	"F9",			// KEY_F9,
	"F10",			// KEY_F10,
	"F11",			// KEY_F11,
	"F12",			// KEY_F12,

	// FIXME: CAPSLOCK/NUMLOCK/SCROLLLOCK all appear above. What are these for?!
	// They only appear in CInputWin32::UpdateToggleButtonState in vgui2
	"CAPSLOCKTOGGLE",	// KEY_CAPSLOCKTOGGLE,
	"NUMLOCKTOGGLE",	// KEY_NUMLOCKTOGGLE,
	"SCROLLLOCKTOGGLE", // KEY_SCROLLLOCKTOGGLE,

	// Mouse
	"MOUSE1",		// MOUSE_LEFT,
	"MOUSE2",		// MOUSE_RIGHT,
	"MOUSE3",		// MOUSE_MIDDLE,
	"MOUSE4",		// MOUSE_4,
	"MOUSE5",		// MOUSE_5,

	"MWHEELUP",		// MOUSE_WHEEL_UP
	"MWHEELDOWN",	// MOUSE_WHEEL_DOWN
};