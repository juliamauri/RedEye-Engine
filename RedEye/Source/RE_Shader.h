#ifndef __RE_SHADER_H__
#define __RE_SHADER_H__

#include "Resource.h"
#include "Cvar.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>

class RE_CompCamera;

struct RE_ShaderSettings {
	eastl::string vertexShader;
	signed long long vlastModified = 0;
	eastl::string fragmentShader;
	signed long long flastModified = 0;
	eastl::string geometryShader;
	signed long long glastModified = 0;
};

class RE_Shader :
	public ResourceContainer
{
public:
	RE_Shader();
	RE_Shader(const char* metaPath);
	~RE_Shader();

	void LoadInMemory() override;
	void UnloadMemory() override;

	void SetAsInternal(const char* vertexBuffer, const char* fragmentBuffer, const char* geometryBuffer = nullptr);

	unsigned int GetID()const { return ID; }

	void SetPaths(const char* vertex, const char* fragment, const char* geometry = nullptr);

	eastl::vector<ShaderCvar> GetUniformValues();

	void UploatMainUniforms(RE_CompCamera* camera, float dt, float time);
	void UploadModel(float* model);

	bool isShaderFilesChanged();

	void ReImport()override;

	bool IsPathOnShader(const char* assetPath);

private:
	void Draw() override;
	void SaveResourceMeta(JSONNode* metaNode) override;
	void LoadResourceMeta(JSONNode* metaNode) override;

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

	void GetVertexFileInfo(const char*& path, signed long long* lastTimeModified)const;
	void GetFragmentFileInfo(const char*& path, signed long long* lastTimeModified)const;
	void GetGeometryFileInfo(const char*& path, signed long long* lastTimeModified)const;

	void ParseAndGetUniforms();
	eastl::vector<eastl::string> GetUniformLines(const char* buffer);
	void MountShaderCvar(eastl::vector<eastl::string> uniformLines);
	void GetLocations();

private:
	unsigned int ID = 0;

	RE_ShaderSettings shaderSettings;

	bool applySave = false;
	RE_ShaderSettings restoreSettings;

	int projection = -1;
	int view = -1;
	int model = -1;
	int time = -1;
	int dt = -1;
	eastl::vector<ShaderCvar> uniforms;
};

#endif // !__RE_SHADER_H__