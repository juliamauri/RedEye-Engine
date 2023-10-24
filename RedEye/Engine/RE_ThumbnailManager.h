#ifndef __RETHUMBNAILMANAGER_H__
#define __RETHUMBNAILMANAGER_H__

#include "RenderView.h"
#include "RE_Camera.h"

#include <EASTL/map.h>
#include <EASTL/stack.h>

class RE_GameObject;
class RE_Material;
class RE_SkyBox;

namespace RE_ThumbnailManager
{
	auto constexpr THUMBNAILPATH = "Library/Thumbnails/";
	auto constexpr DEFTHUMBNAILS = "Internal/Icons/";
	auto constexpr THUMBNAILSIZE = 256;
	auto constexpr THUMBNAILDATASIZE = THUMBNAILSIZE * THUMBNAILSIZE * 4;

	enum class RenderType : ushort
	{
		SCENE,
		GO,
		MAT,
		TEX,
		SKYBOX
	};

	struct PendingThumbnail
	{
		RenderType type;
		const char* resMD5;
		bool redo = false;

		PendingThumbnail(RenderType t, const char* resource, bool re = false) :
			type(t), resMD5(resource), redo(re) {}
		~PendingThumbnail() = default;
	};

	namespace
	{
		// Rendering
		RenderView render_view;
		RE_Camera camera;
		uint mat_vao = 0;
		uint mat_vbo = 0;
		uint mat_ebo = 0;
		uint mat_triangles = 0;
		eastl::stack<PendingThumbnail> pending;

		//uint32_t singleRenderFBO = 0;
		uint32_t pboRender = 0;

		// Default Icons
		uint32_t folder = 0;
		uint32_t file = 0;
		uint32_t selectfile = 0;
		uint32_t shaderFile = 0;
		uint32_t p_emitter = 0;
		uint32_t p_emission = 0;
		uint32_t p_render = 0;

		eastl::map<const char*, uint32_t> thumbnails;
	}

	void Init();
	void Clear();

	void AddThumbnail(const char* md5, bool redo = false);
	void DrawThumbnails();

	// Edits
	void Change(const char* ref, unsigned int id);
	void Delete(const char* ref);

	uintptr_t At(const char* ref);
	uintptr_t GetFolderID() { return folder; }
	uintptr_t GetFileID() { return file; }
	uintptr_t GetSelectFileID() { return selectfile; }
	uintptr_t GetShaderFileID() { return shaderFile; }
	uintptr_t GetPEmitterFileID() { return p_emitter; }
	uintptr_t GetPEmissionFileID() { return p_emission; }
	uintptr_t GetPRenderFileID() { return p_render; }

	void SaveTextureFromFBO(const char* path);
	uint32_t LoadLibraryThumbnail(const char* ref);

	namespace Internal
	{
		void SetupFBO();
		void UploadUniforms();

		uint32_t ThumbnailTexture(const char* ref);
		void ThumbnailGameObject(RE_GameObject* go);
		void ThumbnailMaterial(RE_Material* mat);
		void ThumbnailSkyBox(RE_SkyBox* skybox);
	}
};

#endif // !__RETHUMBNAILMANAGER_H__