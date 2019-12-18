#include "RE_ThumbnailManager.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleWindow.h"
#include "RE_ResourceManager.h"
#include "RE_ShaderImporter.h"
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
#include "RE_Material.h"
#include "RE_SkyBox.h"

#include "Globals.h"


#include "Glew/include/glew.h"

#include "IL/include/il.h"
#include "IL/include/ilu.h"

#include "par_shapes.h"



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

	glDeleteBuffersARB(1, &pboRender);
	glDeleteBuffers(1, &VAOSphere);
	glDeleteBuffers(1, &VBOSphere);
	glDeleteBuffers(1, &EBOSphere);

	DEL(internalCamera);
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
	internalCamera->Update();
	Event::ResumeEvents();

	glGenBuffersARB(1, &pboRender);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboRender);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, THUMBNAILDATASIZE, 0, GL_STREAM_READ_ARB);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

	par_shapes_mesh* sphere = par_shapes_create_parametric_sphere(24, 24);

	float* points = new float[sphere->npoints * 3];
	float* normals = new float[sphere->npoints * 3];
	float* texCoords = new float[sphere->npoints * 2];

	uint meshSize = 0;
	size_t size = sphere->npoints * 3 * sizeof(float);
	uint stride = 0;

	memcpy(points, sphere->points, size);
	meshSize += 3 * sphere->npoints;
	stride += 3;

	memcpy(normals, sphere->normals, size);
	meshSize += 3 * sphere->npoints;
	stride += 3;

	size = sphere->npoints * 2 * sizeof(float);
	memcpy(texCoords, sphere->tcoords, size);
	meshSize += 2 * sphere->npoints;
	stride += 2;

	stride *= sizeof(float);
	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;
	for (uint i = 0; i < sphere->npoints; i++) {
		uint cursorSize = 3;
		size_t size = sizeof(float) * 3;

		memcpy(cursor, &points[i * 3], size);
		cursor += cursorSize;

		memcpy(cursor, &normals[i * 3], size);
		cursor += cursorSize;

		cursorSize = 2;
		size = sizeof(float) * 2;
		memcpy(cursor, &texCoords[i * 2], size);
		cursor += cursorSize;
	}


	glGenVertexArrays(1, &VAOSphere);
	RE_GLCache::ChangeVAO(VAOSphere);

	glGenBuffers(1, &VBOSphere);
	glBindBuffer(GL_ARRAY_BUFFER, VBOSphere);
	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));

	glGenBuffers(1, &EBOSphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOSphere);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere->ntriangles * sizeof(unsigned short) * 3, sphere->triangles, GL_STATIC_DRAW);

	sphere_triangle_count = sphere->ntriangles;

	par_shapes_free_mesh(sphere);
	DEL_A(points);
	DEL_A(normals);
	DEL_A(texCoords);
	DEL_A(meshBuffer);

}

void RE_ThumbnailManager::Add(const char* ref)
{
	ResourceContainer* res = App->resources->At(ref);

	switch (res->GetType())
	{
	case Resource_Type::R_MATERIAL:
		thumbnails.insert(std::pair<const char*, unsigned int>(ref, ThumbnailMaterial(ref)));
		break;
	case Resource_Type::R_SKYBOX:
		thumbnails.insert(std::pair<const char*, unsigned int>(ref, ThumbnailSkyBox(ref)));
		break;
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

	ret = LoadLibraryThumbnail(ref);

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

			internalCamera->SetFOV(math::RadToDeg(0.523599f));
			internalCamera->Update();
			internalCamera->GetTransform()->SetRotation({ 0.0,0.0,0.0 });
			internalCamera->Update();
			internalCamera->GetTransform()->SetPosition(math::vec(0.f, 5.f, -5.f));
			internalCamera->Update();
			internalCamera->LocalRotate(0, -0.5);
			internalCamera->Update();
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

			SaveTextureFromFBO(path.c_str());

			RE_FBOManager::ChangeFBOBind(0, App->window->GetWidth(), App->window->GetHeight());
		}

		Event::ResumeEvents();
	}

	ret = LoadLibraryThumbnail(ref);

	return ret;
}

unsigned int RE_ThumbnailManager::ThumbnailMaterial(const char* ref)
{
	uint ret = 0;

	std::string path(THUMBNAILPATH);
	path += ref;

	if (!App->fs->Exists(path.c_str())) {
		RE_Material* mat = (RE_Material * )App->resources->At(ref);
		
		Event::PauseEvents();
		internalCamera->SetFOV(math::RadToDeg( 0.523599f));
		internalCamera->Update();
		internalCamera->GetTransform()->SetRotation({ 0.0,0.0,0.0 });
		internalCamera->Update();
		internalCamera->GetTransform()->SetPosition(math::vec(0.f, 5.f,  0.f));
		internalCamera->Update();
		internalCamera->LocalRotate(0, 1);
		internalCamera->Update();
		Event::ResumeEvents();

		float time = (App->GetState() == GameState::GS_STOP) ? App->time->GetEngineTimer() : App->time->GetGameTimer();
		float dt = App->time->GetDeltaTime();
		std::vector<const char*> activeShaders = App->resources->GetAllResourcesActiveByType(Resource_Type::R_SHADER);
		for (auto sMD5 : activeShaders) ((RE_Shader*)App->resources->At(sMD5))->UploatMainUniforms(internalCamera, dt, time);

		App->resources->Use(ref);

		RE_FBOManager::ChangeFBOBind(singleRenderFBO, THUMBNAILSIZE, THUMBNAILSIZE);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat->UploadToShader((float*)math::float4x4::identity.ptr(), false, true);
		RE_GLCache::ChangeVAO(VAOSphere);
		glDrawElements(GL_TRIANGLES, sphere_triangle_count * 3, GL_UNSIGNED_SHORT, 0);

		App->resources->UnUse(ref);

		SaveTextureFromFBO(path.c_str());

		RE_FBOManager::ChangeFBOBind(0, App->window->GetWidth(), App->window->GetHeight());
	}

	ret = LoadLibraryThumbnail(ref);

	return ret;
}

unsigned int RE_ThumbnailManager::ThumbnailSkyBox(const char* ref)
{
	uint ret = 0;

	std::string path(THUMBNAILPATH);
	path += ref;

	if (!App->fs->Exists(path.c_str())) {
		RE_SkyBox* skybox = (RE_SkyBox*)App->resources->At(ref);

		Event::PauseEvents();
		internalCamera->ForceFOV(125, 140);
		internalCamera->Update();
		internalCamera->GetTransform()->SetRotation({ 0.0,0.0,0.0 });
		internalCamera->Update();
		internalCamera->GetTransform()->SetPosition(math::vec(0.f, 0.f, 0.f));
		internalCamera->Update();
		Event::ResumeEvents();

		float time = (App->GetState() == GameState::GS_STOP) ? App->time->GetEngineTimer() : App->time->GetGameTimer();
		float dt = App->time->GetDeltaTime();
		std::vector<const char*> activeShaders = App->resources->GetAllResourcesActiveByType(Resource_Type::R_SHADER);
		for (auto sMD5 : activeShaders) ((RE_Shader*)App->resources->At(sMD5))->UploatMainUniforms(internalCamera, dt, time);

		RE_GLCache::ChangeTextureBind(0);

		App->resources->Use(ref);

		RE_FBOManager::ChangeFBOBind(singleRenderFBO, THUMBNAILSIZE, THUMBNAILSIZE);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RE_Shader* skyboxShader = (RE_Shader*)App->resources->At(App->internalResources->GetDefaultSkyBoxShader());
		uint skysphereshader = skyboxShader->GetID();
		RE_GLCache::ChangeShader(skysphereshader);
		RE_ShaderImporter::setInt(skysphereshader, "cubemap", 0);
		skybox->DrawSkybox();
		
		App->resources->UnUse(ref);

		SaveTextureFromFBO(path.c_str());

		RE_FBOManager::ChangeFBOBind(0, App->window->GetWidth(), App->window->GetHeight());
	}

	ret = LoadLibraryThumbnail(ref);

	return ret;
}

void RE_ThumbnailManager::SaveTextureFromFBO(const char* path)
{
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
		RE_FileIO saveThumb(path, App->fs->GetZipPath());
		saveThumb.Save((char*)data, size);
		DEL_A(data);

		ilBindImage(0);
		/* Delete used resources*/
		ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */

		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

unsigned int RE_ThumbnailManager::LoadLibraryThumbnail(const char* ref)
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