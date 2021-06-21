#include "dll.h"

CMenu gMenu;

void OpRequestUnload()
{
	gMenu.mIssueUnload = true;
}

Vars sOldMenuVarsCreate;

void OpCreateConfig(char* name)
{
	if (gMenu.mConfigCount == MAX_CONFIG_COUNT)
	{
		MenuPopup* popup = &gMenu.mPopupWindow;

		SecureZeroMemory(popup, sizeof(MenuPopup));

		string_cpy(popup->name, "error");
		string_cpy(popup->description, "cannot create config (max reached)");
		popup->type = POPUP_OK;

		gMenu.mPopupDrawing = true;

		return;
	}

	if (!gMenu.mCopyDecided)
	{
		char cfg_name[32];
		SecureZeroMemory(cfg_name, sizeof(cfg_name));
		string_cpy(cfg_name, name);

		MenuPopup* popup = &gMenu.mPopupWindow;

		SecureZeroMemory(popup, sizeof(MenuPopup));

		string_cpy(popup->name, "copy");
		string_cpy(popup->description, "copy from current config?");
		popup->type = POPUP_YESNO;
		popup->string_param = (StringParamFn)(OpCreateConfig);
		string_cpy(popup->string, cfg_name);
		popup->variable = &gMenu.mCopyCfg;

		gMenu.mPopupDrawing = true;
		gMenu.mCopyDecided = true;

		return;
	}

	HKEY key;

	if (base->RegCreateKeyExA(HKEY_CURRENT_USER, __REGKEY_NAME__, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, nullptr) != ERROR_SUCCESS)
	{
		base->Warning("CMenu: could not open key\n");
		return;
	}

	// save our current config first
	//gMenu.WriteAll();


	memcpy(&sOldMenuVarsCreate, &gVars, sizeof(Vars));

	SecureZeroMemory(&gMenu.mConfigs[gMenu.mConfigCount], sizeof(MenuConfig));

	gMenu.ReadConfig(key, name, &gMenu.mConfigs[gMenu.mConfigCount]);
	gMenu.mCurrentConfig = gMenu.mConfigs[gMenu.mConfigCount].crc;

	if (gMenu.mCopyCfg)
		memcpy(&gVars, &sOldMenuVarsCreate, sizeof(Vars));

	gMenu.mConfigCount += 1;

	// save the new one
	gMenu.WriteAll();

	base->RegCloseKey(key);

	gMenu.mCopyDecided = false;
	gMenu.mCopyCfg = false;
}

void OpDeleteConfig()
{
	if (gMenu.mConfigCount <= 1)
	{
		MenuPopup* popup = &gMenu.mPopupWindow;

		SecureZeroMemory(popup, sizeof(MenuPopup));

		string_cpy(popup->name, "error");
		string_cpy(popup->description, "cannot delete config (only one left)");
		popup->type = POPUP_OK;

		gMenu.mPopupDrawing = true;

		return;
	}

	HKEY key;

	if (base->RegCreateKeyExA(HKEY_CURRENT_USER, __REGKEY_NAME__, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, nullptr) != ERROR_SUCCESS)
	{
		base->Warning("CMenu: could not open key\n");
		return;
	}

	HKEY subkey = key;

	DWORD subkey_count = 0;

	if (base->RegQueryInfoKeyA(key, nullptr, nullptr, 0, &subkey_count, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
	{
		base->Warning("CMenu: could not query key info\n");
		return;
	}

	// delete the subkey from the registry
	for (int i = 0; i < subkey_count; ++i)
	{
		char name[128];
		SecureZeroMemory(name, sizeof(name));

		DWORD size = 128;

		if (base->RegEnumKeyExA(subkey, i, name, &size, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
			continue;

		if (CRC32_Get(name, string_len(name)) == gMenu.mCurrentConfig)
		{
			base->RegDeleteKeyA(subkey, name);

			break;
		}
	}

	// store the current configs
	memcpy(&gMenu.mCopyConfigs, &gMenu.mConfigs, sizeof(MenuConfig) * MAX_CONFIG_COUNT);

	// reorder the array removing the current config element
	for (int i = 0, n = 0; i < gMenu.mConfigCount; ++i)
	{
		if (gMenu.mCopyConfigs[i].crc == gMenu.mCurrentConfig)
			continue;

		memcpy(&gMenu.mConfigs[n++], &gMenu.mCopyConfigs[i], sizeof(MenuConfig));
	}

	gMenu.mConfigCount -= 1;

	// set the current config
	gMenu.mCurrentConfig = gMenu.mConfigs[gMenu.mConfigCount - 1].crc;

	memcpy(&gVars, &gMenu.mConfigs[gMenu.mConfigCount - 1].vars, sizeof(Vars));

	gMenu.WriteAll();

	base->RegCloseKey(key);
}

void OpResetConfig()
{
	for (int i = 0; i < gMenu.mSectionCount; ++i)
	{
		MenuSection* section = &gMenu.mSections[i];

		for (int n = 0; n < section->tab_count; ++n)
		{
			MenuTab* tab = &section->tabs[n];

			for (int k = 0; k < tab->var_count; ++k)
			{
				SettingVar* var = tab->vars[k];
				
				if (var->group)
				{
					for (int v = 0; v < MAX_GROUP_VARS; ++v)
					{
						GroupVar* group_var = &var->group_vars[v];

						int group_len = string_len(group_var->name);

						if (group_len != 0)
							group_var->value = var->default_value;
					}
				}
				else
				{
					var->value = var->default_value;
				}
			}
		}
	}

	gMenu.WriteAll();
}

void CMenu::KeyThink(int type, int key)
{
	if ((gVars.misc_menukey.value && key == gVars.misc_menukey.value) || key == KEY_INSERT)
	{
		if (type == IE_ButtonPressed)
		{
			if (ShouldHighlightElements())
				mDrawing = !mDrawing;
		}
	}

	if (gVars.misc_unloadkey.value)
	{
		if (key == gVars.misc_unloadkey.value)
		{
			if (type == IE_ButtonPressed)
				mIssueUnload = true;
		}
	}
}

void CMenu::BindThink(int type, int key)
{
	if (gClient->Cfg(gVars.l_aim_mode) == 1)
	{
		if (key == gClient->Cfg(gVars.l_aim_key))
		{
			if (type == IE_ButtonPressed && !mDrawing)
				gLegit.mAimKey = true;
			else if (type == IE_ButtonReleased)
				gLegit.mAimKey = false;
		}
	}
	else
	{
		gLegit.mAimKey = false;
	}

	if (gClient->Cfg(gVars.l_trig_mode) == 1)
	{
		if (key == gClient->Cfg(gVars.l_trig_key))
		{
			if (type == IE_ButtonPressed && !mDrawing)
				gLegit.mTriggerKey = true;
			else if (type == IE_ButtonReleased)
				gLegit.mTriggerKey = false;
		}
	}
	else
	{
		gLegit.mTriggerKey = false;
	}

	/*if (gVars.aim_mode.value == 2)
	{
		if (key == gVars.aim_key.value)
		{
			if (type == IE_ButtonPressed && !(mDrawing && !gAim->mAimKey))
				gAim->mAimKey = !gAim->mAimKey;
		}
	}
	else */if (gVars.aim_mode.value == 2)
	{
		if (key == gVars.aim_key.value)
		{
			if (type == IE_ButtonPressed && !mDrawing)
				gAim->mAimKey = true;
			else if (type == IE_ButtonReleased)
				gAim->mAimKey = false;
		}
	}
	else
	{
		gAim->mAimKey = false;
	}

	if (gVars.aa_left_key.value)
	{
		if (gVars.aa_key_toggle.value)
		{
			if (key == gVars.aa_left_key.value)
			{
				if (type == IE_ButtonPressed && !(mDrawing && !gHvh.mLeftKey))
					gHvh.mLeftKey = !gHvh.mLeftKey;
			}
		}
		else
		{
			if (key == gVars.aa_left_key.value)
			{
				if (type == IE_ButtonPressed && !mDrawing)
					gHvh.mLeftKey = true;
				else if (type == IE_ButtonReleased)
					gHvh.mLeftKey = false;
			}
		}
	}
	else
	{
		gHvh.mLeftKey = false;
	}

	if (gVars.aa_right_key.value)
	{
		if (gVars.aa_key_toggle.value)
		{
			if (key == gVars.aa_right_key.value)
			{
				if (type == IE_ButtonPressed && !(mDrawing && !gHvh.mRightKey))
					gHvh.mRightKey = !gHvh.mRightKey;
			}
		}
		else
		{
			if (key == gVars.aa_right_key.value)
			{
				if (type == IE_ButtonPressed && !mDrawing)
					gHvh.mRightKey = true;
				else if (type == IE_ButtonReleased)
					gHvh.mRightKey = false;
			}
		}
	}
	else
	{
		gHvh.mRightKey = false;
	}

	if (gVars.aa_fakewalk_key.value)
	{
		if (key == gVars.aa_fakewalk_key.value)
		{
			if (type == IE_ButtonPressed && !mDrawing)
				gHvh.mFakewalkKey = true;
			else if (type == IE_ButtonReleased)
				gHvh.mFakewalkKey = false;
		}
	}
	else
	{
		gHvh.mFakewalkKey = false;
	}

	if (gVars.move_left_key.value)
	{
		if (key == gVars.move_left_key.value)
		{
			if (type == IE_ButtonPressed && !mDrawing)
				gClient->mLeftStrafe = true;
			else if (type == IE_ButtonReleased)
				gClient->mLeftStrafe = false;
		}
	}
	else
	{
		gClient->mLeftStrafe = false;
	}

	if (gVars.move_right_key.value)
	{
		if (key == gVars.move_right_key.value)
		{
			if (type == IE_ButtonPressed && !mDrawing)
				gClient->mRightStrafe = true;
			else if (type == IE_ButtonReleased)
				gClient->mRightStrafe = false;
		}
	}
	else
	{
		gClient->mRightStrafe = false;
	}

	if (gVars.move_airstuck_key.value)
	{
		if (key == gVars.move_airstuck_key.value)
		{
			if (type == IE_ButtonPressed && !mDrawing)
				gClient->mAirstuckKey = true;
			else if (type == IE_ButtonReleased)
				gClient->mAirstuckKey = false;
		}
	}

	if (gVars.move_airstuck_toggle.value)
	{
		if (key == gVars.move_airstuck_toggle.value)
		{
			if (type == IE_ButtonPressed && !mDrawing)
				gClient->mAirstuckKey = !gClient->mAirstuckKey;
		}
	}

	if (gVars.move_speedhack_key.value)
	{
		if (key == gVars.move_speedhack_key.value)
		{
			if (type == IE_ButtonPressed && !mDrawing)
				gClient->mSpeedhackKey = true;
			else if (type == IE_ButtonReleased)
				gClient->mSpeedhackKey = false;
		}
	}

	if (!gVars.move_airstuck.value)
		gClient->mAirstuckKey = false;
}

MenuSection* CMenu::AddSection(char* string)
{
	string_cpy(mSections[mSectionCount].name, string);

	return &mSections[mSectionCount++];
}

MenuOption* CMenu::AddOption(char* name, OptionFuncFn func, bool popup, char* description, int type, int string_max)
{
	MenuOption* option = &mOptions[mOptionCount++];

	string_cpy(option->name, name);
	option->func = func;

	option->popup = popup;
	string_cpy(option->description, description);
	option->type = type;
	option->string_max = string_max;

	return option;
}

void CMenu::InitElements()
{
	/*{
		MenuSection* section = AddSection("l_main"); // l_main

		{
			MenuTab* tab = section->AddTab("aim"); // aim

			tab->AddVar(&gVars.l_aim);
			tab->AddVar(&gVars.l_aim_mode);
			tab->AddVar(&gVars.l_aim_key);
			tab->AddVar(&gVars.l_aim_hitbox);
			tab->AddVar(&gVars.l_aim_hitscan);
			tab->AddVar(&gVars.l_rcs_active);
			tab->AddVar(&gVars.l_rcs_extrapolate);
		}

		{
			MenuTab* tab = section->AddTab("factors"); // factors

			tab->AddVar(&gVars.l_aim_fov);
			tab->AddVar(&gVars.l_aim_stop_fov);
			tab->AddVar(&gVars.l_aim_time);
			tab->AddVar(&gVars.l_aim_start_sens);
			tab->AddVar(&gVars.l_aim_stop_sens);
			//tab->AddVar(&gVars.l_aim_start_time);
			tab->AddVar(&gVars.l_aim_smooth);
			//tab->AddVar(&gVars.l_aim_accel);
			tab->AddVar(&gVars.l_aim_sine);
			tab->AddVar(&gVars.l_aim_prediction);
			tab->AddVar(&gVars.l_aim_single);
		}

		{
			MenuTab* tab = section->AddTab("trigger"); // trigger

			tab->AddVar(&gVars.l_trig);
			tab->AddVar(&gVars.l_trig_mode);
			tab->AddVar(&gVars.l_trig_key);
			tab->AddVar(&gVars.l_trig_hitbox);
			tab->AddVar(&gVars.l_trig_attack_delay);
			tab->AddVar(&gVars.l_trig_delay);
			tab->AddVar(&gVars.l_trig_delay_random);
			tab->AddVar(&gVars.l_trig_burst);
			tab->AddVar(&gVars.l_trig_burst_random);
			tab->AddVar(&gVars.l_trig_infov);
		}
	}

	{
		MenuSection* section = AddSection("l_misc"); // l_misc

		{
			MenuTab* tab = section->AddTab("accuracy"); // accuracy

			tab->AddVar(&gVars.l_accuracy);
			tab->AddVar(&gVars.l_refineaim);
			//tab->AddVar(&gVars.l_deltaaim);
			tab->AddVar(&gVars.l_historyaim);
			tab->AddVar(&gVars.l_historyaim_trigger);
			tab->AddVar(&gVars.l_historyaim_time);
			tab->AddVar(&gVars.l_historyaim_pred);
			tab->AddVar(&gVars.l_accuracy_smoke);
			tab->AddVar(&gVars.l_accuracy_flash);
		}

		{
			MenuTab* tab = section->AddTab("misc"); // misc

			tab->AddVar(&gVars.l_hitbox_scale);
			tab->AddVar(&gVars.l_teamaim);
		//	tab->AddVar(&gVars.l_antiaim);
			tab->AddVar(&gVars.l_resolver);
		}
	}*/

	{
		MenuSection* section = AddSection("r_main"); // r_main

		{
			MenuTab* tab = section->AddTab("main"); // main

			tab->AddVar(&gVars.aim_active);
			tab->AddVar(&gVars.aim_mode);
			tab->AddVar(&gVars.aim_key);
			tab->AddVar(&gVars.aim_silent);
			tab->AddVar(&gVars.aim_autofire);
			tab->AddVar(&gVars.aim_sticky);
			tab->AddVar(&gVars.aim_team);
			tab->AddVar(&gVars.aim_manual);

		//	tab->AddVar(&gVars.test);
	//		tab->AddVar(&gVars.test2);
		}
		
		{
			MenuTab* tab = section->AddTab("position"); // position

			tab->AddVar(&gVars.aim_hitbox);
			tab->AddVar(&gVars.aim_headaim);
			tab->AddVar(&gVars.aim_hitscan);
			tab->AddVar(&gVars.aim_hitscan_groups);
			tab->AddVar(&gVars.aim_pointscan);
			tab->AddVar(&gVars.aim_point_groups);
			tab->AddVar(&gVars.aim_pointscale);
			tab->AddVar(&gVars.aim_pointscale_head);
			tab->AddVar(&gVars.aim_body_hitbox);
			tab->AddVar(&gVars.aim_hp);
		}

		{
			MenuTab* tab = section->AddTab("target"); // target

			tab->AddVar(&gVars.aim_autowall);
			tab->AddVar(&gVars.aim_maxshots);
			tab->AddVar(&gVars.aim_maxshots_head);
		//	tab->AddVar(&gVars.aim_maxwalls);
			tab->AddVar(&gVars.aim_grate);
			tab->AddVar(&gVars.aim_autozeus);
			tab->AddVar(&gVars.aim_inair);
			tab->AddVar(&gVars.aim_history);
			tab->AddVar(&gVars.aim_hps_move);
			tab->AddVar(&gVars.aim_fast_trace);
			tab->AddVar(&gVars.aim_fast_history);
			//tab->AddVar(&gVars.aim_history_damage);
		}

		{
			MenuTab* tab = section->AddTab("accuracy"); // accuracy

			//tab->AddVar(&gVars.aim_accuracy);
			tab->AddVar(&gVars.aim_antispread);
			tab->AddVar(&gVars.aim_resolver);
			tab->AddVar(&gVars.aim_resolver_options);
			tab->AddVar(&gVars.aim_resolver_update);
			tab->AddVar(&gVars.aim_stand_delta);
			tab->AddVar(&gVars.aim_resolver_limit);
			tab->AddVar(&gVars.aim_hitchance_factor);
			//tab->AddVar(&gVars.aim_hitchance_min);
			tab->AddVar(&gVars.aim_autostop);
			tab->AddVar(&gVars.aim_autoaction);
		}
	}

	{
		MenuSection* section = AddSection("r_misc"); // r_misc

		{
			MenuTab* tab = section->AddTab("antiaim"); // antiaim

			tab->AddVar(&gVars.aa_active);
			tab->AddVar(&gVars.aa_pitch);
			tab->AddVar(&gVars.aa_yaw);
			tab->AddVar(&gVars.aa_yaw_base);
			tab->AddVar(&gVars.aa_yaw_delta);
			tab->AddVar(&gVars.aa_yaw_move);
			tab->AddVar(&gVars.aa_body);
			tab->AddVar(&gVars.aa_body_yaw);
			tab->AddVar(&gVars.aa_body_update);
			tab->AddVar(&gVars.aa_body_last);
		}

		{
			MenuTab* tab = section->AddTab("antiaim2"); // antiaim2

			tab->AddVar(&gVars.aa_yaw_fake);
			tab->AddVar(&gVars.aa_jitter_delay);
			tab->AddVar(&gVars.aa_base);
			tab->AddVar(&gVars.aa_shoot);
			tab->AddVar(&gVars.aa_body_wrap);
			tab->AddVar(&gVars.aa_faceaim);
		}

		{
			MenuTab* tab = section->AddTab("antiaim3"); // antiaim3
			
			tab->AddVar(&gVars.aa_velocity);
			tab->AddVar(&gVars.aa_left_key);
			tab->AddVar(&gVars.aa_right_key);
			tab->AddVar(&gVars.aa_key_toggle);
			tab->AddVar(&gVars.aa_fakewalk);
			tab->AddVar(&gVars.aa_fakewalk_key);
			tab->AddVar(&gVars.aa_fakewalk_shoot);
		}

		{
			MenuTab* tab = section->AddTab("misc"); // misc

			tab->AddVar(&gVars.aa_jump_lag);
			tab->AddVar(&gVars.aa_stand_lag);
			tab->AddVar(&gVars.aa_fakelag_speed);
			tab->AddVar(&gVars.aa_valvemm);
			tab->AddVar(&gVars.aim_vac_kick);
			tab->AddVar(&gVars.misc_psilent);
			tab->AddVar(&gVars.misc_autopistol);
			tab->AddVar(&gVars.aim_norecoil);
			tab->AddVar(&gVars.aim_noeffects);
		}
	}

	{
		MenuSection* section = AddSection("visual"); // visual

		{
			MenuTab* tab = section->AddTab("player"); // player

			tab->AddVar(&gVars.esp_active);
			tab->AddVar(&gVars.esp_team);
			tab->AddVar(&gVars.esp_name);
			tab->AddVar(&gVars.esp_health);
			tab->AddVar(&gVars.esp_weapon);
			tab->AddVar(&gVars.esp_ammo);
			tab->AddVar(&gVars.esp_box);
			tab->AddVar(&gVars.esp_healthbar);
			tab->AddVar(&gVars.esp_healthbar_fraction);
			tab->AddVar(&gVars.esp_only_dead);
		}

		{
			MenuTab* tab = section->AddTab("entity"); // entity

			tab->AddVar(&gVars.esp_entity);
			tab->AddVar(&gVars.esp_weapons);
			tab->AddVar(&gVars.esp_grenade);
			tab->AddVar(&gVars.esp_effects);
			tab->AddVar(&gVars.esp_bomb);
			tab->AddVar(&gVars.esp_defusekit);
			tab->AddVar(&gVars.esp_grenade_tracer);
			tab->AddVar(&gVars.esp_tracer_log);
		}

		{
			MenuTab* tab = section->AddTab("misc"); // misc

			tab->AddVar(&gVars.esp_speclist);
			tab->AddVar(&gVars.esp_impacts);
			tab->AddVar(&gVars.render_fov);
			tab->AddVar(&gVars.esp_rotation);
			tab->AddVar(&gVars.esp_hitbox);
			tab->AddVar(&gVars.esp_debug);
			tab->AddVar(&gVars.esp_body);
			tab->AddVar(&gVars.esp_no_scope);
			tab->AddVar(&gVars.esp_history);
		}

		{
			MenuTab* tab = section->AddTab("render"); // render

			tab->AddVar(&gVars.render_chams);
			tab->AddVar(&gVars.render_chams_ents);
			tab->AddVar(&gVars.render_chams_glass);
			tab->AddVar(&gVars.render_noteam);

			tab->AddVar(&gVars.render_wirehands);
		//	tab->AddVar(&gVars.render_asus);
		//	tab->AddVar(&gVars.render_asus_blend);
			//tab->AddVar(&gVars.render_asus_additive);
		}

		{
			MenuTab* tab = section->AddTab("color"); // color

			tab->AddVar(&gVars.esp_color_t);
			tab->AddVar(&gVars.esp_color_ct);
			tab->AddVar(&gVars.esp_color_box_t);
			tab->AddVar(&gVars.esp_color_box_ct);

			tab->AddVar(&gVars.render_chams_tvisible);
			tab->AddVar(&gVars.render_chams_tinvisible);
			tab->AddVar(&gVars.render_chams_ctvisible);
			tab->AddVar(&gVars.render_chams_ctinvisible);
			tab->AddVar(&gVars.render_hands_color);

			tab->AddVar(&gVars.menu_color);
		}
	}

	{
		MenuSection* section = AddSection("move"); // move

		{
			MenuTab* tab = section->AddTab("strafer"); // strafer

			tab->AddVar(&gVars.move_strafer);
			tab->AddVar(&gVars.move_speed);
			tab->AddVar(&gVars.move_leftright);
			tab->AddVar(&gVars.move_left_key);
			tab->AddVar(&gVars.move_right_key);
			tab->AddVar(&gVars.move_left_speed);
			tab->AddVar(&gVars.move_right_speed);
			tab->AddVar(&gVars.move_leftright_accel);
		}

		{
			MenuTab* tab = section->AddTab("simulation"); // simulation

			tab->AddVar(&gVars.move_airstuck);
			tab->AddVar(&gVars.move_airstuck_key);
			tab->AddVar(&gVars.move_airstuck_toggle);
			tab->AddVar(&gVars.move_speedhack);
			tab->AddVar(&gVars.move_speedhack_key);
			tab->AddVar(&gVars.move_speedhack_factor);
			tab->AddVar(&gVars.move_duckjump);
		}
	}

	{
		MenuSection* section = AddSection("misc"); // misc

		{
			MenuTab* tab = section->AddTab("main"); // main

			//tab->AddVar(&gVars.peppers);
			tab->AddVar(&gVars.misc_menukey);
			tab->AddVar(&gVars.misc_unloadkey);
			tab->AddVar(&gVars.misc_killsay);
			tab->AddVar(&gVars.misc_exec_buy);
			tab->AddVar(&gVars.misc_textingame);
			//tab->AddVar(&gVars.misc_server);
		}

		{
			MenuTab* tab = section->AddTab("game"); // game
			
			tab->AddVar(&gVars.current_group);
			tab->AddVar(&gVars.groups_active);
			tab->AddVar(&gVars.misc_autohop);
			tab->AddVar(&gVars.misc_accuracy_switch);
			tab->AddVar(&gVars.misc_revolver);
			tab->AddVar(&gVars.misc_revolver_visible);
			tab->AddVar(&gVars.misc_scout);
			tab->AddVar(&gVars.misc_nopure);
			tab->AddVar(&gVars.misc_quicktoss);
			tab->AddVar(&gVars.misc_spectate);
		}

		{
			MenuTab* tab = section->AddTab("game2"); // game2

			tab->AddVar(&gVars.misc_getout);
			tab->AddVar(&gVars.misc_space_name);
			tab->AddVar(&gVars.misc_control_bot);
			tab->AddVar(&gVars.misc_1488);
			//tab->AddVar(&gVars.misc_force_interp);
			tab->AddVar(&gVars.misc_cvarbypass);
		//	tab->AddVar(&gVars.misc_reconnect);
			tab->AddVar(&gVars.misc_antiafk);
		}

		{
			MenuTab* tab = section->AddTab("players"); // players

			tab->special = 1;
		}
	}

	{
		AddOption("unload", (OptionFuncFn)(OpRequestUnload), true, "unload cheat?", POPUP_OKCANCEL, 0);
		AddOption("create cfg", (OptionFuncFn)(OpCreateConfig), true, "enter name of config to create", POPUP_STRING, 8);
		AddOption("delete cfg", (OptionFuncFn)(OpDeleteConfig), true, "delete current config?", POPUP_OKCANCEL, 0);
		AddOption("reset cfg", (OptionFuncFn)(OpResetConfig), true, "reset current config?", POPUP_OKCANCEL, 0);
	}
}

bool sOldDrawingState = false;

void CMenu::Think()
{
	if (!mInit)
		Init();

	if (mDrawing != sOldDrawingState)
	{
		if (!mDrawing)
			WriteAll();

		if (!mDrawing)
			base->ConCommand(base->mBaseEngine, "cl_mouseenable 1");
		else
			base->ConCommand(base->mBaseEngine, "cl_mouseenable 0");
	}

	sOldDrawingState = mDrawing;

	if (mDrawing)
	{
		Draw();

		if (mIssueUnload)
		{
			if (mDrawing)
				WriteAll();

			base->ConCommand(base->mBaseEngine, "cl_mouseenable 1");

			mDrawing = false;
			return;
		}
	}
	else
	{
		mCurrentVar = nullptr;
		mSelectedList = nullptr;
	}
}

bool sTopSelected = false;

void CMenu::Draw()
{
	mSelectedList = nullptr;

	int mouse_x = 0;
	int mouse_y = 0;

	base->GetCursorPos(base->mSurface, &mouse_x, &mouse_y);

	mMouseDeltaX = mouse_x - mMouseX;
	mMouseDeltaY = mouse_y - mMouseY;

	mMouseX = mouse_x;
	mMouseY = mouse_y;

	{
		float px = mMenuPosX;
		float py = mMenuPosY;

		// top bar

		{
			if (mMouseClicked && CursorWithinRect(px, py - BAR_HEIGHT, BASE_WIDTH, BAR_HEIGHT))
				sTopSelected = true;

			if (!mMouseHeld || mOptionMenuDrawing || mPopupDrawing)
				sTopSelected = false;

			if (sTopSelected)
			{
				mMenuPosX += (float)(mMouseDeltaX);
				mMenuPosY += (float)(mMouseDeltaY);

				if (mMenuPosX < 0.0f)
					mMenuPosX = 0.0f;

				if (mMenuPosY < BAR_HEIGHT)
					mMenuPosY = BAR_HEIGHT;

				if ((mMenuPosX + BASE_WIDTH) > gDraw.mWidth)
					mMenuPosX = gDraw.mWidth - BASE_WIDTH;

				if ((mMenuPosY + BASE_HEIGHT) > gDraw.mHeight)
					mMenuPosY = gDraw.mHeight - BASE_HEIGHT;
			}
		}

		gDraw.DrawRect(px, py - BAR_HEIGHT, BASE_WIDTH, BAR_HEIGHT, CLR_BACKGROUND);
		gDraw.DrawOutlined(px, py - BAR_HEIGHT, BASE_WIDTH, BAR_HEIGHT + 1, CLR_BLACK);

		gDraw.DrawTextA(base->mTitle, px + 6, py - BAR_HEIGHT + 2, CLR_WHITE);

		// base

		gDraw.DrawRect(px, py, BASE_WIDTH, BASE_HEIGHT, CLR_BACKGROUND);
		gDraw.DrawOutlined(px, py, BASE_WIDTH, BASE_HEIGHT, CLR_BLACK);

		// section background

		gDraw.DrawOutlined(px, py, SECTION_WIDTH + 1, BASE_HEIGHT, 0, 0, 0, 255);
	}

	DrawSections();
	DrawConfigs();

	DrawTabs();
	DrawItems();

	DrawCurrentVar();
	DrawOptionMenu();
	DrawPopupWindow();

	if (!base->IsCursorVisible(base->mSurface) && !base->Con_IsVisible(base->mBaseEngine))
	{
		base->DrawSetColor(base->mSurface, 255, 255, 255, 80);

		int x = mMouseX;
		int y = mMouseY;

		for (int i = 1; i <= 8; ++i)
		{
			int v1 = x + i;
			int v2 = y + i;

			base->DrawLine(base->mSurface, x, y, v1, y);
			base->DrawLine(base->mSurface, x, y, x, v2);
			base->DrawLine(base->mSurface, v1, y, x, v2);
		}
	}

	mMouseClicked = false;
	mMouseReleased = false;
	mMouse2Clicked = false;
	mMouse2Released = false;

	if (mCurrentVar)
		mVarAcceptClicks = true;
	else
		mVarAcceptClicks = false;
}

bool CMenu::CursorWithinRect(float x, float y, float w, float h)
{
	if (mMouseX < (int)(x) || mMouseY < (int)(y))
		return false;

	if (mMouseX >(int)(x + w) || mMouseY >(int)(y + h))
		return false;

	return true;
}

void CMenu::DrawSections()
{
	float px = mMenuPosX;
	float py = mMenuPosY;

	bool selected = false;

	for (int i = 0; i < mSectionCount; ++i)
	{
		MenuSection* section = &mSections[i];

		if (!mCurrentSection)
			mCurrentSection = section;

		// section base
		{
			if (!selected && ShouldHighlightElements() && CursorWithinRect(px, py + (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT))
			{
				if (mMouseClicked && !mOptionMenuDrawing && !mPopupDrawing)
					mCurrentSection = section;

				if (section == mCurrentSection)
					gDraw.DrawRect(px, py + (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT, CLR_HALF);
				else
					gDraw.DrawRect(px, py + (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT, CLR_SELECT);

				selected = true;
			}
			else
			{
				if (section == mCurrentSection)
					gDraw.DrawRect(px, py + (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT, CLR_HALF);
				else
					gDraw.DrawRect(px, py + (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT, CLR_BRIGHT);
			}

			gDraw.DrawOutlined(px, py + (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT + 1, CLR_BLACK);
		}

		// blue bar
		{
			float bx = px + (SECTION_WIDTH - BAR_WIDTH);

			if (section == mCurrentSection)
				gDraw.DrawRect(bx, py + (SECTION_HEIGHT * i), BAR_WIDTH, SECTION_HEIGHT, CLR_BLUE);
			else
				gDraw.DrawRect(bx, py + (SECTION_HEIGHT * i), BAR_WIDTH, SECTION_HEIGHT, CLR_DARK);

			gDraw.DrawOutlined(bx - 1, py + (SECTION_HEIGHT * i), BAR_WIDTH + 2, SECTION_HEIGHT + 1, CLR_BLACK);
		}

		gDraw.DrawTextA(section->name, px + 7, (py + 3) + (SECTION_HEIGHT * i), 255, 255, 255, 255);
	}
}

void CMenu::DrawConfigs()
{
	float px = mMenuPosX;
	float py = mMenuPosY;
	py += BASE_HEIGHT - SECTION_HEIGHT - 1;

	bool selected = false;

	for (int i = 0; i < mConfigCount; ++i)
	{
		MenuConfig* cfg = &mConfigs[i];

		// config base
		{
			if (!selected && ShouldHighlightElements() && CursorWithinRect(px, py - (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT))
			{
				if (mMouseClicked)
				{
					for (int n = 0; n < mConfigCount; ++n)
					{
						if (mConfigs[n].crc == mCurrentConfig)
						{
							memcpy(&mConfigs[n].vars, &gVars, sizeof(Vars));
							break;
						}
					}
					
					mCurrentConfig = cfg->crc;
					memcpy(&gVars, &cfg->vars, sizeof(Vars));
				}

				if (cfg->crc == mCurrentConfig)
					gDraw.DrawRect(px, py - (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT, CLR_HALF);
				else
					gDraw.DrawRect(px, py - (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT, CLR_SELECT);

				selected = true;
			}
			else
			{
				if (cfg->crc == mCurrentConfig)
					gDraw.DrawRect(px, py - (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT, CLR_HALF);
				else
					gDraw.DrawRect(px, py - (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT, CLR_BRIGHT);
			}

			gDraw.DrawOutlined(px, py - (SECTION_HEIGHT * i), SECTION_WIDTH, SECTION_HEIGHT + 1, CLR_BLACK);
		}

		// green bar
		{
			float bx = px + (SECTION_WIDTH - BAR_WIDTH);

			if (cfg->crc == mCurrentConfig)
				gDraw.DrawRect(bx, py - (SECTION_HEIGHT * i), BAR_WIDTH, SECTION_HEIGHT, CLR_GREEN);
			else
				gDraw.DrawRect(bx, py - (SECTION_HEIGHT * i), BAR_WIDTH, SECTION_HEIGHT, CLR_DARK);

			gDraw.DrawOutlined(bx - 1, py - (SECTION_HEIGHT * i), BAR_WIDTH + 2, SECTION_HEIGHT + 1, CLR_BLACK);
		}

		gDraw.DrawTextA(cfg->name, px + 7, (py + 3) - (SECTION_HEIGHT * i), CLR_WHITE);
	}
}

void CMenu::DrawTabs()
{
	if (!mCurrentSection)
		return;

	MenuSection* section = mCurrentSection;

	float px = mMenuPosX;
	float py = mMenuPosY;

	bool selected = false;

	int count = section->tab_count;
	if (count <= 1)
	{
		section->current_tab = &section->tabs[0];
		return;
	}

	float sz = (BASE_WIDTH - SECTION_WIDTH - (BASE_SPACE * 2)) / count;

	for (int i = 0; i < count; ++i)
	{
		MenuTab* tab = &section->tabs[i];

		if (!section->current_tab)
			section->current_tab = tab;

		if (!selected && ShouldHighlightElements() && CursorWithinRect(px + SECTION_WIDTH + BASE_SPACE + (sz * i), py + BASE_SPACE, sz, TAB_HEIGHT))
		{
			if (mMouseClicked)
				section->current_tab = tab;

			if (tab == section->current_tab)
				gDraw.DrawRect(px + SECTION_WIDTH + BASE_SPACE + (sz * i), py + BASE_SPACE, sz, TAB_HEIGHT, CLR_HALF);
			else
				gDraw.DrawRect(px + SECTION_WIDTH + BASE_SPACE + (sz * i), py + BASE_SPACE, sz, TAB_HEIGHT, CLR_SELECT);

			selected = true;
		}
		else
		{
			if (tab == section->current_tab)
				gDraw.DrawRect(px + SECTION_WIDTH + BASE_SPACE + (sz * i), py + BASE_SPACE, sz, TAB_HEIGHT, CLR_HALF);
			else
				gDraw.DrawRect(px + SECTION_WIDTH + BASE_SPACE + (sz * i), py + BASE_SPACE, sz, TAB_HEIGHT, CLR_BRIGHT);
		}

		gDraw.DrawOutlined(px + SECTION_WIDTH + BASE_SPACE + (sz * i), py + BASE_SPACE, sz + ((i == count) ? 0 : 1), TAB_HEIGHT, CLR_BLACK);

		float add = SECTION_WIDTH + (sz / 2) + (sz * i);

		gDraw.DrawTextA(tab->name, px + 6 + add, py + BASE_SPACE + 1, CLR_WHITE, true, false);
	}
}

void CMenu::DrawCheckbox(SettingVar* var, int i)
{
	int* value = &var->value;

	if (var->group)
		value = &var->group_vars[gVars.current_group.value].value;

	float x = mMenuPosX + BASE_WIDTH - (TICKBOX_SIZE + BASE_SPACE);
	float y = mMenuPosY + ((TICKBOX_SIZE + (BASE_SPACE * 2.0f)) * (float)(i)) + (BASE_SPACE * 2.0f);

	if (mCurrentSection->tab_count != 1)
		y += TAB_HEIGHT + BASE_SPACE;

	x -= TICKBOX_SIZE;

	gDraw.DrawTextA(var->name, mMenuPosX + SECTION_WIDTH + BASE_SPACE + 6, y - 2, CLR_WHITE, false, false);

	if (CursorWithinRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE))
	{
		if (mMouseClicked)
		{
			if (*value)
				*value = 0;
			else
				*value = 1;
		}

		if (*value)
			gDraw.DrawRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_SELECT_GREEN);
		else
			gDraw.DrawRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_SELECT_RED);
	}
	else
	{
		if (*value)
			gDraw.DrawRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_LIGHT_GREEN);
		else
			gDraw.DrawRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_LIGHT_RED);
	}

	gDraw.DrawOutlined(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_BLACK);
}

void CMenu::DrawList(SettingVar* var, int i)
{
	int* value = &var->value;

	if (var->group)
		value = &(var->group_vars[gVars.current_group.value].value);

	if (*value < var->min)
		*value = var->min;

	if (*value > var->max)
		*value = var->max;

	float x = mMenuPosX + BASE_WIDTH - (TICKBOX_SIZE + BASE_SPACE);
	float y = mMenuPosY + ((TICKBOX_SIZE + (BASE_SPACE * 2.0f)) * (float)(i)) + (BASE_SPACE * 2.0f);

	if (mCurrentSection->tab_count != 1)
		y += TAB_HEIGHT + BASE_SPACE;

	x -= LIST_WIDTH;

	gDraw.DrawTextA(var->name, mMenuPosX + SECTION_WIDTH + BASE_SPACE + 6, y - 2, CLR_WHITE, false, false);

	if (ShouldHighlightElements() && CursorWithinRect(x, y, LIST_WIDTH, LIST_HEIGHT))
	{
		if (mMouseClicked)
		{
			mCurrentVar = var;
			mListPosX = x;
			mListPosY = y;
		}

		mSelectedList = var;

		gDraw.DrawRect(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_BRIGHT);
	}
	else
	{
		gDraw.DrawRect(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_HALF);
	}

	gDraw.DrawOutlined(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_BRIGHT);

	gDraw.DrawTextA(var->options[*value], x + 6, y, CLR_WHITE, false, false);
}

void CMenu::DrawKeylist(SettingVar* var, int i)
{
	int* value = &var->value;

	if (var->group)
		value = &var->group_vars[gVars.current_group.value].value;

	if (*value < var->min)
		*value = var->min;

	if (*value > var->max)
		*value = var->max;

	float x = mMenuPosX + BASE_WIDTH - (TICKBOX_SIZE + BASE_SPACE);
	float y = mMenuPosY + ((TICKBOX_SIZE + (BASE_SPACE * 2.0f)) * (float)(i)) + (BASE_SPACE * 2.0f);

	if (mCurrentSection->tab_count != 1)
		y += TAB_HEIGHT + BASE_SPACE;

	x -= LIST_WIDTH;

	gDraw.DrawTextA(var->name, mMenuPosX + SECTION_WIDTH + BASE_SPACE + 6, y - 2, CLR_WHITE, false, false);

	if (ShouldHighlightElements() && CursorWithinRect(x, y, LIST_WIDTH, LIST_HEIGHT))
	{
		if (mMouseClicked)
		{
			mCurrentVar = var;
			mListPosX = x;
			mListPosY = y;
		}

		gDraw.DrawRect(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_BRIGHT);
	}
	else
	{
		gDraw.DrawRect(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_HALF);
	}

	gDraw.DrawOutlined(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_BRIGHT);

	if (mCurrentVar == var)
		gDraw.DrawTextA(">> select <<", x + 6, y, CLR_WHITE, false, false);
	else
		gDraw.DrawTextA(key_names[*value], x + 6, y, CLR_WHITE, false, false);
}

void CMenu::DrawFlaglist(SettingVar* var, int i)
{
	int* value = &var->value;

	if (var->group)
		value = &var->group_vars[gVars.current_group.value].value;

	if (*value < 0)
		*value = 0;

	float x = mMenuPosX + BASE_WIDTH - (TICKBOX_SIZE + BASE_SPACE);
	float y = mMenuPosY + ((TICKBOX_SIZE + (BASE_SPACE * 2.0f)) * (float)(i)) + (BASE_SPACE * 2.0f);

	if (mCurrentSection->tab_count != 1)
		y += TAB_HEIGHT + BASE_SPACE;

	x -= LIST_WIDTH;

	gDraw.DrawTextA(var->name, mMenuPosX + SECTION_WIDTH + BASE_SPACE + 6, y - 2, CLR_WHITE, false, false);

	if (ShouldHighlightElements() && CursorWithinRect(x, y, LIST_WIDTH, LIST_HEIGHT))
	{
		if (mMouseClicked)
		{
			mCurrentVar = var;
			mListPosX = x;
			mListPosY = y;
		}

		gDraw.DrawRect(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_BRIGHT);
	}
	else
	{
		gDraw.DrawRect(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_HALF);
	}

	gDraw.DrawOutlined(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_BRIGHT);

	char result[32];
	SecureZeroMemory(result, sizeof(result));

	if (*value != 0)
		string_cpy(result, "...");
	else
		string_cpy(result, "none");

	gDraw.DrawTextA(result, x + 6, y, CLR_WHITE, false, false);
}

void CMenu::DrawSlider(SettingVar* var, int i)
{
	int* value = &var->value;

	if (var->group)
		value = &var->group_vars[gVars.current_group.value].value;

	if (*value < var->min)
		*value = var->min;

	if (*value > var->max)
		*value = var->max;

	float x = mMenuPosX + BASE_WIDTH - (TICKBOX_SIZE + BASE_SPACE);
	float y = mMenuPosY + ((TICKBOX_SIZE + (BASE_SPACE * 2.0f)) * (float)(i)) + (BASE_SPACE * 2.0f);

	if (mCurrentSection->tab_count != 1)
		y += TAB_HEIGHT + BASE_SPACE;

	x -= SLIDER_WIDTH + SLIDER_OFFSET;

	gDraw.DrawTextA(var->name, mMenuPosX + SECTION_WIDTH + BASE_SPACE + 6, y - 2, CLR_WHITE, false, false);

	if (ShouldHighlightElements() && CursorWithinRect(x, y + (SLIDER_HEIGHT * 0.5f), SLIDER_WIDTH, SLIDER_HEIGHT))
	{
		if (mMouseClicked)
			mCurrentVar = var;

		gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), SLIDER_WIDTH, SLIDER_HEIGHT, CLR_HALF_BRIGHT);
	}
	else
	{
		if (mCurrentVar == var)
			gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), SLIDER_WIDTH, SLIDER_HEIGHT, CLR_HALF_BRIGHT);
		else
			gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), SLIDER_WIDTH, SLIDER_HEIGHT, CLR_HALF);
	}

	float max = (float)(var->max);
	max -= (float)(var->min);

	float size = SLIDER_WIDTH - SLIDER_DOT_SIZE;
	float len = size / max;

	if (mCurrentVar == var)
	{
		float dist = ((float)(mMouseX) - x);
		float incr = (float)(var->incr);

		if (var->slider_type == SLIDERTYPE_INCR)
			*value = (int)(clamp_float(incr * (float)((int)(((max / incr) * (dist / size)) + 0.5f)), 0.0f, max)) + var->min;
		else
			*value = (int)(clamp_float((max * (dist / size)), 0.0f, max)) + var->min;
	}

	gDraw.DrawOutlined(x, y + (SLIDER_HEIGHT * 0.5f), SLIDER_WIDTH, SLIDER_HEIGHT, CLR_BLACK);
	gDraw.DrawRect(x + ((float)(*value - var->min) * len), y + (SLIDER_HEIGHT * 0.5f), SLIDER_DOT_SIZE, SLIDER_HEIGHT, CLR_BRIGHT);
	gDraw.DrawOutlined(x + ((float)(*value - var->min) * len), y + (SLIDER_HEIGHT * 0.5f), SLIDER_DOT_SIZE, SLIDER_HEIGHT, CLR_BLACK);

	switch (var->slider_type)
	{
		case SLIDERTYPE_NORMAL:
		case SLIDERTYPE_INCR:
		{
			char str[128];
			base->wsprintfA(str, "%i", *value);

			gDraw.DrawTextA(str, x + SLIDER_WIDTH + SLIDER_OFFSET, y, CLR_WHITE, true, false);

			break;
		}
		case SLIDERTYPE_PERCENT:
		{
			char str[128];
			base->wsprintfA(str, "%i%%", *value);

			gDraw.DrawTextA(str, x + SLIDER_WIDTH + SLIDER_OFFSET, y, CLR_WHITE, true, false);

			break;
		}
		case SLIDERTYPE_1F:
		{
			float value2 = (float)(*value) * 0.1f;

			int value3 = (int)((value2 * 10.0f) + 0.5f);
			if (value3 < 0)
				value3 = 0;

			char str[128];

			if ((float)((int)(value2)) == value2)
				base->wsprintfA(str, "%i", (int)(value2));
			else
				base->wsprintfA(str, "%i.%i", (int)(value2), value3 % 10);

			gDraw.DrawTextA(str, x + SLIDER_WIDTH + SLIDER_OFFSET, y, CLR_WHITE, true, false);

			break;
		}
		case SLIDERTYPE_2F:
		{
			float value2 = (float)(*value) * 0.01f;

			int value3 = (int)((value2 * 100.0f) + 0.5f);
			if (value3 < 0)
				value3 = 0;

			char str[128];

			if ((float)((int)(value2)) == value2)
				base->wsprintfA(str, "%i", (int)(value2));
			else if ((value3 % 100) < 10)
				base->wsprintfA(str, "%i.0%i", (int)(value2), value3 % 100);
			else
				base->wsprintfA(str, "%i.%i", (int)(value2), value3 % 100);

			gDraw.DrawTextA(str, x + SLIDER_WIDTH + SLIDER_OFFSET, y, CLR_WHITE, true, false);

			break;
		}
	}
}

void CMenu::DrawColorSlider(SettingVar* var, int i)
{
	int* value = &var->value;

	if (var->group)
		value = &var->group_vars[gVars.current_group.value].value;

	if (*value < var->min)
		*value = var->min;

	float x = mMenuPosX + BASE_WIDTH - (TICKBOX_SIZE + BASE_SPACE);
	float y = mMenuPosY + ((TICKBOX_SIZE + (BASE_SPACE * 2.0f)) * (float)(i)) + (BASE_SPACE * 2.0f);

	if (mCurrentSection->tab_count != 1)
		y += TAB_HEIGHT + BASE_SPACE;

	x -= COLOR_SLIDER_WIDTH + (BASE_SPACE * 2.0f);

	gDraw.DrawTextA(var->name, mMenuPosX + SECTION_WIDTH + BASE_SPACE + 6, y - 2, CLR_WHITE, false, false);

	float width = COLOR_SLIDER_WIDTH;
	float size = width / 3;

	BYTE* bytes = (BYTE*)(value);

	{
		if (ShouldHighlightElements() && CursorWithinRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT))
		{
			if (mMouseClicked)
			{
				mCurrentVar = var;
				mColorSelected = 1;
			}

			gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_HALF_BRIGHT);
		}
		else
		{
			if (mCurrentVar == var && mColorSelected == 1)
				gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_HALF_BRIGHT);
			else
				gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_HALF);
		}

		float max = (float)(var->max);
		max -= (float)(var->min);

		float len = (size - SLIDER_DOT_SIZE) / max;

		if (mCurrentVar == var && mColorSelected == 1)
		{
			float dist = ((float)(mMouseX) - x);
			int result = (int)(clamp_float((max * (dist / size)) + 0.5f, 0.0f, max)) + var->min;

			bytes[0] = result;
		}

		gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), ((float)((int)(bytes[0]) - var->min) * len), SLIDER_HEIGHT, CLR_SLIDER_RED);

		gDraw.DrawOutlined(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_BLACK);

		gDraw.DrawRect(x + ((float)((int)(bytes[0]) - var->min) * len), y + (SLIDER_HEIGHT * 0.5f), SLIDER_DOT_SIZE, SLIDER_HEIGHT, CLR_BRIGHT);
		gDraw.DrawOutlined(x + ((float)((int)(bytes[0]) - var->min) * len), y + (SLIDER_HEIGHT * 0.5f), SLIDER_DOT_SIZE, SLIDER_HEIGHT, CLR_BLACK);

		x += size + BASE_SPACE;
	}

	{
		if (ShouldHighlightElements() && CursorWithinRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT))
		{
			if (mMouseClicked)
			{
				mCurrentVar = var;
				mColorSelected = 2;
			}

			gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_HALF_BRIGHT);
		}
		else
		{
			if (mCurrentVar == var && mColorSelected == 2)
				gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_HALF_BRIGHT);
			else
				gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_HALF);
		}

		float max = (float)(var->max);
		max -= (float)(var->min);

		float len = (size - SLIDER_DOT_SIZE) / max;

		if (mCurrentVar == var && mColorSelected == 2)
		{
			float dist = ((float)(mMouseX) - x);
			int result = (int)(clamp_float((max * (dist / size)) + 0.5f, 0.0f, max)) + var->min;

			bytes[1] = result;
		}

		gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), ((float)((int)(bytes[1]) - var->min) * len), SLIDER_HEIGHT, CLR_SLIDER_GREEN);

		gDraw.DrawOutlined(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_BLACK);

		gDraw.DrawRect(x + ((float)((int)(bytes[1]) - var->min) * len), y + (SLIDER_HEIGHT * 0.5f), SLIDER_DOT_SIZE, SLIDER_HEIGHT, CLR_BRIGHT);
		gDraw.DrawOutlined(x + ((float)((int)(bytes[1]) - var->min) * len), y + (SLIDER_HEIGHT * 0.5f), SLIDER_DOT_SIZE, SLIDER_HEIGHT, CLR_BLACK);

		x += size + BASE_SPACE;
	}

	{
		if (ShouldHighlightElements() && CursorWithinRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT))
		{
			if (mMouseClicked)
			{
				mCurrentVar = var;
				mColorSelected = 3;
			}

			gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_HALF_BRIGHT);
		}
		else
		{
			if (mCurrentVar == var && mColorSelected == 3)
				gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_HALF_BRIGHT);
			else
				gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_HALF);
		}

		float max = (float)(var->max);
		max -= (float)(var->min);

		float len = (size - SLIDER_DOT_SIZE) / max;

		if (mCurrentVar == var && mColorSelected == 3)
		{
			float dist = ((float)(mMouseX) - x);
			int result = (int)(clamp_float((max * (dist / size)) + 0.5f, 0.0f, max)) + var->min;

			bytes[2] = result;
		}

		gDraw.DrawRect(x, y + (SLIDER_HEIGHT * 0.5f), ((float)((int)(bytes[2]) - var->min) * len), SLIDER_HEIGHT, CLR_SLIDER_BLUE);

		gDraw.DrawOutlined(x, y + (SLIDER_HEIGHT * 0.5f), size, SLIDER_HEIGHT, CLR_BLACK);

		gDraw.DrawRect(x + ((float)((int)(bytes[2]) - var->min) * len), y + (SLIDER_HEIGHT * 0.5f), SLIDER_DOT_SIZE, SLIDER_HEIGHT, CLR_BRIGHT);
		gDraw.DrawOutlined(x + ((float)((int)(bytes[2]) - var->min) * len), y + (SLIDER_HEIGHT * 0.5f), SLIDER_DOT_SIZE, SLIDER_HEIGHT, CLR_BLACK);
	}
}

void CMenu::DrawCheckboxPlayer(SettingVar* var, int i)
{
	int* value = &var->value;

	float x = (mMenuPosX + BASE_WIDTH + 1) + PCONTENTS_WIDTH - BASE_SPACE;
	float y = mMenuPosY + ((TICKBOX_SIZE + (BASE_SPACE * 2.0f)) * (float)(i + 1));

	x -= TICKBOX_SIZE;

	gDraw.DrawTextA(var->name, (mMenuPosX + BASE_WIDTH + 1) + 6, y - 2, CLR_WHITE, false, false);

	if (CursorWithinRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE))
	{
		if (mMouseClicked)
		{
			if (*value)
				*value = 0;
			else
				*value = 1;
		}

		if (*value)
			gDraw.DrawRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_SELECT_GREEN);
		else
			gDraw.DrawRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_SELECT_RED);
	}
	else
	{
		if (*value)
			gDraw.DrawRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_LIGHT_GREEN);
		else
			gDraw.DrawRect(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_LIGHT_RED);
	}

	gDraw.DrawOutlined(x, y, TICKBOX_SIZE, TICKBOX_SIZE, CLR_BLACK);
}

void CMenu::DrawListPlayer(SettingVar* var, int i)
{
	int* value = &var->value;

	if (*value < var->min)
		*value = var->min;

	if (*value > var->max)
		*value = var->max;

	float x = (mMenuPosX + BASE_WIDTH + 1) + PCONTENTS_WIDTH - BASE_SPACE;
	float y = mMenuPosY + ((TICKBOX_SIZE + (BASE_SPACE * 2.0f)) * (float)(i + 1));

	x -= LIST_WIDTH;

	gDraw.DrawTextA(var->name, (mMenuPosX + BASE_WIDTH + 1) + 6, y - 2, CLR_WHITE, false, false);

	if (ShouldHighlightElements() && CursorWithinRect(x, y, LIST_WIDTH, LIST_HEIGHT))
	{
		if (mMouseClicked)
		{
			mCurrentVar = var;
			mListPosX = x;
			mListPosY = y;
		}

		mSelectedList = var;

		gDraw.DrawRect(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_BRIGHT);
	}
	else
	{
		gDraw.DrawRect(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_HALF);
	}

	gDraw.DrawOutlined(x, y, LIST_WIDTH, LIST_HEIGHT, CLR_BRIGHT);

	gDraw.DrawTextA(var->options[*value], x + 6, y, CLR_WHITE, false, false);
}

void CMenu::DrawItems()
{
	if (!mCurrentSection)
		return;

	MenuSection* section = mCurrentSection;

	if (!section->current_tab)
		return;

	MenuTab* tab = section->current_tab;

	if (tab->special != 0)
	{
		switch (tab->special)
		{
			case SPECIALTAB_PLAYERLIST:
			{
				DrawPlayerList();
				break;
			}
		}

		return;
	}

	for (int i = 0; i < tab->var_count; ++i)
	{
		SettingVar* var = tab->vars[i];

		if (!var)
		{
			base->Warning("var fail\n");
			return;
		}

		switch (var->type)
		{
			case VARTYPE_CHECKBOX:
			{
				DrawCheckbox(var, i);
				break;
			}
			case VARTYPE_LIST:
			{
				DrawList(var, i);
				break;
			}
			case VARTYPE_KEYLIST:
			{
				DrawKeylist(var, i);
				break;
			}
			case VARTYPE_FLAGLIST:
			{
				DrawFlaglist(var, i);
				break;
			}
			case VARTYPE_SLIDER:
			{
				if (var->slider_type == SLIDERTYPE_COLOR)
					DrawColorSlider(var, i);
				else
					DrawSlider(var, i);

				break;
			}
		}
	}
}

/*
check the 3 parameters that need to be set for every player list item
*/

int sPlayerPos = 0;
SettingVar sSliderVar;
Entity* sPlayerEntity = nullptr;

void CMenu::DrawPlayerList()
{
	sSliderVar.type = VARTYPE_SLIDER;

	float x = mMenuPosX + SECTION_WIDTH + BASE_SPACE;
	float y = mMenuPosY + BASE_SPACE;

	if (mCurrentSection->tab_count > 1)
		y += TAB_HEIGHT + BASE_SPACE;

	float width = BASE_WIDTH - SECTION_WIDTH - (BASE_SPACE*2);
	float height = BASE_HEIGHT - (BASE_SPACE*2);
	if (mCurrentSection->tab_count > 1)
		height -= TAB_HEIGHT + BASE_SPACE;

	gDraw.DrawRect(x, y, width, height, CLR_HALF);
	gDraw.DrawOutlined(x, y, width, height, CLR_BLACK);

	y += BASE_SPACE;

	int count = 0;
	Entity* valid[MAX_PLAYERS];
	Entity* cur_player = nullptr;

	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));

	if (local && !base->IsPlayingDemo(base->mBaseEngine))
	{
		for (int i = 1; i <= base->mGlobalVars->clients; ++i)
		{
			Entity* player = base->GetClientEntity(base->mEntList, i);
			if (!player)
				continue;

			if (player == local)
				continue;

			PlayerObject* obj = gPlayers.GetObject(player->GetIndex());
			if (!obj)
				continue;

			valid[count++] = player;
		}
	}

	// list scale

	float sz = (height - (BASE_SPACE * 2)) / 18;

	gDraw.DrawRect(x + width - SLIDER_DOT_SIZE - BASE_SPACE, y, SLIDER_DOT_SIZE, height - (BASE_SPACE*2), CLR_HALF_BRIGHT);

	if (count > 18)
	{
		float size = height - (BASE_SPACE*2) - (SLIDER_DOT_SIZE*2);
		int max = count - 18;
		int pos2 = sPlayerPos - 18;
		if (pos2 < 0)
			pos2 = 0;

		if (CursorWithinRect(x + width - SLIDER_DOT_SIZE - BASE_SPACE, y + ((float)(sPlayerPos) * size), SLIDER_DOT_SIZE, SLIDER_DOT_SIZE*2))
		{
			if (mMouseClicked)
				mCurrentVar = &sSliderVar;

			gDraw.DrawRect(x + width - SLIDER_DOT_SIZE - BASE_SPACE, y + ((float)(sPlayerPos) * size), SLIDER_DOT_SIZE, SLIDER_DOT_SIZE*2, CLR_SELECT);
		}
		else
		{
			gDraw.DrawRect(x + width - SLIDER_DOT_SIZE - BASE_SPACE, y + ((float)(sPlayerPos) * size), SLIDER_DOT_SIZE, SLIDER_DOT_SIZE*2, CLR_BRIGHT);
		}

		gDraw.DrawOutlined(x + width - SLIDER_DOT_SIZE - BASE_SPACE, y + ((float)(sPlayerPos) * size), SLIDER_DOT_SIZE, SLIDER_DOT_SIZE*2, CLR_BLACK);

		if (mCurrentVar == &sSliderVar)
		{
			float dist = ((float)(mMouseY) - y);
			sPlayerPos = (int)(clamp_float((max * (dist / size)), 0.0f, max));
		}
	}

	gDraw.DrawOutlined(x + width - SLIDER_DOT_SIZE - BASE_SPACE, y, SLIDER_DOT_SIZE, height - (BASE_SPACE*2), CLR_BLACK);

	x += BASE_SPACE;

	// list elements

	float width2 = width - SLIDER_DOT_SIZE - (BASE_SPACE*3);

	gDraw.DrawRect(x, y, width2, sz * 18, CLR_BACKGROUND);

	if (count != 0)
	{
		int limit = min(sPlayerPos + 18, count);
		float offset = 0;
		bool select = false;

		for (int i = sPlayerPos; i < limit; ++i)
		{
			Entity* player = valid[i];
			if (!player)
				break;

			PlayerObject* obj = gPlayers.GetObject(player->GetIndex());
			if (!obj)
				continue;

			if (i == mPlayerSelected)
			{
				gDraw.DrawRect(x, y + offset, width2, sz, CLR_HALF);

				if (player != sPlayerEntity)
				{
					gVars.player_hitbox.value = obj->hitbox;
					gVars.player_filter.value = obj->filter;
					gVars.player_fixpitch.value = obj->fixpitch;
					gVars.player_fixyaw.value = obj->fixyaw;
					gVars.player_resolve_pitch.value = obj->resolve_pitch;
					gVars.player_ignore_resolve.value = obj->ignore_resolve;
				}

				cur_player = player;
				sPlayerEntity = player;
			}
			else if (CursorWithinRect(x, y + offset, width2, sz) && !select)
			{
				if (mMouseClicked)
				{
					mPlayerSelected = i;
					cur_player = player;
					sPlayerEntity = player;

					gVars.player_hitbox.value = obj->hitbox;
					gVars.player_filter.value = obj->filter;
					gVars.player_fixpitch.value = obj->fixpitch;
					gVars.player_fixyaw.value = obj->fixyaw;
					gVars.player_resolve_pitch.value = obj->resolve_pitch;
					gVars.player_ignore_resolve.value = obj->ignore_resolve;
				}

				gDraw.DrawRect(x, y + offset, width2, sz, CLR_DARK);
				select = true;
			}

			if (i != (limit - 1))
				gDraw.DrawLine(x, y + offset + sz, x + (width2), y + offset + sz, CLR_BLACK); 

			PlayerInfo info;
			base->GetPlayerInfo(base->mBaseEngine, player->GetIndex(), &info);

			if (*player->m_iTeamNum() == 2)
				gDraw.DrawTextA(info.name, x + 6, (y - 1) + offset, CLR_TERRORIST);
			else if (*player->m_iTeamNum() == 3)
				gDraw.DrawTextA(info.name, x + 6, (y - 1) + offset, CLR_COUNTER_TERRORIST);
			else
				gDraw.DrawTextA(info.name, x + 6, (y - 1) + offset, CLR_WHITE);

			offset += sz;
		}
	}
	else
	{
		gDraw.DrawTextA("no players", x + 6, y, CLR_WHITE);
	}

	gDraw.DrawOutlined(x, y, width2, sz * 18, CLR_BLACK);

	if (!cur_player)
		return;

	PlayerObject* obj = gPlayers.GetObject(cur_player->GetIndex());
	if (!obj)
		return;

	x = mMenuPosX + BASE_WIDTH + 1;
	y = mMenuPosY;

	gDraw.DrawRect(x, y, PCONTENTS_WIDTH, BASE_HEIGHT, CLR_BACKGROUND);
	gDraw.DrawOutlined(x, y, PCONTENTS_WIDTH, BASE_HEIGHT, CLR_BLACK);

	{
		PlayerInfo info;
		base->GetPlayerInfo(base->mBaseEngine, cur_player->GetIndex(), &info);

		gDraw.DrawTextA(info.name, x + 6, y + 2, CLR_WHITE);
	}

	y += (TICKBOX_SIZE + (BASE_SPACE * 2));

	SettingVar* vars[32];
	memset(vars, 0, sizeof(DWORD) * 32);

	int var_count = 0;

	{
		vars[var_count++] = &gVars.player_hitbox;
		vars[var_count++] = &gVars.player_filter;
		vars[var_count++] = &gVars.player_fixpitch;
		vars[var_count++] = &gVars.player_fixyaw;
		vars[var_count++] = &gVars.player_resolve_pitch;
		vars[var_count++] = &gVars.player_ignore_resolve;
	}

	for (int i = 0; i < var_count; ++i)
	{
		SettingVar* var = vars[i];

		if (!var)
		{
			base->Warning("var fail\n");
			return;
		}

		switch (var->type)
		{
			case VARTYPE_CHECKBOX:
			{
				DrawCheckboxPlayer(var, i);
				break;
			}
			case VARTYPE_LIST:
			{
				DrawListPlayer(var, i);
				break;
			}
		}
	}
	
	{
		obj->hitbox = gVars.player_hitbox.value;
		obj->filter = gVars.player_filter.value;
		obj->fixpitch = gVars.player_fixpitch.value;
		obj->fixyaw = gVars.player_fixyaw.value;
		obj->resolve_pitch = gVars.player_resolve_pitch.value;
		obj->ignore_resolve = gVars.player_ignore_resolve.value;
	}
}

void CMenu::DrawCurrentVar()
{
	if (!mCurrentVar)
		return;

	if (mCurrentVar->type == VARTYPE_LIST)
	{
		float x = mListPosX;
		float y = mListPosY;

		float sz = (LIST_HEIGHT * (mCurrentVar->max + 1));

		int* value = &mCurrentVar->value;

		if (mCurrentVar->group)
			value = &(mCurrentVar->group_vars[gVars.current_group.value].value);

		for (int i = 0; i <= mCurrentVar->max; ++i)
		{
			if (CursorWithinRect(mListPosX, mListPosY + (LIST_HEIGHT * i), LIST_WIDTH, LIST_HEIGHT))
			{
				if (mMouseClicked && mVarAcceptClicks)
				{
					*value = i;

					mCurrentVar = nullptr;

					return;
				}

				gDraw.DrawRect(mListPosX, mListPosY + (LIST_HEIGHT * i), LIST_WIDTH, LIST_HEIGHT, CLR_BRIGHT);
			}
			else
			{
				gDraw.DrawRect(mListPosX, mListPosY + (LIST_HEIGHT * i), LIST_WIDTH, LIST_HEIGHT, CLR_HALF);
			}

			gDraw.DrawTextA(mCurrentVar->options[i], x + 6, y + (LIST_HEIGHT * i), CLR_WHITE, false, false);
		}

		gDraw.DrawOutlined(mListPosX, mListPosY, LIST_WIDTH, sz, CLR_BRIGHT);

		if (!CursorWithinRect(mListPosX, mListPosY, LIST_WIDTH, sz))
		{
			if (mMouseClicked && mVarAcceptClicks)
			{
				mCurrentVar = nullptr;

				return;
			}
		}
	}
	else if (mCurrentVar->type == VARTYPE_FLAGLIST)
	{
		float x = mListPosX;
		float y = mListPosY;

		float sz = (LIST_HEIGHT * (mCurrentVar->max + 1));

		int* value = &mCurrentVar->value;

		if (mCurrentVar->group)
			value = &mCurrentVar->group_vars[gVars.current_group.value].value;

		for (int i = 0; i <= mCurrentVar->max; ++i)
		{
			if (CursorWithinRect(mListPosX, mListPosY + (LIST_HEIGHT * i), LIST_WIDTH, LIST_HEIGHT))
			{
				if (mMouseClicked && mVarAcceptClicks)
				{
					if (*value & (1<<i))
						*value &= ~(1<<i);
					else
						*value |= (1<<i);

					return;
				}

				if (*value & (1<<i))
					gDraw.DrawRect(mListPosX, mListPosY + (LIST_HEIGHT * i), LIST_WIDTH, LIST_HEIGHT, CLR_SELECT_GREEN);
				else
					gDraw.DrawRect(mListPosX, mListPosY + (LIST_HEIGHT * i), LIST_WIDTH, LIST_HEIGHT, CLR_SELECT_RED);
			}
			else
			{
				if (*value & (1<<i))
					gDraw.DrawRect(mListPosX, mListPosY + (LIST_HEIGHT * i), LIST_WIDTH, LIST_HEIGHT, CLR_LIGHT_GREEN);
				else
					gDraw.DrawRect(mListPosX, mListPosY + (LIST_HEIGHT * i), LIST_WIDTH, LIST_HEIGHT, CLR_LIGHT_RED);
			}

			gDraw.DrawTextA(mCurrentVar->options[i], x + 6, y + (LIST_HEIGHT * i), CLR_WHITE, false, false);
		}

		gDraw.DrawOutlined(mListPosX, mListPosY, LIST_WIDTH, sz, CLR_BRIGHT);

		if (!CursorWithinRect(mListPosX, mListPosY, LIST_WIDTH, sz))
		{
			if (mMouseClicked && mVarAcceptClicks)
			{
				mCurrentVar = nullptr;

				return;
			}
		}
	}
	else if (mCurrentVar->type == VARTYPE_SLIDER)
	{
		if (!mMouseHeld)
		{
			mCurrentVar = nullptr;
			mColorSelected = 0;
		}
	}
}

void CMenu::DrawOptionMenu()
{
	if (CursorWithinRect(mMenuPosX, mMenuPosY - BAR_HEIGHT, BASE_WIDTH, BAR_HEIGHT))
	{
		if (mMouse2Clicked && !mPopupDrawing)
		{
			if (mOptionMenuDrawing)
			{
				mOptionMenuDrawing = false;
			}
			else
			{
				mOptionMenuDrawing = true;
				mOptionMenuPosX = (float)(mMouseX);
				mOptionMenuPosY = (float)(mMouseY);
			}
		}
	}

	if (!mOptionMenuDrawing)
		return;

	float ox = mOptionMenuPosX;
	float oy = mOptionMenuPosY;

	float sz = (OPTION_ITEM_SIZE * (float)(mOptionCount));

	if (!CursorWithinRect(ox, oy, OPTION_ITEM_WIDTH, sz))
	{
		if (mMouseClicked)
			mOptionMenuDrawing = false;
	}

	for (int i = 0; i < mOptionCount; ++i)
	{
		MenuOption* option = &mOptions[i];

		if (CursorWithinRect(ox, oy + (15 * i), OPTION_ITEM_WIDTH, OPTION_ITEM_SIZE))
		{
			if (mMouseClicked)
			{
				if (option->popup)
				{
					mPopupDrawing = true;

					SecureZeroMemory(&mPopupWindow, sizeof(MenuPopup));

					string_cpy(mPopupWindow.name, option->name);
					string_cpy(mPopupWindow.description, option->description);
					mPopupWindow.type = option->type;

					if (option->type == POPUP_STRING)
						mPopupWindow.string_param = (StringParamFn)(option->func);
					else
						mPopupWindow.no_param = (NoParamFn)(option->func);

					mPopupWindow.string_max = option->string_max;
				}
				else
				{
					// if no popup window then we just call the function right away
					if (option->func)
						option->func();
				}

				mOptionMenuDrawing = false;
			}

			gDraw.DrawRect(ox, oy + (OPTION_ITEM_SIZE * i), OPTION_ITEM_WIDTH, OPTION_ITEM_SIZE, CLR_TRANSPARENT_SELECT);
		}
		else
		{
			gDraw.DrawRect(ox, oy + (OPTION_ITEM_SIZE * i), OPTION_ITEM_WIDTH, OPTION_ITEM_SIZE, CLR_TRANSPARENT);
		}

		gDraw.DrawTextA(option->name, ox + 6, (oy - 1) + (OPTION_ITEM_SIZE * i), CLR_WHITE, false, false);
	}

	gDraw.DrawOutlined(ox, oy, OPTION_ITEM_WIDTH, sz, CLR_BLACK);
}

void CMenu::DrawPopupWindow()
{
	if (!mPopupDrawing)
		return;

	if (mMouse2Clicked)
		mPopupDrawing = false;

	float px = gDraw.mWidth * 0.5f;
	float py = gDraw.mHeight * 0.5f;

	{
		float cx = px - (POPUP_WIDTH * 0.5f);
		float cy = py - (POPUP_HEIGHT * 0.5f);

		gDraw.DrawRect(cx, cy, POPUP_WIDTH, POPUP_HEIGHT, CLR_BACKGROUND);
		gDraw.DrawOutlined(cx, cy, POPUP_WIDTH, POPUP_HEIGHT, CLR_BLACK);
		gDraw.DrawOutlined(cx, cy, POPUP_WIDTH, POPUP_BAR_HEIGHT, CLR_BLACK);

		gDraw.DrawTextA(mPopupWindow.name, cx + 6, cy, CLR_WHITE, false, false);
	}

	if (mPopupWindow.type == POPUP_STRING)
		gDraw.DrawTextA(mPopupWindow.description, px + 6, (py - 2) - (POPUP_HEIGHT * 0.25f), CLR_WHITE, true, false);
	else
		gDraw.DrawTextA(mPopupWindow.description, px + 6, (py - 2) - 15, CLR_WHITE, true, false);

	if (mPopupWindow.type == POPUP_OK)
	{
		if (CursorWithinRect(px - (POPUP_BUTTON_WIDTH * 0.5f), py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT))
		{
			if (mMouseClicked)
			{
				mPopupDrawing = false;

				if (mPopupWindow.no_param)
					mPopupWindow.no_param();
			}

			gDraw.DrawRect(px - (POPUP_BUTTON_WIDTH * 0.5f), py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_HALF);
		}
		else
		{
			gDraw.DrawRect(px - (POPUP_BUTTON_WIDTH * 0.5f), py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BRIGHT);
		}

		gDraw.DrawOutlined(px - (POPUP_BUTTON_WIDTH * 0.5f), py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BLACK);

		gDraw.DrawTextA("ok", px + 6, (py + (POPUP_HEIGHT * 0.25f) + (POPUP_BUTTON_HEIGHT * 0.5f)), CLR_WHITE, true, true);
	}
	else if (mPopupWindow.type == POPUP_YESNO)
	{
		if (CursorWithinRect(px - (POPUP_BUTTON_WIDTH)-BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT))
		{
			if (mMouseClicked)
			{
				*mPopupWindow.variable = true;
				mPopupDrawing = false;

				if (mPopupWindow.string_param)
					mPopupWindow.string_param(mPopupWindow.string);
				else if (mPopupWindow.no_param)
					mPopupWindow.no_param();
			}

			gDraw.DrawRect(px - (POPUP_BUTTON_WIDTH)-BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_HALF);
		}
		else
		{
			gDraw.DrawRect(px - (POPUP_BUTTON_WIDTH)-BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BRIGHT);
		}

		gDraw.DrawOutlined(px - (POPUP_BUTTON_WIDTH)-BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BLACK);

		gDraw.DrawTextA("yes", ((px - (POPUP_BUTTON_WIDTH * 0.5f)) - BASE_SPACE) + 6, (py + (POPUP_HEIGHT * 0.25f) + (POPUP_BUTTON_HEIGHT * 0.5f)), CLR_WHITE, true, true);

		if (CursorWithinRect(px + BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT))
		{
			if (mMouseClicked)
			{
				*mPopupWindow.variable = false;
				mPopupDrawing = false;

				if (mPopupWindow.string_param)
					mPopupWindow.string_param(mPopupWindow.string);
				else if (mPopupWindow.no_param)
					mPopupWindow.no_param();
			}

			gDraw.DrawRect(px + BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_HALF);
		}
		else
		{
			gDraw.DrawRect(px + BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BRIGHT);
		}

		gDraw.DrawOutlined(px + BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BLACK);

		gDraw.DrawTextA("no", ((px + (40)) - BASE_SPACE) + 6, (py + (POPUP_HEIGHT * 0.25f) + (POPUP_TEXTBAR_HEIGHT * 0.5f)), CLR_WHITE, true, true);
	}
	else
	{
		if (CursorWithinRect(px - (POPUP_BUTTON_WIDTH)-BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT))
		{
			if (mMouseClicked)
			{
				mPopupDrawing = false;

				if (mPopupWindow.type == POPUP_STRING)
					mPopupWindow.string_param(mPopupWindow.string);
				else if (mPopupWindow.no_param)
					mPopupWindow.no_param();
			}

			gDraw.DrawRect(px - (POPUP_BUTTON_WIDTH)-BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_HALF);
		}
		else
		{
			gDraw.DrawRect(px - (POPUP_BUTTON_WIDTH)-BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BRIGHT);
		}

		gDraw.DrawOutlined(px - (POPUP_BUTTON_WIDTH)-BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BLACK);

		gDraw.DrawTextA("ok", ((px - (POPUP_BUTTON_WIDTH * 0.5f)) - BASE_SPACE) + 6, (py + (POPUP_HEIGHT * 0.25f) + (POPUP_BUTTON_HEIGHT * 0.5f)), CLR_WHITE, true, true);

		if (CursorWithinRect(px + BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT))
		{
			if (mMouseClicked)
				mPopupDrawing = false;

			gDraw.DrawRect(px + BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_HALF);
		}
		else
		{
			gDraw.DrawRect(px + BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BRIGHT);
		}

		gDraw.DrawOutlined(px + BASE_SPACE, py + (POPUP_HEIGHT * 0.25f), POPUP_BUTTON_WIDTH, POPUP_BUTTON_HEIGHT, CLR_BLACK);

		gDraw.DrawTextA("cancel", ((px + (40)) - BASE_SPACE) + 6, (py + (POPUP_HEIGHT * 0.25f) + (POPUP_TEXTBAR_HEIGHT * 0.5f)), CLR_WHITE, true, true);

		if (mPopupWindow.type == POPUP_STRING)
		{
			gDraw.DrawRect(px - (POPUP_TEXTBAR_WIDTH * 0.5f), py - (POPUP_TEXTBAR_HEIGHT * 0.5f), POPUP_TEXTBAR_WIDTH, POPUP_TEXTBAR_HEIGHT, CLR_HALF);
			gDraw.DrawOutlined(px - (POPUP_TEXTBAR_WIDTH * 0.5f), py - (POPUP_TEXTBAR_HEIGHT * 0.5f), POPUP_TEXTBAR_WIDTH, POPUP_TEXTBAR_HEIGHT, CLR_BLACK);

			gDraw.DrawTextA(mPopupWindow.string, (px - (POPUP_TEXTBAR_WIDTH * 0.5f)) + 6, py, CLR_WHITE, false, true);
		}
	}
}

CRC32 CMenu::ReadConfig(HKEY base_key, char* name, MenuConfig* target)
{
	HKEY sub_key;

	if (base->RegCreateKeyExA(base_key, name, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &sub_key, nullptr) != ERROR_SUCCESS)
	{
		base->Warning("CMenu: could not create sub key\n");
		return 0;
	}

	string_cpy(target->name, name);
	target->crc = CRC32_Get(name, string_len(name));

	DWORD var_count = 0;

	for (int i = 0; i < mSectionCount; ++i)
	{
		MenuSection* section = &mSections[i];

		for (int n = 0; n < section->tab_count; ++n)
		{
			MenuTab* tab = &section->tabs[n];

			for (int k = 0; k < tab->var_count; ++k)
			{
				SettingVar* var = tab->vars[k];

				if (var->group)
				{
					for (int v = 0; v < MAX_GROUP_VARS; ++v)
					{
						GroupVar* group_var = &var->group_vars[v];

						if (string_len(group_var->name) != 0)
						{
							CRC32 crc_section = 0;
							CRC32 crc_tab = 0;
							CRC32 crc_var = 0;
							CRC32 crc_group = 0;

							{
								crc_section = CRC32_Get(section->name, string_len(section->name));
							}

							{
								crc_tab = CRC32_Get(tab->name, string_len(tab->name));
							}

							{
								crc_var = CRC32_Get(var->name, string_len(var->name));
							}

							{
								crc_group = CRC32_Get(group_var->name, string_len(group_var->name));
							}

							char var_name[128];
							base->wsprintfA(var_name, "%x %x %x %x", crc_section, crc_tab, crc_var, crc_group);

							CRC32 crc = CRC32_Get(var_name, string_len(var_name));

							char var_crc[32];
							base->wsprintfA(var_crc, "%x", crc);

							DWORD type = REG_DWORD;
							DWORD out = (DWORD)(var->default_value);
							DWORD size = 4;

							base->RegQueryValueExA(sub_key, var_crc, nullptr, &type, (BYTE*)(&out), &size);

							group_var->value = out;
						}
					}
				}
				else
				{
					CRC32 crc_section = 0;
					CRC32 crc_tab = 0;
					CRC32 crc_var = 0;

					{
						crc_section = CRC32_Get(section->name, string_len(section->name));
					}

					{
						crc_tab = CRC32_Get(tab->name, string_len(tab->name));
					}

					{
						crc_var = CRC32_Get(var->name, string_len(var->name));
					}

					char var_name[128];
					base->wsprintfA(var_name, "%x %x %x", crc_section, crc_tab, crc_var);

					CRC32 crc = CRC32_Get(var_name, string_len(var_name));

					char var_crc[32];
					base->wsprintfA(var_crc, "%x", crc);

					DWORD type = REG_DWORD;
					DWORD out = (DWORD)(var->default_value);
					DWORD size = 4;

					base->RegQueryValueExA(sub_key, var_crc, nullptr, &type, (BYTE*)(&out), &size);

					var->value = out;
				}
			}
		}
	}

	memcpy(&target->vars, &gVars, sizeof(Vars));

	return target->crc;
}

Vars sOldMenuVars;

CRC32 CMenu::WriteConfig(HKEY base_key, char* name, CRC32 copy)
{
	HKEY sub_key;

	if (base->RegCreateKeyExA(base_key, name, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &sub_key, nullptr) != ERROR_SUCCESS)
	{
		base->Warning("CMenu: could not create sub key\n");
		return 0;
	}

	MenuConfig* cfg = nullptr;

	for (int i = 0; i < mConfigCount; ++i)
	{
		MenuConfig* v = &mConfigs[i];
		
		if (v->crc == CRC32_Get(name, string_len(name)))
		{
			cfg = v;
			break;
		}
	}

	if (!cfg)
	{
		cfg = &mConfigs[0];
		string_cpy(cfg->name, name);
		cfg->crc = CRC32_Get(name, string_len(name));
	}

	if (copy != 0)
	{
		if (mConfigCount != 0)
		{
			bool copied = false;

			for (int i = 0; i < mConfigCount; ++i)
			{
				if (mConfigs[i].crc == copy)
				{
					copied = true;

					cfg->vars = mConfigs[i].vars;

					break;
				}
			}
		}
		else
		{
			base->Warning("CMenu: could not copy from config (no configs to iterate)\n");
		}
	}
	
	CRC32 old_config = mCurrentConfig;

	memcpy(&sOldMenuVars, &gVars, sizeof(Vars));

	memcpy(&gVars, &cfg->vars, sizeof(Vars));

	for (int i = 0; i < mSectionCount; ++i)
	{
		MenuSection* section = &mSections[i];

		for (int n = 0; n < section->tab_count; ++n)
		{
			MenuTab* tab = &section->tabs[n];

			for (int k = 0; k < tab->var_count; ++k)
			{
				SettingVar* var = tab->vars[k];
				
				if (var->group)
				{
					for (int v = 0; v < MAX_GROUP_VARS; ++v)
					{
						GroupVar* group_var = &var->group_vars[v];

						if (string_len(group_var->name) != 0)
						{
							CRC32 crc_section = 0;
							CRC32 crc_tab = 0;
							CRC32 crc_var = 0;
							CRC32 crc_group = 0;

							{
								crc_section = CRC32_Get(section->name, string_len(section->name));
							}

							{
								crc_tab = CRC32_Get(tab->name, string_len(tab->name));
							}

							{
								crc_var = CRC32_Get(var->name, string_len(var->name));
							}

							{
								crc_group = CRC32_Get(group_var->name, string_len(group_var->name));
							}

							char var_name[128];
							base->wsprintfA(var_name, "%x %x %x %x", crc_section, crc_tab, crc_var, crc_group);

							CRC32 crc = CRC32_Get(var_name, string_len(var_name));

							char var_crc[32];
							base->wsprintfA(var_crc, "%x", crc);

							DWORD value = (DWORD)(group_var->value);
							base->RegSetValueExA(sub_key, var_crc, 0, REG_DWORD, (BYTE*)(&value), 4);
						}
					}
				}
				else
				{
					CRC32 crc_section = 0;
					CRC32 crc_tab = 0;
					CRC32 crc_var = 0;

					{
						crc_section = CRC32_Get(section->name, string_len(section->name));
					}

					{
						crc_tab = CRC32_Get(tab->name, string_len(tab->name));
					}

					{
						crc_var = CRC32_Get(var->name, string_len(var->name));
					}

					char var_name[128];
					base->wsprintfA(var_name, "%x %x %x", crc_section, crc_tab, crc_var);

					CRC32 crc = CRC32_Get(var_name, string_len(var_name));

					char var_crc[32];
					base->wsprintfA(var_crc, "%x", crc);

					DWORD value = (DWORD)(var->value);
					base->RegSetValueExA(sub_key, var_crc, 0, REG_DWORD, (BYTE*)(&value), 4);
				}
			}
		}
	}

	mCurrentConfig = old_config;
	memcpy(&gVars, &sOldMenuVars, sizeof(Vars));

	base->RegCloseKey(sub_key);

	return cfg->crc;
}

void CMenu::WriteAll()
{
	HKEY key;

	if (base->RegCreateKeyExA(HKEY_CURRENT_USER, __REGKEY_NAME__, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, nullptr) != ERROR_SUCCESS)
	{
		base->Warning("CMenu: could not open key\n");
		return;
	}

	base->RegSetValueExA(key, "config", 0, REG_DWORD, (BYTE*)(&mCurrentConfig), 4);

	for (int i = 0; i < mConfigCount; ++i)
	{
		if (mConfigs[i].crc == mCurrentConfig)
		{
			memcpy(&mConfigs[i].vars, &gVars, sizeof(Vars));
			break;
		}
	}

	for (int i = 0; i < mConfigCount; ++i)
		WriteConfig(key, mConfigs[i].name, 0);

	base->RegCloseKey(key);
}

void CMenu::RegistryInit()
{
	HKEY key;

	if (base->RegCreateKeyExA(HKEY_CURRENT_USER, __REGKEY_NAME__, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, nullptr) != ERROR_SUCCESS)
	{
		base->Warning("CMenu: could not open key\n");
		mInit = true;
		mIssueUnload = true;

		return;
	}

	DWORD subkey_count = 0;

	if (base->RegQueryInfoKeyA(key, nullptr, nullptr, 0, &subkey_count, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
	{
		base->Warning("CMenu: could not query key info\n");
		mInit = true;
		mIssueUnload = true;

		return;
	}

	if (subkey_count == 0)
	{
		CRC32 crc = WriteConfig(key, "default", 0);

		if (crc == 0)
		{
			base->Warning("CMenu: could not write default config\n");
			mInit = true;
			mIssueUnload = true;

			return;
		}

		mCurrentConfig = crc;
		subkey_count = 1;
	}
	else
	{
		DWORD type = REG_DWORD;
		DWORD out = CRC32_Get("default", string_len("default"));
		DWORD size = 4;

		if (base->RegQueryValueExA(key, "config", nullptr, &type, (BYTE*)(&out), &size) != ERROR_SUCCESS)
			base->RegSetValueExA(key, "config", 0, REG_DWORD, (BYTE*)(&out), 4);

		mCurrentConfig = out;
	}

	int current_cfg = 0;
	mConfigCount = 0;

	for (int i = 0; i < subkey_count; ++i)
	{
		char name[128];
		SecureZeroMemory(name, sizeof(name));

		DWORD size = 128;

		if (base->RegEnumKeyExA(key, i, name, &size, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
			continue;

		DWORD crc = ReadConfig(key, name, &mConfigs[i]);

		if (crc == mCurrentConfig)
			current_cfg = i;

		mConfigCount += 1;
	}

	memcpy(&gVars, &mConfigs[current_cfg].vars, sizeof(Vars));

	base->RegCloseKey(key);
}

void CMenu::Init()
{
	gVars.Init();
	InitElements();

	RegistryInit();

	mInit = true;
}