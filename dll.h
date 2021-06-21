#pragma once

#pragma warning (disable: 4244)
#pragma warning (disable: 4715)
#pragma warning (disable: 4305)
#pragma warning (disable: 4800)

#define __COOL_DATE__ __DATE__
#define __HACK_NAME__ "CSGO legit/rage"
#define __HACK_STR__ __HACK_NAME__ " " __COOL_DATE__
#define __REGKEY_NAME__ "Software\\csgolr"
#define WIN32_LEAN_AND_MEAN

#pragma warning(disable: 4996)
#pragma warning(disable: 4805)

#define SOURCE_DLL

#include "windows.h"
#include "crc32.h"
#include "trig.h"
#include "util.h"
#include "md5.h"

#include "vec.h"
#include "const.h"
#include "structs.h"

#include "base.h"

extern CBaseClass* base;

#include "entity.h"
#include "weapon.h"

#include "csgo.h"
#include "vars.h"

#include "draw.h"
#include "menu.h"

#include "players.h"
#include "legitaim.h"
#include "aimbot.h"
#include "esp.h"
#include "render.h"
#include "hvh.h"

#include "hurt.h"
#include "impact.h"

extern DWORD __cdecl GetBaseClassAddress();
extern DWORD __cdecl GetBaseClassSize();