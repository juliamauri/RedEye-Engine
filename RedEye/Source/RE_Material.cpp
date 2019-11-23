#include "RE_Material.h"

#include "Application.h"
#include "FileSystem.h"
#include "OutputLog.h"
#include "RE_ResourceManager.h"

#include "ModuleEditor.h"
#include "EditorWindows.h"

#include "Globals.h"

#include "ImGui/imgui.h"

RE_Material::RE_Material() { }

RE_Material::RE_Material(const char* metapath) : ResourceContainer(metapath) { }
     

RE_Material::~RE_Material() { }

void RE_Material::LoadInMemory()
{
	//TODO ResourcesManager, apply count referending textures
	if (App->fs->Exists(GetLibraryPath())) {
		BinaryDeserialize();
	}
	else if (App->fs->Exists(GetAssetPath())) {
		JsonDeserialize();
		BinarySerialize();
	}
	else {
		LOG_ERROR("Material %s not found on project", GetName());
	}
}

void RE_Material::UnloadMemory()
{
	//TODO ResourcesManager, apply count referending textures
	tDiffuse.clear();
	tSpecular.clear();
	tAmbient.clear();
	tEmissive.clear();
	tOpacity.clear();
	tShininess.clear();
	tHeight.clear();
	tNormals.clear();
	tReflection.clear();
	tUnknown.clear();

	cDiffuse = math::float3::zero;
	cSpecular = math::float3::zero;
	cAmbient = math::float3::zero;
	cEmissive = math::float3::zero;
	cTransparent = math::float3::zero;

	backFaceCulling = true;
	blendMode = false;

	opacity = 1.0f;
	shininess = 0.f;
	shininessStrenght = 1.0f;
	refraccti = 1.0f;

	ResourceContainer::inMemory = false;
}

void RE_Material::Save()
{
	JsonSerialize();
	BinarySerialize();
	SaveMeta();
}

void RE_Material::Draw()
{
	if (whereToApply && !App->editor->GetSelectWindow()->IsActive()) {
		ResourceContainer* textureToApply = App->editor->GetSelectWindow()->GetSelectedTexture();

		if (changeToApply != whereToApply->end()) {
			*changeToApply = textureToApply->GetMD5();
			changeToApply = whereToApply->end();
		}
		else {
			whereToApply->push_back(textureToApply->GetMD5());
		}
		applySave = true;
		whereToApply = nullptr;
	}

	if (ImGui::Button("Save Changes")) {
		if (applySave) {

			Save();

			applySave = false;
		}
	}

	int shadingMode = shadingType;
	if (ImGui::ListBox("Shader type", &shadingMode, shadingItems, 10)) {

		if (shadingMode != shadingType) {
			shadingType = (RE_ShadingMode)shadingMode;
			applySave = true;

			// TODO ResourcesManager count referending textures
		}
	}

	if (ImGui::ColorEdit3("Diffuse Color", &cDiffuse[0]))
		applySave = true;
	DrawTextures("Diffuse textures", &tDiffuse);

	if (ImGui::ColorEdit3("Specular Color", &cSpecular[0]))
		applySave = true;
	DrawTextures("Specular textures", &tSpecular);

	if (ImGui::ColorEdit3("Ambient Color", &cAmbient[0]))
		applySave = true;
	DrawTextures("Ambient textures", &tAmbient);

	if (ImGui::ColorEdit3("Emissive Color", &cEmissive[0]))
		applySave = true;
	DrawTextures("Emissive textures", &tEmissive);

	if (ImGui::ColorEdit3("Transparent Color", &cTransparent[0]))
		applySave = true;

	if(ImGui::Checkbox("Back face culling", &backFaceCulling))
		applySave = true;

	if (ImGui::Checkbox("Blend mode", &blendMode))
		applySave = true;

	if(ImGui::InputFloat("Opacity", &opacity))
		applySave = true;
	DrawTextures("Opacity textures", &tEmissive);

	if (ImGui::InputFloat("Shininess", &shininess))
		applySave = true;
	if (ImGui::InputFloat("Shininess strenght", &shininessStrenght))
		applySave = true;
	DrawTextures("Shininess textures", &tShininess);

	if (ImGui::InputFloat("Refraccti", &refraccti))
		applySave = true;

	DrawTextures("Height textures", &tHeight);
	DrawTextures("Normals textures", &tNormals);
	DrawTextures("Reflection textures", &tReflection);
	DrawTextures("Unknown textures", &tUnknown);
}

void RE_Material::DrawTextures(const char* texturesName, std::vector<const char*>* textures)
{
	ImGui::Text(texturesName);
	ImGui::Separator();
	if (!textures->empty()) {
		std::vector<const char*>::iterator toDelete = textures->end();
		for (std::vector<const char*>::iterator md5 = textures->begin(); md5 != textures->end(); ++md5) {
			ResourceContainer* resource = App->resources->At(*md5);

			ImGui::Text("Texture Name: %s\nAssets path: %s", resource->GetName(), resource->GetAssetPath());

			if (ImGui::Button("Change Texture")) {
				changeToApply = md5;
				whereToApply = textures;
				App->editor->GetSelectWindow()->SelectTexture();
			}

			if (ImGui::Button("Delete Texture")) {
				toDelete = md5;
				applySave = true;
			}
		}
		if (toDelete != textures->end()) textures->erase(toDelete);
		ImGui::Separator();
	}
	else {
		ImGui::Text("Empty textures");
	}

	if (ImGui::Button("Add Texture")) {
		whereToApply = textures;
		changeToApply = textures->end();
		App->editor->GetSelectWindow()->SelectTexture();
	}
}

void RE_Material::JsonDeserialize()
{
	Config material(GetAssetPath(), App->fs->GetZipPath());
	if (material.Load()) {
		JSONNode* nodeMat = material.GetRootNode("Material");

		shadingType = (RE_ShadingMode)nodeMat->PullInt("Shader Type", RE_ShadingMode::S_FLAT);

		JSONNode* diffuseNode = nodeMat->PullJObject("DiffuseTextures");
		PullTexturesJson(diffuseNode, &tDiffuse);
		DEL(diffuseNode);
		cDiffuse = nodeMat->PullFloatVector("DiffuseColor", math::vec::zero);

		JSONNode* specularNode = nodeMat->PullJObject("SpecularTextures");
		PullTexturesJson(specularNode, &tSpecular);
		DEL(specularNode);
		cSpecular = nodeMat->PullFloatVector("SpecularColor", math::vec::zero);

		JSONNode* ambientNode = nodeMat->PullJObject("AmbientTextures");
		PullTexturesJson(ambientNode, &tAmbient);
		DEL(ambientNode);
		cAmbient = nodeMat->PullFloatVector("AmbientColor", math::vec::zero);

		JSONNode* emissiveNode = nodeMat->PullJObject("EmissiveTextures");
		PullTexturesJson(emissiveNode, &tEmissive);
		DEL(emissiveNode);
		cEmissive = nodeMat->PullFloatVector("EmissiveColor", math::vec::zero);

		cTransparent = nodeMat->PullFloatVector("TransparentColor", math::vec::zero);

		backFaceCulling = nodeMat->PullBool("BackFaceCulling", true);

		blendMode = nodeMat->PullBool("BlendMode", false);

		JSONNode* opacityNode = nodeMat->PullJObject("OpacityTextures");
		uint opacitySize = opacityNode->PullInt("Size", 0);
		PullTexturesJson(opacityNode, &tOpacity);
		DEL(opacityNode);
		opacity = nodeMat->PullFloat("Opacity", 1.0f);

		JSONNode* shininessNode = nodeMat->PullJObject("ShininessTextures");
		PullTexturesJson(shininessNode, &tShininess);
		DEL(shininessNode);
		shininess = nodeMat->PullFloat("Shininess", 0.0f);
		shininessStrenght = nodeMat->PullFloat("ShininessStrenght", 1.0f);

		refraccti = nodeMat->PullFloat("Refraccti", 1.0f);

		JSONNode* heightNode = nodeMat->PullJObject("HeightTextures");
		PullTexturesJson(heightNode, &tHeight);
		DEL(heightNode);

		JSONNode* normalsNode = nodeMat->PullJObject("NormalsTextures");
		PullTexturesJson(normalsNode, &tNormals);
		DEL(normalsNode);

		JSONNode* reflectionNode = nodeMat->PullJObject("ReflectionTextures");
		PullTexturesJson(reflectionNode, &tReflection);
		DEL(reflectionNode);

		JSONNode* unknownNode = nodeMat->PullJObject("UnknownTextures");
		PullTexturesJson(unknownNode, &tUnknown);
		DEL(unknownNode);

		DEL(nodeMat);

		ResourceContainer::inMemory = true;
	}
}

void RE_Material::PullTexturesJson(JSONNode * texturesNode, std::vector<const char*>* textures)
{
	uint texturesSize = texturesNode->PullInt("Size", 0);
	for (uint i = 0; i < texturesSize; i++) {
		std::string idref = "MetaPath";
		idref += std::to_string(i).c_str();
		std::string textureMaT = texturesNode->PullString(idref.c_str(), "");
		const char* textureMD5 = App->resources->FindMD5ByMETAPath(textureMaT.c_str());
		if (textureMD5) textures->push_back(textureMD5);
		else LOG_ERROR("No texture found.\nPath: %s", textureMaT.c_str());
	}
}

void RE_Material::JsonSerialize()
{
	Config materialSerialize(GetAssetPath(), App->fs->GetZipPath());

	JSONNode* materialNode = materialSerialize.GetRootNode("Material");

	materialNode->PushInt("ShaderType", (int)shadingType);

	JSONNode* diffuseNode = materialNode->PushJObject("DiffuseTextures");
	PushTexturesJson(diffuseNode, &tDiffuse);
	DEL(diffuseNode);
	materialNode->PushFloatVector("DiffuseColor", cDiffuse);

	JSONNode* specularNode = materialNode->PushJObject("SpecularTextures");
	PushTexturesJson(specularNode, &tSpecular);
	DEL(specularNode);
	materialNode->PushFloatVector("SpecularColor", cSpecular);

	JSONNode* ambientNode = materialNode->PushJObject("SpecularTextures");
	PushTexturesJson(ambientNode, &tAmbient);
	DEL(ambientNode);
	materialNode->PushFloatVector("AmbientColor", cAmbient);

	JSONNode* emissiveNode = materialNode->PushJObject("EmissiveTextures");
	PushTexturesJson(emissiveNode, &tEmissive);
	DEL(emissiveNode);
	materialNode->PushFloatVector("EmissiveColor", cEmissive);

	materialNode->PushFloatVector("TransparentColor", cTransparent);

	materialNode->PushBool("BackFaceCulling", backFaceCulling);

	materialNode->PushBool("BlendMode", blendMode);

	JSONNode*opacityNode = materialNode->PushJObject("OpacityTextures");
	PushTexturesJson(opacityNode, &tOpacity);
	DEL(opacityNode);
	materialNode->PushFloat("Opacity", opacity);
	
	JSONNode* shininessNode = materialNode->PushJObject("ShininessTextures");
	PushTexturesJson(shininessNode, &tShininess);
	DEL(shininessNode);
	materialNode->PushFloat("Shininess", shininess);
	materialNode->PushFloat("ShininessStrenght", shininessStrenght);

	materialNode->PushFloat("Refraccti", refraccti);

	JSONNode* heightNode = materialNode->PushJObject("HeightTextures");
	PushTexturesJson(heightNode, &tHeight);
	DEL(heightNode);

	JSONNode* normalsNode = materialNode->PushJObject("NormalsTextures");
	PushTexturesJson(normalsNode, &tNormals);
	DEL(normalsNode);

	JSONNode* reflectionNode = materialNode->PushJObject("ReflectionTextures");
	PushTexturesJson(reflectionNode, &tReflection);
	DEL(reflectionNode);

	JSONNode* unknownNode = materialNode->PushJObject("UnknownTextures");
	PushTexturesJson(unknownNode, &tUnknown);
	DEL(unknownNode);

	materialSerialize.Save();
	SetMD5(materialSerialize.GetMd5().c_str());

	std::string libraryPath("Library/Materials/");
	libraryPath += GetMD5();
	SetLibraryPath(libraryPath.c_str());

	DEL(materialNode);
}

void RE_Material::PushTexturesJson(JSONNode * texturesNode, std::vector<const char*>* textures)
{
	uint texturesSize = textures->size();
	texturesNode->PushUInt("Size", texturesSize);
	for (uint i = 0; i < texturesSize; i++) {
		std::string idref = "MetaPath";
		idref += std::to_string(i).c_str();
		texturesNode->PushString(idref.c_str(), App->resources->At(textures->operator[](i))->GetMetaPath());
	}
}

void RE_Material::BinaryDeserialize()
{
	RE_FileIO libraryFile(GetLibraryPath());
	if (libraryFile.Load()) {
		char* cursor = libraryFile.GetBuffer();

		size_t size = sizeof(RE_ShadingMode);
		memcpy(&shadingType, cursor, size);
		cursor += size;

		DeserializeTexturesBinary(cursor, &tDiffuse);
		size = sizeof(math::float3);
		memcpy(&cDiffuse, cursor, size);
		cursor += size;

		DeserializeTexturesBinary(cursor, &tSpecular);
		size = sizeof(math::float3);
		memcpy(&cSpecular, cursor, size);
		cursor += size;

		DeserializeTexturesBinary(cursor, &tAmbient);
		size = sizeof(math::float3);
		memcpy(&cAmbient, cursor, size);
		cursor += size;

		DeserializeTexturesBinary(cursor, &tEmissive);
		size = sizeof(math::float3);
		memcpy(&cEmissive, cursor, size);
		cursor += size;

		memcpy(&cTransparent, cursor, size);
		cursor += size;

		size = sizeof(bool);
		memcpy(&backFaceCulling, cursor, size);
		cursor += size;

		memcpy(&blendMode, cursor, size);
		cursor += size;

		DeserializeTexturesBinary(cursor, &tOpacity);
		size = sizeof(float);
		memcpy(&opacity, cursor, size);
		cursor += size;

		DeserializeTexturesBinary(cursor, &tShininess);
		size = sizeof(float);
		memcpy(&shininess, cursor, size);
		cursor += size;
		memcpy(&shininessStrenght, cursor, size);
		cursor += size;

		memcpy(&refraccti, cursor, size);
		cursor += size;

		DeserializeTexturesBinary(cursor, &tHeight);

		DeserializeTexturesBinary(cursor, &tNormals);

		DeserializeTexturesBinary(cursor, &tReflection);

		DeserializeTexturesBinary(cursor, &tUnknown);

		ResourceContainer::inMemory = true;
	}
}

void RE_Material::DeserializeTexturesBinary(char * &cursor, std::vector<const char*>* textures)
{
	size_t size = sizeof(uint);
	uint vectorSize = 0;
	memcpy(&vectorSize, cursor, size);
	cursor += size;
	for (uint i = 0; i < vectorSize; i++) {
		size = sizeof(uint);
		uint pathSize = 0;
		memcpy(&pathSize, cursor, size);
		cursor += size;
		size = sizeof(char) * pathSize;
		std::string metaPath(cursor, pathSize);
		cursor += size;
		const char* textureMD5 = App->resources->FindMD5ByMETAPath(metaPath.c_str());
		if (textureMD5) textures->push_back(textureMD5);
		else LOG_ERROR("No texture found.\nPath: %s", metaPath.c_str());
	}
}

void RE_Material::BinarySerialize()
{
	RE_FileIO libraryFile(GetLibraryPath(), App->fs->GetZipPath());

	uint bufferSize = GetBinarySize();
	char* buffer = new char[bufferSize];
	char* cursor = buffer;

	size_t size = sizeof(RE_ShadingMode);
	memcpy(cursor, &shadingType, size);
	cursor += size;

	SerializeTexturesBinary(cursor, &tDiffuse);
	size = sizeof(math::float3);
	memcpy(cursor, &cDiffuse, size);
	cursor += size;

	SerializeTexturesBinary(cursor, &tSpecular);
	size = sizeof(math::float3);
	memcpy(cursor, &cSpecular, size);
	cursor += size;

	SerializeTexturesBinary(cursor, &tAmbient);
	size = sizeof(math::float3);
	memcpy(cursor, &cAmbient, size);
	cursor += size;

	SerializeTexturesBinary(cursor, &tEmissive);
	size = sizeof(math::float3);
	memcpy(cursor, &cEmissive, size);
	cursor += size;

	memcpy(cursor, &cTransparent, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(cursor, &backFaceCulling, size);
	cursor += size;

	memcpy(cursor, &blendMode, size);
	cursor += size;

	SerializeTexturesBinary(cursor, &tOpacity);
	size = sizeof(float);
	memcpy(cursor, &opacity, size);
	cursor += size;

	SerializeTexturesBinary(cursor, &tShininess);
	size = sizeof(float);
	memcpy(cursor, &shininess, size);
	cursor += size;
	memcpy(cursor, &shininessStrenght, size);
	cursor += size;

	memcpy(cursor, &refraccti, size);
	cursor += size;

	SerializeTexturesBinary(cursor, &tHeight);

	SerializeTexturesBinary(cursor, &tNormals);

	SerializeTexturesBinary(cursor, &tReflection);

	SerializeTexturesBinary(cursor, &tUnknown);

	libraryFile.Save(buffer, bufferSize);
}

void RE_Material::SerializeTexturesBinary(char * &cursor, std::vector<const char*>* textures)
{
	size_t size = sizeof(uint);
	uint vectorSize = tDiffuse.size();
	memcpy(cursor, &vectorSize, size);
	cursor += size;
	for (auto md5 : *textures) {
		ResourceContainer* resource = App->resources->At(md5);
		const char* metaPath = resource->GetMetaPath();
		uint sizePath = std::strlen(metaPath);
		size = sizeof(uint);
		memcpy(cursor, &sizePath, size);
		cursor += size;
		size = sizeof(char) * std::strlen(metaPath);
		memcpy(cursor, metaPath, size);
		cursor += size;
	}
}

unsigned int RE_Material::GetBinarySize()
{
	uint charCount = 0;
	
	for (auto md5 : tDiffuse) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	for (auto md5 : tSpecular) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	for (auto md5 : tAmbient) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	for (auto md5 : tEmissive) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	for (auto md5 : tOpacity) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	for (auto md5 : tShininess) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	for (auto md5 : tHeight) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	for (auto md5 : tNormals) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	for (auto md5 : tReflection) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	for (auto md5 : tUnknown) {
		charCount += std::strlen(App->resources->At(md5)->GetMetaPath());
	}

	return sizeof(uint) * 10 + sizeof(char) * charCount + sizeof(RE_ShadingMode) + sizeof(float) * 4 + sizeof(math::float3) * 5 + sizeof(bool) * 2;
}
