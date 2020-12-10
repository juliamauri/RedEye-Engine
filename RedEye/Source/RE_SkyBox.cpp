#include "RE_SkyBox.h"

#include "RE_ConsoleLog.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "RE_TextureImporter.h"
#include "RE_SkyBoxImporter.h"
#include "RE_ResourceManager.h"
#include "RE_GLCacheManager.h"
#include "RE_ThumbnailManager.h"
#include "RE_Texture.h"

#include "Glew/include/glew.h"
#include "ImGui/imgui.h"
#include "par_shapes.h"
#include <EASTL/vector.h>

const char* RE_SkyBox::texturesname[6] = { "Right", "Left", "Top", "Bottom", "Front", "Back" };

void RE_SkyBox::LoadInMemory()
{
	if (RE_FileSystem::Exists(GetLibraryPath()))
	{
		LibraryLoad();
	}
	else if (RE_FileSystem::Exists(GetAssetPath()))
	{
		AssetLoad();
		LibrarySave();
	}
	else RE_LOG_ERROR("SkyBox %s not found in project", GetName());
}

void RE_SkyBox::UnloadMemory()
{
	glDeleteTextures(1, &ID);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	ResourceContainer::inMemory = false;
}

void RE_SkyBox::use() { glBindTexture(GL_TEXTURE_CUBE_MAP, ID); }

void RE_SkyBox::SetAsInternal()
{
	SetInternal(true);

	Config toMD5("", "");
	RE_Json* node = toMD5.GetRootNode("skybox");
	//For differentMD5
	node->PushString("SKname", GetName());
	node->PushString("SKPath", GetAssetPath());

	node->PushFloat("skyBoxSize", skyBoxSettings.skyBoxSize);
	DEL(node);

	SetMD5(toMD5.GetMd5().c_str());

	RE_SkyboxImporter::LoadSkyBoxInMemory(skyBoxSettings, &ID, true);
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
	if (applySave && !isInternal())
	{
		if (ImGui::Button("Save"))
		{
			restoreSettings = skyBoxSettings;
			if (skyBoxSettings.texturesChanged(restoreSettings) || skyBoxSettings.skyBoxSize != restoreSettings.skyBoxSize) AssetSave();
			SaveMeta();

			if (ResourceContainer::inMemory)
			{
				if (applySize) LoadSkyBoxSphere();
				if (applyTextures)
				{
					if (ID != 0) glDeleteTextures(1, &ID);
					RE_SkyboxImporter::LoadSkyBoxInMemory(skyBoxSettings, &ID);
				}

			}

			App::renderer3d->PushThumnailRend(GetMD5(), true);
			applySize = applyTextures = applySave = false;
		}
		if (ImGui::Button("Restore"))
		{
			if (ResourceContainer::inMemory)
			{
				skyBoxSettings = restoreSettings;
				UnloadMemory();
				LoadInMemory();
			}
			else skyBoxSettings = restoreSettings;

			applySize = applyTextures = applySave = false;
		}
	}
	
	DrawEditSkyBox();

	if (ResourceContainer::inMemory && (applySize || applyTextures) && ImGui::Button((!isInternal()) ? "Apply Texture/Size Changes" : "Apply Size Changes")) {

		if (applySize) LoadSkyBoxSphere();
		if (applyTextures && !isInternal())
		{
			if (ID != 0) glDeleteTextures(1, &ID);
			RE_SkyboxImporter::LoadSkyBoxInMemory(skyBoxSettings, &ID);
		}

		applySize = applyTextures = false;
	}

	ImGui::Image(reinterpret_cast<void*>(RE_ThumbnailManager::At(GetMD5())), { 256, 256 }, { 0,1 }, { 1, 0 });

	if (applySave && skyBoxSettings == restoreSettings) applySave = false;
}

void RE_SkyBox::DrawEditSkyBox()
{
	if (ImGui::SliderFloat("SkyBox size", &skyBoxSettings.skyBoxSize, 0.0f, 10000.0f))
		applySize = applySave = true;

	ImGui::Separator();
	ImGui::Text("Textures by face:");
	static eastl::string texture = "Texture ";
	static eastl::string id;
	static RE_TextureFace toDelete = RE_NOFACE;
	for (uint i = 0; i < 6 && !isInternal(); i++)
	{
		if (ImGui::BeginMenu(texturesname[i]))
		{
			if (skyBoxSettings.textures[i].textureMD5)
			{
				ResourceContainer* resource = RE_ResourceManager::At(skyBoxSettings.textures[i].textureMD5);
				id = texture + resource->GetName();
				if (ImGui::Button(id.c_str())) RE_ResourceManager::PushSelected(resource->GetMD5());

				ImGui::SameLine();
				id = "Delete";
				if (ImGui::Button(id.c_str()))
				{
					toDelete = skyBoxSettings.textures[i].face;
					applyTextures = applySave = true;
				}

				id = "Change";
				if (ImGui::BeginMenu(id.c_str()))
				{
					eastl::vector<ResourceContainer*> allTex = RE_ResourceManager::GetResourcesByType(Resource_Type::R_TEXTURE);
					for (auto textRes : allTex)
					{
						if (ImGui::MenuItem(textRes->GetName()))
						{
							skyBoxSettings.textures[i].textureMD5 = textRes->GetMD5();
							applyTextures = applySave = true;
						}
					}
					ImGui::EndMenu();
				}
			}
			else
			{
				ImGui::Text("%s texture don't exists", texturesname[i]);
				id = "Add";
				if (ImGui::BeginMenu(id.c_str()))
				{
					eastl::vector<ResourceContainer*> allTex = RE_ResourceManager::GetResourcesByType(Resource_Type::R_TEXTURE);
					for (auto textRes : allTex)
					{
						if (ImGui::MenuItem(textRes->GetName()))
						{
							skyBoxSettings.textures[i].textureMD5 = textRes->GetMD5();
							applyTextures = applySave = true;
						}
					}
					ImGui::EndMenu();
				}
			}
			ImGui::Separator();
			ImGui::EndMenu();

			if (toDelete != RE_TextureFace::RE_NOFACE)
			{
				skyBoxSettings.textures[toDelete].textureMD5 = nullptr;
				toDelete = RE_NOFACE;
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				skyBoxSettings.textures[i].textureMD5 = *static_cast<const char**>(dropped->Data);
				applyTextures = applySave = true;
			}
			ImGui::EndDragDropTarget();
		}
	}
	ImGui::Separator();
	ImGui::Text("OpenGL texture settings:");

	static const char* minf_combo = "Nearest\0Linear\0Nearest Mipmap Nearest\0Linear Mipmap Nearest\0Nearest Mipmap Linear\0Linear Mipmap Linear";
	int minIndex = RE_Texture::GetComboFilter(skyBoxSettings.min_filter);
	if (ImGui::Combo("Minify filter", &minIndex, minf_combo))
	{
		RE_TextureFilters newfilter = RE_Texture::GetFilterCombo(minIndex);
		if (skyBoxSettings.min_filter != newfilter)
		{
			if ((ResourceContainer::inMemory && ((skyBoxSettings.min_filter <= RE_LINEAR && newfilter > RE_LINEAR)
				|| (newfilter <= RE_LINEAR && skyBoxSettings.min_filter > RE_LINEAR))) && !isInternal())
			{
				skyBoxSettings.min_filter = newfilter;
				UnloadMemory();
				LoadInMemory();
			}
			else if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_MIN_FILTER, skyBoxSettings.min_filter = newfilter);
			else skyBoxSettings.min_filter = newfilter;

			applyTextures = applySave = true;
		}
	}

	static const char* mag_combo = "Nearest\0Linear";
	int magIndex = RE_Texture::GetComboFilter(skyBoxSettings.mag_filter);
	if (ImGui::Combo("Magnify filter", &magIndex, mag_combo))
	{
		RE_TextureFilters newfilter = RE_Texture::GetFilterCombo(magIndex);
		if (skyBoxSettings.mag_filter != newfilter)
		{
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_MAG_FILTER, skyBoxSettings.mag_filter = newfilter);
			applyTextures = applySave = true;
		}
	}

	static const char* wrap_combo = "Repeat\0Clamp to border\0Clamp to edge\0Mirrored Repeat";
	int wrapSIndex = RE_Texture::GetComboWrap(skyBoxSettings.wrap_s);
	if (ImGui::Combo("Wrap S", &wrapSIndex, wrap_combo))
	{
		RE_TextureWrap newwrap = RE_Texture::GetWrapCombo(wrapSIndex);
		if (skyBoxSettings.wrap_s != newwrap)
		{
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_WRAP_S, skyBoxSettings.wrap_s = newwrap);
			applyTextures = applySave = true;
		}
	}

	int wrapTIndex = RE_Texture::GetComboWrap(skyBoxSettings.wrap_t);
	if (ImGui::Combo("Wrap T", &wrapTIndex, wrap_combo))
	{
		RE_TextureWrap newwrap = RE_Texture::GetWrapCombo(wrapTIndex);
		if (skyBoxSettings.wrap_t != newwrap)
		{
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_WRAP_T, skyBoxSettings.wrap_t = newwrap);
			applyTextures = applySave = true;
		}
	}

	int wrapRIndex = RE_Texture::GetComboWrap(skyBoxSettings.wrap_r);
	if (ImGui::Combo("Wrap R", &wrapRIndex, wrap_combo))
	{
		RE_TextureWrap newwrap = RE_Texture::GetWrapCombo(wrapRIndex);
		if (skyBoxSettings.wrap_r != newwrap)
		{
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_WRAP_R, skyBoxSettings.wrap_r = newwrap);
			applyTextures = applySave = true;
		}
	}
}

bool RE_SkyBox::isFacesFilled() const
{
	bool ret = true;
	for (uint i = 0; i < 6; i++)
	{
		if (!skyBoxSettings.textures[i].textureMD5)
		{
			ret = false;
			break;
		}
	}
	return ret;
}

void RE_SkyBox::SaveResourceMeta(RE_Json* metaNode)
{
	metaNode->PushInt("minFilter", skyBoxSettings.min_filter);
	metaNode->PushInt("magFilter", skyBoxSettings.mag_filter);
	metaNode->PushInt("wrapS", skyBoxSettings.wrap_s);
	metaNode->PushInt("wrapT", skyBoxSettings.wrap_t);
	metaNode->PushInt("wrapR", skyBoxSettings.wrap_r);

	RE_Json* nodeTex = metaNode->PushJObject("textures");
	for (uint i = 0; i < 6; i++)
	{
		if (texturesname[i] != nullptr && skyBoxSettings.textures[i].textureMD5 != nullptr)
		{
			eastl::string key = texturesname[i];
			nodeTex->PushString((key + "textureMD5").c_str(), skyBoxSettings.textures[i].textureMD5);
		}
	}
}

void RE_SkyBox::LoadResourceMeta(RE_Json* metaNode)
{
	skyBoxSettings.min_filter = static_cast<RE_TextureFilters>(metaNode->PullInt("minFilter", RE_LINEAR));
	skyBoxSettings.mag_filter = static_cast<RE_TextureFilters>(metaNode->PullInt("magFilter", RE_LINEAR));
	skyBoxSettings.wrap_s =		static_cast<RE_TextureWrap>(metaNode->PullInt("wrapS", RE_CLAMP_TO_EDGE));
	skyBoxSettings.wrap_t =		static_cast<RE_TextureWrap>(metaNode->PullInt("wrapT", RE_CLAMP_TO_EDGE));
	skyBoxSettings.wrap_r =		static_cast<RE_TextureWrap>(metaNode->PullInt("wrapR", RE_CLAMP_TO_EDGE));

	RE_Json* nodeTex = metaNode->PullJObject("textures");
	for (uint i = 0; i < 6; i++)
	{
		eastl::string key(texturesname[i]);
		eastl::string texMD5 = nodeTex->PullString(eastl::string(key + "textureMD5").c_str(), "");
		skyBoxSettings.textures[i].textureMD5 = RE_ResourceManager::IsReference(texMD5.c_str(), Resource_Type::R_TEXTURE);
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
	Config toLoad(GetAssetPath(),RE_FileSystem::GetZipPath());
	if (toLoad.Load())
	{
		RE_Json* node = toLoad.GetRootNode("skybox");
		skyBoxSettings.skyBoxSize = node->PullFloat("skyBoxSize", 5000);
		DEL(node);
		
		if (generateLibraryPath)
		{
			eastl::string newMd5 = toLoad.GetMd5();
			SetMD5(newMd5.c_str());
			eastl::string libraryPath("Library/SkyBoxes/");
			libraryPath += newMd5.c_str();
			SetLibraryPath(libraryPath.c_str());
		}
	}
	RE_SkyboxImporter::LoadSkyBoxInMemory(skyBoxSettings, &ID);
	LoadSkyBoxSphere();
	ResourceContainer::inMemory = true;
}

void RE_SkyBox::AssetSave()
{
	eastl::string assetPath("Assets/Skyboxes/");
	(assetPath += GetName()) += ".sk";
	SetAssetPath(assetPath.c_str());
	Config toSave(assetPath.c_str(), RE_FileSystem::GetZipPath());
	RE_Json* node = toSave.GetRootNode("skybox");

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
	RE_GLCacheManager::ChangeVAO(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	RE_GLCacheManager::ChangeVAO(0);
	RE_GLCacheManager::ChangeShader(0);
}

void RE_SkyBox::LibraryLoad()
{
	RE_FileBuffer fileLibray(GetLibraryPath());

	if (fileLibray.Load())
	{
		char* cursor = fileLibray.GetBuffer();
		size_t size = sizeof(float);
		memcpy( &skyBoxSettings.skyBoxSize, cursor, size);
		cursor += size;

		RE_SkyboxImporter::LoadSkyBoxInMemory(skyBoxSettings, &ID);
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

	RE_FileBuffer saveLibrary(GetLibraryPath(), RE_FileSystem::GetZipPath());

	saveLibrary.Save(libraryBuffer, totalSize + 1);
	DEL_A(libraryBuffer);
}

void RE_SkyBox::TexParameteri(unsigned int pname, int param)
{
	if (ResourceContainer::inMemory)
	{
		GLuint boundTexture = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, reinterpret_cast<GLint*>(&boundTexture));
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
	RE_GLCacheManager::ChangeVAO(VAO);

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

	RE_GLCacheManager::ChangeVAO(0);

	triangle_count = sphere->ntriangles;
	par_shapes_free_mesh(sphere);
}
