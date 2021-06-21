class CDrawManager
{
public:
	CDrawManager()
	{
		SecureZeroMemory(this, sizeof(*this));
	}

	void Init();
	void Think();
	void DrawTextA(char* text, float x, float y, int r, int g, int b, int a, bool center_x = false, bool center_y = false);
	void DrawRect(float x, float y, float w, float h, int r, int g, int b, int a);
	void DrawOutlined(float x, float y, float w, float h, int r, int g, int b, int a);
	void DrawLine(float x1, float y1, float x2, float y2, int r, int g, int b, int a);

	bool mInit;
	DWORD mFont;
	DWORD mFontBackup;

	float mWidth;
	float mHeight;
};

extern CDrawManager gDraw;