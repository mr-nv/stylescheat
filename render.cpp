#include "dll.h"

CRenderManager gRender;

void CRenderManager::Init()
{
	mTextureMaterial = base->FindMaterial(base->mMaterialSystem, "debug/debugambientcube", "Model Textures", false, nullptr);
	if (!mTextureMaterial)
		return;

	mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
	mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
	mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
	mTextureMaterial->IncrementReferenceCount();

	mFlatMaterial = base->FindMaterial(base->mMaterialSystem, "debug/debugdrawflat", "Model Textures", false, nullptr);
	if (!mFlatMaterial)
		return;

	mFlatMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
	mFlatMaterial->IncrementReferenceCount();

	mWireMaterial = base->FindMaterial(base->mMaterialSystem, "debug/debugwireframe", "Model Textures", false, nullptr);
	if (!mWireMaterial)
		return;

	mWireMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
	mWireMaterial->IncrementReferenceCount();

	mInit = true;
	mRenderingChams = false;
}

void CRenderManager::Think()
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return;

	if (!mInit)
		Init();

	// do chams
	RenderPlayers(local);

	// do asus walls
	//AsusThink();
}

void CRenderManager::AsusThink()
{
	float blend = 0.99f;

	if (!gVars.render_asus.value)
		blend = 0.0f;
	else if (gVars.render_asus_blend.value)
		blend = (float)(gVars.render_asus_blend.value) * 0.01f;

	bool changed = false;

	static float last_blend = 0.0f;

	if (mAsusReload || blend != last_blend)
		changed = true;

	last_blend = blend;

	static bool last_additive = false;
	bool additive = (bool)(gVars.render_asus_additive.value);

	if (additive != last_additive)
		changed = true;

	last_additive = additive;
	
	if (changed)
	{
		int test = 0;
		short handle = base->FirstMaterial(base->mMaterialSystem);

		while (handle != -1)
		{
			if (test > 9999)
			{
				base->Warning("CRenderManager::AsusThink error: overflow\n");
				break;
			}

			Material* mat = base->GetMaterial(base->mMaterialSystem, handle);
			if (!mat)
			{
				handle = base->NextMaterial(base->mMaterialSystem, handle); 
				continue;
			}

			char* str = mat->GetTextureGroup();
			CRC32 crc = CRC32_Get(str, string_len(str));

			if (crc == 0xb2194cb3 || crc == 0x68fa6ba7) // "World textures" and "StaticProp textures"
			{
				if (string_find(mat->GetName(), "sky"))
				{
					if (gVars.render_asus.value)
					{
						mat->SetAlpha(0.01f);
						mat->SetMaterialVarFlag(MATERIAL_VAR_ADDITIVE, true);
					}
					else
					{
						mat->SetAlpha(1.0f);
						mat->SetMaterialVarFlag(MATERIAL_VAR_ADDITIVE, false);
					}
				}
				else
				{
					char* shader = mat->GetShaderName();
					CRC32 shader_crc = CRC32_Get(shader, string_len(shader));

					if (shader_crc == 0x1d78b58b) // "LightmappedGeneric"
					{
						if (gVars.render_asus.value)
						{
							mat->SetAlpha(blend);

							if (additive)
							{
								mat->SetMaterialVarFlag(MATERIAL_VAR_ADDITIVE, true);
								mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
							}
							else
							{
								mat->SetMaterialVarFlag(MATERIAL_VAR_ADDITIVE, false);
								mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
							}
						}
						else
						{
							mat->SetAlpha(1.0f);

							mat->SetMaterialVarFlag(MATERIAL_VAR_ADDITIVE, false);
							mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						}
					}
				}
			}
			else if (crc == 0xe0d2525) // "SkyBox textures"
			{
				if (gVars.render_asus.value)
					mat->SetColor(0.0f, 0.0f, 0.0f);
				else
					mat->SetColor(1.0f, 1.0f, 1.0f);
			}

			handle = base->NextMaterial(base->mMaterialSystem, handle);
		}
	}
}

void CRenderManager::RenderPlayers(Entity* local)
{
	if (!gVars.render_chams.value)
	{
		mRenderingChams = false;
		return;
	}

	mRenderingChams = true;

	if (gVars.render_chams.value == 1)
	{
		mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, false);
		mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, false);
		mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
		mFlatMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
	}
	else
	{
		mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
		mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
		mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
		mFlatMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
	}

	if (gVars.render_chams_glass.value)
		base->MaterialOverride(base->mModelRender, mFlatMaterial, nullptr, nullptr);
	else
		base->MaterialOverride(base->mModelRender, mTextureMaterial, nullptr, nullptr);

	base->SetBlend(base->mView, gVars.render_chams_glass.value ? 0.65f : 1.0f);

	BYTE* t_vis = (BYTE*)(&(gVars.render_chams_tvisible.value));
	BYTE* t_invis = (BYTE*)(&(gVars.render_chams_tinvisible.value));
	BYTE* ct_vis = (BYTE*)(&(gVars.render_chams_ctvisible.value));
	BYTE* ct_invis = (BYTE*)(&(gVars.render_chams_ctinvisible.value));

	float color[4];

	if (gVars.render_chams.value == 3)
	{
		for (int i = 1; i <= base->mGlobalVars->clients; ++i)
		{
			Entity* player = base->GetClientEntity(base->mEntList, i);
			if (!IsEntityValid(player))
				continue;

			if (!ShouldChamsRender(player))
				continue;

			if (*player->m_iTeamNum() == 2)
			{
				color[0] = (float)(t_invis[0]) / 255.0f;
				color[1] = (float)(t_invis[1]) / 255.0f;
				color[2] = (float)(t_invis[2]) / 255.0f;
				color[3] = 1.0f;
			}
			else
			{
				color[0] = (float)(ct_invis[0]) / 255.0f;
				color[1] = (float)(ct_invis[1]) / 255.0f;
				color[2] = (float)(ct_invis[2]) / 255.0f;
				color[3] = 1.0f;
			}

			base->SetColorModulation(base->mView, color);

			player->DrawModel(1, nullptr);
		}

		mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, false);
		mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, false);
		mTextureMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
		mFlatMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
	}

	for (int i = 1; i <= base->mGlobalVars->clients; ++i)
	{
		Entity* player = base->GetClientEntity(base->mEntList, i);
		if (!IsEntityValid(player))
			continue;

		if (!ShouldChamsRender(player))
			continue;

		if (*player->m_iTeamNum() == 2)
		{
			color[0] = (float)(t_vis[0]) / 255.0f;
			color[1] = (float)(t_vis[1]) / 255.0f;
			color[2] = (float)(t_vis[2]) / 255.0f;
			color[3] = 1.0f;
		}
		else
		{
			color[0] = (float)(ct_vis[0]) / 255.0f;
			color[1] = (float)(ct_vis[1]) / 255.0f;
			color[2] = (float)(ct_vis[2]) / 255.0f;
			color[3] = 1.0f;
		}

		base->SetColorModulation(base->mView, color);

		player->DrawModel(1, nullptr);
	}

	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	color[3] = 1.0f;

	base->SetColorModulation(base->mView, color);

	base->MaterialOverride(base->mModelRender, nullptr, nullptr, nullptr);

	base->SetBlend(base->mView, 1.0f);

	mRenderingChams = false;
}

int CRenderManager::ProcessDrawModel(Entity* entity)
{
	//if (esp->m_screen)
	//	return DRAWMODEL_CONTINUE;

	if (!IsEntityValid(entity))
		return DRAWMODEL_CONTINUE;

	void* model = entity->GetModel();

	if (!model)
		return DRAWMODEL_CONTINUE;

	char* model_name = base->GetModelName(base->mModelInfo, model);

	if (model_name[0] == '?')
		return DRAWMODEL_CONTINUE;

	if (gVars.render_noteam.value)
	{
		Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));

		if (IsEntityValid(local))
		{
			char* class_name = entity->GetClientClass()->name;

			if (CRC32_Get(class_name, string_len(class_name)) == 0x9a4ec3b8 && entity != local && *entity->m_iTeamNum() == *local->m_iTeamNum())
				return DRAWMODEL_RETURN;
		}
	}

	if (gVars.render_noweapon.value)
	{
		char* class_name = entity->GetClientClass()->name;

		if (CRC32_Get(class_name, string_len(class_name)) == 0xa73f279e) // CBaseWeaponWorldModel
			return DRAWMODEL_RETURN;
	}

	if (gVars.render_chams.value)
	{	
		if (ShouldChamsRender(entity))
		{
			if (mRenderingChams)
				return DRAWMODEL_PLAYER;
			else
				return DRAWMODEL_RETURN;
		}
	}

	if (gVars.render_wirehands.value)
	{
		if (CRC32_Get(model_name, 29) == 0x3752a302) // models/weapons/v_models/arms/
		{
			mWireMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

			base->MaterialOverride(base->mModelRender, mWireMaterial, nullptr, nullptr);

			BYTE* clr = (BYTE*)(&(gVars.render_hands_color.value));

			float color[4];
			color[0] = (float)(clr[0]) / 255.0f;
			color[1] = (float)(clr[1]) / 255.0f;
			color[2] = (float)(clr[2]) / 255.0f;
			color[3] = 1.0f;

			base->StudioSetColorModulation(base->mStudioRender, color);

			return DRAWMODEL_RENDER;
		}
	}

	return DRAWMODEL_CONTINUE;
}

bool CRenderManager::ShouldChamsRender(Entity* entity)
{
	Entity* local = base->GetClientEntity(base->mEntList, base->GetLocalPlayer(base->mBaseEngine));
	if (!IsEntityValid(local))
		return false;

	ClientClass* cls = entity->GetClientClass();
	if (!cls)
		return false;

	char* class_name = cls->name;

	if (CRC32_Get(class_name, string_len(class_name)) != 0x9a4ec3b8)
		return false;

	if (!(gVars.render_chams_ents.value & (1<<0)))
	{
		if (entity == local)
			return false;
	}

	if (!(gVars.render_chams_ents.value & (1<<1)) && entity != local)
	{
		if (*entity->m_iTeamNum() == *local->m_iTeamNum())
			return false;
	}

	if (!(gVars.render_chams_ents.value & (1<<2)))
	{
		if (*entity->m_iTeamNum() != *local->m_iTeamNum())
			return false;
	}

	if (*entity->m_lifeState() != 0)
		return false;

	if (entity->IsDormant())
		return false;

	if (*entity->m_bGunGameImmunity())
		return false;

	return true;
}