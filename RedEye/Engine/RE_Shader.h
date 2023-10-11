#ifndef __RE_SHADER_H__
#define __RE_SHADER_H__

#include "RE_DataTypes.h"

class RE_Shader : public ResourceContainer
{
public:
	RE_Shader() = default;
	RE_Shader(const char* metaPath) : ResourceContainer(metaPath) {}
	~RE_Shader() final = default;

	void SetAsInternal(const char* vertexBuffer, const char* fragmentBuffer, const char* geometryBuffer = nullptr);
	void SetPaths(const char* vertex, const char* fragment, const char* geometry = nullptr);

	void UploadMainUniforms(const class RE_Camera& camera, float window_h, float window_w, bool clipDistance, math::float4 clipPlane) const;
	void UploadModel(const float* model) const;
	void UploadDepth(int texture) const;

	bool ShaderFilesChanged();
	bool IsPathOnShader(const char* assetPath) const;

	void LoadInMemory() final;
	void UnloadMemory() final;
	void ReImport() final;

	struct Settings
	{
		eastl::string vertexShader;
		signed long long vlastModified = 0;

		eastl::string fragmentShader;
		signed long long flastModified = 0;

		eastl::string geometryShader;
		signed long long glastModified = 0;
	};

	unsigned int GetID() const { return ID; }
	eastl::vector<RE_Shader_Cvar> GetUniformValues() { return uniforms; }
	bool NeedUploadDepth() const { return (depth != -1); }

private:

	void Draw() final;

	void SaveResourceMeta(RE_Json* metaNode) const final;
	void LoadResourceMeta(RE_Json* metaNode) final;

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave() const;

	void GetVertexFileInfo(const char*& path, signed long long* lastTimeModified) const;
	void GetFragmentFileInfo(const char*& path, signed long long* lastTimeModified) const;
	void GetGeometryFileInfo(const char*& path, signed long long* lastTimeModified) const;

	void ParseAndGetUniforms();
	eastl::vector<eastl::string> GetUniformLines(const char* buffer);
	void MountRE_Shader_Cvar(eastl::vector<eastl::string> uniformLines);
	void GetLocations();

public:

	unsigned int ID = 0;
	Settings shaderSettings;

	bool applySave = false;
	Settings restoreSettings;

	int projection = -1;
	int view = -1;
	int model = -1;
	int time = -1;
	int dt = -1;
	int depth = -1;
	int viewport_w = -1;
	int viewport_h = -1;
	int near_plane = -1;
	int far_plane = -1;
	int clip_plane = -1;
	int using_clip_plane = -1;
	int view_pos = -1;
	eastl::vector<RE_Shader_Cvar> uniforms;
};

#endif // !__RE_SHADER_H__