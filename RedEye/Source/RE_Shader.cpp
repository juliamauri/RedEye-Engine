#include "RE_Shader.h"

#include "Application.h"
#include "RE_FileSystem.h"

#include "RE_ShaderImporter.h"
#include "RE_CompCamera.h"

#include "RE_GLCache.h"

#include "OutputLog.h"
#include "md5.h"

#include "ImGui/imgui.h"

RE_Shader::RE_Shader() { }

RE_Shader::RE_Shader(const char* metaPath) : ResourceContainer(metaPath) { }

RE_Shader::~RE_Shader() { }

void RE_Shader::LoadInMemory()
{
	if (App->fs->Exists(shaderSettings.vertexShader.c_str()) && App->fs->Exists(shaderSettings.fragmentShader.c_str())) {
		AssetLoad();
	}
	else {
		LOG_ERROR("Texture %s not found on project", GetName());
	}
}

void RE_Shader::UnloadMemory()
{
	RE_ShaderImporter::Delete(ID);
	ResourceContainer::inMemory = false;
}

void RE_Shader::SetAsInternal(const char* vertexBuffer, const char* fragmentBuffer, const char* geometryBuffer)
{
	SetInternal(true);
	
	uint vertexLenght = strlen(vertexBuffer) + 1;
	uint fragmentLenght = strlen(fragmentBuffer) + 1;
	uint geometryLenght = (geometryBuffer) ? strlen(fragmentBuffer) + 1 : 0;
	std::string vbuffer(vertexBuffer, vertexLenght);
	std::string fbuffer(vertexBuffer, vertexLenght);
	std::string gbuffer;
	if (geometryBuffer) gbuffer = std::string(geometryBuffer, geometryLenght);

	std::string totalBuffer = vbuffer;
	totalBuffer += fbuffer;
	totalBuffer += gbuffer;

	SetMD5(MD5(totalBuffer).hexdigest().c_str());
	std::string libraryPath("Library/Shaders/");
	libraryPath += GetMD5();

	App->shaders->LoadFromBuffer(&ID, vertexBuffer, vertexLenght, fragmentBuffer, fragmentLenght, geometryBuffer, geometryLenght);

	ResourceContainer::inMemory = true;
}

void RE_Shader::SetPaths(const char* vertex, const char* fragment, const char* geometry)
{
	shaderSettings.vertexShader = vertex;
	shaderSettings.fragmentShader = fragment;
	if (geometry) shaderSettings.geometryShader = geometry;

	std::string buffer = "";

	if (!shaderSettings.vertexShader.empty()) {
		RE_FileIO vertexBuffer(shaderSettings.vertexShader.c_str());
		if (vertexBuffer.Load()) {
			std::string vbuffer(vertexBuffer.GetBuffer(), vertexBuffer.GetSize());
			buffer += vbuffer;
		}
	}
	if (!shaderSettings.fragmentShader.empty()) {
		RE_FileIO fragmentBuffer(shaderSettings.fragmentShader.c_str());
		if (fragmentBuffer.Load()) {
			std::string fbuffer(fragmentBuffer.GetBuffer(), fragmentBuffer.GetSize());
			buffer += fbuffer;
		}
	}
	if (!shaderSettings.geometryShader.empty()) {
		RE_FileIO geometryBuffer(shaderSettings.geometryShader.c_str());
		if (geometryBuffer.Load()) {
			std::string gbuffer(geometryBuffer.GetBuffer(), geometryBuffer.GetSize());
			buffer += gbuffer;
		}
	}

	SetMD5(MD5(buffer).hexdigest().c_str());
	std::string libraryPath("Library/Shaders/");
	libraryPath += GetMD5();

	if (!isInternal()) SetMetaPath("Assets/Shaders/");
}

std::vector<Cvar> RE_Shader::GetUniformValues()
{
	return customUniform;
}

void RE_Shader::UploadCameraMatrices(RE_CompCamera* camera)
{
	RE_GLCache::ChangeShader(ID);
	RE_ShaderImporter::setFloat4x4(ID, "view", camera->GetViewPtr());
	RE_ShaderImporter::setFloat4x4(ID, "projection", camera->GetProjectionPtr());
}

void RE_Shader::UploadModel(float* model)
{
	RE_GLCache::ChangeShader(ID);
	RE_ShaderImporter::setFloat4x4(ID, "model", model);
}

void RE_Shader::Draw()
{
	//Todo drag & drop of shader files
	ImGui::Text("Vertex Shader path: %s", shaderSettings.vertexShader.c_str());
	ImGui::Text("Fragment Shader path: %s", shaderSettings.fragmentShader.c_str());
	ImGui::Text("Geometry Shader path: %s", shaderSettings.geometryShader.c_str());
}

void RE_Shader::SaveResourceMeta(JSONNode* metaNode)
{
	metaNode->PushString("vertexPath", shaderSettings.vertexShader.c_str());
	metaNode->PushString("fragmentPath", shaderSettings.fragmentShader.c_str());
	metaNode->PushString("geometryPath", shaderSettings.geometryShader.c_str());
}

void RE_Shader::LoadResourceMeta(JSONNode* metaNode)
{
	shaderSettings.vertexShader = metaNode->PullString("vertexPath", "");
	shaderSettings.fragmentShader = metaNode->PullString("fragmentPath", "");
	shaderSettings.geometryShader = metaNode->PullString("geometryPath", "");

	restoreSettings = shaderSettings;
}

void RE_Shader::AssetLoad()
{
	bool loaded = false;
	loaded = App->shaders->LoadFromAssets(&ID, (!shaderSettings.vertexShader.empty()) ? shaderSettings.vertexShader.c_str() : nullptr,
		(!shaderSettings.fragmentShader.empty()) ? shaderSettings.fragmentShader.c_str() : nullptr,
		(!shaderSettings.geometryShader.empty()) ? shaderSettings.geometryShader.c_str() : nullptr);

	if (!loaded) {
		LOG_ERROR("Error while loading shader %s on assets:\n%s\n", GetName(), App->shaders->GetShaderError());
	}
	else
		ResourceContainer::inMemory = true;
}

void RE_Shader::LibraryLoad()
{
	RE_FileIO libraryLoad(GetLibraryPath());
	if (libraryLoad.Load()) {
		App->shaders->LoadFromBinary(libraryLoad.GetBuffer(), libraryLoad.GetSize(), &ID);
		ResourceContainer::inMemory = true;
	}
}

void RE_Shader::LibrarySave()
{
	RE_FileIO librarySave(GetLibraryPath(), App->fs->GetZipPath());

	char** buffer = nullptr;
	int size = 0;
	if (App->shaders->GetBinaryProgram(ID, buffer, &size))
		librarySave.Save(*buffer, size);
}
