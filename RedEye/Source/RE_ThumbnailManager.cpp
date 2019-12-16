#include "RE_ThumbnailManager.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleWindow.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_TextureImporter.h"

#include "RE_GLCache.h"
#include "RE_FBOManager.h"
#include "Event.h"
#include "TimeManager.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"

#include "RE_Shader.h"
#include "RE_Texture.h"
#include "RE_Model.h"
#include "RE_Scene.h"
#include "RE_Prefab.h"
#include "RE_Model.h"

#include "QuadTree.h"

#include "Glew/include/glew.h"

#include "IL/include/il.h"
#include "IL/include/ilu.h"

#include "Globals.h"

#include <string>

#define THUMBNAILPATH "Library/Thumbnails/"

#define DEFTHUMBNAILS "Settings/Icons/"

#define THUMBNAILSIZE 256
#define THUMBNAILDATASIZE THUMBNAILSIZE * THUMBNAILSIZE * 4

RE_ThumbnailManager::RE_ThumbnailManager()
{
}

RE_ThumbnailManager::~RE_ThumbnailManager()
{
	glDeleteTextures(1, &folder);
	glDeleteTextures(1, &file);
	glDeleteTextures(1, &selectfile);
	glDeleteTextures(1, &shaderFile);
	for (auto thumb : thumbnails) glDeleteTextures(1, &thumb.second);
}

void RE_ThumbnailManager::Init()
{
	folder = LoadDefIcon("folder.dds");
	file = LoadDefIcon("file.dds");
	selectfile = LoadDefIcon("selectfile.dds");
	shaderFile = LoadDefIcon("shaderFile.dds");

	singleRenderFBO = App->fbomanager->CreateFBO(THUMBNAILSIZE, THUMBNAILSIZE);
	Event::PauseEvents();
	internalCamera = new RE_CompCamera();
	internalCamera->SetBounds(THUMBNAILSIZE, THUMBNAILSIZE);
	internalCamera->LocalRotate(0, 0.5);
	internalCamera->Update();
	Event::ResumeEvents();

	glGenBuffersARB(1, &pboRender);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboRender);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, THUMBNAILDATASIZE, 0, GL_STREAM_READ_ARB);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

void RE_ThumbnailManager::Add(const char* ref)
{
	ResourceContainer* res = App->resources->At(ref);

	switch (res->GetType())
	{
	case Resource_Type::R_TEXTURE:
		thumbnails.insert(std::pair<const char*, unsigned int>(ref, ThumbnailTexture(ref)));
		break;
	case Resource_Type::R_MODEL:
	case Resource_Type::R_PREFAB:
	case Resource_Type::R_SCENE:
		thumbnails.insert(std::pair<const char*, unsigned int>(ref, ThumbnailGameObject(ref)));
		break;
	}
}

void RE_ThumbnailManager::Change(const char* ref)
{
	if (thumbnails.find(ref) == thumbnails.end())
		Add(ref);
	else
	{
		thumbnails.find(ref)._Ptr->_Myval.second = ThumbnailGameObject(ref);
	}
}

unsigned int RE_ThumbnailManager::At(const char* ref)
{
	if(thumbnails.find(ref) == thumbnails.end())
		return 0;
	else
		return thumbnails.at(ref);
}

unsigned int RE_ThumbnailManager::LoadDefIcon(const char* filename)
{
	uint ret = 0;
	std::string path(DEFTHUMBNAILS);
	path += filename;
	RE_FileIO filderIcon(path.c_str());
	if (filderIcon.Load()) {
		RE_TextureSettings defTexSettings;
		int tmp1, tmp2;
		App->textures->LoadTextureInMemory(filderIcon.GetBuffer(), filderIcon.GetSize(), TextureType::RE_DDS, &ret, &tmp1, &tmp2, defTexSettings);
	}
	return ret;
}

unsigned int RE_ThumbnailManager::ThumbnailTexture(const char* ref)
{
	uint ret = 0;

	std::string path(THUMBNAILPATH);
	path += ref;

	if (!App->fs->Exists(path.c_str())) {
		ResourceContainer* res = App->resources->At(ref);
		RE_Texture* tex = (RE_Texture*)res;
		RE_FileIO texFile(res->GetAssetPath());
		if (texFile.Load());
		{
			unsigned int imageID = 0;
			ilGenImages(1, &imageID);
			ilBindImage(imageID);

			if (IL_FALSE != ilLoadL(tex->DetectExtension(), texFile.GetBuffer(), texFile.GetSize())) {
				iluScale(THUMBNAILSIZE, THUMBNAILSIZE, 1);

				RE_FileIO saveThumb(path.c_str(), App->fs->GetZipPath());

				ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
				ILubyte* data = new ILubyte[size];

				ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
				saveThumb.Save((char*)data, size);
				DEL_A(data);	
			}

			ilBindImage(0);
			/* Delete used resources*/
			ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
		}
	}

	ret = LoadLibraryTexThumbnail(ref);

	return ret;
}

unsigned int RE_ThumbnailManager::ThumbnailGameObject(const char* ref)
{
	uint ret = 0;

	std::string path(THUMBNAILPATH);
	path += ref;

	if (!App->fs->Exists(path.c_str())) {
		Event::PauseEvents();

		RE_GameObject* goToThumbnail = nullptr;

		ResourceContainer* res = App->resources->At(ref);
		switch (res->GetType())
		{
		case Resource_Type::R_MODEL:
			goToThumbnail = ((RE_Model*)res)->GetRoot();
			break;
		case Resource_Type::R_PREFAB:
			goToThumbnail = ((RE_Prefab*)res)->GetRoot();
			break;
		case Resource_Type::R_SCENE:
			goToThumbnail = ((RE_Scene*)res)->GetRoot();
			break;
		}

		if (goToThumbnail) {
			goToThumbnail->UseResources();
			goToThumbnail->ResetBoundingBoxFromChilds();
			goToThumbnail->TransformModified(false);
			
			internalCamera->Focus(goToThumbnail);
			internalCamera->Update();

			float time = (App->GetState() == GameState::GS_STOP) ? App->time->GetEngineTimer() : App->time->GetGameTimer();
			float dt = App->time->GetDeltaTime();
			std::vector<const char*> activeShaders = App->resources->GetAllResourcesActiveByType(Resource_Type::R_SHADER);
			for (auto sMD5 : activeShaders) ((RE_Shader*)App->resources->At(sMD5))->UploatMainUniforms(internalCamera, dt, time);

			RE_FBOManager::ChangeFBOBind(singleRenderFBO, THUMBNAILSIZE, THUMBNAILSIZE);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			goToThumbnail->DrawWithChilds();

			goToThumbnail->UnUseResources();
			DEL(goToThumbnail);

			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboRender);
			glReadPixels(0, 0, THUMBNAILSIZE, THUMBNAILSIZE, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			if (ptr)
			{
				uint imageID = 0;
				ilGenImages(1, &imageID);
				ilBindImage(imageID);

				if (IL_FALSE == ilTexImage(THUMBNAILSIZE, THUMBNAILSIZE, 1, 4, IL_RGBA, GL_UNSIGNED_BYTE, ptr)) {
					int i = 0;
				}

				ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
				ILubyte* data = new ILubyte[size];

				ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
				RE_FileIO saveThumb(path.c_str(), App->fs->GetZipPath());
				saveThumb.Save((char*)data, size);
				DEL_A(data);

				ilBindImage(0);
				/* Delete used resources*/
				ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */

				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			}
			glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
			RE_FBOManager::ChangeFBOBind(0, App->window->GetWidth(), App->window->GetHeight());
		}

		Event::ResumeEvents();
	}

	ret = LoadLibraryTexThumbnail(ref);

	return ret;
}

unsigned int RE_ThumbnailManager::LoadLibraryTexThumbnail(const char* ref)
{
	uint ret = 0;

	std::string path(THUMBNAILPATH);
	path += ref;

	RE_FileIO thumbFile(path.c_str());
	if (thumbFile.Load()) {
		RE_TextureSettings defSettings;
		uint imageID = 0;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);

		if (IL_FALSE != ilLoadL(TextureType::RE_DDS, thumbFile.GetBuffer(), thumbFile.GetSize())) {


			/* OpenGL texture binding of the image loaded by DevIL  */
			glGenTextures(1, &ret); /* Texture name generation */
			RE_GLCache::ChangeTextureBind(ret); /* Binding of texture name */

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, defSettings.mag_filter); /* We will use linear interpolation for magnification filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, defSettings.min_filter); /* We will use linear interpolation for minifying filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, defSettings.wrap_s); /* We will use linear interpolation for minifying filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, defSettings.wrap_t); /* We will use linear interpolation for minifying filter */

			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), THUMBNAILSIZE, THUMBNAILSIZE, 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */

			RE_GLCache::ChangeTextureBind(0);
			ilBindImage(0);
			/* Delete used resources*/
			ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
		}
	}
	return ret;
}