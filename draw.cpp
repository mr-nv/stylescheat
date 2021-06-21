#include "dll.h"

static unsigned int s_52c24973[] = {0x98bc593f, 0xbf8d625c, 0xf8922f5c, 0xf2d46359, 0xd7fb0a15}; // CSGO hvh %i/%i/%i
static unsigned int k_52c24973 = 0xafbd77c;

static unsigned int s_9958d171[] = {0xec1f1d42, 0xbe181768, 0x9e1d174f}; // Courier New
static unsigned int k_9958d171 = 0x726a9e01;

CDrawManager gDraw;

void CDrawManager::Init()
{
	if (!mFont)
	{
		mFont = base->CreateFont(base->mSurface);
		base->SetFontGlyphSet(base->mSurface, mFont, "Courier New", 14, 0, 0, 0, 0x200, 0, 0);
		mFontBackup = mFont;
	}

	mInit = true;
}

void CDrawManager::Think()
{
	if (!mInit)
		Init();

	{
		static int baller = 0;
		if (!baller)
		{
			baller = base->CreateFont(base->mSurface);
			base->SetFontGlyphSet(base->mSurface, baller, "Verdana", 12, 700, 0, 0, 0x10, 0, 0);
		}

		if (gVars.peppers.value)
			mFont = baller;
		else
			mFont = mFontBackup;
	}

	// get the active players
	gPlayers.Think();

	// get screen dimensions
	{
		int screen_width = 0;
		int screen_height = 0;

		base->GetScreenSize(base->mBaseEngine, &screen_width, &screen_height);

		mWidth = (float)(screen_width);
		mHeight = (float)(screen_height);
	}
	
	if (!gMenu.mMenuPosInit)
	{
		if (mWidth != 0 && mHeight != 0)
		{
			gMenu.mMenuPosX = (mWidth * 0.5f) - (BASE_WIDTH * 0.5f);
			gMenu.mMenuPosY = (mHeight * 0.5f) - (BASE_HEIGHT * 0.5f) + (BAR_HEIGHT * 0.5f);
			gMenu.mMenuPosInit = true;
		}
	}

	bool draw = !base->IsInGame(base->mBaseEngine);
	if (gVars.misc_textingame.value)
		draw = true;

	if (draw)
		DrawTextA(base->mTitle, mWidth * 0.005f, mHeight * 0.005f, 255, 255, 255, 255);

	gESP.Think();
	gMenu.Think();
}

void CDrawManager::DrawTextA(char* text, float x, float y, int r, int g, int b, int a, bool center_x, bool center_y)
{
	base->SetFont(base->mSurface, mFont);

	wchar_t result[128];
	SecureZeroMemory(result, sizeof(result));

	int num = base->MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0) - 1;
	base->MultiByteToWideChar(CP_UTF8, 0, text, -1, result, num);

	if (center_x || center_y)
	{
		int x2 = 0;
		int y2 = 0;

		base->GetTextSize(base->mSurface, gDraw.mFont, result, &x2, &y2);

		float xoff = 0.0f;
		float yoff = 0.0f;

		if (center_x)
			xoff = (float)(x2) * 0.5f;

		if (center_y)
			yoff = (float)(y2) * 0.5f;

		base->SetTextPos(base->mSurface, (int)(x - xoff), (int)(y - yoff));
	}
	else
	{
		base->SetTextPos(base->mSurface, (int)(x), (int)(y));
	}
	
	base->SetTextColor(base->mSurface, r, g, b, a);

	base->DrawText(base->mSurface, result, num, 0);
}

void CDrawManager::DrawRect(float x, float y, float w, float h, int r, int g, int b, int a)
{
	base->DrawSetColor(base->mSurface, r, g, b, a);
	base->DrawFilledRect(base->mSurface, (int)(x), (int)(y), (int)(x + w), (int)(y + h));
}

void CDrawManager::DrawOutlined(float x, float y, float w, float h, int r, int g, int b, int a)
{
	base->DrawSetColor(base->mSurface, r, g, b, a);
	base->DrawOutlinedRect(base->mSurface, (int)(x), (int)(y), (int)(x + w), (int)(y + h));
}

void CDrawManager::DrawLine(float x1, float y1, float x2, float y2, int r, int g, int b, int a)
{
	base->DrawSetColor(base->mSurface, r, g, b, a);
	base->DrawLine(base->mSurface, (int)(x1), (int)(y1), (int)(x2), (int)(y2));
}