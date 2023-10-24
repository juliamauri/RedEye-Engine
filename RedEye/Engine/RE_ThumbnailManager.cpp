#include "RE_ThumbnailManager.h"

#include "RE_Profiler.h"
#include "RE_Memory.h"
#include "RE_DataTypes.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "RE_TextureImporter.h"
#include "RE_ResourceManager.h"
#include "RE_FBOManager.h"
#include "RE_PrimitiveManager.h"
#include "RE_InternalResources.h"
#include "RE_ShaderImporter.h"
#include "RE_ECS_Pool.h"
#include "RE_GLCache.h"
#include "RE_Shader.h"
#include "RE_GameObject.h"
#include "RE_Texture.h"
#include "RE_Scene.h"
#include "RE_Prefab.h"
#include "RE_Model.h"
#include "RE_Material.h"
#include "RE_SkyBox.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <EASTL/string.h>

uint32_t LoadDefIcon(const char* filename)
{
	uint32_t ret = 0;
	eastl::string path(RE_ThumbnailManager::DEFTHUMBNAILS);
	path += filename;
	RE_FileBuffer filderIcon(path.c_str());
	if (filderIcon.Load())
	{
		RE_TextureSettings defTexSettings;
		int tmp1, tmp2;
		RE_TextureImporter::LoadTextureInMemory(
			filderIcon.GetBuffer(),
			static_cast<ILuint>(filderIcon.GetSize()),
			RE_TextureSettings::Type::DDS,
			&ret,
			&tmp1,
			&tmp2,
			defTexSettings);
	}
	return ret;
}

void RE_ThumbnailManager::Init()
{
	// Rendering
	render_view.fbos = { RE_FBOManager::CreateFBO(THUMBNAILSIZE, THUMBNAILSIZE),0 };
	camera.SetupFrustum();
	camera.SetBounds(THUMBNAILSIZE, THUMBNAILSIZE);
	RE_PrimitiveManager::CreateSphere(24, 24, mat_vao, mat_vbo, mat_ebo, mat_triangles);

	// Default Icons
	folder = LoadDefIcon("folder.dds");
	file = LoadDefIcon("file.dds");
	selectfile = LoadDefIcon("selectfile.dds");
	shaderFile = LoadDefIcon("shaderFile.dds");
	p_emitter = LoadDefIcon("pemitter.dds");
	p_emission = LoadDefIcon("pemissor.dds");
	p_render = LoadDefIcon("prender.dds");
	
	glGenBuffersARB(1, &pboRender);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboRender);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, THUMBNAILDATASIZE, 0, GL_STREAM_READ_ARB);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

void RE_ThumbnailManager::Clear()
{
	// Default Icons
	glDeleteTextures(1, &folder);
	glDeleteTextures(1, &file);
	glDeleteTextures(1, &selectfile);
	glDeleteTextures(1, &shaderFile);
	glDeleteTextures(1, &p_emitter);
	glDeleteTextures(1, &p_emission);
	glDeleteTextures(1, &p_render);
	for (auto& thumb : thumbnails) glDeleteTextures(1, &thumb.second);

	glDeleteBuffersARB(1, &pboRender);
}

void RE_ThumbnailManager::AddThumbnail(const char* md5, bool redo)
{
	switch (RE_RES->At(md5)->GetType())
	{
	case ResourceContainer::Type::SCENE:
	case ResourceContainer::Type::MODEL:
	case ResourceContainer::Type::PREFAB:	pending.push({ RenderType::GO,	  md5, redo }); break;
	case ResourceContainer::Type::MATERIAL:	pending.push({ RenderType::MAT,	  md5, redo }); break;
	case ResourceContainer::Type::TEXTURE:	pending.push({ RenderType::TEX,	  md5, redo }); break;
	case ResourceContainer::Type::SKYBOX:	pending.push({ RenderType::SKYBOX, md5, redo }); break;
	default: break;
	}
}

void RE_ThumbnailManager::DrawThumbnails()
{
	RE_PROFILE(RE_ProfiledFunc::DrawThumbnails, RE_ProfiledClass::ThumbnailManager)

	while (!pending.empty())
	{
		PendingThumbnail rend = pending.top();
		pending.pop();

		RE_ECS_Pool* poolGOThumbnail = nullptr;
		eastl::string path(THUMBNAILPATH);
		path += rend.resMD5;

		bool exist = RE_FS->Exists(path.c_str());
		if (rend.redo && exist)
		{
			RE_FileBuffer fileToDelete(path.c_str());
			fileToDelete.Delete();
			exist = false;
		}

		RE_GLCache::ChangeVAO(0);

		switch (rend.type)
		{
		case RenderType::GO:
		{
			RE_INPUT->PauseEvents();
			if (!exist)
			{
				ResourceContainer* res = RE_RES->At(rend.resMD5);
				RE_RES->Use(rend.resMD5);
				switch (res->GetType())
				{
				case ResourceContainer::Type::MODEL: poolGOThumbnail = dynamic_cast<RE_Model*>(res)->GetPool(); break;
				case ResourceContainer::Type::PREFAB: poolGOThumbnail = dynamic_cast<RE_Prefab*>(res)->GetPool(); break;
				case ResourceContainer::Type::SCENE: poolGOThumbnail = dynamic_cast<RE_Scene*>(res)->GetPool(); break;
				default: break;
				}
				if (poolGOThumbnail != nullptr)
				{
					poolGOThumbnail->UseResources();
					Internal::ThumbnailGameObject(poolGOThumbnail->GetRootPtr());
					poolGOThumbnail->UnUseResources();
					RE_RES->UnUse(rend.resMD5);
					SaveTextureFromFBO(path.c_str());
					DEL(poolGOThumbnail)
				}
			}

			Change(rend.resMD5, LoadLibraryThumbnail(rend.resMD5));
			RE_INPUT->ResumeEvents();

			break;
		}
		case RenderType::MAT:
		{
			if (!exist)
			{
				ResourceContainer* res = RE_RES->At(rend.resMD5);
				RE_RES->Use(rend.resMD5);
				dynamic_cast<RE_Material*>(res)->UseResources();
				Internal::ThumbnailMaterial(dynamic_cast<RE_Material*>(res));
				dynamic_cast<RE_Material*>(res)->UnUseResources();
				RE_RES->UnUse(rend.resMD5);
				SaveTextureFromFBO(path.c_str());
			}

			Change(rend.resMD5, LoadLibraryThumbnail(rend.resMD5));
			break;
		}
		case RenderType::TEX:
			Change(rend.resMD5, Internal::ThumbnailTexture(rend.resMD5));
			break;
		case RenderType::SKYBOX:
			if (!exist)
			{
				ResourceContainer* res = RE_RES->At(rend.resMD5);

				RE_RES->Use(rend.resMD5);
				Internal::ThumbnailSkyBox(dynamic_cast<RE_SkyBox*>(res));
				RE_RES->UnUse(rend.resMD5);
				SaveTextureFromFBO(path.c_str());
			}
			Change(rend.resMD5, LoadLibraryThumbnail(rend.resMD5));
			break;
		}
	}
}

void RE_ThumbnailManager::Change(const char* ref, unsigned int id)
{
	auto iter = thumbnails.find(ref);
	if (iter == thumbnails.end()) thumbnails.insert(eastl::pair<const char*, unsigned int>(ref, id));
	else iter->second = id;
}

void RE_ThumbnailManager::Delete(const char* ref)
{
	thumbnails.erase(ref);
	eastl::string path(THUMBNAILPATH);

	if (!RE_FS->Exists((path += ref).c_str()))
		return;

	RE_FileBuffer fileToDelete(path.c_str());
	fileToDelete.Delete();
}

uintptr_t RE_ThumbnailManager::At(const char* ref)
{ 
	return (thumbnails.find(ref) != thumbnails.end()) ? thumbnails.at(ref) : 0;
}

void RE_ThumbnailManager::SaveTextureFromFBO(const char* path)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboRender);
	glReadPixels(0, 0, THUMBNAILSIZE, THUMBNAILSIZE, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	GLubyte* ptr = static_cast<GLubyte*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
	if (ptr)
	{
		uint imageID = 0;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);
		ilTexImage(THUMBNAILSIZE, THUMBNAILSIZE, 1, 4, IL_RGBA, GL_UNSIGNED_BYTE, ptr);

		ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
		ILubyte* data = new ILubyte[size];

		ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
		RE_FileBuffer saveThumb(path);
		saveThumb.Save(reinterpret_cast<char*>(data), size);
		DEL_A(data);

		ilBindImage(0);
		/* Delete used resources*/
		ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */

		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

uint32_t RE_ThumbnailManager::LoadLibraryThumbnail(const char* ref)
{
	uint ret = 0u;
	eastl::string path(THUMBNAILPATH);
	RE_FileBuffer thumbFile((path += ref).c_str());
	if (thumbFile.Load())
	{
		RE_TextureSettings defSettings;
		uint imageID = 0;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);

		if (IL_FALSE != ilLoadL(static_cast<ILenum>(RE_TextureSettings::Type::DDS), thumbFile.GetBuffer(), static_cast<ILuint>(thumbFile.GetSize())))
		{
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
			ilDeleteImages(1u, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
		}
	}
	return ret;
}

#pragma region Internal

void RE_ThumbnailManager::Internal::SetupFBO()
{
	auto fbo = render_view.fbos.first;
	RE_FBOManager::ChangeFBOBind(fbo, RE_FBOManager::GetWidth(fbo), RE_FBOManager::GetHeight(fbo));
	RE_FBOManager::ClearFBOBuffers(fbo, render_view.clear_color.ptr());
}

void RE_ThumbnailManager::Internal::UploadUniforms()
{
	for (auto sMD5 : ModuleRenderer3D::GetActiveShaders())
	{
		auto shader = dynamic_cast<const RE_Shader*>(RE_RES->At(sMD5));
		if (!shader->uniforms.empty())
			shader->UploadMainUniforms(camera, THUMBNAILSIZE, THUMBNAILSIZE, false, {});
	}
}

uint32_t RE_ThumbnailManager::Internal::ThumbnailTexture(const char* ref)
{
	eastl::string path(THUMBNAILPATH);
	if (!RE_FS->Exists((path += ref).c_str()))
	{
		ResourceContainer* res = RE_RES->At(ref);
		RE_Texture* tex = dynamic_cast<RE_Texture*>(res);
		RE_FileBuffer texFile(res->GetAssetPath());
		if (texFile.Load())
		{
			unsigned int imageID = 0;
			ilGenImages(1, &imageID);
			ilBindImage(imageID);

			if (IL_FALSE != ilLoadL(static_cast<ILenum>(tex->DetectExtension()), texFile.GetBuffer(), static_cast<ILuint>(texFile.GetSize())))
			{
				iluScale(THUMBNAILSIZE, THUMBNAILSIZE, 1);
				RE_FileBuffer saveThumb(path.c_str());
				ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
				ILubyte* data = new ILubyte[size];
				ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
				saveThumb.Save(reinterpret_cast<char*>(data), size);
				DEL_A(data);
			}

			ilBindImage(0);
			/* Delete used resources*/
			ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
		}
	}

	return LoadLibraryThumbnail(ref);
}

void RE_ThumbnailManager::Internal::ThumbnailGameObject(RE_GameObject* go)
{
	SetupFBO();

	// Reset Camera
	camera.SetupFrustum();
	camera.SetFrame(
		{ 0.0f,1.0f,5.0f },
		math::Quat::FromEulerXYZ(math::DegToRad(45.0f), 0.0f, 0.0f) * math::vec::unitZ,
		math::vec::unitY);

	// Focus GO's AABB
	go->ResetGOandChildsAABB();
	camera.Focus(go->GetGlobalBoundingBoxWithChilds());

	UploadUniforms();

	go->DrawChilds();
}

void RE_ThumbnailManager::Internal::ThumbnailMaterial(RE_Material* mat)
{
	SetupFBO();

	camera.SetupFrustum();

	UploadUniforms();

	mat->UploadToShader(math::float4x4::identity.ptr(), false, true);

	RE_GLCache::ChangeVAO(mat_vao);
	glDrawElements(GL_TRIANGLES, mat_triangles * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCache::ChangeVAO(0);
	RE_GLCache::ChangeShader(0);
	RE_GLCache::ChangeTextureBind(0);
}

void RE_ThumbnailManager::Internal::ThumbnailSkyBox(RE_SkyBox* skybox)
{
	SetupFBO();

	camera.SetupFrustum();
	camera.SetFOVDegrees(125.f);
	camera.SetFrame(math::vec::zero, math::vec::unitZ, math::vec::unitY);

	UploadUniforms();

	RE_GLCache::ChangeTextureBind(0);

	auto skyboxShader = dynamic_cast<const RE_Shader*>(RE_RES->At(RE_InternalResources::GetDefaultSkyBoxShader()));
	uint skysphereshader = skyboxShader->GetID();
	RE_GLCache::ChangeShader(skysphereshader);
	RE_ShaderImporter::setInt(skysphereshader, "cubemap", 0);
	skybox->DrawSkybox();
}

#pragma endregion