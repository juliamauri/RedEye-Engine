#ifndef __RE_THUMBNAIL_MANAGER_H__
#define __RE_THUMBNAIL_MANAGER_H__

#include "RenderView.h"
#include "RE_Camera.h"

#include <EASTL/stack.h>
#include <EASTL/map.h>

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
	void DrawPendingThumbnails();
	void DrawThumbnail(PendingThumbnail& to_render);

	// Edits
	void Change(const char* ref, uint32_t id);
	void Delete(const char* ref);

	uintptr_t At(const char* ref);
	uintptr_t GetFolderID();
	uintptr_t GetFileID();
	uintptr_t GetSelectFileID();
	uintptr_t GetShaderFileID();
	uintptr_t GetPEmitterFileID();
	uintptr_t GetPEmissionFileID();
	uintptr_t GetPRenderFileID();

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

#endif // !__RE_THUMBNAIL_MANAGER_H__