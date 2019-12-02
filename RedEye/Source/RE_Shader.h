#ifndef __RE_SHADER_H__
#define __RE_SHADER_H__

#include "Resource.h"
#include "Cvar.h"
#include <string>
#include <vector>

class RE_CompCamera;

struct RE_ShaderSettings {
	std::string vertexShader;
	std::string fragmentShader;
	std::string geometryShader;
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

	unsigned int GetID()const { return ID; }

	void SetPaths(const char* vertex, const char* fragment, const char* geometry = nullptr);

	std::vector<Cvar> GetUniformValues();

	void DrawCustomUniforms(std::vector<Cvar>* FromMatValues);

	void UploadCameraMatrices(RE_CompCamera* camera);
	void UploadModel(float* model);
	void UploadCustomUniforms(std::vector<Cvar>* FromMatValues);

private:
	void Draw() override;
	void SaveResourceMeta(JSONNode* metaNode) override;
	void LoadResourceMeta(JSONNode* metaNode) override;

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

private:
	unsigned int ID = 0;

	RE_ShaderSettings shaderSettings;

	bool applySave = false;
	RE_ShaderSettings restoreSettings;

	std::vector<Cvar> customUniform;
};

#endif // !__RE_SHADER_H__