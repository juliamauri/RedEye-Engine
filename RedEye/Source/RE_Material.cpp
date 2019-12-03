#include "RE_Material.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "OutputLog.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_ShaderImporter.h"
#include "RE_Shader.h"
#include "RE_Texture.h"

#include "ModuleEditor.h"
#include "EditorWindows.h"

#include "Globals.h"

#include "ImGui/imgui.h"
#include "Glew/include/glew.h"

RE_Material::RE_Material() { }

RE_Material::RE_Material(const char* metapath) : ResourceContainer(metapath) { }
     

RE_Material::~RE_Material() { }

void RE_Material::LoadInMemory()
{
	//TODO ResourcesManager, apply count referending textures
	if (App->fs->Exists(GetLibraryPath())) {
		BinaryDeserialize();
	}
	if (App->fs->Exists(GetAssetPath())) {
		JsonDeserialize();
		BinarySerialize();
	}
	else if (isInternal())
		ResourceContainer::inMemory = true;
	else {
		LOG_ERROR("Material %s not found on project", GetName());
	}
}

void RE_Material::UnloadMemory()
{
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
	JsonSerialize();
	BinarySerialize();
	SaveMeta();
	applySave = false;
	UnloadMemory();
}

void RE_Material::Draw()
{
	if (!isInternal() && applySave && ImGui::Button("Save Changes")) {
		Save();
		applySave = false;
	}

	DrawMaterialEdit();
}

void RE_Material::SaveResourceMeta(JSONNode* metaNode)
{
	metaNode->PushString("shaderMeta", (shaderMD5) ? App->resources->At(shaderMD5)->GetMetaPath() : "NOMETAPATH");

	JSONNode* diffuseNode = metaNode->PushJObject("DiffuseTextures");
	PushTexturesJson(diffuseNode, &tDiffuse);
	DEL(diffuseNode);
	JSONNode* specularNode = metaNode->PushJObject("SpecularTextures");
	PushTexturesJson(specularNode, &tSpecular);
	DEL(specularNode);
	JSONNode* ambientNode = metaNode->PushJObject("AmbientTextures");
	PushTexturesJson(ambientNode, &tAmbient);
	DEL(ambientNode);
	JSONNode* emissiveNode = metaNode->PushJObject("EmissiveTextures");
	PushTexturesJson(emissiveNode, &tEmissive);
	DEL(emissiveNode);
	JSONNode* opacityNode = metaNode->PushJObject("OpacityTextures");
	PushTexturesJson(opacityNode, &tOpacity);
	DEL(opacityNode);
	JSONNode* shininessNode = metaNode->PushJObject("ShininessTextures");
	PushTexturesJson(shininessNode, &tShininess);
	DEL(shininessNode);
	JSONNode* heightNode = metaNode->PushJObject("HeightTextures");
	PushTexturesJson(heightNode, &tHeight);
	DEL(heightNode);
	JSONNode* normalsNode = metaNode->PushJObject("NormalsTextures");
	PushTexturesJson(normalsNode, &tNormals);
	DEL(normalsNode);
	JSONNode* reflectionNode = metaNode->PushJObject("ReflectionTextures");
	PushTexturesJson(reflectionNode, &tReflection);
	DEL(reflectionNode);
	JSONNode* unknownNode = metaNode->PushJObject("UnknownTextures");
	PushTexturesJson(unknownNode, &tUnknown);
	DEL(unknownNode);
}

void RE_Material::LoadResourceMeta(JSONNode* metaNode)
{
	std::string shaderMeta = metaNode->PullString("shaderMeta", "NOMETAPATH");
	if (shaderMeta.compare("NOMETAPATH") != 0) shaderMD5 = App->resources->FindMD5ByMETAPath(shaderMeta.c_str(), Resource_Type::R_SHADER);

	JSONNode* diffuseNode = metaNode->PullJObject("DiffuseTextures");
	PullTexturesJson(diffuseNode, &tDiffuse);
	DEL(diffuseNode);
	JSONNode* specularNode = metaNode->PullJObject("SpecularTextures");
	PullTexturesJson(specularNode, &tSpecular);
	DEL(specularNode);
	JSONNode* ambientNode = metaNode->PullJObject("AmbientTextures");
	PullTexturesJson(ambientNode, &tAmbient);
	DEL(ambientNode);
	JSONNode* emissiveNode = metaNode->PullJObject("EmissiveTextures");
	PullTexturesJson(emissiveNode, &tEmissive);
	DEL(emissiveNode);
	JSONNode* opacityNode = metaNode->PullJObject("OpacityTextures");
	PullTexturesJson(opacityNode, &tOpacity);
	DEL(opacityNode);
	JSONNode* shininessNode = metaNode->PullJObject("ShininessTextures");
	PullTexturesJson(shininessNode, &tShininess);
	DEL(shininessNode);
	JSONNode* heightNode = metaNode->PullJObject("HeightTextures");
	PullTexturesJson(heightNode, &tHeight);
	DEL(heightNode);
	JSONNode* normalsNode = metaNode->PullJObject("NormalsTextures");
	PullTexturesJson(normalsNode, &tNormals);
	DEL(normalsNode);
	JSONNode* reflectionNode = metaNode->PullJObject("ReflectionTextures");
	PullTexturesJson(reflectionNode, &tReflection);
	DEL(reflectionNode);
	JSONNode* unknownNode = metaNode->PullJObject("UnknownTextures");
	PullTexturesJson(unknownNode, &tUnknown);
	DEL(unknownNode);
}

void RE_Material::DrawMaterialEdit()
{
	static int shadingMode = shadingType;
	RE_Shader* matShader = (shaderMD5) ? (RE_Shader*)App->resources->At(shaderMD5) : (RE_Shader*)App->resources->At(App->internalResources->GetDefaultShader());
	
	ImGui::Text("Shader selected: %s", matShader->GetMD5());
	
	if (!shaderMD5) ImGui::Text("This shader is using the default shader.");

	if (ImGui::Button(matShader->GetName()))
		App->resources->PushSelected(matShader->GetMD5(), (App->resources->GetSelected() != nullptr && (App->resources->At(App->resources->GetSelected())->GetType() == Resource_Type::R_TEXTURE || App->resources->At(App->resources->GetSelected())->GetType() == Resource_Type::R_SHADER)));

	if (shaderMD5) {
		ImGui::SameLine();
		if (ImGui::Button("Back to Default Shader")) {
			shaderMD5 = nullptr;
			applySave = true;
		}
	}

	if (ImGui::BeginMenu("Change shader"))
	{
		std::vector<ResourceContainer*> shaders = App->resources->GetResourcesByType(Resource_Type::R_SHADER);
		bool none = true;
		for (auto  shader :  shaders) {
			if (shader->isInternal())
				continue;
			none = false;
			if (ImGui::MenuItem(shader->GetName())) {
				if (ResourceContainer::inMemory && shaderMD5) App->resources->UnUse(shaderMD5);
				shaderMD5 = shader->GetMD5();
				if (ResourceContainer::inMemory && shaderMD5) App->resources->Use(shaderMD5);
				applySave = true;
			}
		}
		if (none) ImGui::Text("No custom shaders on assets");
		ImGui::EndMenu();
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Diffuse values")) {
		if (ImGui::ColorEdit3("Diffuse Color", &cDiffuse[0]))
			applySave = true;
		DrawTextures("Diffuse", &tDiffuse);

		ImGui::EndMenu();
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Specular values")) {
		if (ImGui::ColorEdit3("Specular Color", &cSpecular[0]))
			applySave = true;
		DrawTextures("Specular", &tSpecular);
		ImGui::EndMenu();
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Ambient values")) {
		if (ImGui::ColorEdit3("Ambient Color", &cAmbient[0]))
			applySave = true;
		DrawTextures("Ambient", &tAmbient);
		ImGui::EndMenu();
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Emissive values")) {
		if (ImGui::ColorEdit3("Emissive Color", &cEmissive[0]))
			applySave = true;
		DrawTextures("Emissive", &tEmissive);
		ImGui::EndMenu();
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Opacity values")) {
		if (ImGui::InputFloat("Opacity", &opacity))
			applySave = true;
		DrawTextures("Opacity", &tOpacity);
		ImGui::EndMenu();
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Shininess values")) {
		if (ImGui::InputFloat("Shininess", &shininess))
			applySave = true;
		if (ImGui::InputFloat("Shininess strenght", &shininessStrenght))
			applySave = true;
		DrawTextures("Shininess", &tShininess);
		ImGui::EndMenu();
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Height values")) {
		DrawTextures("Height", &tHeight);
		ImGui::EndMenu();
	}
	ImGui::Separator();
	if (ImGui::BeginMenu("Normals values")) {
		DrawTextures("Normals", &tNormals);
		ImGui::EndMenu();
	}
	ImGui::Separator();
	if (ImGui::BeginMenu("Reflection values")) {
		DrawTextures("Reflection", &tReflection);
		ImGui::EndMenu();
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Others values")) {
		if (ImGui::ColorEdit3("Transparent", &cTransparent[0]))
			applySave = true;
		ImGui::Separator();
		if (ImGui::Checkbox("Back face culling", &backFaceCulling))
			applySave = true;
		ImGui::Separator();
		if (ImGui::Checkbox("Blend mode", &blendMode))
			applySave = true;
		ImGui::Separator();
		if (ImGui::InputFloat("Refraccti", &refraccti))
			applySave = true;
		ImGui::Separator();
		DrawTextures("Unknown", &tUnknown);
		ImGui::EndMenu();
	}
}

void RE_Material::DrawTextures(const char* texturesName, std::vector<const char*>* textures)
{
	ImGui::Text(std::string(texturesName + std::string(" textures:")).c_str());
	if (!textures->empty()) {
		ImGui::Separator();
		std::vector<const char*>::iterator toDelete = textures->end();
		uint count = 1;
		for (std::vector<const char*>::iterator md5 = textures->begin(); md5 != textures->end(); ++md5) {
			ResourceContainer* resource = App->resources->At(*md5);

			if (ImGui::Button(std::string("Texture #" + std::to_string(count) + " " + std::string(resource->GetName())).c_str()))
				App->resources->PushSelected(*md5, (App->resources->GetSelected() != nullptr && (App->resources->At(App->resources->GetSelected())->GetType() == Resource_Type::R_TEXTURE || App->resources->At(App->resources->GetSelected())->GetType() == Resource_Type::R_SHADER)));
			
			ImGui::SameLine();
			std::string deletetexture("Delete #");
			deletetexture += std::to_string(count);
			if (ImGui::Button(deletetexture.c_str())) {
				toDelete = md5;
				applySave = true;
			}
			ImGui::SameLine();
			std::string changetexture("Change #");
			changetexture += std::to_string(count++);
			if (ImGui::BeginMenu(changetexture.c_str()))
			{
				std::vector<ResourceContainer*> allTex = App->resources->GetResourcesByType(Resource_Type::R_TEXTURE);
				for (auto textRes : allTex) {
					if (ImGui::MenuItem(textRes->GetName())) {
						if (ResourceContainer::inMemory) App->resources->UnUse(*md5);
						*md5 = textRes->GetMD5();
						if (ResourceContainer::inMemory) App->resources->Use(*md5);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
		}
		if (toDelete != textures->end()) {
			if(ResourceContainer::inMemory) App->resources->UnUse(*toDelete);
			textures->erase(toDelete);
		}
	}
	else {
		ImGui::Text("Empty textures");
	}
	std::string addtexture("Add #");
	addtexture += texturesName;
	addtexture += " texture";
	if (ImGui::BeginMenu(addtexture.c_str()))
	{
		std::vector<ResourceContainer*> allTex = App->resources->GetResourcesByType(Resource_Type::R_TEXTURE);
		for (auto textRes : allTex) {
			if (ImGui::MenuItem(textRes->GetName())) {
				textures->push_back(textRes->GetMD5());
				if (ResourceContainer::inMemory) App->resources->Use(textures->back());
			}
		}
		ImGui::EndMenu();
	}
}

void RE_Material::JsonDeserialize(bool generateLibraryPath)
{
	Config material(GetAssetPath(), App->fs->GetZipPath());
	if (material.Load()) {
		JSONNode* nodeMat = material.GetRootNode("Material");

		shadingType = (RE_ShadingMode)nodeMat->PullInt("ShaderType", RE_ShadingMode::S_FLAT);

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

		DEL(nodeMat);

		if (generateLibraryPath) {
			SetMD5(material.GetMd5().c_str());
			std::string libraryPath("Library/Materials/");
			libraryPath += GetMD5();
			SetLibraryPath(libraryPath.c_str());
		}

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

void RE_Material::JsonSerialize(bool onlyMD5)
{
	Config materialSerialize(GetAssetPath(), App->fs->GetZipPath());

	JSONNode* materialNode = materialSerialize.GetRootNode("Material");

	materialNode->PushString("name", GetName()); //for get different md5

	materialNode->PushInt("ShaderType", (int)shadingType);

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

	if(!onlyMD5) materialSerialize.Save();
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

		size = sizeof(float) * 3;
		cDiffuse = math::vec((float*)cursor);
		cursor += size;

		cSpecular = math::vec((float*)cursor);
		cursor += size;

		cAmbient = math::vec((float*)cursor);
		cursor += size;

		cEmissive = math::vec((float*)cursor);
		cursor += size;

		cTransparent = math::vec((float*)cursor);
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

		ResourceContainer::inMemory = true;
	}
}

void RE_Material::BinarySerialize()
{
	RE_FileIO libraryFile(GetLibraryPath(), App->fs->GetZipPath());

	uint bufferSize = GetBinarySize();
	char* buffer = new char[bufferSize + 1];
	char* cursor = buffer;

	size_t size = sizeof(RE_ShadingMode);
	memcpy(cursor, &shadingType, size);
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

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	libraryFile.Save(buffer, bufferSize + 1);
	DEL_A(buffer);
}

void RE_Material::UseTextureResources()
{
	for (auto t : tDiffuse) App->resources->Use(t);
	for (auto t : tSpecular) App->resources->Use(t);
	for (auto t : tAmbient) App->resources->Use(t);
	for (auto t : tEmissive) App->resources->Use(t);
	for (auto t : tOpacity) App->resources->Use(t);
	for (auto t : tShininess) App->resources->Use(t);
	for (auto t : tHeight) App->resources->Use(t);
	for (auto t : tNormals) App->resources->Use(t);
	for (auto t : tReflection) App->resources->Use(t);
	for (auto t : tUnknown) App->resources->Use(t);
}

void RE_Material::UnUseTextureResources()
{
	for (auto t : tDiffuse) App->resources->UnUse(t);
	for (auto t : tSpecular) App->resources->UnUse(t);
	for (auto t : tAmbient) App->resources->UnUse(t);
	for (auto t : tEmissive) App->resources->UnUse(t);
	for (auto t : tOpacity) App->resources->UnUse(t);
	for (auto t : tShininess) App->resources->UnUse(t);
	for (auto t : tHeight) App->resources->UnUse(t);
	for (auto t : tNormals) App->resources->UnUse(t);
	for (auto t : tReflection) App->resources->UnUse(t);
	for (auto t : tUnknown) App->resources->UnUse(t);

}

void RE_Material::UploadToShader(float* model, bool usingChekers)
{
	const char* usingShader = (shaderMD5) ? shaderMD5 : App->internalResources->GetDefaultShader();
	RE_Shader* shaderRes = (RE_Shader*)App->resources->At(usingShader);
	shaderRes->UploadModel(model);

	unsigned int ShaderID = shaderRes->GetID();
	// Bind Textures
	if (usingChekers || tDiffuse.empty())
	{
		RE_ShaderImporter::setFloat(ShaderID, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(ShaderID, "useTexture", 1.0f);

		usingChekers = true;
		glActiveTexture(GL_TEXTURE0);
		std::string name = "texture_diffuse0";
		RE_ShaderImporter::setUnsignedInt(ShaderID, name.c_str(), 0);
		glBindTexture(GL_TEXTURE_2D, App->internalResources->GetTextureChecker());
	}
	else if (!tDiffuse.empty())
	{
		// Bind diffuse textures
		unsigned int diffuseNr = 1;

		RE_ShaderImporter::setFloat(ShaderID, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(ShaderID, "useTexture", 1.0f);

		for (unsigned int i = 0; i < tDiffuse.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			std::string name = "texture_diffuse";
			name += std::to_string(diffuseNr++);
			RE_ShaderImporter::setUnsignedInt(ShaderID, name.c_str(), i);
			((RE_Texture*)App->resources->At(tDiffuse[i]))->use();
		}
		/* TODO DRAW WITH MATERIAL

			unsigned int specularNr = 1;
			unsigned int normalNr = 1;
			unsigned int heightNr = 1;

				if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++); // transfer unsigned int to stream
			else if (name == "texture_normal")
				number = std::to_string(normalNr++); // transfer unsigned int to stream
			else if (name == "texture_height")
				number = std::to_string(heightNr++); // transfer unsigned int to stream
		*/
	}

}

unsigned int RE_Material::GetShaderID() const
{
	const char* usingShader = (shaderMD5) ? shaderMD5 : App->internalResources->GetDefaultShader();
	return ((RE_Shader*)App->resources->At(usingShader))->GetID();
}

unsigned int RE_Material::GetBinarySize()
{
	uint charCount = 0;

	charCount += sizeof(RE_ShadingMode);
	charCount += sizeof(float) * 15;
	charCount += sizeof(bool) * 2;
	charCount += sizeof(float) * 4;

	return charCount;
}
