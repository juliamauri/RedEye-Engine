#include "RE_SkyBox.h"

#include "Application.h"

#include "FileSystem.h"

#include "Globals.h"
#include "OutputLog.h"

#include "RE_TextureImporter.h"

#include "Glew/include/glew.h"

#include "ImGui/imgui.h"

#define MINFCOMBO "Nearest\0Linear\0Nearest Mipmap Nearest\0Linear Mipmap Nearest\0Nearest Mipmap Linear\0Linear Mipmap Linear"
#define MAGFOMBO "Nearest\0Linear"
#define WRAPOMBO "Repeat\0Clamp to border\0Clamp to edge\0Mirrored Repeat"

RE_SkyBox::RE_SkyBox() { }

RE_SkyBox::RE_SkyBox(const char* metaPath) :ResourceContainer(metaPath) { }

RE_SkyBox::~RE_SkyBox() { }

void RE_SkyBox::LoadInMemory()
{
	if (App->fs->Exists(GetLibraryPath()))
		LibraryLoad();
	else if (App->fs->Exists(GetAssetPath())) {
		AssetLoad();
		LibrarySave();
	}
	else {
		LOG_ERROR("SkyBox %s not found on project", GetName());
	}
}

void RE_SkyBox::UnloadMemory()
{
	glDeleteTextures(1, &ID);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	ResourceContainer::inMemory = false;
}

void RE_SkyBox::use()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

void RE_SkyBox::AddTexture(RE_TextureFace face, const char* aPath)
{
	std::string assetPath(aPath);
	std::string filename = assetPath.substr(assetPath.find_last_of("/") + 1);
	std::string extensionStr = filename.substr(filename.find_last_of(".") + 1);
	const char* extension = extensionStr.c_str();

	TextureType texType;
	if (std::strcmp(extension, "dds") == 0)
		texType = RE_DDS;
	else if (std::strcmp(extension, "png") == 0)
		texType = RE_PNG;
	else if (std::strcmp(extension, "jpg") == 0)
		texType = RE_JPG;
	else if (std::strcmp(extension, "tga") == 0)
		texType = RE_TGA;
	else if (std::strcmp(extension, "tiff") == 0)
		texType = RE_TIFF;
	else if (std::strcmp(extension, "bmp") == 0)
		texType = RE_BMP;
	else
		texType = RE_TEXTURE_UNKNOWN;

	skyBoxSettings.textures[face].assetPath = assetPath;
	skyBoxSettings.textures[face].texType = texType;
}

void RE_SkyBox::Draw()
{
	if (applySave) {
		if (ImGui::Button("Save")) {
			restoreSettings = skyBoxSettings;
			if (skyBoxSettings.texturesChanged(restoreSettings) || skyBoxSettings.skyBoxSize != restoreSettings.skyBoxSize)
				AssetSave();
			SaveMeta();

			if (ResourceContainer::inMemory) {
				if (applySize) LoadSkyBoxCube();

				if (applyTextures) {
					if (ID != 0) glDeleteTextures(1, &ID);
					App->textures->LoadSkyBoxInMemory(skyBoxSettings, &ID);
				}

				applySize = false;
				applyTextures = false;
			}

			applySave = false;
		}
		if (ImGui::Button("Restore")) {
			if (ResourceContainer::inMemory) {
				skyBoxSettings = restoreSettings;
				UnloadMemory();
				LoadInMemory();
			}
			else
				skyBoxSettings = restoreSettings;

			applySave = false;
		}
	}
	
	if (ImGui::SliderFloat("SkyBox size", &skyBoxSettings.skyBoxSize, 0.0f, 10000.0f)) {

		if(ResourceContainer::inMemory) applySize = true;
		applySave = true;
	}
		

	for (uint i = 0; i < 6; i++) {

		if (ImGui::TreeNodeEx(texturesname[i], ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) {

			if (!skyBoxSettings.textures[i].assetPath.empty())
				ImGui::Text("Path: %s", skyBoxSettings.textures[i].assetPath.c_str());
			else
				ImGui::Text("%s texture don't exists", texturesname[i]);

			//TODO Drag&Drop
			if (NULL) {
				if (ResourceContainer::inMemory) applyTextures = true;
				applySave = true;
			}
			ImGui::TreePop();
		}
	}

	int minIndex = RE_Texture::GetComboFilter(skyBoxSettings.min_filter);
	if (ImGui::Combo("Minify filter", &minIndex, MINFCOMBO)) {
		RE_TextureFilters newfilter = RE_Texture::GetFilterCombo(minIndex);
		if (skyBoxSettings.min_filter != newfilter) {
			if (ResourceContainer::inMemory && ((skyBoxSettings.min_filter <= RE_LINEAR && newfilter > RE_LINEAR)
				|| (newfilter <= RE_LINEAR && skyBoxSettings.min_filter > RE_LINEAR))) {
				skyBoxSettings.min_filter = newfilter;
				UnloadMemory();
				LoadInMemory();
			}
			else if (ResourceContainer::inMemory) {
				TexParameteri(GL_TEXTURE_MIN_FILTER, skyBoxSettings.min_filter = newfilter);
			}
			else
				skyBoxSettings.min_filter = newfilter;
			applySave = true;
		}
	}

	int magIndex = RE_Texture::GetComboFilter(skyBoxSettings.mag_filter);
	if (ImGui::Combo("Magnify filter", &magIndex, MAGFOMBO)) {
		RE_TextureFilters newfilter = RE_Texture::GetFilterCombo(magIndex);
		if (skyBoxSettings.mag_filter != newfilter) {
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_MAG_FILTER, skyBoxSettings.mag_filter = newfilter);
			applySave = true;
		}
	}

	int wrapSIndex = RE_Texture::GetComboWrap(skyBoxSettings.wrap_s);
	if (ImGui::Combo("Wrap S", &wrapSIndex, WRAPOMBO)) {
		RE_TextureWrap newwrap = RE_Texture::GetWrapCombo(wrapSIndex);
		if (skyBoxSettings.wrap_s != newwrap) {
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_WRAP_S, skyBoxSettings.wrap_s = newwrap);
			applySave = true;
		}
	}

	int wrapTIndex = RE_Texture::GetComboWrap(skyBoxSettings.wrap_t);
	if (ImGui::Combo("Wrap T", &wrapTIndex, WRAPOMBO)) {
		RE_TextureWrap newwrap = RE_Texture::GetWrapCombo(wrapTIndex);
		if (skyBoxSettings.wrap_t != newwrap) {
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_WRAP_T, skyBoxSettings.wrap_t = newwrap);
			applySave = true;
		}
	}

	int wrapRIndex = RE_Texture::GetComboWrap(skyBoxSettings.wrap_r);
	if (ImGui::Combo("Wrap R", &wrapRIndex, WRAPOMBO)) {
		RE_TextureWrap newwrap = RE_Texture::GetWrapCombo(wrapRIndex);
		if (skyBoxSettings.wrap_r != newwrap) {
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_WRAP_R, skyBoxSettings.wrap_r = newwrap);
			applySave = true;
		}
	}

	if (ResourceContainer::inMemory && (applySize || applyTextures) && ImGui::Button("Texture/Size Changes")) {

		if (applySize) LoadSkyBoxCube();

		if (applyTextures) {
			if (ID != 0) glDeleteTextures(1, &ID);
			App->textures->LoadSkyBoxInMemory(skyBoxSettings, &ID);
		}

		applySize = false;
		applyTextures = false;
	}

	if (applySave && skyBoxSettings == restoreSettings) {
		applySave = false;
	}
}

void RE_SkyBox::SaveResourceMeta(JSONNode* metaNode)
{
	metaNode->PushInt("minFilter", skyBoxSettings.min_filter);
	metaNode->PushInt("magFilter", skyBoxSettings.mag_filter);
	metaNode->PushInt("wrapS", skyBoxSettings.wrap_s);
	metaNode->PushInt("wrapT", skyBoxSettings.wrap_t);
	metaNode->PushInt("wrapR", skyBoxSettings.wrap_r);
}

void RE_SkyBox::LoadResourceMeta(JSONNode* metaNode)
{
	skyBoxSettings.min_filter = (RE_TextureFilters)metaNode->PullInt("minFilter", RE_LINEAR);
	skyBoxSettings.mag_filter = (RE_TextureFilters)metaNode->PullInt("magFilter", RE_LINEAR);
	skyBoxSettings.wrap_s = (RE_TextureWrap)metaNode->PullInt("wrapS", RE_CLAMP_TO_EDGE);
	skyBoxSettings.wrap_t = (RE_TextureWrap)metaNode->PullInt("wrapT", RE_CLAMP_TO_EDGE);
	skyBoxSettings.wrap_r = (RE_TextureWrap)metaNode->PullInt("wrapR", RE_CLAMP_TO_EDGE);
}

void RE_SkyBox::AssetLoad()
{
	Config toLoad(GetAssetPath(),App->fs->GetZipPath());
	if (toLoad.Load()) {
		JSONNode* node = toLoad.GetRootNode("skybox");
		skyBoxSettings.skyBoxSize = node->PullFloat("skyBoxSize", 5000);

		JSONNode* nodeTex = node->PullJObject("textures");
		for (uint i = 0; i < MAXSKYBOXTEXTURES; i++) {
			std::string key(texturesname[i]);
			skyBoxSettings.textures[i].assetPath = nodeTex->PullString(std::string(key + "assetPath").c_str(), "");
			skyBoxSettings.textures[i].texType = (TextureType)nodeTex->PullInt(std::string(key + "Type").c_str(), TextureType::RE_TEXTURE_UNKNOWN);
		}
		DEL(node);
		DEL(nodeTex);
	}
	App->textures->LoadSkyBoxInMemory(skyBoxSettings, &ID);
	LoadSkyBoxCube();
	ResourceContainer::inMemory = true;
}

void RE_SkyBox::AssetSave()
{
	std::string assetPath("Assets/Skyboxes/");
	assetPath += GetName();
	assetPath += ".sk";
	SetAssetPath(assetPath.c_str());
	Config toSave(assetPath.c_str(), App->fs->GetZipPath());
	JSONNode* node = toSave.GetRootNode("skybox");
	node->PushFloat("skyBoxSize", skyBoxSettings.skyBoxSize);
	JSONNode* nodeTex = node->PushJObject("textures");
	for (uint i = 0; i < MAXSKYBOXTEXTURES; i++) {
		std::string key(texturesname[i]);
		nodeTex->PushString(std::string(key + "assetPath").c_str(), skyBoxSettings.textures[i].assetPath.c_str());
		nodeTex->PushInt(std::string(key + "Type").c_str(), skyBoxSettings.textures[i].texType);
	}
	toSave.Save();

	if (App->fs->Exists(GetLibraryPath())) {

		//TODO Delete existing Librari File
	}

	std::string newMd5 = toSave.GetMd5();
	SetMD5(newMd5.c_str());
	std::string libraryPath("Library/SkyBoxes/");
	libraryPath += newMd5.c_str();
	SetLibraryPath(libraryPath.c_str());

	DEL(node);
	DEL(nodeTex);

	LibrarySave();
}

void RE_SkyBox::LibraryLoad()
{
	RE_FileIO fileLibray(GetLibraryPath());

	if (fileLibray.Load()) {
		char* textures[6];
		uint texturesSize[6];
		char* cursor = fileLibray.GetBuffer();

		size_t size = sizeof(float);
		memcpy( &skyBoxSettings.skyBoxSize, cursor, size);
		cursor += size;

		for (uint i = 0; i < MAXSKYBOXTEXTURES; i++) {
			size = sizeof(uint);
			memcpy(&texturesSize[i], cursor, size);
			cursor += size;

			textures[i] = cursor;
			cursor += texturesSize[i];
		}

		App->textures->LoadSkyBoxInMemory(textures, texturesSize, skyBoxSettings, &ID);
		LoadSkyBoxCube();
		ResourceContainer::inMemory = true;
	}
}

void RE_SkyBox::LibrarySave()
{
	std::string texturesBuffer[6];
	uint texturesSize[6] = { 0,0,0,0,0,0 };
	uint totalSize = 0;
	totalSize += sizeof(uint) * 6;
	totalSize += sizeof(float);

	for (uint i = 0; i < MAXSKYBOXTEXTURES; i++) {
		RE_FileIO assetsTex(skyBoxSettings.textures[i].assetPath.c_str());
		if (assetsTex.Load()) {
			texturesBuffer[i] = App->textures->TransformToDDS(assetsTex.GetBuffer(), assetsTex.GetSize(), skyBoxSettings.textures[i].texType, &texturesSize[i]);
			totalSize += texturesSize[i];
		}
	}

	char* libraryBuffer = new char[totalSize];
	char* cursor = libraryBuffer;

	size_t size = sizeof(float);
	memcpy(cursor, &skyBoxSettings.skyBoxSize, size);
	cursor += size;

	for (uint i = 0; i < MAXSKYBOXTEXTURES; i++) {
		size = sizeof(uint);
		memcpy(cursor, &texturesSize[i], size);
		cursor += size;

		size = texturesSize[i];
		memcpy(cursor, texturesBuffer[i].c_str(), size);
		cursor += size;
	}

	RE_FileIO saveLibrary(GetLibraryPath(), App->fs->GetZipPath());

	saveLibrary.Save(libraryBuffer, totalSize);
}

void RE_SkyBox::TexParameteri(unsigned int pname, int param)
{
	if (ResourceContainer::inMemory) {
		GLuint boundTexture = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, pname, param);
		glBindTexture(GL_TEXTURE_2D, boundTexture);
	}
}

void RE_SkyBox::LoadSkyBoxCube()
{
	if (VAO != 0) glDeleteBuffers(1, &VAO);
	if (VBO != 0) glDeleteBuffers(1, &VBO);

	float cubeSize = skyBoxSettings.skyBoxSize;
	float skyboxVertices[] = {
		// positions          
		-cubeSize,  cubeSize, -cubeSize,
		-cubeSize, -cubeSize, -cubeSize,
		 cubeSize, -cubeSize, -cubeSize,
		 cubeSize, -cubeSize, -cubeSize,
		 cubeSize,  cubeSize, -cubeSize,
		-cubeSize,  cubeSize, -cubeSize,

		-cubeSize, -cubeSize,  cubeSize,
		-cubeSize, -cubeSize, -cubeSize,
		-cubeSize,  cubeSize, -cubeSize,
		-cubeSize,  cubeSize, -cubeSize,
		-cubeSize,  cubeSize,  cubeSize,
		-cubeSize, -cubeSize,  cubeSize,

		 cubeSize, -cubeSize, -cubeSize,
		 cubeSize, -cubeSize,  cubeSize,
		 cubeSize,  cubeSize,  cubeSize,
		 cubeSize,  cubeSize,  cubeSize,
		 cubeSize,  cubeSize, -cubeSize,
		 cubeSize, -cubeSize, -cubeSize,

		-cubeSize, -cubeSize,  cubeSize,
		-cubeSize,  cubeSize,  cubeSize,
		 cubeSize,  cubeSize,  cubeSize,
		 cubeSize,  cubeSize,  cubeSize,
		 cubeSize, -cubeSize,  cubeSize,
		-cubeSize, -cubeSize,  cubeSize,

		-cubeSize,  cubeSize, -cubeSize,
		 cubeSize,  cubeSize, -cubeSize,
		 cubeSize,  cubeSize,  cubeSize,
		 cubeSize,  cubeSize,  cubeSize,
		-cubeSize,  cubeSize,  cubeSize,
		-cubeSize,  cubeSize, -cubeSize,

		-cubeSize, -cubeSize, -cubeSize,
		-cubeSize, -cubeSize,  cubeSize,
		 cubeSize, -cubeSize, -cubeSize,
		 cubeSize, -cubeSize, -cubeSize,
		-cubeSize, -cubeSize,  cubeSize,
		 cubeSize, -cubeSize,  cubeSize
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}
