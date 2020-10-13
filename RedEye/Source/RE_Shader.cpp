#include "RE_Shader.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"

#include "RE_ShaderImporter.h"
#include "RE_CompCamera.h"
#include "ModuleEditor.h"

#include "RE_GLCache.h"
#include "TimeManager.h"
#include "Event.h"

#include "OutputLog.h"
#include "md5.h"

#include "ImGui/imgui.h"
#include "PhysFS\include\physfs.h"

RE_Shader::RE_Shader() { }

RE_Shader::RE_Shader(const char* metaPath) : ResourceContainer(metaPath) { }

RE_Shader::~RE_Shader() { }

void RE_Shader::LoadInMemory()
{
	if (App->fs->Exists(GetLibraryPath())) {
		LibraryLoad();
	}
	else if (App->fs->Exists(shaderSettings.vertexShader.c_str()) && App->fs->Exists(shaderSettings.fragmentShader.c_str())) {
		AssetLoad();
		LibrarySave();
	}
	else {
		LOG_ERROR("Texture %s not found on project", GetName());
	}

	if (isInMemory()) 
		GetLocations();
}

void RE_Shader::UnloadMemory()
{
	RE_ShaderImporter::Delete(ID);
	ResourceContainer::inMemory = false;
}

void RE_Shader::SetAsInternal(const char* vertexBuffer, const char* fragmentBuffer, const char* geometryBuffer)
{
	SetInternal(true);
	
	uint vertexLenght = strlen(vertexBuffer);
	uint fragmentLenght = strlen(fragmentBuffer);
	uint geometryLenght = (geometryBuffer) ? strlen(fragmentBuffer) : 0;
	eastl::string vbuffer(vertexBuffer, vertexLenght);
	eastl::string fbuffer(fragmentBuffer, fragmentLenght);
	eastl::string gbuffer;
	if (geometryBuffer) gbuffer = eastl::string(geometryBuffer, geometryLenght);

	eastl::string totalBuffer = vbuffer;
	totalBuffer += fbuffer;
	totalBuffer += gbuffer;

	SetMD5(MD5(totalBuffer).hexdigest().c_str());
	eastl::string libraryPath("Library/Shaders/");
	libraryPath += GetMD5();
	SetLibraryPath(libraryPath.c_str());

	App->shaders->LoadFromBuffer(&ID, vertexBuffer, vertexLenght, fragmentBuffer, fragmentLenght, geometryBuffer, geometryLenght);

	uniforms.clear();
	eastl::vector<eastl::string> lines;
	if (vertexBuffer) {
		eastl::vector<eastl::string> slines = GetUniformLines(vertexBuffer);
		if (!slines.empty()) lines.insert(lines.end(), slines.begin(), slines.end());
	}
	if (fragmentBuffer) {
		eastl::vector<eastl::string> slines = GetUniformLines(fragmentBuffer);
		if (!slines.empty()) lines.insert(lines.end(), slines.begin(), slines.end());
	}
	if (geometryBuffer) {
		eastl::vector<eastl::string> slines = GetUniformLines(geometryBuffer);
		if (!slines.empty()) lines.insert(lines.end(), slines.begin(), slines.end());
	}

	MountShaderCvar(lines);
	GetLocations();

	LibrarySave();

	ResourceContainer::inMemory = true;
}

void RE_Shader::SetPaths(const char* vertex, const char* fragment, const char* geometry)
{
	shaderSettings.vertexShader = vertex;
	shaderSettings.fragmentShader = fragment;
	if (geometry) shaderSettings.geometryShader = geometry;

	eastl::string buffer = "";

	if (!shaderSettings.vertexShader.empty()) {
		RE_FileIO vertexBuffer(shaderSettings.vertexShader.c_str());
		if (vertexBuffer.Load()) {
			eastl::string vbuffer(vertexBuffer.GetBuffer(), vertexBuffer.GetSize());
			buffer += vbuffer;
		}
	}
	if (!shaderSettings.fragmentShader.empty()) {
		RE_FileIO fragmentBuffer(shaderSettings.fragmentShader.c_str());
		if (fragmentBuffer.Load()) {
			eastl::string fbuffer(fragmentBuffer.GetBuffer(), fragmentBuffer.GetSize());
			buffer += fbuffer;
		}
	}
	if (!shaderSettings.geometryShader.empty()) {
		RE_FileIO geometryBuffer(shaderSettings.geometryShader.c_str());
		if (geometryBuffer.Load()) {
			eastl::string gbuffer(geometryBuffer.GetBuffer(), geometryBuffer.GetSize());
			buffer += gbuffer;
		}
	}

	ParseAndGetUniforms();

	SetMD5(MD5(buffer).hexdigest().c_str());
	eastl::string libraryPath("Library/Shaders/");
	libraryPath += GetMD5();
	SetLibraryPath(libraryPath.c_str());

	if (!isInternal()) SetMetaPath("Assets/Shaders/");
}

eastl::vector<ShaderCvar> RE_Shader::GetUniformValues()
{
	return uniforms;
}

void RE_Shader::UploadMainUniforms(RE_CompCamera* camera, float _dt, float _time, float window_h, float window_w, bool clipDistance, math::float4 clipPlane)
{
	RE_GLCache::ChangeShader(ID);
	if(view != -1) RE_ShaderImporter::setFloat4x4(uniforms[view].location, camera->GetViewPtr());
	if(projection != -1) RE_ShaderImporter::setFloat4x4(uniforms[projection].location, camera->GetProjectionPtr());
	if (dt != -1) RE_ShaderImporter::setFloat(uniforms[dt].location, _dt);
	if (time != -1) RE_ShaderImporter::setFloat(uniforms[time].location, _time);
	if (viewport_h != -1) RE_ShaderImporter::setFloat(uniforms[viewport_h].location, window_h);
	if (viewport_w != -1) RE_ShaderImporter::setFloat(uniforms[viewport_w].location, window_w);
	if (near_plane != -1) RE_ShaderImporter::setFloat(uniforms[near_plane].location, camera->GetNearPlane());
	if (far_plane != -1) RE_ShaderImporter::setFloat(uniforms[far_plane].location, camera->GetFarPlane());
	if (using_clip_plane != -1) RE_ShaderImporter::setFloat(uniforms[using_clip_plane].location, (clipDistance) ? 1.0f : -1.0f);
	if (clip_plane != -1) RE_ShaderImporter::setFloat(uniforms[clip_plane].location, clipPlane.x, clipPlane.y, clipPlane.z, clipPlane.w);
}

void RE_Shader::UploadModel(float* _model)
{
	RE_GLCache::ChangeShader(ID);
	if(model != -1) RE_ShaderImporter::setFloat4x4(uniforms[model].location, _model);
}

void RE_Shader::UploadDepth(int texture)
{
	RE_GLCache::ChangeShader(ID);
	if (depth != -1) RE_ShaderImporter::setInt(uniforms[depth].location, texture);
}

bool RE_Shader::isShaderFilesChanged()
{
	bool ret = false;

	const char* shaderpath;
	signed long long lastTimeModified = 0;
	if (!shaderSettings.vertexShader.empty()) {
		GetVertexFileInfo(shaderpath, &lastTimeModified);
		PHYSFS_Stat shaderfilestat;
		if (PHYSFS_stat(shaderpath, &shaderfilestat)) {
			if (lastTimeModified != shaderfilestat.modtime) {
				shaderSettings.vlastModified = shaderfilestat.modtime;
				ret = true;
			}
		}
	}

	if (!shaderSettings.fragmentShader.empty()) {
		GetFragmentFileInfo(shaderpath, &lastTimeModified);
		PHYSFS_Stat shaderfilestat;
		if (PHYSFS_stat(shaderpath, &shaderfilestat)) {
			if (lastTimeModified != shaderfilestat.modtime) {
				shaderSettings.flastModified = shaderfilestat.modtime;
				ret = true;
			}
		}
	}

	if (!shaderSettings.geometryShader.empty()) {
	GetGeometryFileInfo(shaderpath, &lastTimeModified);
		PHYSFS_Stat shaderfilestat;
		if (PHYSFS_stat(shaderpath, &shaderfilestat)) {
			if (lastTimeModified != shaderfilestat.modtime) {
				shaderSettings.glastModified = shaderfilestat.modtime;
				ret = true;
			}
		}
	}

	return ret;
}

void RE_Shader::ReImport()
{
	bool reload = (ResourceContainer::inMemory);
	if (reload) UnloadMemory();
	SetPaths(shaderSettings.vertexShader.c_str(), shaderSettings.fragmentShader.c_str(), (!shaderSettings.geometryShader.empty()) ? shaderSettings.geometryShader.c_str() : nullptr);
	AssetLoad();
	ParseAndGetUniforms();
	LibrarySave();
	SaveMeta();
	if (!reload) UnloadMemory();
	else GetLocations();

	//Send Resource Event
	Event::Push(RE_EventType::RESOURCE_CHANGED, App->resources, Cvar(GetMD5()));
}

bool RE_Shader::IsPathOnShader(const char* assetPath)
{
	eastl::string path = assetPath;
	if(path == shaderSettings.vertexShader || path == shaderSettings.fragmentShader || path == shaderSettings.geometryShader)
		return true;
	return false;
}

void RE_Shader::GetVertexFileInfo(const char*& path, signed long long* lastTimeModified) const
{
	path = shaderSettings.vertexShader.c_str();
	*lastTimeModified = shaderSettings.vlastModified;
}

void RE_Shader::GetFragmentFileInfo(const char*& path, signed long long* lastTimeModified) const
{
	path = shaderSettings.fragmentShader.c_str();
	*lastTimeModified = shaderSettings.flastModified;
}

void RE_Shader::GetGeometryFileInfo(const char*& path, signed long long* lastTimeModified) const
{
	path = shaderSettings.geometryShader.c_str();
	*lastTimeModified = shaderSettings.glastModified;
}

void RE_Shader::ParseAndGetUniforms()
{
	uniforms.clear();
	eastl::vector<eastl::string> lines;
	if (!shaderSettings.vertexShader.empty()) {
		RE_FileIO sFile(shaderSettings.vertexShader.c_str());
		if (sFile.Load()) {
			eastl::vector<eastl::string> slines = GetUniformLines(sFile.GetBuffer());
			if (!slines.empty()) lines.insert(lines.end(), slines.begin(), slines.end());
		}
	}
	if (!shaderSettings.fragmentShader.empty()) {
		RE_FileIO sFile(shaderSettings.fragmentShader.c_str());
		if (sFile.Load()) {
			eastl::vector<eastl::string> slines = GetUniformLines(sFile.GetBuffer());
			if (!slines.empty()) lines.insert(lines.end(), slines.begin(), slines.end());
		}
	}
	if (!shaderSettings.geometryShader.empty()) {
		RE_FileIO sFile(shaderSettings.geometryShader.c_str());
		if (sFile.Load()) {
			eastl::vector<eastl::string> slines = GetUniformLines(sFile.GetBuffer());
			if (!slines.empty()) lines.insert(lines.end(), slines.begin(), slines.end());
		}
	}
	MountShaderCvar(lines);
}

eastl::vector<eastl::string> RE_Shader::GetUniformLines(const char* buffer)
{
	eastl::vector<eastl::string> lines;
	eastl::string parse(buffer);
	int linePos = parse.find_first_of('\n');

	while (linePos != eastl::string::npos)
	{
		eastl::string line = parse.substr(0, linePos);
		int exitsUniform = line.find("uniform");
		if (exitsUniform != eastl::string::npos) lines.push_back(line);
		parse.erase(0, linePos + 1);
		linePos = parse.find_first_of('\n');
	} 

	return lines;
}

void RE_Shader::MountShaderCvar(eastl::vector<eastl::string> uniformLines)
{
	projection = -1;
	view = -1;
	model = -1;
	time = -1;
	dt = -1;
	depth = -1;
	viewport_w = -1;
	viewport_h = -1;
	near_plane = -1;
	far_plane = -1;
	uniforms.clear();

	static const char* internalNames[32] = { "useTexture", "useColor", "useClipPlane", "clip_plane", "time", "dt", "near_plane", "far_plane", "viewport_w", "viewport_h", "model", "view", "projection", "cdiffuse", "tdiffuse", "cspecular", "tspecular", "cambient", "tambient", "cemissive", "temissive", "ctransparent", "opacity", "topacity", "tshininess", "shininess", "shininessST", "refraccti", "theight", "tnormals", "treflection", "currentDepth" };
	for (auto uniform : uniformLines)
	{
		int pos = uniform.find_first_of(" ");
		if (pos != eastl::string::npos)
		{
			ShaderCvar sVar;

			pos++;
			eastl::string typeAndName = uniform.substr(pos);

			eastl::string varType = typeAndName.substr(0, pos = typeAndName.find_first_of(" "));

			sVar.name = typeAndName.substr(pos + 1, typeAndName.size() - pos - 2);

			bool b2[2] = { false, false };
			bool b3[3] = { false, false, false };
			bool b4[4] = { false, false, false, false };

			int i2[2] = { 0, 0 };
			int i3[3] = { 0, 0, 0 };
			int i4[4] = { 0, 0, 0, 0 };
			float f = 0.0;

			//Find type
			if (varType.compare("bool") == 0)
				sVar.SetValue(true, true);
			else if (varType.compare("int") == 0)
				sVar.SetValue(-1, true);
			else if (varType.compare("sampler2DShadow") == 0 || varType.compare("sampler1DShadow") == 0 || varType.compare("samplerCube") == 0 || varType.compare("sampler3D") == 0 || varType.compare("sampler1D") == 0 || varType.compare("sampler2D") == 0)
				sVar.SetSampler(nullptr, true);
			else if (varType.compare("float") == 0)
				sVar.SetValue(f, true);
			else if (varType.compare("vec2") == 0)
				sVar.SetValue(math::float2::zero, true);
			else if (varType.compare("vec3") == 0)
				sVar.SetValue(math::float3::zero, true);
			else if (varType.compare("vec4") == 0)
				sVar.SetValue(math::float4::zero, false, true);
			else if (varType.compare("bvec2") == 0)
				sVar.SetValue(b2, 2, true);
			else if (varType.compare("bvec3") == 0)
				sVar.SetValue(b3, 3, true);
			else if (varType.compare("bvec4") == 0)
				sVar.SetValue(b4, 4, true);
			else if (varType.compare("ivec2") == 0)
				sVar.SetValue(i2, 2, true);
			else if (varType.compare("ivec3") == 0)
				sVar.SetValue(i3, 3, true);
			else if (varType.compare("ivec4") == 0)
				sVar.SetValue(i4, 4, true);
			else if (varType.compare("mat2") == 0)
				sVar.SetValue(math::float4::zero, true, true);
			else if (varType.compare("mat3") == 0)
				sVar.SetValue(math::float3x3::zero, true);
			else if (varType.compare("mat4") == 0)
				sVar.SetValue(math::float4x4::zero, true);
			else
				continue;

			int pos = sVar.name.find_first_of("0123456789");

			eastl::string name = (pos != eastl::string::npos) ? sVar.name.substr(0, pos) : sVar.name;

			//Custom or internal variables
			for (uint i = 0; i < 32; i++) {
				if (name.compare(internalNames[i]) == 0) {
					sVar.custom = false;
					break;
				}
			}

			uniforms.push_back(sVar);

			if (!sVar.custom) {
				if (projection == -1 && sVar.name.compare("projection") == 0)
					projection = uniforms.size() - 1;
				else if (view == -1 && sVar.name.compare("view") == 0)
					view = uniforms.size() - 1;
				else if (model == -1 && sVar.name.compare("model") == 0)
					model = uniforms.size() - 1;
				else if (dt == -1 && sVar.name.compare("dt") == 0)
					dt = uniforms.size() - 1;
				else if (time == -1 && sVar.name.compare("time") == 0)
					time = uniforms.size() - 1;
				else if (depth == -1 && sVar.name.compare("currentDepth") == 0)
					depth = uniforms.size() - 1;
				else if (viewport_w == -1 && sVar.name.compare("viewport_w") == 0)
					viewport_w = uniforms.size() - 1;
				else if (viewport_h == -1 && sVar.name.compare("viewport_h") == 0)
					viewport_h = uniforms.size() - 1;
				else if (near_plane == -1 && sVar.name.compare("near_plane") == 0)
					near_plane = uniforms.size() - 1;
				else if (far_plane == -1 && sVar.name.compare("far_plane") == 0)
					far_plane = uniforms.size() - 1;
				else if (using_clip_plane == -1 && sVar.name.compare("useClipPlane") == 0)
					using_clip_plane = uniforms.size() - 1;
				else if (clip_plane == -1 && sVar.name.compare("clip_plane") == 0)
					clip_plane = uniforms.size() - 1;
			}
		}
	}
}

void RE_Shader::GetLocations()
{
	for (unsigned int i = 0; i < uniforms.size(); i++) 
		uniforms[i].location = RE_ShaderImporter::getLocation(ID, uniforms[i].name.c_str());
}

bool RE_Shader::NeedUploadDepth() const
{
	return (depth != -1);
}

void RE_Shader::Draw()
{
	//Todo drag & drop of shader files
	ImGui::Text("Vertex Shader path: %s", shaderSettings.vertexShader.c_str());
	if (!shaderSettings.vertexShader.empty())
		if (ImGui::Button("Edit vertex"))
			App->editor->OpenTextEditor(shaderSettings.vertexShader.c_str(), &shaderSettings.vertexShader);
	ImGui::Text("Fragment Shader path: %s", shaderSettings.fragmentShader.c_str());
	if (!shaderSettings.fragmentShader.empty())
		if (ImGui::Button("Edit fragment"))
			App->editor->OpenTextEditor(shaderSettings.fragmentShader.c_str(), &shaderSettings.fragmentShader);
	ImGui::Text("Geometry Shader path: %s", shaderSettings.geometryShader.c_str());
	if (!shaderSettings.geometryShader.empty())
		if (ImGui::Button("Edit geometry"))
			App->editor->OpenTextEditor(shaderSettings.geometryShader.c_str(), &shaderSettings.geometryShader);
}

void RE_Shader::SaveResourceMeta(JSONNode* metaNode)
{
	metaNode->PushString("vertexPath", shaderSettings.vertexShader.c_str());
	metaNode->PushSignedLongLong("vLastModified", shaderSettings.vlastModified);
	metaNode->PushString("fragmentPath", shaderSettings.fragmentShader.c_str());
	metaNode->PushSignedLongLong("fLastModified", shaderSettings.flastModified);
	metaNode->PushString("geometryPath", shaderSettings.geometryShader.c_str());
	metaNode->PushSignedLongLong("gLastModified", shaderSettings.glastModified);

	JSONNode* nuniforms = metaNode->PushJObject("uniforms");
	nuniforms->PushUInt("size", uniforms.size());
	if (!uniforms.empty()) {
		eastl::string id;
		for (uint i = 0; i < uniforms.size(); i++)
		{
			id = "name";
			id += eastl::to_string(i);
			nuniforms->PushString(id.c_str(), uniforms[i].name.c_str());
			id = "type";
			id += eastl::to_string(i);
			nuniforms->PushUInt(id.c_str(), uniforms[i].GetType());
			id = "custom";
			id += eastl::to_string(i);
			nuniforms->PushBool(id.c_str(), uniforms[i].custom);
		}
	}
	DEL(nuniforms);
}

void RE_Shader::LoadResourceMeta(JSONNode* metaNode)
{
	shaderSettings.vertexShader = metaNode->PullString("vertexPath", "");
	shaderSettings.vlastModified = metaNode->PullSignedLongLong("vLastModified", 0);
	shaderSettings.fragmentShader = metaNode->PullString("fragmentPath", "");
	shaderSettings.flastModified = metaNode->PullSignedLongLong("fLastModified", 0);
	shaderSettings.geometryShader = metaNode->PullString("geometryPath", "");
	shaderSettings.glastModified = metaNode->PullSignedLongLong("gLastModified", 0);

	projection = -1;
	view = -1;
	model = -1;
	time = -1;
	dt = -1;
	depth = -1;
	viewport_w = -1;
	viewport_h = -1;
	near_plane = -1;
	far_plane = -1;
	uniforms.clear();
	JSONNode* nuniforms = metaNode->PullJObject("uniforms");
	uint size = nuniforms->PullUInt("size", 0);
	if (size) {
		eastl::string id;
		for (uint i = 0; i < size; i++)
		{
			ShaderCvar sVar;
			id = "name";
			id += eastl::to_string(i);
			sVar.name = nuniforms->PullString(id.c_str(), "");
			id = "type";
			id += eastl::to_string(i);
			Cvar::VAR_TYPE vT = (Cvar::VAR_TYPE)nuniforms->PullUInt(id.c_str(), ShaderCvar::UNDEFINED);
			id = "custom";
			id += eastl::to_string(i);
			sVar.custom = nuniforms->PullBool(id.c_str(), true);

			bool b2[2] = { false, false };
			bool b3[3] = { false, false, false };
			bool b4[4] = { false, false, false, false };

			int i2[2] = { 0, 0 };
			int i3[3] = { 0, 0, 0 };
			int i4[4] = { 0, 0, 0, 0 };
			float f = 0.0;

			switch (vT)
			{
			case Cvar::BOOL:
				sVar.SetValue(true, true);
				break;
			case Cvar::BOOL2:
				sVar.SetValue(b2, 2, true);
				break;
			case Cvar::BOOL3:
				sVar.SetValue(b3, 3, true);
				break;
			case Cvar::BOOL4:
				sVar.SetValue(b4, 4, true);
				break;
			case Cvar::INT:
				sVar.SetValue(-1, true);
				break;
			case Cvar::INT2:
				sVar.SetValue(i2, 2, true);
				break;
			case Cvar::INT3:
				sVar.SetValue(i3, 3, true);
				break;
			case Cvar::INT4:
				sVar.SetValue(i4, 4, true);
				break;
			case Cvar::FLOAT:
				sVar.SetValue(f, true);
				break;
			case Cvar::FLOAT2:
				sVar.SetValue(math::float2::zero, true);
				break;
			case Cvar::FLOAT3:
				sVar.SetValue(math::float3::zero, true);
				break;
			case Cvar::FLOAT4:
				sVar.SetValue(math::float4::zero, false, true);
				break;
			case Cvar::MAT2:
				sVar.SetValue(math::float4::zero, true, true);
				break;
			case Cvar::MAT3:
				sVar.SetValue(math::float3x3::zero, true);
				break;
			case Cvar::MAT4:
				sVar.SetValue(math::float4x4::zero, true);
				break;
			case Cvar::SAMPLER:
				sVar.SetSampler(nullptr, true);
				break;
			}

			uniforms.push_back(sVar);
			if (!sVar.custom) {
				if (projection == -1 && sVar.name.compare("projection") == 0)
					projection = uniforms.size() -1;
				else if (view == -1 && sVar.name.compare("view") == 0)
					view = uniforms.size() - 1;
				else if (model == -1 && sVar.name.compare("model") == 0)
					model = uniforms.size() - 1;
				else if (dt == -1 && sVar.name.compare("dt") == 0)
					dt = uniforms.size() - 1;
				else if (time == -1 && sVar.name.compare("time") == 0)
					time = uniforms.size() - 1;
				else if (depth == -1 && sVar.name.compare("currentDepth") == 0)
					depth = uniforms.size() - 1;
				else if (viewport_w == -1 && sVar.name.compare("viewport_w") == 0)
					viewport_w = uniforms.size() - 1;
				else if (viewport_h == -1 && sVar.name.compare("viewport_h") == 0)
					viewport_h = uniforms.size() - 1;
				else if (near_plane == -1 && sVar.name.compare("near_plane") == 0)
					near_plane = uniforms.size() - 1;
				else if (far_plane == -1 && sVar.name.compare("far_plane") == 0)
					far_plane = uniforms.size() - 1;
				else if (using_clip_plane == -1 && sVar.name.compare("useClipPlane") == 0)
					using_clip_plane = uniforms.size() - 1;
				else if (clip_plane == -1 && sVar.name.compare("clip_plane") == 0)
					clip_plane = uniforms.size() - 1;
			}
		}
	}
	DEL(nuniforms);

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
		if (!App->shaders->LoadFromBinary(libraryLoad.GetBuffer(), libraryLoad.GetSize(), &ID)) {
			AssetLoad();
			LibrarySave();
		}
		ResourceContainer::inMemory = true;
	}
}

void RE_Shader::LibrarySave()
{
	RE_FileIO librarySave(GetLibraryPath(), App->fs->GetZipPath());

	char* buffer = nullptr;
	int size = 0;
	if (App->shaders->GetBinaryProgram(ID, &buffer, &size))
		librarySave.Save(buffer, size);
	DEL_A(buffer);
}
