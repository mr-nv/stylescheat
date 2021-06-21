class CRenderManager
{
public:
	CRenderManager()
	{
		SecureZeroMemory(this, sizeof(*this));
	}

	void Init();
	void Think();
	void AsusThink();
	void RenderPlayers(Entity* local);
	int ProcessDrawModel(Entity* entity);
	bool ShouldChamsRender(Entity* entity);

	bool mInit;
	Material* mTextureMaterial;
	Material* mFlatMaterial;
	Material* mWireMaterial;
	bool mRenderingChams;
	bool mAsusReload;
};

extern CRenderManager gRender;

#define DRAWMODEL_CONTINUE 0
#define DRAWMODEL_RETURN 1
#define DRAWMODEL_RENDER 2
#define DRAWMODEL_PLAYER 3