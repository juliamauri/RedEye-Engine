#include "RE_SkyBox.h"

#include "Application.h"

#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"

#include "RE_Texture.h"

#include "Globals.h"
#include "OutputLog.h"

#include "RE_TextureImporter.h"

#include "RE_GLCache.h"
#include "RE_ThumbnailManager.h"

#include "Glew/include/glew.h"

#include "ImGui/imgui.h"

#include "par_shapes.h"

#include <EASTL/vector.h>

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
	glDeleteBuffers(1, &EBO);

	ResourceContainer::inMemory = false;
}

void RE_SkyBox::use()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

void RE_SkyBox::SetAsInternal()
{
	SetInternal(true);

	Config toMD5("", "");
	JSONNode* node = toMD5.GetRootNode("skybox");
	//For differentMD5
	node->PushString("SKname", GetName());
	node->PushString("SKPath", GetAssetPath());

	node->PushFloat("skyBoxSize", skyBoxSettings.skyBoxSize);
	DEL(node);

	SetMD5(toMD5.GetMd5().c_str());

	App->textures->LoadSkyBoxInMemory(skyBoxSettings, &ID, true);
	LoadSkyBoxSphere();

	ResourceContainer::inMemory = true;
}

void RE_SkyBox::AddTexture(RE_TextureFace face, const char* textureMD5)
{
	skyBoxSettings.textures[face].textureMD5 = textureMD5;
}

void RE_SkyBox::AddTexturePath(RE_TextureFace face, const char* path)
{
	skyBoxSettings.textures[face].path = path;
}

void RE_SkyBox::Draw()
{
	if (applySave && !isInternal()) {
		if (ImGui::Button("Save")) {
			restoreSettings = skyBoxSettings;
			if (skyBoxSettings.texturesChanged(restoreSettings) || skyBoxSettings.skyBoxSize != restoreSettings.skyBoxSize)
				AssetSave();
			SaveMeta();

			if (ResourceContainer::inMemory) {
				if (applySize) LoadSkyBoxSphere();

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
	
	DrawEditSkyBox();

	if (ResourceContainer::inMemory && (applySize || applyTextures) && ImGui::Button((!isInternal()) ? "Apply Texture/Size Changes" : "Apply Size Changes")) {

		if (applySize) LoadSkyBoxSphere();

		if (applyTextures && !isInternal()) {
			if (ID != 0) glDeleteTextures(1, &ID);
			App->textures->LoadSkyBoxInMemory(skyBoxSettings, &ID);
		}

		applySize = false;
		applyTextures = false;
	}

	ImGui::Image((void*)App->thumbnail->At(GetMD5()), { 256, 256 }, { 0,1 }, { 1, 0 });

	if (applySave && skyBoxSettings == restoreSettings) {
		applySave = false;
	}
}

void RE_SkyBox::DrawEditSkyBox()
{
	if (ImGui::SliderFloat("SkyBox size", &skyBoxSettings.skyBoxSize, 0.0f, 10000.0f)) {

		if (ResourceContainer::inMemory) applySize = true;
		applySave = true;
	}

	ImGui::Separator();
	ImGui::Text("Textures by face:");
	static eastl::string texture = "Texture ";
	static eastl::string id;
	static RE_TextureFace toDelete = RE_NOFACE;
	for (uint i = 0; i < 6 && !isInternal(); i++) {
		if (ImGui::BeginMenu(texturesname[i])) {
			if (skyBoxSettings.textures[i].textureMD5)
			{
				ResourceContainer* resource = App->resources->At(skyBoxSettings.textures[i].textureMD5);
				id = texture + resource->GetName();
				if (ImGui::Button(id.c_str()))
					App->resources->PushSelected(resource->GetMD5());

				ImGui::SameLine();
				id = "Delete";
				if (ImGui::Button(id.c_str())) {
					if (ResourceContainer::inMemory) App->resources->Use(resource->GetMD5());
					toDelete = skyBoxSettings.textures[i].face;
					applySave = true;
				}

				id = "Change";
				if (ImGui::BeginMenu(id.c_str()))
				{
					eastl::vector<ResourceContainer*> allTex = App->resources->GetResourcesByType(Resource_Type::R_TEXTURE);
					for (auto textRes : allTex) {
						if (ImGui::MenuItem(textRes->GetName())) {
							if (ResourceContainer::inMemory) App->resources->UnUse(resource->GetMD5());
							skyBoxSettings.textures[i].textureMD5 = textRes->GetMD5();
							if (ResourceContainer::inMemory) App->resources->Use(textRes->GetMD5());
							applySave = true;
						}
					}
					ImGui::EndMenu();
				}
			}
			else {
				ImGui::Text("%s texture don't exists", texturesname[i]);
				id = "Add";
				if (ImGui::BeginMenu(id.c_str()))
				{
					eastl::vector<ResourceContainer*> allTex = App->resources->GetResourcesByType(Resource_Type::R_TEXTURE);
					for (auto textRes : allTex) {
						if (ImGui::MenuItem(textRes->GetName())) {
							skyBoxSettings.textures[i].textureMD5 = textRes->GetMD5();
							if (ResourceContainer::inMemory) App->resources->Use(textRes->GetMD5());
						}
					}
					ImGui::EndMenu();
				}
			}
			ImGui::Separator();
			ImGui::EndMenu();

			if (toDelete != RE_TextureFace::RE_NOFACE) {
				skyBoxSettings.textures[toDelete].textureMD5 = nullptr;
				toDelete = RE_NOFACE;
			}
		}

		if (ImGui::BeginDragDropTarget()) {

			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference")) {
				if (ResourceContainer::inMemory && skyBoxSettings.textures[i].textureMD5) App->resources->UnUse(skyBoxSettings.textures[i].textureMD5);
				skyBoxSettings.textures[i].textureMD5 = *static_cast<const char**>(dropped->Data);
				if (ResourceContainer::inMemory) App->resources->Use(skyBoxSettings.textures[i].textureMD5);
				applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}
	ImGui::Separator();
	ImGui::Text("OpenGL texture settings:");

	int minIndex = RE_Texture::GetComboFilter(skyBoxSettings.min_filter);
	if (ImGui::Combo("Minify filter", &minIndex, MINFCOMBO)) {
		RE_TextureFilters newfilter = RE_Texture::GetFilterCombo(minIndex);
		if (skyBoxSettings.min_filter != newfilter) {
			if (ResourceContainer::inMemory && ((skyBoxSettings.min_filter <= RE_LINEAR && newfilter > RE_LINEAR)
				|| (newfilter <= RE_LINEAR && skyBoxSettings.min_filter > RE_LINEAR))) {
				if (!isInternal()) {
					skyBoxSettings.min_filter = newfilter;
					UnloadMemory();
					LoadInMemory();
				}
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
}

bool RE_SkyBox::isFacesFilled() const
{
	bool ret = true;
	for (uint i = 0; i < MAXSKYBOXTEXTURES; i++) {
		if (skyBoxSettings.textures[i].textureMD5 == nullptr) {
			ret = false;
			break;
		}
	}
	return ret;
}

void RE_SkyBox::SaveResourceMeta(JSONNode* metaNode)
{
	metaNode->PushInt("minFilter", skyBoxSettings.min_filter);
	metaNode->PushInt("magFilter", skyBoxSettings.mag_filter);
	metaNode->PushInt("wrapS", skyBoxSettings.wrap_s);
	metaNode->PushInt("wrapT", skyBoxSettings.wrap_t);
	metaNode->PushInt("wrapR", skyBoxSettings.wrap_r);

	JSONNode* nodeTex = metaNode->PushJObject("textures");
	for (uint i = 0; i < MAXSKYBOXTEXTURES; i++) {
		eastl::string key(texturesname[i]);
		nodeTex->PushString(eastl::string(key + "textureMD5").c_str(), skyBoxSettings.textures[i].textureMD5);
	}
}

void RE_SkyBox::LoadResourceMeta(JSONNode* metaNode)
{
	skyBoxSettings.min_filter = (RE_TextureFilters)metaNode->PullInt("minFilter", RE_LINEAR);
	skyBoxSettings.mag_filter = (RE_TextureFilters)metaNode->PullInt("magFilter", RE_LINEAR);
	skyBoxSettings.wrap_s = (RE_TextureWrap)metaNode->PullInt("wrapS", RE_CLAMP_TO_EDGE);
	skyBoxSettings.wrap_t = (RE_TextureWrap)metaNode->PullInt("wrapT", RE_CLAMP_TO_EDGE);
	skyBoxSettings.wrap_r = (RE_TextureWrap)metaNode->PullInt("wrapR", RE_CLAMP_TO_EDGE);

	JSONNode* nodeTex = metaNode->PullJObject("textures");
	for (uint i = 0; i < MAXSKYBOXTEXTURES; i++) {
		eastl::string key(texturesname[i]);
		eastl::string texMD5 = nodeTex->PullString(eastl::string(key + "textureMD5").c_str(), "");
		skyBoxSettings.textures[i].textureMD5 = App->resources->IsReference(texMD5.c_str(), Resource_Type::R_TEXTURE);
	}

	restoreSettings = skyBoxSettings;
}

void RE_SkyBox::Import(bool keepInMemory)
{
	AssetLoad(true);
	LibrarySave();
	if (!keepInMemory) UnloadMemory();
}

void RE_SkyBox::AssetLoad(bool generateLibraryPath)
{
	Config toLoad(GetAssetPath(),App->fs->GetZipPath());
	if (toLoad.Load()) {
		JSONNode* node = toLoad.GetRootNode("skybox");
		skyBoxSettings.skyBoxSize = node->PullFloat("skyBoxSize", 5000);
		DEL(node);
		
		if (generateLibraryPath) {
			eastl::string newMd5 = toLoad.GetMd5();
			SetMD5(newMd5.c_str());
			eastl::string libraryPath("Library/SkyBoxes/");
			libraryPath += newMd5.c_str();
			SetLibraryPath(libraryPath.c_str());
		}
	}
	App->textures->LoadSkyBoxInMemory(skyBoxSettings, &ID);
	LoadSkyBoxSphere();
	ResourceContainer::inMemory = true;
}

void RE_SkyBox::AssetSave()
{
	eastl::string assetPath("Assets/Skyboxes/");
	assetPath += GetName();
	assetPath += ".sk";
	SetAssetPath(assetPath.c_str());
	Config toSave(assetPath.c_str(), App->fs->GetZipPath());
	JSONNode* node = toSave.GetRootNode("skybox");
	//For differentMD5
	node->PushString("SKname", GetName());
	node->PushString("SKPath", GetAssetPath());

	node->PushFloat("skyBoxSize", skyBoxSettings.skyBoxSize);
	DEL(node);

	toSave.Save();

	eastl::string newMd5 = toSave.GetMd5();
	SetMD5(newMd5.c_str());
	eastl::string libraryPath("Library/SkyBoxes/");
	libraryPath += newMd5.c_str();
	SetLibraryPath(libraryPath.c_str());

	LibrarySave();
}

void RE_SkyBox::DrawSkybox() const
{
	RE_GLCache::ChangeVAO(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void RE_SkyBox::LibraryLoad()
{
	RE_FileIO fileLibray(GetLibraryPath());

	if (fileLibray.Load()) {
		char* cursor = fileLibray.GetBuffer();

		size_t size = sizeof(float);
		memcpy( &skyBoxSettings.skyBoxSize, cursor, size);
		cursor += size;

		App->textures->LoadSkyBoxInMemory(skyBoxSettings, &ID);
		LoadSkyBoxSphere();
		ResourceContainer::inMemory = true;
	}
}

void RE_SkyBox::LibrarySave()
{
	uint totalSize = sizeof(float);
	
	char* libraryBuffer = new char[totalSize + 1];
	char* cursor = libraryBuffer;

	size_t size = sizeof(float);
	memcpy(cursor, &skyBoxSettings.skyBoxSize, size);
	cursor += size;

	RE_FileIO saveLibrary(GetLibraryPath(), App->fs->GetZipPath());

	saveLibrary.Save(libraryBuffer, totalSize + 1);
	DEL_A(libraryBuffer);
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

void RE_SkyBox::LoadSkyBoxSphere()
{
	if (VAO != 0) glDeleteBuffers(1, &VAO);
	if (VBO != 0) glDeleteBuffers(1, &VBO);
	if (EBO != 0) glDeleteBuffers(1, &EBO);


	par_shapes_mesh* sphere = par_shapes_create_parametric_sphere(24, 24);
	par_shapes_scale(sphere, skyBoxSettings.skyBoxSize, skyBoxSettings.skyBoxSize, skyBoxSettings.skyBoxSize);

	glGenVertexArrays(1, &VAO);
	RE_GLCache::ChangeVAO(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sphere->npoints * sizeof(float), sphere->points, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);

	//Invert triangle face
	eastl::vector<unsigned short> indexes;
	for (int i = sphere->ntriangles * 3 - 1; i >= 0; i--)indexes.push_back(sphere->triangles[i]);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere->ntriangles * sizeof(unsigned short) * 3, &indexes[0], GL_STATIC_DRAW);

	triangle_count =  sphere->ntriangles;

	par_shapes_free_mesh(sphere);
}
