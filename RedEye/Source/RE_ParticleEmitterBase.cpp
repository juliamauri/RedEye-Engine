#include "RE_ParticleEmitterBase.h"

void RE_ParticleEmitterBase::LoadInMemory()
{
	if (RE_FS->Exists(GetLibraryPath()))
	{
		LibraryLoad();
	}
	else if (RE_FS->Exists(GetAssetPath()))
	{
		AssetLoad();
		LibrarySave();
	}
	else RE_LOG_ERROR("SkyBox %s not found in project", GetName());
}

void RE_ParticleEmitterBase::UnloadMemory()
{
	cDiffuse = cSpecular = cAmbient = cEmissive = cTransparent = math::float3::zero;
	backFaceCulling = true;
	blendMode = false;
	opacity = shininess = shininessStrenght = refraccti = 1.0f;
	ResourceContainer::inMemory = false;
}

void RE_ParticleEmitterBase::Import(bool keepInMemory)
{
	AssetLoad(true);
	LibrarySave();
	if (!keepInMemory) UnloadMemory();
}

void RE_ParticleEmitterBase::SomeResourceChanged(const char* resMD5)
{
	if (shaderMD5 == resMD5)
	{
		if (!isInMemory())
		{
			LoadInMemory();
			ResourceContainer::inMemory = false;
		}
		eastl::vector<RE_Shader_Cvar> beforeCustomUniforms = fromShaderCustomUniforms;
		GetAndProcessUniformsFromShader();

		for (uint b = 0; b < beforeCustomUniforms.size(); b++)
		{
			for (uint i = 0; i < fromShaderCustomUniforms.size(); i++)
			{
				if (beforeCustomUniforms[b].name == fromShaderCustomUniforms[i].name && beforeCustomUniforms[b].GetType() == fromShaderCustomUniforms[i].GetType())
				{
					fromShaderCustomUniforms[i].SetValue(beforeCustomUniforms[b]);
					break;
				}
			}
		}

		Save();
	}
}

void RE_ParticleEmitterBase::Save()
{
	JsonSerialize();
	BinarySerialize();
	SaveMeta();
}

void RE_ParticleEmitterBase::ProcessMD5()
{
	//md5 from emissor md5 + render md5
}


void RE_ParticleEmitterBase::Draw()
{
	if (!isInternal() && applySave && ImGui::Button("Save Changes"))
	{
		Save();
		RE_RENDER->PushThumnailRend(GetMD5(), true);
		applySave = false;
	}

	//Button workspace particle edittor

	//Drag and drop resources emission and render


	//ImGui::Image(reinterpret_cast<void*>(RE_EDITOR->thumbnails->At(GetMD5())), { 256, 256 }, { 0,1 }, { 1, 0 });
}

void RE_ParticleEmitterBase::SaveResourceMeta(RE_Json* metaNode)
{
	metaNode->PushString("shaderMeta", (shaderMD5) ? RE_RES->At(shaderMD5)->GetMetaPath() : "NOMETAPATH");

	RE_Json* diffuseNode = metaNode->PushJObject("DiffuseTextures");
	PushTexturesJson(diffuseNode, &tDiffuse);
	DEL(diffuseNode);
	RE_Json* specularNode = metaNode->PushJObject("SpecularTextures");
	PushTexturesJson(specularNode, &tSpecular);
	DEL(specularNode);
}

void RE_ParticleEmitterBase::LoadResourceMeta(RE_Json* metaNode)
{
	eastl::string shaderMeta = metaNode->PullString("shaderMeta", "NOMETAPATH");
	if (shaderMeta.compare("NOMETAPATH") != 0) shaderMD5 = RE_RES->FindMD5ByMETAPath(shaderMeta.c_str(), Resource_Type::R_SHADER);

	RE_Json* diffuseNode = metaNode->PullJObject("DiffuseTextures");
	PullTexturesJson(diffuseNode, &tDiffuse);
	DEL(diffuseNode);
	RE_Json* specularNode = metaNode->PullJObject("SpecularTextures");
	PullTexturesJson(specularNode, &tSpecular);
	DEL(specularNode);
}

void RE_ParticleEmitterBase::JsonDeserialize(bool generateLibraryPath)
{
}




void RE_ParticleEmitterBase::AssetLoad(bool generateLibraryPath)
{
	Config jsonLoad(GetAssetPath(), RE_FS->GetZipPath());

	if (jsonLoad.Load())
	{
		RE_Json* prefabNode = jsonLoad.GetRootNode("prefab");
		loaded = RE_ECS_Importer::JsonDeserialize(prefabNode);
		DEL(prefabNode);

		if (generateLibraryPath)
		{
			eastl::string md5 = jsonLoad.GetMd5();
			SetMD5(md5.c_str());
			eastl::string libraryPath("Library/Prefabs/");
			libraryPath += md5;
			SetLibraryPath(libraryPath.c_str());
		}
	}

	ResourceContainer::inMemory = true;
}

void RE_ParticleEmitterBase::LibraryLoad()
{
	RE_FileBuffer binaryLoad(GetLibraryPath());
	if (binaryLoad.Load())
	{
		char* cursor = binaryLoad.GetBuffer();
		loaded = RE_ECS_Importer::BinaryDeserialize(cursor);
	}
	ResourceContainer::inMemory = true;
}

void RE_ParticleEmitterBase::LibrarySave()
{
	RE_FileBuffer assetFile(GetAssetPath());
	RE_FileBuffer libraryFile(GetLibraryPath(), RE_FS->GetZipPath());
	if (assetFile.Load()) RE_TextureImporter::SaveOwnFormat(assetFile.GetBuffer(), assetFile.GetSize(), texType, &libraryFile);
}

void RE_ParticleEmitterBase::BinaryDeserialize()
{
}

void RE_ParticleEmitterBase::BinarySerialize()
{
}

unsigned int RE_ParticleEmitterBase::GetBinarySize() const
{
	return 0;
}

