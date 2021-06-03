#include "RE_Material.h"

#include "Globals.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_ShaderImporter.h"
#include "RE_GLCache.h"
#include "RE_ThumbnailManager.h"
#include "RE_Shader.h"
#include "RE_Texture.h"

#include "ImGui/imgui.h"
#include "Glew/include/glew.h"
#include <EAStdC/EAString.h>

RE_Material::RE_Material() {}
RE_Material::RE_Material(const char* metapath) : ResourceContainer(metapath) {}
RE_Material::~RE_Material() {}

void RE_Material::LoadInMemory()
{
	if (RE_FS->Exists(GetLibraryPath()))
	{
		BinaryDeserialize();
	}
	else if (RE_FS->Exists(GetAssetPath()))
	{
		JsonDeserialize();
		BinarySerialize();
	}
	else if (isInternal())
	{
		ResourceContainer::inMemory = true;
	}
	else
	{
		RE_LOG_ERROR("Material %s not found on project", GetName());
	}
}

void RE_Material::UnloadMemory()
{
	cDiffuse = cSpecular = cAmbient = cEmissive = cTransparent = math::float3::zero;
	backFaceCulling = true;
	blendMode = false;
	opacity = shininess = shininessStrenght = refraccti = 1.0f;
	ResourceContainer::inMemory = false;
}

void RE_Material::Import(bool keepInMemory)
{
	JsonDeserialize(true);
	BinarySerialize();
	if (!keepInMemory) UnloadMemory();
}

void RE_Material::ProcessMD5()
{
	JsonSerialize(true);
}

void RE_Material::Save()
{
	if(!shaderMD5)
	{
		//Default Shader
		usingOnMat[CDIFFUSE] = 1;
		usingOnMat[TDIFFUSE] = 1;
		usingOnMat[OPACITY] = 1;
		usingOnMat[SHININESS] = 1;
	}
	JsonSerialize();
	BinarySerialize();
	SaveMeta();
	applySave = false;
}

void RE_Material::Draw()
{
	if (!isInternal() && applySave && ImGui::Button("Save Changes"))
	{
		Save();
		RE_RENDER->PushThumnailRend(GetMD5(), true);
		applySave = false;
	}

	if (RE_RES->TotalReferenceCount(GetMD5()) == 0)
	{
		ImGui::Text("Material is unloaded, load for watch and modify values.");
		if (ImGui::Button("Load"))
		{
			LoadInMemory();
			applySave = false;
			ResourceContainer::inMemory = false;
		}
		ImGui::Separator();
	}

	DrawMaterialEdit();
	ImGui::Image(reinterpret_cast<void*>(RE_EDITOR->thumbnails->At(GetMD5())), { 256, 256 }, { 0,1 }, { 1, 0 });
}

void RE_Material::SaveResourceMeta(RE_Json* metaNode)
{
	metaNode->PushString("shaderMeta", (shaderMD5) ? RE_RES->At(shaderMD5)->GetMetaPath() : "NOMETAPATH");

	RE_Json* diffuseNode = metaNode->PushJObject("DiffuseTextures");
	PushTexturesJson(diffuseNode, &tDiffuse);
	DEL(diffuseNode);
	RE_Json* specularNode = metaNode->PushJObject("SpecularTextures");
	PushTexturesJson(specularNode, &tSpecular);
	DEL(specularNode);
	RE_Json* ambientNode = metaNode->PushJObject("AmbientTextures");
	PushTexturesJson(ambientNode, &tAmbient);
	DEL(ambientNode);
	RE_Json* emissiveNode = metaNode->PushJObject("EmissiveTextures");
	PushTexturesJson(emissiveNode, &tEmissive);
	DEL(emissiveNode);
	RE_Json* opacityNode = metaNode->PushJObject("OpacityTextures");
	PushTexturesJson(opacityNode, &tOpacity);
	DEL(opacityNode);
	RE_Json* shininessNode = metaNode->PushJObject("ShininessTextures");
	PushTexturesJson(shininessNode, &tShininess);
	DEL(shininessNode);
	RE_Json* heightNode = metaNode->PushJObject("HeightTextures");
	PushTexturesJson(heightNode, &tHeight);
	DEL(heightNode);
	RE_Json* normalsNode = metaNode->PushJObject("NormalsTextures");
	PushTexturesJson(normalsNode, &tNormals);
	DEL(normalsNode);
	RE_Json* reflectionNode = metaNode->PushJObject("ReflectionTextures");
	PushTexturesJson(reflectionNode, &tReflection);
	DEL(reflectionNode);
	RE_Json* unknownNode = metaNode->PushJObject("UnknownTextures");
	PushTexturesJson(unknownNode, &tUnknown);
	DEL(unknownNode);
}

void RE_Material::LoadResourceMeta(RE_Json* metaNode)
{
	eastl::string shaderMeta = metaNode->PullString("shaderMeta", "NOMETAPATH");
	if (shaderMeta.compare("NOMETAPATH") != 0) shaderMD5 = RE_RES->FindMD5ByMETAPath(shaderMeta.c_str(), Resource_Type::R_SHADER);

	RE_Json* diffuseNode = metaNode->PullJObject("DiffuseTextures");
	PullTexturesJson(diffuseNode, &tDiffuse);
	DEL(diffuseNode);
	RE_Json* specularNode = metaNode->PullJObject("SpecularTextures");
	PullTexturesJson(specularNode, &tSpecular);
	DEL(specularNode);
	RE_Json* ambientNode = metaNode->PullJObject("AmbientTextures");
	PullTexturesJson(ambientNode, &tAmbient);
	DEL(ambientNode);
	RE_Json* emissiveNode = metaNode->PullJObject("EmissiveTextures");
	PullTexturesJson(emissiveNode, &tEmissive);
	DEL(emissiveNode);
	RE_Json* opacityNode = metaNode->PullJObject("OpacityTextures");
	PullTexturesJson(opacityNode, &tOpacity);
	DEL(opacityNode);
	RE_Json* shininessNode = metaNode->PullJObject("ShininessTextures");
	PullTexturesJson(shininessNode, &tShininess);
	DEL(shininessNode);
	RE_Json* heightNode = metaNode->PullJObject("HeightTextures");
	PullTexturesJson(heightNode, &tHeight);
	DEL(heightNode);
	RE_Json* normalsNode = metaNode->PullJObject("NormalsTextures");
	PullTexturesJson(normalsNode, &tNormals);
	DEL(normalsNode);
	RE_Json* reflectionNode = metaNode->PullJObject("ReflectionTextures");
	PullTexturesJson(reflectionNode, &tReflection);
	DEL(reflectionNode);
	RE_Json* unknownNode = metaNode->PullJObject("UnknownTextures");
	PullTexturesJson(unknownNode, &tUnknown);
	DEL(unknownNode);
}

void RE_Material::DrawMaterialEdit()
{
	RE_Shader* matShader = dynamic_cast<RE_Shader*>(RE_RES->At(shaderMD5 ? shaderMD5 : RE_RES->internalResources->GetDefaultShader()));

	ImGui::Text("Shader selected: %s", matShader->GetMD5());
	
	if (!shaderMD5) ImGui::Text("This shader is using the default shader.");

	if (ImGui::Button(matShader->GetName()))
	{
		bool popAll = false;
		const char* selected = RE_RES->GetSelected();
		if (selected)
		{
			Resource_Type t = RE_RES->At(selected)->GetType();
			popAll = (t == Resource_Type::R_SHADER || t == Resource_Type::R_TEXTURE);
		}
		RE_RES->PushSelected(matShader->GetMD5(), popAll);
	}

	if (shaderMD5)
	{
		ImGui::SameLine();
		if (ImGui::Button("Back to Default Shader"))
		{
			shaderMD5 = nullptr;
			applySave = true;
			GetAndProcessUniformsFromShader();
		}
	}

	if (ImGui::BeginMenu("Change shader"))
	{
		eastl::vector<ResourceContainer*> shaders = RE_RES->GetResourcesByType(Resource_Type::R_SHADER);
		bool none = true;
		for (auto  shader :  shaders)
		{
			if (shader->isInternal()) continue;
			none = false;
			if (ImGui::MenuItem(shader->GetName()))
			{
				if (ResourceContainer::inMemory && shaderMD5) RE_RES->UnUse(shaderMD5);
				shaderMD5 = shader->GetMD5();
				if (ResourceContainer::inMemory && shaderMD5) RE_RES->Use(shaderMD5);
				GetAndProcessUniformsFromShader();
				applySave = true;
			}
		}
		if (none) ImGui::Text("No custom shaders on assets");
		ImGui::EndMenu();
	}

	if (!fromShaderCustomUniforms.empty())
	{
		ImGui::Separator();
		static bool displayShaderValues = false;
		ImGui::Checkbox("Display all custom shader values", &displayShaderValues);
		ImGui::Separator();

		if (!displayShaderValues)
		{
			if (ImGui::BeginMenu("Custom values from shader"))
			{
				for (uint i = 0; i < fromShaderCustomUniforms.size(); i++)
				{
					if (fromShaderCustomUniforms[i].DrawPropieties(ResourceContainer::inMemory)) applySave = true;
					ImGui::Separator();
				}
				ImGui::EndMenu();
			}
		}
		else
		{
			for (uint i = 0; i < fromShaderCustomUniforms.size(); i++)
			{
				if (fromShaderCustomUniforms[i].DrawPropieties(ResourceContainer::inMemory)) applySave = true;
				ImGui::Separator();
			}
		}
	}

	if (usingOnMat[CDIFFUSE] || usingOnMat[TDIFFUSE])
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Diffuse values"))
		{
			if (usingOnMat[CDIFFUSE] && ImGui::ColorEdit3("Diffuse Color", &cDiffuse[0])) applySave = true;
			if (usingOnMat[TDIFFUSE]) DrawTextures("Diffuse", &tDiffuse);

			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				tDiffuse.push_back(*static_cast<const char**>(dropped->Data));
				if (ResourceContainer::inMemory) RE_RES->Use(tDiffuse.back());
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}

	if (usingOnMat[CSPECULAR] || usingOnMat[TSPECULAR])
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Specular values"))
		{
			if (usingOnMat[CSPECULAR] && ImGui::ColorEdit3("Specular Color", &cSpecular[0])) applySave = true;
			if (usingOnMat[TSPECULAR]) DrawTextures("Specular", &tSpecular);
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				tSpecular.push_back(*static_cast<const char**>(dropped->Data));
				if (ResourceContainer::inMemory) RE_RES->Use(tSpecular.back());
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}

	if (usingOnMat[CAMBIENT] || usingOnMat[TAMBIENT])
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Ambient values"))
		{
			if (usingOnMat[CAMBIENT] && ImGui::ColorEdit3("Ambient Color", &cAmbient[0])) applySave = true;
			if (usingOnMat[TAMBIENT]) DrawTextures("Ambient", &tAmbient);
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				tAmbient.push_back(*static_cast<const char**>(dropped->Data));
				if (ResourceContainer::inMemory) RE_RES->Use(tAmbient.back());
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}

	if (usingOnMat[CEMISSIVE] || usingOnMat[TEMISSIVE])
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Emissive values"))
		{
			if (usingOnMat[CEMISSIVE] && ImGui::ColorEdit3("Emissive Color", &cEmissive[0])) applySave = true;
			if (usingOnMat[TEMISSIVE]) DrawTextures("Emissive", &tEmissive);
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				tAmbient.push_back(*static_cast<const char**>(dropped->Data));
				if (ResourceContainer::inMemory) RE_RES->Use(tEmissive.back());
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}

	if (usingOnMat[OPACITY] || usingOnMat[TOPACITY])
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Opacity values"))
		{
			if (usingOnMat[OPACITY] && ImGui::InputFloat("Opacity", &opacity)) applySave = true;
			if (usingOnMat[TOPACITY]) DrawTextures("Opacity", &tOpacity);
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				tOpacity.push_back(*static_cast<const char**>(dropped->Data));
				if (ResourceContainer::inMemory) RE_RES->Use(tOpacity.back());
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}

	if (usingOnMat[SHININESS] || usingOnMat[SHININESSSTRENGHT] || usingOnMat[TSHININESS])
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Shininess values"))
		{
			if (usingOnMat[SHININESS] && ImGui::DragFloat("Shininess", &shininess, 0.1f, 1.0f, 32.0f)) applySave = true;
			if (usingOnMat[SHININESSSTRENGHT] && ImGui::InputFloat("Shininess strenght", &shininessStrenght)) applySave = true;
			if (usingOnMat[TSHININESS]) DrawTextures("Shininess", &tShininess);
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				tShininess.push_back(*static_cast<const char**>(dropped->Data));
				if (ResourceContainer::inMemory) RE_RES->Use(tShininess.back());
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}

	if (usingOnMat[THEIGHT])
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Height values"))
		{
			DrawTextures("Height", &tHeight);
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				tHeight.push_back(*static_cast<const char**>(dropped->Data));
				if (ResourceContainer::inMemory) RE_RES->Use(tHeight.back());
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}
	if (usingOnMat[TNORMALS])
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Normals values"))
		{
			DrawTextures("Normals", &tNormals);
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				tNormals.push_back(*static_cast<const char**>(dropped->Data));
				if (ResourceContainer::inMemory) RE_RES->Use(tNormals.back());
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}
	if (usingOnMat[TREFLECTION])
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Reflection values"))
		{
			DrawTextures("Reflection", &tReflection);
			ImGui::EndMenu();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				tReflection.push_back(*static_cast<const char**>(dropped->Data));
				if (ResourceContainer::inMemory) RE_RES->Use(tReflection.back());
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Others values"))
	{
		if (usingOnMat[CTRANSPARENT])
		{
			if (ImGui::ColorEdit3("Transparent", &cTransparent[0])) applySave = true;
			ImGui::Separator();
		}

		if (ImGui::Checkbox("Back face culling", &backFaceCulling)) applySave = true;
		ImGui::Separator();

		if (ImGui::Checkbox("Blend mode", &blendMode)) applySave = true;
		ImGui::Separator();

		if (usingOnMat[REFRACCTI])
		{
			if (ImGui::InputFloat("Refraccti", &refraccti)) applySave = true;
			ImGui::Separator();
		}

		DrawTextures("Unknown", &tUnknown);
		ImGui::EndMenu();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
		{
			tUnknown.push_back(*static_cast<const char**>(dropped->Data));
			if (ResourceContainer::inMemory) RE_RES->Use(tUnknown.back());
			applySave = true;
		}
		ImGui::EndDragDropTarget();
	}
}

void RE_Material::DrawMaterialParticleEdit(bool tex)
{
	if (!isInternal() && applySave && ImGui::Button("Save Changes"))
	{
		Save();
		RE_RENDER->PushThumnailRend(GetMD5(), true);
		applySave = false;
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Diffuse values"))
	{
		if (ImGui::ColorEdit3("Diffuse Color", &cDiffuse[0])) applySave = true;
		if(tex) DrawTextures("Diffuse", &tDiffuse);

		ImGui::EndMenu();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
		{
			tDiffuse.push_back(*static_cast<const char**>(dropped->Data));
			if (ResourceContainer::inMemory) RE_RES->Use(tDiffuse.back());
			applySave = true;
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Specular values"))
	{
		if (ImGui::ColorEdit3("Specular Color", &cSpecular[0])) applySave = true;
		if (tex) DrawTextures("Specular", &tSpecular);
		ImGui::EndMenu();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
		{
			tSpecular.push_back(*static_cast<const char**>(dropped->Data));
			if (ResourceContainer::inMemory) RE_RES->Use(tSpecular.back());
			applySave = true;
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::Separator();

	if (ImGui::BeginMenu("Shininess values"))
	{
		if (ImGui::DragFloat("Shininess", &shininess, 0.1f, 1.0f, 32.0f)) applySave = true;
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Opacity values"))
	{
		if (ImGui::InputFloat("Opacity", &opacity)) applySave = true;
		ImGui::EndMenu();
	}
	ImGui::Separator();

}

void RE_Material::SomeResourceChanged(const char* resMD5)
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

bool RE_Material::ExitsOnShader(const char* shader) { return (shaderMD5 == shader); }

bool RE_Material::ExistsOnTexture(const char* texture)
{
	bool ret = ExistsOnTexture(texture, &tDiffuse);
	if(!ret) ret = ExistsOnTexture(texture, &tSpecular);
	if(!ret) ret = ExistsOnTexture(texture, &tAmbient);
	if(!ret) ret = ExistsOnTexture(texture, &tEmissive);
	if(!ret) ret = ExistsOnTexture(texture, &tOpacity);
	if(!ret) ret = ExistsOnTexture(texture, &tShininess);
	if(!ret) ret = ExistsOnTexture(texture, &tHeight);
	if(!ret) ret = ExistsOnTexture(texture, &tNormals);
	if(!ret) ret = ExistsOnTexture(texture, &tReflection);
	if(!ret) ret = ExistsOnTexture(texture, &tUnknown);
	return ret;
}

void RE_Material::SetShader(const char* sMD5)
{
	shaderMD5 = sMD5;
	GetAndProcessUniformsFromShader();
	Save();
}

void RE_Material::DeleteShader()
{
	shaderMD5 = nullptr;
	GetAndProcessUniformsFromShader();
	Save();
}

void RE_Material::DeleteTexture(const char* texMD5)
{
	DeleteTexture(texMD5, &tDiffuse);
	DeleteTexture(texMD5, &tSpecular);
	DeleteTexture(texMD5, &tAmbient);
	DeleteTexture(texMD5, &tEmissive);
	DeleteTexture(texMD5, &tOpacity);
	DeleteTexture(texMD5, &tShininess);
	DeleteTexture(texMD5, &tHeight);
	DeleteTexture(texMD5, &tNormals);
	DeleteTexture(texMD5, &tReflection);
	DeleteTexture(texMD5, &tUnknown);
	SaveMeta();
}

void RE_Material::DrawTextures(const char* texturesName, eastl::vector<const char*>* textures)
{
	ImGui::Text(eastl::string(texturesName + eastl::string(" textures:")).c_str());
	if (!textures->empty())
	{
		ImGui::Separator();
		eastl::vector<const char*>::iterator toDelete = textures->end();
		uint count = 1;
		for (eastl::vector<const char*>::iterator md5 = textures->begin(); md5 != textures->end(); ++md5)
		{
			ResourceContainer* resource = RE_RES->At(*md5);
			if (ImGui::Button(eastl::string("Texture #" + eastl::to_string(count) + " " + eastl::string(resource->GetName())).c_str()))
			{

				bool popAll = false;
				const char* selected = RE_RES->GetSelected();
				if (selected)
				{
					Resource_Type t = RE_RES->At(selected)->GetType();
					popAll = (t == Resource_Type::R_SHADER || t == Resource_Type::R_TEXTURE);
				}
				RE_RES->PushSelected(*md5, popAll);
			}

			ImGui::SameLine();
			eastl::string deletetexture("Delete #");
			deletetexture += eastl::to_string(count);
			if (ImGui::Button(deletetexture.c_str()))
			{
				if (ResourceContainer::inMemory) RE_RES->UnUse(*md5);
				toDelete = md5;
				applySave = true;
			}

			eastl::string changetexture("Change #");
			changetexture += eastl::to_string(count++);
			if (ImGui::BeginMenu(changetexture.c_str()))
			{
				eastl::vector<ResourceContainer*> allTex = RE_RES->GetResourcesByType(Resource_Type::R_TEXTURE);
				for (auto textRes : allTex)
				{
					if (ImGui::MenuItem(textRes->GetName()))
					{
						if (ResourceContainer::inMemory) RE_RES->UnUse(*md5);
						*md5 = textRes->GetMD5();
						if (ResourceContainer::inMemory) RE_RES->Use(*md5);
						applySave = true;
					}
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
		}

		if (toDelete != textures->end()) textures->erase(toDelete);
	}
	else
	{
		ImGui::Text("Empty textures");
	}

	eastl::string addtexture = "Add #";
	if (ImGui::BeginMenu(((addtexture += texturesName) += " texture").c_str()))
	{
		eastl::vector<ResourceContainer*> allTex = RE_RES->GetResourcesByType(Resource_Type::R_TEXTURE);
		for (auto textRes : allTex)
		{
			if (ImGui::MenuItem(textRes->GetName()))
			{
				textures->push_back(textRes->GetMD5());
				if (ResourceContainer::inMemory) RE_RES->Use(textures->back());
				applySave = true;
			}
		}
		ImGui::EndMenu();
	}
}

void RE_Material::JsonDeserialize(bool generateLibraryPath)
{
	Config material(GetAssetPath(), RE_FS->GetZipPath());
	if (material.Load()) {
		RE_Json* nodeMat = material.GetRootNode("Material");

		shadingType = (RE_ShadingMode)nodeMat->PullInt("ShaderType", RE_ShadingMode::S_FLAT);

		unsigned int* uOM = nodeMat->PullUInt("usingOnMat", 18, 0);
		memcpy(usingOnMat, uOM, sizeof(uint) * 18);
		DEL_A(uOM);

		cDiffuse = nodeMat->PullFloatVector("DiffuseColor", math::vec::zero);
		cSpecular = nodeMat->PullFloatVector("SpecularColor", math::vec::zero);
		cAmbient = nodeMat->PullFloatVector("AmbientColor", math::vec::zero);
		cEmissive = nodeMat->PullFloatVector("EmissiveColor", math::vec::zero);
		cTransparent = nodeMat->PullFloatVector("TransparentColor", math::vec::zero);
		backFaceCulling = nodeMat->PullBool("BackFaceCulling", true);
		blendMode = nodeMat->PullBool("BlendMode", false);
		opacity = nodeMat->PullFloat("Opacity", 1.0f);
		shininess = nodeMat->PullFloat("Shininess", 0.0f);
		shininessStrenght = nodeMat->PullFloat("ShininessStrenght", 1.0f);
		refraccti = nodeMat->PullFloat("Refraccti", 1.0f);

		fromShaderCustomUniforms.clear();
		RE_Json* nuniforms = nodeMat->PullJObject("uniforms");
		uint size = nuniforms->PullUInt("size", 0);
		if (size)
		{
			bool* b = nullptr;
			int* intPtr = nullptr;
			eastl::string id;
			for (uint i = 0; i < size; i++)
			{
				RE_Shader_Cvar sVar;
				id = "name" + eastl::to_string(i);
				sVar.name = nuniforms->PullString(id.c_str(), "");

				id = "type" + eastl::to_string(i);
				RE_Cvar::VAR_TYPE vT = (RE_Cvar::VAR_TYPE)nuniforms->PullUInt(id.c_str(), RE_Shader_Cvar::UNDEFINED);

				id = "custom" + eastl::to_string(i);
				sVar.custom = nuniforms->PullBool(id.c_str(), true);

				id = "value" + eastl::to_string(i);
				switch (vT)
				{
				case RE_Cvar::BOOL: sVar.SetValue(nuniforms->PullBool(id.c_str(), true), true); break;
				case RE_Cvar::BOOL2:
				{
					b = nuniforms->PullBool(id.c_str(), 2, true);
					sVar.SetValue(b, 2, true);
					DEL_A(b);
					break;
				}
				case RE_Cvar::BOOL3:
				{
					b = nuniforms->PullBool(id.c_str(), 3, true);
					sVar.SetValue(b, 3, true);
					DEL_A(b);
					break; 
				}
				case RE_Cvar::BOOL4:
				{
					b = nuniforms->PullBool(id.c_str(), 4, true);
					sVar.SetValue(b, 4, true);
					DEL_A(b);
					break; 
				}
				case RE_Cvar::INT: sVar.SetValue(-1, true); break;
				case RE_Cvar::INT2:
				{
					intPtr = nuniforms->PullInt(id.c_str(), 2, true);
					sVar.SetValue(intPtr, 2, true);
					DEL_A(b);
					break;
				}
				case RE_Cvar::INT3:
				{
					intPtr = nuniforms->PullInt(id.c_str(), 3, true);
					sVar.SetValue(intPtr, 3, true);
					DEL_A(b);
					break; 
				}
				case RE_Cvar::INT4:
				{
					intPtr = nuniforms->PullInt(id.c_str(), 4, true);
					sVar.SetValue(intPtr, 4, true);
					DEL_A(b);
					break; 
				}
				case RE_Cvar::FLOAT: sVar.SetValue(nuniforms->PullFloat(id.c_str(), 0), true); break;
				case RE_Cvar::FLOAT2: sVar.SetValue(nuniforms->PullFloat(id.c_str(), math::float2::zero), true); break;
				case RE_Cvar::FLOAT3: sVar.SetValue(nuniforms->PullFloatVector(id.c_str(), math::float3::zero), true); break;
				case RE_Cvar::FLOAT4:
				case RE_Cvar::MAT2: sVar.SetValue(nuniforms->PullFloat4(id.c_str(), math::float4::zero), true); break;
				case RE_Cvar::MAT3: sVar.SetValue(nuniforms->PullMat3(id.c_str(), math::float3x3::zero), true); break;
				case RE_Cvar::MAT4: sVar.SetValue(nuniforms->PullMat4(id.c_str(), math::float4x4::zero), true); break;
				case RE_Cvar::SAMPLER: sVar.SetSampler(RE_RES->IsReference(nuniforms->PullString(id.c_str(),"")), true); break;
				}

				fromShaderCustomUniforms.push_back(sVar);
			}
		}

		DEL(nuniforms);
		DEL(nodeMat);

		if (generateLibraryPath)
		{
			SetMD5(material.GetMd5().c_str());
			eastl::string libraryPath("Library/Materials/");
			libraryPath += GetMD5();
			SetLibraryPath(libraryPath.c_str());
		}

		ResourceContainer::inMemory = true;
	}
}

void RE_Material::PullTexturesJson(RE_Json * texturesNode, eastl::vector<const char*>* textures)
{
	uint texturesSize = texturesNode->PullInt("Size", 0);
	for (uint i = 0; i < texturesSize; i++)
	{
		eastl::string idref = "MetaPath";
		idref += eastl::to_string(i).c_str();
		eastl::string textureMaT = texturesNode->PullString(idref.c_str(), "");
		const char* textureMD5 = RE_RES->FindMD5ByMETAPath(textureMaT.c_str());
		if (textureMD5) textures->push_back(textureMD5);
		else RE_LOG_ERROR("No texture found.\nPath: %s", textureMaT.c_str());
	}
}

void RE_Material::JsonSerialize(bool onlyMD5)
{
	Config materialSerialize(GetAssetPath(), RE_FS->GetZipPath());
	RE_Json* materialNode = materialSerialize.GetRootNode("Material");
	materialNode->PushString("name", GetName()); //for get different md5
	materialNode->PushInt("ShaderType", static_cast<int>(shadingType));
	materialNode->PushUInt("usingOnMat", usingOnMat, 18);
	materialNode->PushFloatVector("DiffuseColor", cDiffuse);
	materialNode->PushFloatVector("SpecularColor", cSpecular);
	materialNode->PushFloatVector("AmbientColor", cAmbient);
	materialNode->PushFloatVector("EmissiveColor", cEmissive);
	materialNode->PushFloatVector("TransparentColor", cTransparent);
	materialNode->PushBool("BackFaceCulling", backFaceCulling);
	materialNode->PushBool("BlendMode", blendMode);
	materialNode->PushFloat("Opacity", opacity);
	materialNode->PushFloat("Shininess", shininess);
	materialNode->PushFloat("ShininessStrenght", shininessStrenght);
	materialNode->PushFloat("Refraccti", refraccti);

	RE_Json* nuniforms = materialNode->PushJObject("uniforms");
	nuniforms->PushUInt("size", fromShaderCustomUniforms.size());
	if (!fromShaderCustomUniforms.empty()) {
		eastl::string id;
		for (uint i = 0; i < fromShaderCustomUniforms.size(); i++)
		{
			id = "name" + eastl::to_string(i);
			nuniforms->PushString(id.c_str(), fromShaderCustomUniforms[i].name.c_str());

			id = "type" + eastl::to_string(i);
			nuniforms->PushUInt(id.c_str(), fromShaderCustomUniforms[i].GetType());

			id = "custom" + eastl::to_string(i);
			nuniforms->PushBool(id.c_str(), fromShaderCustomUniforms[i].custom);

			id = "value" + eastl::to_string(i);
			switch (fromShaderCustomUniforms[i].GetType())
			{
			case RE_Cvar::BOOL: nuniforms->PushBool(id.c_str(), fromShaderCustomUniforms[i].AsBool()); break;
			case RE_Cvar::BOOL2: nuniforms->PushBool(id.c_str(), fromShaderCustomUniforms[i].AsBool2(), 2); break;
			case RE_Cvar::BOOL3: nuniforms->PushBool(id.c_str(), fromShaderCustomUniforms[i].AsBool3(), 3); break;
			case RE_Cvar::BOOL4: nuniforms->PushBool(id.c_str(), fromShaderCustomUniforms[i].AsBool4(), 4); break;
			case RE_Cvar::INT: nuniforms->PushInt(id.c_str(), fromShaderCustomUniforms[i].AsInt()); break;
			case RE_Cvar::INT2: nuniforms->PushInt(id.c_str(), fromShaderCustomUniforms[i].AsInt2(),2); break;
			case RE_Cvar::INT3: nuniforms->PushInt(id.c_str(), fromShaderCustomUniforms[i].AsInt3(), 3); break;
			case RE_Cvar::INT4: nuniforms->PushInt(id.c_str(), fromShaderCustomUniforms[i].AsInt4(), 4); break;
			case RE_Cvar::FLOAT: nuniforms->PushFloat(id.c_str(), fromShaderCustomUniforms[i].AsFloat()); break;
			case RE_Cvar::FLOAT2: nuniforms->PushFloat(id.c_str(), fromShaderCustomUniforms[i].AsFloat2()); break;
			case RE_Cvar::FLOAT3: nuniforms->PushFloatVector(id.c_str(), fromShaderCustomUniforms[i].AsFloat3()); break;
			case RE_Cvar::FLOAT4:
			case RE_Cvar::MAT2: nuniforms->PushFloat4(id.c_str(), fromShaderCustomUniforms[i].AsFloat4()); break;
			case RE_Cvar::MAT3: nuniforms->PushMat3(id.c_str(), fromShaderCustomUniforms[i].AsMat3()); break;
			case RE_Cvar::MAT4: nuniforms->PushMat4(id.c_str(), fromShaderCustomUniforms[i].AsMat4()); break;
			case RE_Cvar::SAMPLER: nuniforms->PushString(id.c_str(), (fromShaderCustomUniforms[i].AsCharP()) ? fromShaderCustomUniforms[i].AsCharP() : ""); break;
			}
		}
	}

	if(!onlyMD5) materialSerialize.Save();
	SetMD5(materialSerialize.GetMd5().c_str());

	eastl::string libraryPath("Library/Materials/");
	libraryPath += GetMD5();
	SetLibraryPath(libraryPath.c_str());

	DEL(nuniforms);
	DEL(materialNode);
}

void RE_Material::PushTexturesJson(RE_Json * texturesNode, eastl::vector<const char*>* textures)
{
	uint texturesSize = textures->size();
	texturesNode->PushUInt("Size", texturesSize);
	for (uint i = 0; i < texturesSize; i++)
	{
		eastl::string idref = "MetaPath";
		idref += eastl::to_string(i).c_str();
		texturesNode->PushString(idref.c_str(), RE_RES->At(textures->operator[](i))->GetMetaPath());
	}
}

void RE_Material::BinaryDeserialize()
{
	RE_FileBuffer libraryFile(GetLibraryPath());
	if (libraryFile.Load())
	{
		char* cursor = libraryFile.GetBuffer();

		size_t size = sizeof(RE_ShadingMode);
		memcpy(&shadingType, cursor, size);
		cursor += size;

		size = sizeof(uint) * 18;
		memcpy(usingOnMat, cursor, size);
		cursor += size;

		size = sizeof(float) * 3;
		cDiffuse = math::vec(reinterpret_cast<float*>(cursor));
		cursor += size;

		cSpecular = math::vec(reinterpret_cast<float*>(cursor));
		cursor += size;

		cAmbient = math::vec(reinterpret_cast<float*>(cursor));
		cursor += size;

		cEmissive = math::vec(reinterpret_cast<float*>(cursor));
		cursor += size;

		cTransparent = math::vec(reinterpret_cast<float*>(cursor));
		cursor += size;

		size = sizeof(bool);
		memcpy(&backFaceCulling, cursor, size);
		cursor += size;

		memcpy(&blendMode, cursor, size);
		cursor += size;

		size = sizeof(float);
		memcpy(&opacity, cursor, size);
		cursor += size;

		memcpy(&shininess, cursor, size);
		cursor += size;

		memcpy(&shininessStrenght, cursor, size);
		cursor += size;

		memcpy(&refraccti, cursor, size);
		cursor += size;

		fromShaderCustomUniforms.clear();
		uint uSize = 0;
		size = sizeof(uint);
		memcpy( &uSize, cursor, size);
		cursor += size;

		if (uSize > 0)
		{
			for (uint i = 0; i < uSize; i++)
			{
				RE_Shader_Cvar sVar;
				uint nSize = 0;
				size = sizeof(uint);
				memcpy(&nSize, cursor, size);
				cursor += size;

				size = sizeof(char) * nSize;
				sVar.name = eastl::string(cursor, nSize);
				cursor += size;

				uint uType = 0;
				size = sizeof(uint);
				memcpy(&uType, cursor, size);
				cursor += size;

				size = sizeof(bool);
				memcpy(&sVar.custom, cursor, size);
				cursor += size;

				float f;
				bool b;
				int int_v;
				int* intCursor;
				float* floatCursor;
				bool* boolCursor;
				switch (RE_Cvar::VAR_TYPE(uType))
				{
				case RE_Cvar::BOOL:
				{
					size = sizeof(bool);
					memcpy(&b, cursor, size);
					cursor += size;
					sVar.SetValue(b, true);
					break; 
				}
				case RE_Cvar::BOOL2:
				{
					size = sizeof(bool) * 2;
					boolCursor = reinterpret_cast<bool*>(cursor);
					sVar.SetValue(boolCursor, 2, true);
					cursor += size;
					break; 
				}
				case RE_Cvar::BOOL3:
				{
					size = sizeof(bool) * 3;
					boolCursor = reinterpret_cast<bool*>(cursor);
					sVar.SetValue(boolCursor, 3, true);
					cursor += size;
					break; 
				}
				case RE_Cvar::BOOL4:
				{
					size = sizeof(bool) * 4;
					boolCursor = reinterpret_cast<bool*>(cursor);
					sVar.SetValue(boolCursor, 4, true);
					cursor += size;
					break; 
				}
				case RE_Cvar::INT:
				{
					size = sizeof(int);
					memcpy(&int_v, cursor, size);
					cursor += size;
					sVar.SetValue(int_v, true);
					break; 
				}
				case RE_Cvar::INT2:
				{
					size = sizeof(int) * 2;
					intCursor = reinterpret_cast<int*>(cursor);
					sVar.SetValue(intCursor, 2, true);
					cursor += size;
					break; 
				}
				case RE_Cvar::INT3:
				{
					size = sizeof(int) * 3;
					intCursor = reinterpret_cast<int*>(cursor);
					sVar.SetValue(intCursor, 3, true);
					cursor += size;
					break; 
				}
				case RE_Cvar::INT4:
				{
					size = sizeof(int) * 4;
					intCursor = reinterpret_cast<int*>(cursor);
					sVar.SetValue(intCursor, 4, true);
					cursor += size;
					break; 
				}
				case RE_Cvar::FLOAT:
				{
					size = sizeof(float);
					memcpy(&f, cursor, size);
					cursor += size;
					sVar.SetValue(f, true);
					break; 
				}
				case RE_Cvar::FLOAT2:
				{
					size = sizeof(float) * 2;
					floatCursor = reinterpret_cast<float*>(cursor);
					sVar.SetValue(math::float2(floatCursor), true);
					cursor += size;
					break; 
				}
				case RE_Cvar::FLOAT3:
				{
					size = sizeof(float) * 3;
					floatCursor = reinterpret_cast<float*>(cursor);
					sVar.SetValue(math::float3(floatCursor), true);
					cursor += size;
					break; 
				}
				case RE_Cvar::FLOAT4:
				case RE_Cvar::MAT2:
				{
					size = sizeof(float) * 4;
					floatCursor = reinterpret_cast<float*>(cursor);
					sVar.SetValue(math::float4(floatCursor), true);
					cursor += size;
					break; 
				}
				case RE_Cvar::MAT3:
				{
					size = sizeof(float) * 9;
					floatCursor = reinterpret_cast<float*>(cursor);
					{
						math::float3x3 mat3;
						mat3.Set(floatCursor);
						sVar.SetValue(mat3, true);
					}
					cursor += size;
					break; 
				}
				case RE_Cvar::MAT4:
				{
					size = sizeof(float) * 16;
					floatCursor = reinterpret_cast<float*>(cursor);
					{
						math::float4x4 mat4;
						mat4.Set(floatCursor);
						sVar.SetValue(mat4, true);
					}
					cursor += size;
					break; 
				}
				case RE_Cvar::SAMPLER:
				{
					nSize = 0;
					size = sizeof(uint);
					memcpy(&nSize, cursor, size);
					cursor += size;
					if (nSize > 0)
					{
						size = sizeof(char) * nSize;
						sVar.SetSampler(RE_RES->IsReference(eastl::string(cursor, nSize).c_str()), true);
						cursor += size;
					}
					else sVar.SetSampler(nullptr, true);
					break; 
				}
				}
				fromShaderCustomUniforms.push_back(sVar);
			}
		}
		ResourceContainer::inMemory = true;
	}
}

void RE_Material::BinarySerialize()
{
	RE_FileBuffer libraryFile(GetLibraryPath(), RE_FS->GetZipPath());

	uint bufferSize = GetBinarySize() + 1;
	char* buffer = new char[bufferSize];
	char* cursor = buffer;

	size_t size = sizeof(int);
	memcpy(cursor, &shadingType, size);
	cursor += size;

	size = sizeof(uint) * 18;
	memcpy(cursor, usingOnMat, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, &cDiffuse.x, size);
	cursor += size;

	memcpy(cursor, &cSpecular.x, size);
	cursor += size;

	memcpy(cursor, &cAmbient.x, size);
	cursor += size;

	memcpy(cursor, &cEmissive.x, size);
	cursor += size;

	memcpy(cursor, &cTransparent.x, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(cursor, &backFaceCulling, size);
	cursor += size;

	memcpy(cursor, &blendMode, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &opacity, size);
	cursor += size;

	memcpy(cursor, &shininess, size);
	cursor += size;

	memcpy(cursor, &shininessStrenght, size);
	cursor += size;

	memcpy(cursor, &refraccti, size);
	cursor += size;

	uint uSize = fromShaderCustomUniforms.size();
	size = sizeof(uint);
	memcpy(cursor, &uSize, size);
	cursor += size;

	if (uSize > 0)
	{
		for (uint i = 0; i < uSize; i++)
		{
			uint nSize = fromShaderCustomUniforms[i].name.size();
			size = sizeof(uint);
			memcpy(cursor, &nSize, size);
			cursor += size;

			size = sizeof(char) * nSize;
			memcpy(cursor, fromShaderCustomUniforms[i].name.c_str(), size);
			cursor += size;

			uint uType = fromShaderCustomUniforms[i].GetType();
			size = sizeof(uint);
			memcpy(cursor, &uType, size);
			cursor += size;

			size = sizeof(bool);
			memcpy(cursor, &fromShaderCustomUniforms[i].custom, size);
			cursor += size;

			bool b;
			int int_v;
			switch (fromShaderCustomUniforms[i].GetType())
			{
			case RE_Cvar::BOOL:
			{
				b = fromShaderCustomUniforms[i].AsBool();
				size = sizeof(bool);
				memcpy(cursor, &b, size);
				cursor += size;
				break; 
			}
			case RE_Cvar::BOOL2:
			{
				size = sizeof(bool) * 2;
				memcpy(cursor, fromShaderCustomUniforms[i].AsBool2(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::BOOL3:
			{
				size = sizeof(bool) * 3;
				memcpy(cursor, fromShaderCustomUniforms[i].AsBool3(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::BOOL4:
			{
				size = sizeof(bool) * 4;
				memcpy(cursor, fromShaderCustomUniforms[i].AsBool4(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::INT:
			{
				int_v = fromShaderCustomUniforms[i].AsInt();
				size = sizeof(int);
				memcpy(cursor, &int_v, size);
				cursor += size;
				break; 
			}
			case RE_Cvar::INT2:
			{
				size = sizeof(int) * 2;
				memcpy(cursor, fromShaderCustomUniforms[i].AsInt2(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::INT3:
			{
				size = sizeof(int) * 3;
				memcpy(cursor, fromShaderCustomUniforms[i].AsInt3(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::INT4:
			{
				size = sizeof(int) * 4;
				memcpy(cursor, fromShaderCustomUniforms[i].AsInt4(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::FLOAT:
			{
				size = sizeof(float);
				memcpy(cursor, fromShaderCustomUniforms[i].AsFloatPointer(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::FLOAT2:
			{
				size = sizeof(float) * 2;
				memcpy(cursor, fromShaderCustomUniforms[i].AsFloatPointer(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::FLOAT3:
			{
				size = sizeof(float) * 3;
				memcpy(cursor, fromShaderCustomUniforms[i].AsFloatPointer(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::FLOAT4:
			case RE_Cvar::MAT2:
			{
				size = sizeof(float) * 4;
				memcpy(cursor, fromShaderCustomUniforms[i].AsFloatPointer(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::MAT3:
			{
				size = sizeof(float) * 9;
				memcpy(cursor, fromShaderCustomUniforms[i].AsFloatPointer(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::MAT4:
			{
				size = sizeof(float) * 16;
				memcpy(cursor, fromShaderCustomUniforms[i].AsFloatPointer(), size);
				cursor += size;
				break; 
			}
			case RE_Cvar::SAMPLER:
			{
				nSize = (fromShaderCustomUniforms[i].AsCharP()) ? strlen(fromShaderCustomUniforms[i].AsCharP()) : 0;
				size = sizeof(uint);
				memcpy(cursor, &nSize, size);
				cursor += size;
				if (nSize > 0)
				{
					size = sizeof(char) * nSize;
					memcpy(cursor, fromShaderCustomUniforms[i].AsCharP(), size);
					cursor += size;
				}
				break; 
			}
			}
		}
	}

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	libraryFile.Save(buffer, bufferSize);
	DEL_A(buffer);
}

void RE_Material::UseResources()
{
	if(shaderMD5) RE_RES->Use(shaderMD5);
	for (auto t : tDiffuse) RE_RES->Use(t);
	for (auto t : tSpecular) RE_RES->Use(t);
	for (auto t : tAmbient) RE_RES->Use(t);
	for (auto t : tEmissive) RE_RES->Use(t);
	for (auto t : tOpacity) RE_RES->Use(t);
	for (auto t : tShininess) RE_RES->Use(t);
	for (auto t : tHeight) RE_RES->Use(t);
	for (auto t : tNormals) RE_RES->Use(t);
	for (auto t : tReflection) RE_RES->Use(t);
	for (auto t : tUnknown) RE_RES->Use(t);
}

void RE_Material::UnUseResources()
{
	if (shaderMD5) RE_RES->UnUse(shaderMD5);
	for (auto t : tDiffuse) RE_RES->UnUse(t);
	for (auto t : tSpecular) RE_RES->UnUse(t);
	for (auto t : tAmbient) RE_RES->UnUse(t);
	for (auto t : tEmissive) RE_RES->UnUse(t);
	for (auto t : tOpacity) RE_RES->UnUse(t);
	for (auto t : tShininess) RE_RES->UnUse(t);
	for (auto t : tHeight) RE_RES->UnUse(t);
	for (auto t : tNormals) RE_RES->UnUse(t);
	for (auto t : tReflection) RE_RES->UnUse(t);
	for (auto t : tUnknown) RE_RES->UnUse(t);
}

void RE_Material::UploadToShader(const float* model, bool usingChekers, bool defaultShader)
{
	const char* usingShader = (shaderMD5 && !defaultShader) ? shaderMD5 : RE_RES->internalResources->GetDefaultShader();
	RE_Shader* shaderRes = dynamic_cast<RE_Shader*>(RE_RES->At(usingShader));
	RE_GLCache::ChangeShader(shaderRes->GetID());
	shaderRes->UploadModel(model);

	unsigned int ShaderID = shaderRes->GetID();
	bool onlyColor = true;
	if (!usingChekers)
	{
		RE_ShaderImporter::setFloat(ShaderID, "useColor",
			(usingOnMat[CDIFFUSE] || usingOnMat[CSPECULAR] || usingOnMat[CAMBIENT] ||
			 usingOnMat[CEMISSIVE] || usingOnMat[CTRANSPARENT] || usingOnMat[OPACITY] ||
			 usingOnMat[SHININESS] || usingOnMat[SHININESSSTRENGHT] || usingOnMat[REFRACCTI]) ? 1.0f : 0.0f);

		if ((usingOnMat[TDIFFUSE] && !tDiffuse.empty()) || (usingOnMat[TSPECULAR] && !tSpecular.empty())
			|| (usingOnMat[TAMBIENT] && !tAmbient.empty()) || (usingOnMat[TEMISSIVE] && !tEmissive.empty())
			|| (usingOnMat[TOPACITY] && !tOpacity.empty() || (usingOnMat[TSHININESS] && !tShininess.empty())
			|| (usingOnMat[THEIGHT] && !tHeight.empty()) || !usingOnMat[TNORMALS] && !tNormals.empty())
			|| (usingOnMat[TREFLECTION] && !tReflection.empty())
			|| shaderRes->NeedUploadDepth())
		{
			RE_ShaderImporter::setFloat(ShaderID, "useTexture", 1.0f);
			onlyColor = false;
		}
		else RE_ShaderImporter::setFloat(ShaderID, "useTexture", 0.0f);
	}
	else
	{
		RE_ShaderImporter::setFloat(ShaderID, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(ShaderID, "useTexture", 1.0f);
		onlyColor = false;
	}

	unsigned int textureCounter = 0;
	if (shaderRes->NeedUploadDepth())
	{
		glActiveTexture(GL_TEXTURE0 + textureCounter);
		RE_GLCache::ChangeTextureBind(RE_RENDER->GetDepthTexture());
		shaderRes->UploadDepth(textureCounter++);
	}

	// Bind Textures
	if (onlyColor)
	{
		if(usingOnMat[CDIFFUSE])			RE_ShaderImporter::setFloat(ShaderID, "cdiffuse", cDiffuse);
		if(usingOnMat[CSPECULAR])			RE_ShaderImporter::setFloat(ShaderID, "cspecular", cSpecular);
		if(usingOnMat[CAMBIENT])			RE_ShaderImporter::setFloat(ShaderID, "cambient", cAmbient);
		if(usingOnMat[CEMISSIVE])			RE_ShaderImporter::setFloat(ShaderID, "cemissive", cEmissive);
		if(usingOnMat[CTRANSPARENT])		RE_ShaderImporter::setFloat(ShaderID, "ctransparent", cTransparent);
		if(usingOnMat[OPACITY])				RE_ShaderImporter::setFloat(ShaderID, "opacity", opacity);
		if(usingOnMat[SHININESS])			RE_ShaderImporter::setFloat(ShaderID, "shininess", shininess);
		if(usingOnMat[SHININESSSTRENGHT])	RE_ShaderImporter::setFloat(ShaderID, "shininessST", shininessStrenght);
		if(usingOnMat[REFRACCTI])			RE_ShaderImporter::setFloat(ShaderID, "refraccti", refraccti);
	}
	else if (usingChekers)
	{
		glActiveTexture(GL_TEXTURE0 + textureCounter);
		RE_ShaderImporter::setInt(ShaderID, "tdiffuse0", textureCounter++);
		RE_GLCache::ChangeTextureBind(RE_RES->internalResources->GetTextureChecker());
	}
	else
	{
		if (usingOnMat[CDIFFUSE])			RE_ShaderImporter::setFloat(ShaderID, "cdiffuse", cDiffuse);
		if (usingOnMat[CSPECULAR])			RE_ShaderImporter::setFloat(ShaderID, "cspecular", cSpecular);
		if (usingOnMat[CAMBIENT])			RE_ShaderImporter::setFloat(ShaderID, "cambient", cAmbient);
		if (usingOnMat[CEMISSIVE])			RE_ShaderImporter::setFloat(ShaderID, "cemissive", cEmissive);
		if (usingOnMat[CTRANSPARENT])		RE_ShaderImporter::setFloat(ShaderID, "ctransparent", cTransparent);
		if (usingOnMat[OPACITY])			RE_ShaderImporter::setFloat(ShaderID, "opacity", opacity);
		if (usingOnMat[SHININESS])			RE_ShaderImporter::setFloat(ShaderID, "shininess", shininess);
		if (usingOnMat[SHININESSSTRENGHT])	RE_ShaderImporter::setFloat(ShaderID, "shininessST", shininessStrenght);
		if (usingOnMat[REFRACCTI])			RE_ShaderImporter::setFloat(ShaderID, "refraccti", refraccti);

		for (unsigned int i = 0; i < tDiffuse.size() || i < usingOnMat[TDIFFUSE]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(ShaderID, ("tdiffuse" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tDiffuse[i]))->use();
		}
		for (unsigned int i = 0; i < tSpecular.size() && i < usingOnMat[TSPECULAR]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(ShaderID, ("tspecular" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tSpecular[i]))->use();
		}
		for (unsigned int i = 0; i < tAmbient.size() && i < usingOnMat[TAMBIENT]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(ShaderID, ("tambient" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tAmbient[i]))->use();
		}
		for (unsigned int i = 0; i < tEmissive.size() && i < usingOnMat[TEMISSIVE]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(ShaderID, ("temissive" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tEmissive[i]))->use();
		}
		for (unsigned int i = 0; i < tOpacity.size() && i < usingOnMat[TOPACITY]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(ShaderID, ("topacity" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tOpacity[i]))->use();
		}
		for (unsigned int i = 0; i < tShininess.size() && i < usingOnMat[TSHININESS]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(ShaderID, ("tshininess" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tShininess[i]))->use();
		}
		for (unsigned int i = 0; i < tHeight.size() && i < usingOnMat[THEIGHT]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(ShaderID, ("theight" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tHeight[i]))->use();
		}
		for (unsigned int i = 0; i < tNormals.size() && i < usingOnMat[TNORMALS]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(ShaderID, ("tnormals" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tNormals[i]))->use();
		}
		for (unsigned int i = 0; i < tReflection.size() && i < usingOnMat[TREFLECTION]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(ShaderID, ("treflection" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tReflection[i]))->use();
		}
	}

	for (uint i = 0; i < fromShaderCustomUniforms.size(); i++)
	{
		bool* b;
		int* int_pv;
		float* f_ptr;
		switch (fromShaderCustomUniforms[i].GetType())
		{
		case RE_Cvar::BOOL: RE_ShaderImporter::setBool(ShaderID, fromShaderCustomUniforms[i].name.c_str(), fromShaderCustomUniforms[i].AsBool()); break;
		case RE_Cvar::BOOL2:
		{
			b = fromShaderCustomUniforms[i].AsBool2();
			RE_ShaderImporter::setBool(ShaderID, fromShaderCustomUniforms[i].name.c_str(), b[0], b[1]);
			break; 
		}
		case RE_Cvar::BOOL3:
		{
			b = fromShaderCustomUniforms[i].AsBool3();
			RE_ShaderImporter::setBool(ShaderID, fromShaderCustomUniforms[i].name.c_str(), b[0], b[1], b[2]);
			break; 
		}
		case RE_Cvar::BOOL4:
		{
			b = fromShaderCustomUniforms[i].AsBool4();
			RE_ShaderImporter::setBool(ShaderID, fromShaderCustomUniforms[i].name.c_str(), b[0], b[1], b[2], b[3]);
			break; 
		}
		case RE_Cvar::INT: RE_ShaderImporter::setInt(ShaderID, fromShaderCustomUniforms[i].name.c_str(), fromShaderCustomUniforms[i].AsInt()); break;
		case RE_Cvar::INT2:
		{
			int_pv = fromShaderCustomUniforms[i].AsInt2();
			RE_ShaderImporter::setInt(ShaderID, fromShaderCustomUniforms[i].name.c_str(), int_pv[0], int_pv[1]);
			break; 
		}
		case RE_Cvar::INT3:
		{
			int_pv = fromShaderCustomUniforms[i].AsInt3();
			RE_ShaderImporter::setInt(ShaderID, fromShaderCustomUniforms[i].name.c_str(), int_pv[0], int_pv[1], int_pv[2]);
			break; 
		}
		case RE_Cvar::INT4:
		{
			int_pv = fromShaderCustomUniforms[i].AsInt4();
			RE_ShaderImporter::setInt(ShaderID, fromShaderCustomUniforms[i].name.c_str(), int_pv[0], int_pv[1], int_pv[2], int_pv[3]);
			break; 
		}
		case RE_Cvar::FLOAT: RE_ShaderImporter::setFloat(ShaderID, fromShaderCustomUniforms[i].name.c_str(), fromShaderCustomUniforms[i].AsFloat()); break; 
		case RE_Cvar::FLOAT2:
		{
			f_ptr = fromShaderCustomUniforms[i].AsFloatPointer();
			RE_ShaderImporter::setFloat(ShaderID, fromShaderCustomUniforms[i].name.c_str(), f_ptr[0], f_ptr[1]);
			break; 
		}
		case RE_Cvar::FLOAT3:
		{
			f_ptr = fromShaderCustomUniforms[i].AsFloatPointer();
			RE_ShaderImporter::setFloat(ShaderID, fromShaderCustomUniforms[i].name.c_str(), f_ptr[0], f_ptr[1], f_ptr[2]);
			break; 
		}
		case RE_Cvar::FLOAT4:
		case RE_Cvar::MAT2:
		{
			f_ptr = fromShaderCustomUniforms[i].AsFloatPointer();
			RE_ShaderImporter::setFloat(ShaderID, fromShaderCustomUniforms[i].name.c_str(), f_ptr[0], f_ptr[1], f_ptr[2], f_ptr[3]);
			break; 
		}
		case RE_Cvar::MAT3:
		{
			f_ptr = fromShaderCustomUniforms[i].AsFloatPointer();
			RE_ShaderImporter::setFloat3x3(ShaderID, fromShaderCustomUniforms[i].name.c_str(), f_ptr);
			break; 
		}
		case RE_Cvar::MAT4:
		{
			f_ptr = fromShaderCustomUniforms[i].AsFloatPointer();
			RE_ShaderImporter::setFloat4x4(ShaderID, fromShaderCustomUniforms[i].name.c_str(), f_ptr);
			break;
		}
		case RE_Cvar::SAMPLER:
		{
			if (fromShaderCustomUniforms[i].AsCharP())
			{
				glActiveTexture(GL_TEXTURE0 + textureCounter);
				RE_ShaderImporter::setUnsignedInt(ShaderID, fromShaderCustomUniforms[i].name.c_str(), textureCounter++);
				dynamic_cast<RE_Texture*>(RE_RES->At(fromShaderCustomUniforms[i].AsCharP()))->use();
			}
			break;
		}
		}
	}
}

void RE_Material::UploadAsParticleDataToShader(unsigned int shaderID, bool useTextures, bool lighting)
{
	RE_ShaderImporter::setFloat(shaderID, "useColor", 1.0f);

	RE_ShaderImporter::setFloat(shaderID, "cdiffuse", cDiffuse);
	RE_ShaderImporter::setFloat(shaderID, "opacity", opacity);
	if (lighting) {
		RE_ShaderImporter::setFloat(shaderID, "cspecular", cSpecular);
		RE_ShaderImporter::setFloat(shaderID, "shininess", shininess);
	}else
		RE_ShaderImporter::setFloat(shaderID, "opacity", 1.0f);

	if (useTextures) {
		RE_ShaderImporter::setFloat(shaderID, "useTexture", static_cast<float>(tDiffuse.size() > 0 && tSpecular.size() > 0));

		unsigned int textureCounter = 0;
		for (unsigned int i = 0; i < tDiffuse.size() && i < usingOnMat[TDIFFUSE]; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureCounter);
			RE_ShaderImporter::setInt(shaderID, ("tdiffuse" + eastl::to_string(i)).c_str(), textureCounter++);
			dynamic_cast<RE_Texture*>(RE_RES->At(tDiffuse[i]))->use();
		}
		if (lighting) {
			for (unsigned int i = 0; i < tSpecular.size() && i < usingOnMat[TSPECULAR]; i++)
			{
				glActiveTexture(GL_TEXTURE0 + textureCounter);
				RE_ShaderImporter::setInt(shaderID, ("tspecular" + eastl::to_string(i)).c_str(), textureCounter++);
				dynamic_cast<RE_Texture*>(RE_RES->At(tSpecular[i]))->use();
			}
		}
	}
}

unsigned int RE_Material::GetShaderID() const
{
	const char* usingShader = (shaderMD5) ? shaderMD5 : RE_RES->internalResources->GetDefaultShader();
	return dynamic_cast<RE_Shader*>(RE_RES->At(usingShader))->GetID();
}

unsigned int RE_Material::GetBinarySize()
{
	uint charCount = sizeof(RE_ShadingMode) + (sizeof(uint) * 18) + (sizeof(float) * 15) + (sizeof(bool) * 2) + (sizeof(float) * 4);

	//Custom uniforms
	charCount += sizeof(uint);
	if (!fromShaderCustomUniforms.empty())
	{
		charCount += (sizeof(uint) * 2 + sizeof(float)) * fromShaderCustomUniforms.size();
		for (uint i = 0; i < fromShaderCustomUniforms.size(); i++)
		{
			charCount += sizeof(char) * fromShaderCustomUniforms[i].name.size();
			switch (fromShaderCustomUniforms[i].GetType()) {
			case RE_Cvar::BOOL: charCount += sizeof(bool); break;
			case RE_Cvar::BOOL2: charCount += sizeof(bool) * 2; break;
			case RE_Cvar::BOOL3: charCount += sizeof(bool) * 3; break;
			case RE_Cvar::BOOL4: charCount += sizeof(bool) * 4; break;
			case RE_Cvar::INT: charCount += sizeof(int); break;
			case RE_Cvar::INT2: charCount += sizeof(int) * 2; break;
			case RE_Cvar::INT3: charCount += sizeof(int) * 3; break;
			case RE_Cvar::INT4: charCount += sizeof(int) * 4; break;
			case RE_Cvar::FLOAT: charCount += sizeof(float); break;
			case RE_Cvar::FLOAT2: charCount += sizeof(float) * 2; break;
			case RE_Cvar::FLOAT3: charCount += sizeof(float) * 3; break;
			case RE_Cvar::FLOAT4:
			case RE_Cvar::MAT2: charCount += sizeof(float) * 4; break;
			case RE_Cvar::MAT3: charCount += sizeof(float) * 9; break;
			case RE_Cvar::MAT4: charCount += sizeof(float) * 16; break;
			case RE_Cvar::SAMPLER:
			{
				charCount += sizeof(uint);
				if (fromShaderCustomUniforms[i].AsCharP()) charCount += sizeof(char) * strlen(fromShaderCustomUniforms[i].AsCharP());
				break;
			}}
		}
	}
	return charCount;
}

void RE_Material::GetAndProcessUniformsFromShader()
{
	for (uint i = 0; i < 18; i++) usingOnMat[i] = 0;
	fromShaderCustomUniforms.clear();

	static const char* materialNames[18] = {  "cdiffuse", "tdiffuse", "cspecular", "tspecular", "cambient",  "tambient", "cemissive", "temissive", "ctransparent", "opacity", "topacity",  "shininess", "shininessST", "tshininess", "refraccti", "theight",  "tnormals", "treflection" };
	static const char* materialTextures[9] = {  "tdiffuse", "tspecular", "tambient", "temissive", "topacity", "tshininess", "theight", "tnormals", "treflection" };
	
	eastl::vector<RE_Shader_Cvar> fromShader = dynamic_cast<RE_Shader*>(RE_RES->At(shaderMD5 ? shaderMD5 : RE_RES->internalResources->GetDefaultShader()))->GetUniformValues();
	for (auto &sVar : fromShader)
	{
		if (sVar.custom)
		{
			fromShaderCustomUniforms.push_back(sVar);
		}
		else
		{
			int pos = sVar.name.find_first_of("0123456789");
			eastl::string name = (pos != eastl::string::npos) ? sVar.name.substr(0, pos) : sVar.name;
			MaterialUINT index = UNDEFINED;
			for (uint i = 0; i < 18; i++)
			{
				if (name.compare(materialNames[i]) == 0)
				{
					index = (MaterialUINT)i;
					break;
				}
			}

			if (index != UNDEFINED)
			{
				bool texture = false;
				for (uint i = 0; i < 9; i++)
				{
					if (name.compare(materialTextures[i]) == 0)
					{
						texture = true;
						break;
					}
				}

				if (texture)
				{
					unsigned int count = EA::StdC::AtoU32(&sVar.name.back()) + 1;
					if (usingOnMat[index] < count) usingOnMat[index] = count;
				}
				else usingOnMat[index] = 1;
			}
		}
	}
}

bool RE_Material::ExistsOnTexture(const char* texture, eastl::vector<const char*>* textures)
{
	for (auto tex : *textures)
	{
		if (tex == texture)
			return true;
	}
	return false;
}

void RE_Material::DeleteTexture(const char* texMD5, eastl::vector<const char*>* textures)
{
	auto iter = textures->begin();
	while (iter != textures->end())
	{
		if (*iter == texMD5) textures->erase(iter);
		else iter++;
	}
}
