
#define VARTYPE_CHECKBOX 0
#define VARTYPE_LIST 1
#define VARTYPE_KEYLIST 2
#define VARTYPE_FLAGLIST 3
#define VARTYPE_SLIDER 4

#define SLIDERTYPE_NORMAL 0
#define SLIDERTYPE_PERCENT 1
#define SLIDERTYPE_1F 2
#define SLIDERTYPE_2F 3
#define SLIDERTYPE_COLOR 4
#define SLIDERTYPE_INCR 5

#define MAX_LIST_OPTIONS 16
#define MAX_GROUP_VARS 10

struct GroupVar
{
	char name[32];
	int value;
};

struct SettingVar
{
	// prerequisites
	char name[32];
	int type;
	int value;
	int default_value;

	// for lists
	char options[MAX_LIST_OPTIONS][32];

	// for sliders
	int slider_type;
	int incr;

	// min and max values
	int min;
	int max;

	// we use this to store values in groups
	bool group;
	GroupVar group_vars[MAX_GROUP_VARS];
};

struct OptionString
{
	char options[16][32];
};

#define GROUP_RIFLES (1<<0)
#define GROUP_PISTOLS (1<<1)
#define GROUP_DEAGLE (1<<2)
#define GROUP_REVOLVER (1<<3)
#define GROUP_AUTOS (1<<4)
#define GROUP_SCOUT (1<<5)
#define GROUP_AWP (1<<6)
#define GROUP_ZEUS (1<<7)

struct Vars
{
	Vars()
	{
		SecureZeroMemory(this, sizeof(*this));
	}

	void Init();

	void AddCheckbox(SettingVar* var, char* name, int default_value);
	void AddCheckbox(SettingVar* var, char* name, int default_value, GroupVar* groups);

	void AddList(SettingVar* var, char* name, int max, OptionString op, int default_value);
	void AddList(SettingVar* var, char* name, int max, OptionString op, int default_value, GroupVar* groups);

	void AddKeyList(SettingVar* var, char* name, int default_value);
	void AddKeyList(SettingVar* var, char* name, int default_value, GroupVar* groups);

	void AddFlagList(SettingVar* var, char* name, int max, OptionString op, int default_value);
	void AddFlagList(SettingVar* var, char* name, int max, OptionString op, int default_value, GroupVar* groups);

	void AddSlider(SettingVar* var, char* name, int min, int max, int slider_type, int default_value);
	void AddIncrSlider(SettingVar* var, char* name, int min, int max, int slider_type, int default_value, int incr);
	void AddSlider(SettingVar* var, char* name, int min, int max, int slider_type, int default_value, GroupVar* groups);

	SettingVar current_group;
	SettingVar groups_active;

	SettingVar l_aim;
	SettingVar l_aim_mode;
	SettingVar l_aim_key;
	SettingVar l_aim_hitbox;
	SettingVar l_aim_hitscan;
	SettingVar l_aim_single;
	SettingVar l_aim_fov;
	SettingVar l_aim_stop_fov;
	SettingVar l_aim_time;
	SettingVar l_aim_smooth;
	SettingVar l_aim_sine;
	SettingVar l_aim_prediction;
	SettingVar l_aim_start_sens;
	SettingVar l_aim_stop_sens;

	SettingVar l_trig;
	SettingVar l_trig_mode;
	SettingVar l_trig_key;
	SettingVar l_trig_hitbox;
	SettingVar l_trig_attack_delay;
	SettingVar l_trig_delay;
	SettingVar l_trig_delay_random;
	SettingVar l_trig_burst;
	SettingVar l_trig_burst_random;
	SettingVar l_trig_infov;

	SettingVar l_accuracy;
	SettingVar l_refineaim;
	SettingVar l_deltaaim;
	SettingVar l_historyaim;
	SettingVar l_historyaim_trigger;
	SettingVar l_historyaim_time;
	SettingVar l_historyaim_pred;
	SettingVar l_accuracy_smoke;
	SettingVar l_accuracy_flash;

	SettingVar l_rcs_active;
	SettingVar l_rcs_extrapolate;

	SettingVar l_teamaim;
	SettingVar l_hitbox_scale;
	SettingVar l_antiaim;
	SettingVar l_resolver;

	SettingVar aim_active;
	SettingVar aim_mode;
	SettingVar aim_key;
	SettingVar aim_silent;
	SettingVar aim_autofire;
	SettingVar aim_sticky;
	SettingVar aim_team;
	SettingVar aim_manual;

	SettingVar aim_hitbox;
	SettingVar aim_headaim;
	SettingVar aim_hitscan;
	SettingVar aim_hitscan_groups;
	SettingVar aim_pointscan;
	SettingVar aim_point_groups;
	SettingVar aim_pointscale;
	SettingVar aim_pointscale_head;
	SettingVar aim_body_hitbox;
	SettingVar aim_hp;

	SettingVar aim_autowall;
	SettingVar aim_maxshots;
	SettingVar aim_maxshots_head;
	SettingVar aim_maxwalls;
	SettingVar aim_grate;
	SettingVar aim_autozeus;
	SettingVar aim_inair;
	SettingVar aim_history;
	SettingVar aim_hps_move;
	SettingVar aim_fast_trace;
	SettingVar aim_fast_history;

	SettingVar aim_antispread;

	SettingVar aim_resolver;
	SettingVar aim_resolver_options;
	SettingVar aim_resolver_update;
	SettingVar aim_stand_delta;
	SettingVar aim_resolver_limit;

	SettingVar aim_hitchance_factor;
	SettingVar aim_hitchance_min;
	SettingVar aim_autostop;
	SettingVar aim_autoaction;

	SettingVar aim_vac_kick;
	SettingVar aim_norecoil;
	SettingVar aim_noeffects;

	SettingVar esp_active;
	SettingVar esp_team;
	SettingVar esp_name;
	SettingVar esp_health;
	SettingVar esp_weapon;
	SettingVar esp_ammo;
	SettingVar esp_box;
	SettingVar esp_healthbar;
	SettingVar esp_healthbar_fraction;
	SettingVar esp_only_dead;

	SettingVar esp_entity;
	SettingVar esp_weapons;
	SettingVar esp_grenade;
	SettingVar esp_effects;
	SettingVar esp_bomb;
	SettingVar esp_defusekit;
	SettingVar esp_grenade_tracer;
	SettingVar esp_tracer_log;

	SettingVar esp_speclist;
	SettingVar esp_impacts;
	SettingVar render_fov;
	SettingVar render_fov_viewmodel;
	SettingVar esp_hitbox;
	SettingVar esp_rotation;
	SettingVar esp_debug;
	SettingVar esp_body;
	SettingVar esp_no_scope;
	SettingVar esp_history;

	SettingVar render_chams;
	SettingVar render_chams_ents;
	SettingVar render_chams_glass;
	SettingVar render_wirehands;
	SettingVar render_noteam;
	SettingVar render_noweapon;
	SettingVar render_asus;
	SettingVar render_asus_blend;
	SettingVar render_asus_additive;

	SettingVar esp_color_t;
	SettingVar esp_color_ct;
	SettingVar esp_color_box_t;
	SettingVar esp_color_box_ct;
	SettingVar render_chams_tvisible;
	SettingVar render_chams_tinvisible;
	SettingVar render_chams_ctvisible;
	SettingVar render_chams_ctinvisible;
	SettingVar render_hands_color;
	SettingVar menu_color;

	SettingVar aa_active;
	SettingVar aa_pitch;
	SettingVar aa_yaw;
	SettingVar aa_yaw_base;
	SettingVar aa_yaw_delta;
	SettingVar aa_yaw_move;
	SettingVar aa_body;
	SettingVar aa_body_yaw;
	SettingVar aa_body_update;
	SettingVar aa_body_last;

	SettingVar aa_yaw_fake;
	SettingVar aa_jitter_delay;
	SettingVar aa_base;
	SettingVar aa_shoot;
	SettingVar aa_faceaim;
	
	SettingVar aa_body_wrap;
	SettingVar aa_velocity;
	SettingVar aa_left_key;
	SettingVar aa_right_key;
	SettingVar aa_key_toggle;
	SettingVar aa_fakewalk;
	SettingVar aa_fakewalk_key;
	SettingVar aa_fakewalk_shoot;

	SettingVar aa_jump_lag;
	SettingVar aa_stand_lag;
	SettingVar aa_fakelag_speed;
	SettingVar aa_valvemm;

	SettingVar move_strafer;
	SettingVar move_speed;
	SettingVar move_leftright;
	SettingVar move_left_key;
	SettingVar move_right_key;
	SettingVar move_left_speed;
	SettingVar move_right_speed;
	SettingVar move_leftright_accel;

	SettingVar move_airstuck;
	SettingVar move_airstuck_key;
	SettingVar move_airstuck_toggle;
	SettingVar move_speedhack;
	SettingVar move_speedhack_key;
	SettingVar move_speedhack_factor;
	SettingVar move_duckjump;

	SettingVar peppers;
	SettingVar misc_menukey;
	SettingVar misc_unloadkey;
	SettingVar misc_antiscreen;
	SettingVar misc_killsay;
	SettingVar misc_exec_buy;
	SettingVar misc_textingame;
	SettingVar misc_server;

	SettingVar misc_psilent;
	SettingVar misc_autohop;
	SettingVar misc_autopistol;
	SettingVar misc_quicktoss;
	SettingVar misc_accuracy_switch;
	SettingVar misc_revolver;
	SettingVar misc_revolver_visible;
	SettingVar misc_scout;
	SettingVar misc_nopure;
	SettingVar misc_spectate;

	SettingVar misc_getout;
	SettingVar misc_space_name;
	SettingVar misc_control_bot;
	SettingVar misc_1488;
	SettingVar misc_force_interp;
	SettingVar misc_cvarbypass;
	SettingVar misc_reconnect;
	SettingVar misc_antiafk;

	SettingVar player_filter;
	SettingVar player_hitbox;
	SettingVar player_fixpitch;
	SettingVar player_fixyaw;
	SettingVar player_resolve_pitch;
	SettingVar player_ignore_resolve;

	SettingVar test;
	SettingVar test2;
};

extern Vars gVars;