#ifndef __RE_SHADER_H__
#define __RE_SHADER_H__

#include "Resource.h"
#include "Cvar.h"
#include <string>
#include <vector>

class RE_CompCamera;

struct RE_ShaderSettings {
	std::string vertexShader;
	signed long long vlastModified = 0;
	std::string fragmentShader;
	signed long long flastModified = 0;
	std::string geometryShader;
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

	std::vector<ShaderCvar> GetUniformValues();

	void DrawCustomUniforms(std::vector<Cvar>* FromMatValues);

	void UploadCameraMatrices(RE_CompCamera* camera);
	void UploadModel(float* model);
	void UploadCustomUniforms(std::vector<Cvar>* FromMatValues);

	bool isShaderFilesChanged();

	void ReImport()override;

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

private:
	unsigned int ID = 0;

	RE_ShaderSettings shaderSettings;

	bool applySave = false;
	RE_ShaderSettings restoreSettings;

	std::vector<ShaderCvar> uniforms;
};

#endif // !__RE_SHADER_H__