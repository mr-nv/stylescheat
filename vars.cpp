#include "dll.h"

Vars gVars;

void Vars::Init()
{
	/*GroupVar groups[MAX_GROUP_VARS];
	string_cpy(groups[0].name, "pistols");
	string_cpy(groups[1].name, "rifles");
	string_cpy(groups[2].name, "snipers");
	string_cpy(groups[3].name, "smgs");
	string_cpy(groups[4].name, "shotguns");

	SecureZeroMemory(&op, sizeof(OptionString));
	string_cpy(op.options[0], "forward");
	string_cpy(op.options[1], "backwards");
	string_cpy(op.options[2], "left");
	string_cpy(op.options[3], "right");
	*/

	OptionString op;
	memset(&op, 0, sizeof(OptionString));

	GroupVar groups[MAX_GROUP_VARS];
	memset(groups, 0, sizeof(GroupVar) * MAX_GROUP_VARS);
	string_cpy(groups[0].name, "default");
	string_cpy(groups[1].name, "rifles");
	string_cpy(groups[2].name, "pistols");
	string_cpy(groups[3].name, "deagle");
	string_cpy(groups[4].name, "revolver");
	string_cpy(groups[6].name, "autos");
	string_cpy(groups[7].name, "scout");
	string_cpy(groups[8].name, "awp");
	string_cpy(groups[9].name, "zeus");

	// legit

	// aim

	AddCheckbox(&l_aim, "active", 0, groups); // active

	string_cpy(op.options[0], "always");
	string_cpy(op.options[1], "key");
	string_cpy(op.options[2], "+attack");
	string_cpy(op.options[3], "+speed");

	AddList(&l_aim_mode, "aim mode", 3, op, 0, groups); // active

	AddKeyList(&l_aim_key, "aim key", 0, groups);

	string_cpy(op.options[0], "head");
	string_cpy(op.options[1], "neck");
	string_cpy(op.options[2], "lower body");
	string_cpy(op.options[3], "upper body");

	AddList(&l_aim_hitbox, "aim hitbox", 3, op, 0, groups); // active

	AddCheckbox(&l_aim_hitscan, "hitscan", 0, groups);

	AddCheckbox(&l_rcs_active, "rcs active", 0, groups);
	AddCheckbox(&l_rcs_extrapolate, "rcs extrapolate", 0, groups);

	AddSlider(&l_aim_fov, "aim fov", 0, 150, SLIDERTYPE_1F, 0, groups);
	AddSlider(&l_aim_stop_fov, "stop fov", 0, 20, SLIDERTYPE_1F, 0, groups);
	AddSlider(&l_aim_time, "aim time", 0, 50, SLIDERTYPE_2F, 0, groups);
	AddSlider(&l_aim_start_sens, "start sens", 0, 100, SLIDERTYPE_2F, 0, groups);
	AddSlider(&l_aim_stop_sens, "stop sens", 0, 100, SLIDERTYPE_2F, 0, groups);
	AddSlider(&l_aim_smooth, "smooth factor", 0, 100, SLIDERTYPE_1F, 0, groups);
	AddSlider(&l_aim_sine, "smooth sine", 0, 100, SLIDERTYPE_PERCENT, 0, groups);
	AddCheckbox(&l_aim_prediction, "aim prediction", 0, groups);
	AddCheckbox(&l_aim_single, "single target", 0, groups);

	// trigger

	AddCheckbox(&l_trig, "active", 0, groups); // active

	string_cpy(op.options[0], "always");
	string_cpy(op.options[1], "key");
	string_cpy(op.options[2], "+attack");
	string_cpy(op.options[3], "+speed");

	AddList(&l_trig_mode, "trigger mode", 3, op, 0, groups); // active
	
	AddKeyList(&l_trig_key, "trigger key", 0, groups);

	string_cpy(op.options[0], "head");
	string_cpy(op.options[1], "all");

	AddList(&l_trig_hitbox, "trigger hitbox", 1, op, 0, groups);

	AddSlider(&l_trig_attack_delay, "+attack delay", 1, 20, SLIDERTYPE_2F, 3, groups);

	AddSlider(&l_trig_delay, "trigger delay", 0, 20, SLIDERTYPE_2F, 0, groups);
	AddSlider(&l_trig_delay_random, "delay random", 0, 100, SLIDERTYPE_PERCENT, 0, groups);

	AddSlider(&l_trig_burst, "trigger burst", 0, 30, SLIDERTYPE_2F, 0, groups);
	AddSlider(&l_trig_burst_random, "burst random", 0, 100, SLIDERTYPE_PERCENT, 0, groups);

	AddSlider(&l_trig_infov, "infov", 0, 50, SLIDERTYPE_2F, 0, groups);

	// accuracy

	AddCheckbox(&l_accuracy, "active", 1, groups); // active

	AddSlider(&l_refineaim, "refine aim", 0, 10, SLIDERTYPE_1F, 10, groups);

	AddCheckbox(&l_deltaaim, "delta aim", 0, groups);

	AddCheckbox(&l_historyaim, "history aim", 1, groups);

	AddCheckbox(&l_historyaim_trigger, "history aim trigger", 0, groups);

	AddSlider(&l_historyaim_time, "history aim time", 1, 100, SLIDERTYPE_PERCENT, 100, groups);

	AddCheckbox(&l_historyaim_pred, "history aim pred", 1, groups);

	AddCheckbox(&l_accuracy_smoke, "thru smoke", 0, groups);
	AddCheckbox(&l_accuracy_flash, "thru flash", 0, groups);

	// misc

	AddSlider(&l_hitbox_scale, "hitbox scale", 1, 100, SLIDERTYPE_PERCENT, 90, groups);

	AddCheckbox(&l_teamaim, "team aim", 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "normal");
	string_cpy(op.options[2], "at targets");

	AddList(&l_antiaim, "anti aim", 2, op, 0);

	AddCheckbox(&l_resolver, "resolver", 0);

	// aim

	AddCheckbox(&aim_active, "active", 0); // active

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "always");
	string_cpy(op.options[2], "key");
	string_cpy(op.options[3], "toggle key");

	AddList(&aim_mode, "aim mode", 2, op, 0);

	AddKeyList(&aim_key, "aim key", 0);

	AddCheckbox(&aim_silent, "silent aim", 1);
	AddCheckbox(&aim_autofire, "auto fire", 1);

	AddCheckbox(&aim_sticky, "sticky aim", 1);
	AddCheckbox(&aim_team, "aim at team", 0);

	AddCheckbox(&aim_manual, "stop on manual", 0);

	AddSlider(&test, "test", 0, 360, SLIDERTYPE_NORMAL, 0);
	AddSlider(&test2, "test2", 0, 100, SLIDERTYPE_NORMAL, 0);

	// position

	string_cpy(op.options[0], "head");
	string_cpy(op.options[1], "body");
	string_cpy(op.options[2], "body only");

	AddList(&aim_hitbox, "aim hitbox", 2, op, 0);

	string_cpy(op.options[0], "normal");
	string_cpy(op.options[1], "moving");
	string_cpy(op.options[2], "hsonly moving");

	AddList(&aim_headaim, "head aim", 2, op, 0);

	AddCheckbox(&aim_hitscan, "hitscan", 0);

	string_cpy(op.options[0], "head");
	string_cpy(op.options[1], "neck");
	string_cpy(op.options[2], "lower body");
	string_cpy(op.options[3], "upper body");
	string_cpy(op.options[4], "arms");
	string_cpy(op.options[5], "legs");
	string_cpy(op.options[6], "forearms");
	string_cpy(op.options[7], "calves");
	string_cpy(op.options[8], "hands");
	string_cpy(op.options[9], "feet");

	AddFlagList(&aim_hitscan_groups, "hitscan groups", 9, op, (1<<0) | (1<<2));

	AddCheckbox(&aim_pointscan, "pointscan", 1);

	string_cpy(op.options[0], "head");
	string_cpy(op.options[1], "neck");
	string_cpy(op.options[2], "lower body");
	string_cpy(op.options[3], "upper body");
	string_cpy(op.options[4], "arms");
	string_cpy(op.options[5], "legs");
	string_cpy(op.options[6], "forearms");
	string_cpy(op.options[7], "calves");

	AddFlagList(&aim_point_groups, "pointscan groups", 7, op, (1<<0) | (1<<2));

	AddSlider(&aim_pointscale, "point scale", 1, 98, SLIDERTYPE_2F, 95);
	AddSlider(&aim_pointscale_head, "head point scale", 0, 98, SLIDERTYPE_2F, 60);

	string_cpy(op.options[0], "lower stomach");
	string_cpy(op.options[1], "stomach");
	string_cpy(op.options[2], "lower chest");
	string_cpy(op.options[3], "chest");
	string_cpy(op.options[4], "upper chest");

	AddList(&aim_body_hitbox, "body hitbox", 4, op, 1);

	AddCheckbox(&aim_hp, "aim on hp", 1);

	// target

	AddCheckbox(&aim_autowall, "autowall", 1);

	AddSlider(&aim_maxshots, "max shots", 0, 20, SLIDERTYPE_NORMAL, 1);

	AddSlider(&aim_maxshots_head, "max shots head", -1, 20, SLIDERTYPE_NORMAL, 1);

	AddSlider(&aim_maxwalls, "max walls", 0, 4, SLIDERTYPE_NORMAL, 4);

	AddCheckbox(&aim_grate, "ignore grate", 1);
	AddCheckbox(&aim_autozeus, "auto zeus", 1);

	AddCheckbox(&aim_inair, "bodyaim in air", 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "normal");
	string_cpy(op.options[2], "normal + body");

	AddList(&aim_history, "history aim", 2, op, 0);

	AddCheckbox(&aim_hps_move, "ignore hps on move", 0);

	AddCheckbox(&aim_fast_trace, "fast trace", 0);
	AddCheckbox(&aim_fast_history, "fast history", 0);

	// accuracy

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "normal");
	string_cpy(op.options[2], "closest");

	AddList(&aim_antispread, "anti spread", 2, op, 2);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "3 step");
	string_cpy(op.options[2], "5 step");

	AddList(&aim_resolver, "resolver", 2, op, 0);

	string_cpy(op.options[0], "spreadfix");
	string_cpy(op.options[1], "update");
	string_cpy(op.options[2], "fixleget");
	string_cpy(op.options[3], "alt limit");
	string_cpy(op.options[4], "stopupdate");
	string_cpy(op.options[5], "body");
//	string_cpy(op.options[6], "bodyaim");

	AddFlagList(&aim_resolver_options, "resolver options", 5, op, 0);

	string_cpy(op.options[0], "relative");
	string_cpy(op.options[1], "360");

	AddFlagList(&aim_resolver_update, "update options", 1, op, 0);

	AddSlider(&aim_stand_delta, "stand delta", 10, 90, SLIDERTYPE_NORMAL, 60);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "iterate");
	string_cpy(op.options[2], "force");

	AddList(&aim_resolver_limit, "limit resolver", 2, op, 0);

	AddSlider(&aim_hitchance_factor, "hitchance factor", 0, 100, SLIDERTYPE_NORMAL, 100);

	AddSlider(&aim_hitchance_min, "min hitchance", 0, 100, SLIDERTYPE_NORMAL, 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "normal");
	string_cpy(op.options[2], "invert");

	AddList(&aim_autostop, "auto stop", 2, op, 0);

	string_cpy(op.options[0], "duck");
	string_cpy(op.options[1], "scope");
	string_cpy(op.options[2], "velocity");
	string_cpy(op.options[3], "maintain");

	AddFlagList(&aim_autoaction, "auto action", 3, op, 0);

	// player

	AddCheckbox(&esp_active, "active", 1);
	AddCheckbox(&esp_team, "draw team", 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "side");
	string_cpy(op.options[2], "top");
	string_cpy(op.options[3], "bottom");
	string_cpy(op.options[4], "center");
	string_cpy(op.options[5], "center aligned");

	AddList(&esp_name, "name text", 5, op, 1);
	AddList(&esp_health, "health text", 5, op, 0);
	AddList(&esp_weapon, "weapon text", 5, op, 1);

	AddCheckbox(&esp_ammo, "ammo text", 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "2d");
	string_cpy(op.options[2], "3d");

	AddList(&esp_box, "box", 2, op, 0);

	AddCheckbox(&esp_healthbar, "health bar", 1);

	AddSlider(&esp_healthbar_fraction, "health bar fraction", 2, 10, SLIDERTYPE_NORMAL, 6);

	AddCheckbox(&esp_only_dead, "only when dead", 0);

	// entity

	AddCheckbox(&esp_entity, "active", 0); // active
	AddCheckbox(&esp_weapons, "draw weapons", 1);
	AddCheckbox(&esp_grenade, "draw grenades", 1);
	AddCheckbox(&esp_effects, "draw effects", 1);
	AddCheckbox(&esp_bomb, "draw bomb", 1);
	AddCheckbox(&esp_defusekit, "draw defuse kits", 1);

	AddCheckbox(&esp_grenade_tracer, "draw grenade tracer", 1);
	AddSlider(&esp_tracer_log, "tracer log time", 0, 10, SLIDERTYPE_NORMAL, 0);

	// misc

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "local only");
	string_cpy(op.options[2], "all");

	AddList(&esp_speclist, "spectator list", 2, op, 0);

	AddCheckbox(&esp_impacts, "show wallbang impacts", 0);

	AddSlider(&render_fov, "fov", 0, 179, SLIDERTYPE_NORMAL, 0);
	AddSlider(&render_fov_viewmodel, "viewmodel fov", 0, 179, SLIDERTYPE_NORMAL, 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "server");
	string_cpy(op.options[2], "client");
	string_cpy(op.options[3], "body");

	AddList(&esp_rotation, "model rotation", 3, op, 1);

	string_cpy(op.options[0], "server");
	string_cpy(op.options[1], "client");
	string_cpy(op.options[2], "body");

	AddFlagList(&esp_hitbox, "draw hitboxes", 2, op, 0);

	AddCheckbox(&esp_debug, "esp debug info", 0);

	AddCheckbox(&esp_body, "body angle", 0);

	AddCheckbox(&esp_no_scope, "no scope", 0);

	AddCheckbox(&esp_history, "history aim", 0);

	// chams

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "normal");
	string_cpy(op.options[2], "normal xqz");
	string_cpy(op.options[3], "two sided");

	AddList(&render_chams, "chams", 3, op, 3);

	string_cpy(op.options[0], "me");
	string_cpy(op.options[1], "team");
	string_cpy(op.options[2], "enemy");

	AddFlagList(&render_chams_ents, "chams entities", 2, op, (1<<0) | (1<<1) | (1<<2));

	AddCheckbox(&render_chams_glass, "glass chams", 1);

	AddCheckbox(&render_noteam, "no team rendering", 0);
	AddCheckbox(&render_noweapon, "no weapon rendering", 0);

	AddCheckbox(&render_wirehands, "wire hands", 1);

	AddCheckbox(&render_asus, "asus walls", 0);
	AddSlider(&render_asus_blend, "asus blend", 0, 100, SLIDERTYPE_2F, 0);
	AddCheckbox(&render_asus_additive, "asus additive", 0);

	// color

	AddSlider(&esp_color_t, "t color", 0, 255, SLIDERTYPE_COLOR, 6349055);
	AddSlider(&esp_color_ct, "ct color", 0, 255, SLIDERTYPE_COLOR, 16760928);

	AddSlider(&esp_color_box_t, "t box", 0, 255, SLIDERTYPE_COLOR, 6349055);
	AddSlider(&esp_color_box_ct, "ct box", 0, 255, SLIDERTYPE_COLOR, 16760928);

	AddSlider(&render_chams_tvisible, "t visible", 0, 255, SLIDERTYPE_COLOR, 1660415);
	AddSlider(&render_chams_tinvisible, "t invisible", 0, 255, SLIDERTYPE_COLOR, 3014645);
	AddSlider(&render_chams_ctvisible, "ct visible", 0, 255, SLIDERTYPE_COLOR, 3669870);
	AddSlider(&render_chams_ctinvisible, "ct invisible", 0, 255, SLIDERTYPE_COLOR, 16737280);

	AddSlider(&render_hands_color, "hands color", 0, 255, SLIDERTYPE_COLOR, 65280);

	AddSlider(&menu_color, "menu color", 0, 255, SLIDERTYPE_COLOR, 1842204);

	// hvh
	
	// antiaim

	AddCheckbox(&aa_active, "active", 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "down");
	string_cpy(op.options[2], "forward");
	string_cpy(op.options[3], "up");

	AddList(&aa_pitch, "pitch", 3, op, 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "backwards");
	string_cpy(op.options[2], "jitter");
	string_cpy(op.options[3], "spin");
	string_cpy(op.options[4], "weed spin");
	string_cpy(op.options[5], "random");
	string_cpy(op.options[6], "random left");
	string_cpy(op.options[7], "random right");

	AddList(&aa_yaw, "yaw", 7, op, 0);

	AddIncrSlider(&aa_yaw_base, "yaw base", 0, 355, SLIDERTYPE_INCR, 0, 5);
	AddSlider(&aa_yaw_delta, "yaw delta", 0, 90, SLIDERTYPE_NORMAL, 90);
	AddIncrSlider(&aa_yaw_move, "move yaw", 0, 360, SLIDERTYPE_INCR, 180, 5);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "view");
	string_cpy(op.options[2], "thru");
	string_cpy(op.options[3], "static");
	string_cpy(op.options[4], "relative");

	AddList(&aa_body, "body antiaim", 4, op, 0);

	AddIncrSlider(&aa_body_yaw, "body yaw", 0, 360, SLIDERTYPE_INCR, 0, 5);

	AddCheckbox(&aa_body_update, "body delay", 0);

	AddSlider(&aa_body_last, "body on slow", 0, 3, SLIDERTYPE_NORMAL, 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "forward");
	string_cpy(op.options[2], "left");
	string_cpy(op.options[3], "right");
	string_cpy(op.options[4], "forward +45");
	string_cpy(op.options[5], "random");
	string_cpy(op.options[6], "view");

	AddList(&aa_yaw_fake, "fake yaw", 6, op, 4);

	AddSlider(&aa_jitter_delay, "jitter delay", 0, 10, SLIDERTYPE_NORMAL, 1);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "thru");
	string_cpy(op.options[2], "thru fov");
	string_cpy(op.options[3], "static");

	AddList(&aa_base, "base angle", 3, op, 1);

	// antiaim2

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "normal");
	string_cpy(op.options[2], "up");
	string_cpy(op.options[3], "forward");

	AddList(&aa_shoot, "shoot antiaim", 3, op, 4);

	AddCheckbox(&aa_body_wrap, "wrap body to angle", 0);

	AddCheckbox(&aa_faceaim, "faceaim", 0);

	AddCheckbox(&aa_velocity, "velocity angle", 0);

	AddKeyList(&aa_left_key, "left angle key", 0);
	AddKeyList(&aa_right_key, "right angle key", 0);
	AddCheckbox(&aa_key_toggle, "key toggle", 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "normal");
	string_cpy(op.options[2], "invert");

	AddList(&aa_fakewalk, "fake walk", 2, op, 0);

	AddKeyList(&aa_fakewalk_key, "fake walk key", 0);

	AddCheckbox(&aa_fakewalk_shoot, "fake walk while shooting", 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "adaptive");
	string_cpy(op.options[2], "jitter");
	string_cpy(op.options[3], "jitter step");

	AddList(&aa_jump_lag, "+jump fakelag", 3, op, 0);

	AddSlider(&aa_stand_lag, "stand fakelag", 0, 14, SLIDERTYPE_NORMAL, 0);

	AddCheckbox(&aa_fakelag_speed, "body fakelag", 0);

	AddCheckbox(&aa_valvemm, "fix valvemm", 0);

	AddCheckbox(&aim_vac_kick, "anti vac kick", 1);

	AddCheckbox(&aim_norecoil, "no recoil", 1);
	AddCheckbox(&aim_noeffects, "no view effects", 1);

	// move

	// strafer

	AddCheckbox(&move_strafer, "active", 1);
	AddCheckbox(&move_speed, "speed strafer", 1);

	AddCheckbox(&move_leftright, "left/right strafer", 1);
	AddKeyList(&move_left_key, "left key", 0);
	AddKeyList(&move_right_key, "right key", 0);
	AddSlider(&move_left_speed, "left speed", 10, 40, SLIDERTYPE_1F, 40);
	AddSlider(&move_right_speed, "right speed", 10, 40, SLIDERTYPE_1F, 32);
	AddSlider(&move_leftright_accel, "left/right step", 0, 10, SLIDERTYPE_NORMAL, 5);

	// simulation

	AddCheckbox(&move_airstuck, "airstuck", 1);
	AddKeyList(&move_airstuck_key, "airstuck key", 0);
	AddKeyList(&move_airstuck_toggle, "airstuck toggle", 0);

	AddCheckbox(&move_speedhack, "speedhack", 0);
	AddKeyList(&move_speedhack_key, "speedhack key", 0);
	AddSlider(&move_speedhack_factor, "speedhack factor", 2, 14, SLIDERTYPE_NORMAL, 14);

	AddCheckbox(&move_duckjump, "duck jump", 0);

	// misc

	AddCheckbox(&peppers, "Fresh Text", 0);
	AddKeyList(&misc_menukey, "menu key", 0);
	AddKeyList(&misc_unloadkey, "unload key", 0);
	AddCheckbox(&misc_antiscreen, "anti screen", 1);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "normal");
	string_cpy(op.options[2], "prefix");

	AddList(&misc_killsay, "killsay cfg", 2, op, 0);
	AddList(&misc_exec_buy, "exec buy cfg", 2, op, 0);

	AddCheckbox(&misc_textingame, "text in game", 0);

	string_cpy(op.options[0], "eyeang");
	string_cpy(op.options[1], "duck");

	AddFlagList(&misc_server, "server", 1, op, 0);

	AddCheckbox(&misc_getout, "getout spam", 0);
	
	// game

	string_cpy(op.options[0], "default");
	string_cpy(op.options[1], "rifles");
	string_cpy(op.options[2], "pistols");
	string_cpy(op.options[3], "deagle");
	string_cpy(op.options[4], "revolver");
	string_cpy(op.options[5], "autos");
	string_cpy(op.options[6], "scout");
	string_cpy(op.options[7], "awp");
	string_cpy(op.options[8], "zeus");

	AddList(&current_group, "weapon cfg", 8, op, 0);

	string_cpy(op.options[0], "rifles");
	string_cpy(op.options[1], "pistols");
	string_cpy(op.options[2], "deagle");
	string_cpy(op.options[3], "revolver");
	string_cpy(op.options[4], "autos");
	string_cpy(op.options[5], "scout");
	string_cpy(op.options[6], "awp");
	string_cpy(op.options[7], "zeus");

	AddFlagList(&groups_active, "active cfgs", 7, op, 0);

	AddCheckbox(&misc_psilent, "psilent", 1);
	AddCheckbox(&misc_autohop, "auto hop", 1);
	AddCheckbox(&misc_autopistol, "auto pistol", 1);
	AddCheckbox(&misc_quicktoss, "quick grenade toss", 0);

	string_cpy(op.options[0], "deagle");
	string_cpy(op.options[1], "revolver");
	string_cpy(op.options[2], "awp");
	string_cpy(op.options[3], "scout");

	AddFlagList(&misc_accuracy_switch, "accuracy switch", 3, op, 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "normal");
	string_cpy(op.options[2], "freeze");

	AddList(&misc_revolver, "auto revolver", 2, op, 0);

	AddCheckbox(&misc_revolver_visible, "revolver only visible", 1);

	AddCheckbox(&misc_scout, "auto scout", 0);

	AddCheckbox(&misc_nopure, "sv_pure bypass", 0);

	AddCheckbox(&misc_spectate, "thirdperson spectate", 0);

	AddCheckbox(&misc_space_name, "large name", 0);

	AddCheckbox(&misc_control_bot, "auto control bot", 0);
	AddCheckbox(&misc_1488, "1488", 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "extra");
	string_cpy(op.options[2], "max");

	AddList(&misc_force_interp, "GloAim", 2, op, 0);

	AddCheckbox(&misc_cvarbypass, "cvar bypass", 0);
	AddCheckbox(&misc_reconnect, "auto reconnect", 0);
	AddCheckbox(&misc_antiafk, "anti afk", 0);

	// players

	string_cpy(op.options[0], "normal");
	string_cpy(op.options[1], "ignore");
	string_cpy(op.options[2], "rage");

	AddList(&player_filter, "aim filter", 2, op, 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "head");
	string_cpy(op.options[2], "body");
	string_cpy(op.options[3], "body only");
	string_cpy(op.options[4], "head only");

	AddList(&player_hitbox, "aim hitbox", 4, op, 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "down");
	string_cpy(op.options[2], "up");

	AddList(&player_fixpitch, "fix pitch", 2, op, 0);

	string_cpy(op.options[0], "off");
	string_cpy(op.options[1], "forward");
	string_cpy(op.options[2], "backwards");
	string_cpy(op.options[3], "left");
	string_cpy(op.options[4], "right");
	string_cpy(op.options[5], "thru backwards");
	string_cpy(op.options[6], "thru forward");
	string_cpy(op.options[7], "thru left");
	string_cpy(op.options[8], "thru right");

	AddList(&player_fixyaw, "fix yaw", 8, op, 0);

	AddCheckbox(&player_resolve_pitch, "resolve pitch", 0);
	AddCheckbox(&player_ignore_resolve, "ignore resolve", 0);
}

void Vars::AddCheckbox(SettingVar* var, char* name, int default_value)
{
	string_cpy(var->name, name);

	var->type = VARTYPE_CHECKBOX;
	var->value = default_value;
	var->default_value = default_value;
	var->group = false;

	var->min = 0;
	var->max = 1;
}

void Vars::AddCheckbox(SettingVar* var, char* name, int default_value, GroupVar* groups)
{
	AddCheckbox(var, name, default_value);

	var->group = true;

	for (int i = 0; i < MAX_GROUP_VARS; ++i)
		var->group_vars[i] = groups[i];
}

void Vars::AddList(SettingVar* var, char* name, int max, OptionString op, int default_value)
{
	string_cpy(var->name, name);

	var->type = VARTYPE_LIST;
	var->value = default_value;
	var->default_value = default_value;
	var->group = false;

	var->min = 0;

	if (max > MAX_LIST_OPTIONS)
	{
		base->Warning("CVars: max list options overflow\n");
		max = MAX_LIST_OPTIONS;
	}

	var->max = max;

	for (int i = 0; i <= max; ++i)
		memcpy(&var->options[i], &op.options[i], 32);
}

void Vars::AddList(SettingVar* var, char* name, int max, OptionString op, int default_value, GroupVar* groups)
{
	AddList(var, name, max, op, default_value);

	var->group = true;

	for (int i = 0; i < MAX_GROUP_VARS; ++i)
		memcpy(&var->group_vars[i], &groups[i], sizeof(GroupVar));//var->group_vars[i] = groups[i];
}

void Vars::AddKeyList(SettingVar* var, char* name, int default_value)
{
	string_cpy(var->name, name);

	var->type = VARTYPE_KEYLIST;
	var->value = default_value;
	var->default_value = default_value;
	var->group = false;

	var->min = 0;
	var->max = KEY_MAX;
}

void Vars::AddKeyList(SettingVar* var, char* name, int default_value, GroupVar* groups)
{
	AddKeyList(var, name, default_value);

	var->group = true;

	for (int i = 0; i < MAX_GROUP_VARS; ++i)
		var->group_vars[i] = groups[i];
}

void Vars::AddFlagList(SettingVar* var, char* name, int max, OptionString op, int default_value)
{
	string_cpy(var->name, name);

	var->type = VARTYPE_FLAGLIST;
	var->value = default_value;
	var->default_value = default_value;
	var->group = false;

	var->min = 0;

	if (max > MAX_LIST_OPTIONS)
	{
		base->Warning("CVars: max list options overflow\n");
		max = MAX_LIST_OPTIONS;
	}

	var->max = max;

	for (int i = 0; i <= max; ++i)
		memcpy(&var->options[i], &op.options[i], 32);
}

void Vars::AddFlagList(SettingVar* var, char* name, int max, OptionString op, int default_value, GroupVar* groups)
{
	AddFlagList(var, name, max, op, default_value);

	var->group = true;

	for (int i = 0; i < MAX_GROUP_VARS; ++i)
		var->group_vars[i] = groups[i];
}

void Vars::AddSlider(SettingVar* var, char* name, int min, int max, int slider_type, int default_value)
{
	string_cpy(var->name, name);

	var->type = VARTYPE_SLIDER;
	var->value = default_value;
	var->default_value = default_value;
	var->group = false;
	var->slider_type = slider_type;

	var->min = min;
	var->max = max;
}

void Vars::AddIncrSlider(SettingVar* var, char* name, int min, int max, int slider_type, int default_value, int incr)
{
	string_cpy(var->name, name);

	var->type = VARTYPE_SLIDER;
	var->value = default_value;
	var->default_value = default_value;
	var->group = false;
	var->slider_type = slider_type;
	var->incr = incr;

	var->min = min;
	var->max = max;
}

void Vars::AddSlider(SettingVar* var, char* name, int min, int max, int slider_type, int default_value, GroupVar* groups)
{
	AddSlider(var, name, min, max, slider_type, default_value);

	var->group = true;

	for (int i = 0; i < MAX_GROUP_VARS; ++i)
		var->group_vars[i] = groups[i];
}