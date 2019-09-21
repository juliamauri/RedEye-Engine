#include "FileSystem.h"

#include "Application.h"
#include "OutputLog.h"
#include "Globals.h"
#include "ImGui\imgui.h"
#include "SDL2\include\SDL.h"
#include "SDL2\include\SDL_assert.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompMesh.h"
#include "RE_Mesh.h"
#include "ResourceManager.h"

#include "RapidJson\include\pointer.h"
#include "RapidJson\include\stringbuffer.h"
#include "RapidJson\include\writer.h"

#include "libzip/include/zip.h"

#include "PhysFS\include\physfs.h"

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

#ifdef _DEBUG
#pragma comment( lib, "libzip/zip_d.lib" )

#else
#pragma comment( lib, "libzip/zip_r.lib" )
#endif // _DEBUG

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include <fstream>

FileSystem::FileSystem() : engine_config(nullptr)
{}

FileSystem::~FileSystem()
{
	DEL(engine_config);

	PHYSFS_deinit();
}


bool FileSystem::Init(int argc, char* argv[])
{
	bool ret = false;

	if (PHYSFS_init(argv[0]) != 0)
	{
		PHYSFS_Version physfs_version;
		PHYSFS_VERSION(&physfs_version);
		char tmp[8];
		sprintf_s(tmp, 8, "%u.%u.%u", (int)physfs_version.major, (int)physfs_version.minor, (int)physfs_version.patch);
		App->ReportSoftware("PhysFS", tmp, "https://icculus.org/physfs/");
		App->ReportSoftware("Rapidjson", RAPIDJSON_VERSION_STRING, "http://rapidjson.org/");
		
		engine_path = "engine";
		library_path = "Library";
		assets_path = "Assets";


		zip_path = (GetExecutableDirectory());
		zip_path += "data.zip";
		PHYSFS_mount(zip_path.c_str(), NULL, 1);

		char **i;

		for (i = PHYSFS_getSearchPath(); *i != NULL; i++)
			LOG("[%s] is in the search path.\n", *i);
		PHYSFS_freeList(*i);

		const char* config_file = "Settings/config.json";
		engine_config = new Config(config_file, zip_path.c_str());
		if (engine_config->Load())
			ret = true;
		else
			LOG("Error while loading Engine Configuration file: %s\nRed Eye Engine will initialize with default configuration parameters.", config_file);
	}
	else
	{
		LOG("PhysFS could not initialize! Error: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}

	return ret;
}

Config* FileSystem::GetConfig() const
{
	return engine_config;
}

void FileSystem::DrawEditor()
{
	ImGui::Text("Executable Directory:");
	ImGui::TextWrappedV(GetExecutableDirectory(), "");

	ImGui::Separator();

	//ImGui::Text("Read Directories");
	//for (std::list<std::string>::iterator it = paths.begin(); it != paths.end(); ++it)
		//ImGui::TextWrappedV(it->c_str(), "");

	ImGui::Separator();

	ImGui::Text("Write Directory");
	ImGui::TextWrappedV(write_path.c_str(), "");
}

bool FileSystem::AddPath(const char * path_or_zip, const char * mount_point)
{
	bool ret = true;

	if (PHYSFS_mount(path_or_zip, mount_point, 1) == 0)
	{
		LOG("File System error while adding a path or zip(%s): %s\n", path_or_zip, PHYSFS_getLastError());
		ret = false;
	}
	else
	{
		//paths.push_back(path_or_zip);
	}

	return ret;
}

bool FileSystem::RemovePath(const char * path_or_zip)
{
	bool ret = true;

	if (PHYSFS_removeFromSearchPath(path_or_zip) == 0)
	{
		LOG("Error removing PhysFS Directory (%s): %s", path_or_zip, PHYSFS_getLastError());
		ret = false;
	}

	//paths.remove(path_or_zip);

	return ret;
}

bool FileSystem::SetWritePath(const char * dir)
{
	bool ret = true;

	if (!PHYSFS_setWriteDir(dir))
	{
		LOG("Error setting PhysFS Directory: %s", PHYSFS_getLastError());
		ret = false;
	}
	else
	{
		write_path = dir;
	}

	return ret;
}

const char * FileSystem::GetWritePath() const
{
	return write_path.c_str();
}

void FileSystem::LogFolderItems(const char * folder)
{
	char **rc = PHYSFS_enumerateFiles(folder);
	char **i;

	for (i = rc; *i != NULL; i++)
		LOG(" * We've got [%s].\n", *i);

	PHYSFS_freeList(rc);
}

// Quick Buffer From Platform-Dependent Path
RE_FileIO* FileSystem::QuickBufferFromPDPath(const char * full_path)// , char** buffer, unsigned int size)
{
	RE_FileIO* ret = nullptr;

	if (full_path != nullptr)
	{
		std::string file_path = full_path;
		std::string file_name = file_path.substr(file_path.find_last_of("\\") + 1);
		std::string ext = file_name.substr(file_name.find_last_of(".") + 1);
		file_path.erase(file_path.length() - file_name.length(), file_path.length());

		ret = new RE_FileIO(file_name.c_str());
		if (App->fs->AddPath(file_path.c_str()))
		{
			if (!ret->Load()) DEL(ret);
			App->fs->RemovePath(file_path.c_str());
		}
		else
		{
			DEL(ret);
		}
	}

	return ret;
}

bool FileSystem::Exists(const char* file) const
{
	return PHYSFS_exists(file) != 0;
}

bool FileSystem::IsDirectory(const char* file) const
{
	return PHYSFS_isDirectory(file) != 0;
}

const char* FileSystem::GetExecutableDirectory() const
{
	return PHYSFS_getBaseDir();
}

const char * FileSystem::GetZipPath()
{
	return zip_path.c_str();
}


////////////////////////////////////////////////////////////////////
///////////////////RE_FileIO
////////////////////////////////////////////////////////////////////

RE_FileIO::RE_FileIO(const char* file_name, const char* from_zip) : buffer(nullptr), file_name(file_name), from_zip(from_zip) {}

RE_FileIO::~RE_FileIO()
{
	DEL_A(buffer);
}

bool RE_FileIO::Load()
{
	size = HardLoad();
	return size > 0;
}

void RE_FileIO::Save()
{
	HardSave(buffer);
}

void RE_FileIO::Save(char * buffer, unsigned int size)
{
	WriteFile(from_zip, file_name, buffer, this->size = ((size == 0) ? strnlen_s(buffer, 0xffff) : size));
}

void RE_FileIO::ClearBuffer()
{
	delete[] buffer;
	buffer = nullptr;
}

char * RE_FileIO::GetBuffer()
{
	return (buffer);
}

const char* RE_FileIO::GetBuffer() const
{
	return (buffer);
}

inline bool RE_FileIO::operator!() const
{
	return buffer == nullptr;
}

unsigned int RE_FileIO::GetSize()
{
	return size;
}

unsigned int RE_FileIO::HardLoad()
{
	unsigned int ret = 0u;

	if (PHYSFS_exists(file_name))
	{
		PHYSFS_File* fs_file = PHYSFS_openRead(file_name);

		if (fs_file != NULL)
		{
			signed long long sll_size = PHYSFS_fileLength(fs_file);
			if (sll_size > 0)
			{
				buffer = new char[(unsigned int)sll_size + 1];
				signed long long amountRead = PHYSFS_read(fs_file, buffer, 1, (signed int)sll_size);
				
				if (amountRead != sll_size)
				{
					LOG("File System error while reading from file %s: %s", file_name, PHYSFS_getLastError());
					delete (buffer);
				}
				else
				{
					ret = (unsigned int)amountRead;
					buffer[ret] = '\0';
				}
			}

			if (PHYSFS_close(fs_file) == 0)
			{
				LOG("File System error while closing file %s: %s", file_name, PHYSFS_getLastError());
			}
		}
		else
		{
			LOG("File System error while opening file %s: %s", file_name, PHYSFS_getLastError());
		}
	}
	else
	{
		LOG("File System error while checking file %s: %s", file_name, PHYSFS_getLastError());
	}

	return ret;
}

void RE_FileIO::HardSave(const char* buffer)
{

	PHYSFS_file* file = PHYSFS_openWrite(file_name);




	if (file != NULL)
	{
		long long written = PHYSFS_write(file, (const void*)buffer, 1, size = (strnlen_s(buffer, 0xffff)));
		if (written != size)
		{
			LOG("Error while writing to file %s: %s", file, PHYSFS_getLastError());
		}

		if (PHYSFS_close(file) == 0)
			LOG("Error while closing save file %s: %s", file, PHYSFS_getLastError());
	}
	else
		LOG("Error while opening save file %s: %s", file, PHYSFS_getLastError());
}

void RE_FileIO::WriteFile(const char * zip_path, const char * filename, const char * buffer, unsigned int size)
{
	if (PHYSFS_removeFromSearchPath(from_zip) == 0)
		LOG("Ettot when unmount: %s", PHYSFS_getLastError());

	struct zip *f_zip = NULL;
	int error = 0;
	f_zip = zip_open(zip_path, ZIP_CHECKCONS, &error); /* on ouvre l'archive zip */
	if (error)	LOG("could not open or create archive: %s", zip_path);

	zip_source_t *s;

	s = zip_source_buffer(f_zip, buffer, size, 0);

	if (s == NULL ||
		zip_file_add(f_zip, filename, s, ZIP_FL_OVERWRITE + ZIP_FL_ENC_UTF_8) < 0) {
		zip_source_free(s);
		LOG("error adding file: %s\n", zip_strerror(f_zip));
	}

	zip_close(f_zip);
	f_zip = NULL;

	PHYSFS_mount(from_zip, NULL, 1);
}


////////////////////////////////////////////////////////////////////
///////////////////Config
////////////////////////////////////////////////////////////////////

//Tutorial http://rapidjson.org/md_doc_tutorial.html

Config::Config(const char* file_name, const char* from_zip) : RE_FileIO(file_name)
{
	zip_path = from_zip;
	this->from_zip = zip_path.c_str();
}

bool Config::Load()
{
	// Open file
	bool ret = false;

	size = HardLoad();

	if (ret = (size > 0))
	{
		rapidjson::StringStream s(buffer);
		document.ParseStream(s);
	}

	return ret;
}

void Config::Save()
{
	rapidjson::StringBuffer s_buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s_buffer);
	document.Accept(writer);
	s_buffer.Put('\0');
	size = s_buffer.GetSize();
	const char* output = s_buffer.GetString();

	std::string file(file_name);
	WriteFile(from_zip, file.c_str(), output, size);
}

JSONNode* Config::GetRootNode(const char* member)
{
	JSONNode* ret = nullptr;

	if (member != nullptr)
	{
		std::string path("/");
		path += member;
		rapidjson::Value* value = rapidjson::Pointer(path.c_str()).Get(document);

		if (value == nullptr)
		{
			value = &rapidjson::Pointer(path.c_str()).Create(document);
			LOG("Configuration node not found for %s, created new pointer", path.c_str());
		}

		ret = new JSONNode(path.c_str(), this);
	}
	else
	{
		LOG("Error Loading Configuration node: Empty Member Name");
	}

	return ret;
}

inline bool Config::operator!() const
{
	return document.IsNull();
}


////////////////////////////////////////////////////////////////////
///////////////////JSONNode
////////////////////////////////////////////////////////////////////

JSONNode::JSONNode(const char* path, Config* config, bool isArray) : pointerPath(path), config(config)
{
	if (isArray)	rapidjson::Pointer(path).Get(config->document)->SetArray();
}

JSONNode::JSONNode(JSONNode& node) : pointerPath(node.pointerPath), config(node.config)
{}

JSONNode::~JSONNode()
{
	config = nullptr;
}


// Push ============================================================

void JSONNode::PushBool(const char* name, const bool value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushInt(const char* name, const int value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;
		
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushUInt(const char* name, const unsigned int value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushFloat(const char* name, const float value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushFloatVector(const char * name, math::vec vector)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value float_array(rapidjson::kArrayType);
		float_array.PushBack(vector.x, config->document.GetAllocator()).PushBack(vector.y, config->document.GetAllocator()).PushBack(vector.z, config->document.GetAllocator());

		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void JSONNode::PushDouble(const char* name, const double value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushString(const char* name, const char* value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushValue(rapidjson::Value * val)
{
	rapidjson::Value* val_push = rapidjson::Pointer(pointerPath.c_str()).Get(config->document);
	if (val_push->IsArray())
		val_push->PushBack(*val, config->document.GetAllocator());
}

void JSONNode::PushMeshVertex(std::vector<Vertex>& vertexes, std::vector<unsigned int>& indeces)
{
	rapidjson::Value position(rapidjson::kArrayType);
	rapidjson::Value normals(rapidjson::kArrayType);
	rapidjson::Value TextCoods(rapidjson::kArrayType);
	rapidjson::Value indeces_val(rapidjson::kArrayType);

	unsigned int i = 0;
	for (auto vertex : vertexes)
	{
		rapidjson::Value float_array(rapidjson::kArrayType);

		float_array.PushBack(vertex.Position.x, config->document.GetAllocator()).PushBack(vertex.Position.y, config->document.GetAllocator()).PushBack(vertex.Position.z, config->document.GetAllocator());
		position.PushBack(float_array.Move(), config->document.GetAllocator());
		
		float_array.SetArray();
		float_array.PushBack(vertex.Normal.x, config->document.GetAllocator()).PushBack(vertex.Normal.y, config->document.GetAllocator()).PushBack(vertex.Normal.z, config->document.GetAllocator());
		normals.PushBack(float_array.Move(), config->document.GetAllocator());
		
		float_array.SetArray();
		float_array.PushBack(vertex.TexCoords.x, config->document.GetAllocator()).PushBack(vertex.TexCoords.y, config->document.GetAllocator());
		TextCoods.PushBack(float_array.Move(), config->document.GetAllocator());

		i++;
	}
	
	for (unsigned int index : indeces)
		indeces_val.PushBack(index, config->document.GetAllocator());

	rapidjson::Value* mesh = rapidjson::Pointer(pointerPath.c_str()).Get(config->document);

	mesh->AddMember(rapidjson::Value::StringRefType("num_vertexes"), rapidjson::Value().SetUint(i), config->document.GetAllocator());
	mesh->AddMember(rapidjson::Value::StringRefType("positions"), position, config->document.GetAllocator());
	mesh->AddMember(rapidjson::Value::StringRefType("normals"), normals, config->document.GetAllocator());
	mesh->AddMember(rapidjson::Value::StringRefType("texturecoods"), TextCoods, config->document.GetAllocator());
	mesh->AddMember(rapidjson::Value::StringRefType("indexes"), indeces_val, config->document.GetAllocator());
}

JSONNode* JSONNode::PushJObject(const char* name)
{
	JSONNode* ret = nullptr;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		ret = new JSONNode(path.c_str(), config);
	}

	return ret;
}

// Pull ============================================================

bool JSONNode::PullBool(const char* name, bool deflt)
{
	bool ret = false;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetBool() : deflt;
	}

	return ret;
}

int JSONNode::PullInt(const char* name, int deflt)
{
	int ret = 0;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetInt() : deflt;
	}

	return ret;
}

unsigned int JSONNode::PullUInt(const char* name, const unsigned int deflt)
{
	unsigned int ret = 0;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetUint() : deflt;
	}

	return ret;
}


float JSONNode::PullFloat(const char* name, float deflt)
{
	float ret = 0;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetFloat() : deflt;
	}

	return ret;
}

math::vec JSONNode::PullFloatVector(const char * name, math::vec deflt)
{

	math::vec ret = math::vec::zero;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);

		if (val != nullptr)
			ret.Set(val->GetArray()[0].GetFloat(),
			val->GetArray()[1].GetFloat(),
			val->GetArray()[2].GetFloat());
		else
			ret = deflt;
	}

	return ret;
}

double JSONNode::PullDouble(const char* name, double deflt)
{
	double ret = 0;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetDouble() : deflt;
	}

	return ret;
}

const char*	JSONNode::PullString(const char* name, const char* deflt)
{
	const char* ret = nullptr;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);

		ret = (val != nullptr) ? val->GetString() : deflt;
	}

	return ret;
}

void JSONNode::PullMeshVertex(std::vector<Vertex>* vertexes, std::vector<unsigned int>* indeces)
{
	std::string path(pointerPath);
	path += "/";

	std::string num_vertexess_path(path);
	num_vertexess_path += "num_vertexes";
	unsigned int num_vertexes = rapidjson::Pointer(num_vertexess_path.c_str()).Get(config->document)->GetUint();

	std::string positions_path(path);
	positions_path += "positions";
	rapidjson::Value* positions_val = rapidjson::Pointer(positions_path.c_str()).Get(config->document);

	std::string normals_path(path);
	normals_path += "normals";
	rapidjson::Value* normals_val = rapidjson::Pointer(normals_path.c_str()).Get(config->document);

	std::string texturecoods_path(path);
	texturecoods_path += "texturecoods";
	rapidjson::Value* texturecoods_val = rapidjson::Pointer(texturecoods_path.c_str()).Get(config->document);

	std::string indexes_path(path);
	indexes_path += "indexes";
	rapidjson::Value* indexes_val = rapidjson::Pointer(indexes_path.c_str()).Get(config->document);

	if (positions_val->IsArray() && normals_val->IsArray() && texturecoods_val->IsArray() && indexes_val->IsArray())
	{
		for (unsigned int i = 0; i < num_vertexes; i++)
		{
			Vertex vert;
			rapidjson::Value fill = positions_val->GetArray()[i].GetArray();
			vert.Position.Set(fill[0].GetFloat(), fill[1].GetFloat(), fill[2].GetFloat());

			fill = normals_val->GetArray()[i].GetArray();
			vert.Normal.Set(fill[0].GetFloat(), fill[1].GetFloat(), fill[2].GetFloat());

			fill = texturecoods_val->GetArray()[i].GetArray();
			vert.TexCoords.Set(fill[0].GetFloat(), fill[1].GetFloat());

			vertexes->push_back(vert);
		}

		for (auto& index : indexes_val->GetArray())
			indeces->push_back(index.GetUint());
	}
}

JSONNode* JSONNode::PullJObject(const char * name)
{
	JSONNode* ret = nullptr;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		ret = new JSONNode(path.c_str(), config);
	}

	return ret;
}

RE_GameObject * JSONNode::FillGO()
{
	RE_GameObject * go = nullptr;

	bool first = true;
	rapidjson::Value* val = rapidjson::Pointer(pointerPath.c_str()).Get(config->document);

	if (val->IsArray())
	{
		for (auto& v : val->GetArray())
		{
			UUID uuid;
			UUID parent_uuid;

			if (first)
			{
				UuidFromStringA((RPC_CSTR)v.FindMember("UUID")->value.GetString(), &uuid);
				go = new RE_GameObject(v.FindMember("name")->value.GetString(), uuid);

				rapidjson::Value& vector = v.FindMember("position")->value;
				go->GetTransform()->SetPosition({ vector.GetArray()[0].GetFloat() , vector.GetArray()[1].GetFloat() , vector.GetArray()[2].GetFloat() });

				vector = v.FindMember("scale")->value;
				go->GetTransform()->SetScale({ vector.GetArray()[0].GetFloat() , vector.GetArray()[1].GetFloat() , vector.GetArray()[2].GetFloat() });

				vector = v.FindMember("rotation")->value;
				go->GetTransform()->SetRotation({ vector.GetArray()[0].GetFloat() , vector.GetArray()[1].GetFloat() , vector.GetArray()[2].GetFloat() });

				first = false;
				continue;
			}
			UuidFromStringA((RPC_CSTR)v.FindMember("UUID")->value.GetString(), &uuid);
			UuidFromStringA((RPC_CSTR)v.FindMember("Parent UUID")->value.GetString(), &parent_uuid);

			RE_GameObject* new_go = new RE_GameObject(v.FindMember("name")->value.GetString(), uuid, go->GetGoFromUUID(parent_uuid));

			rapidjson::Value& vector = v.FindMember("position")->value;
			new_go->GetTransform()->SetPosition({ vector.GetArray()[0].GetFloat() , vector.GetArray()[1].GetFloat() , vector.GetArray()[2].GetFloat() });

			vector = v.FindMember("scale")->value;
			new_go->GetTransform()->SetScale({ vector.GetArray()[0].GetFloat() , vector.GetArray()[1].GetFloat() , vector.GetArray()[2].GetFloat() });

			vector = v.FindMember("rotation")->value;
			new_go->GetTransform()->SetRotation({ vector.GetArray()[0].GetFloat() , vector.GetArray()[1].GetFloat() , vector.GetArray()[2].GetFloat() });
		
			rapidjson::Value& components = v.FindMember("components")->value;
			if (components.IsArray())
			{
				for (auto& c : components.GetArray())
				{
					ComponentType type = (ComponentType)c.FindMember("type")->value.GetInt();

					RE_CompMesh* mesh = nullptr;
					rapidjson::Value* textures_val = nullptr;
					rapidjson::Value* vector = nullptr;
					math::vec position = math::vec::zero;
					math::vec scale = math::vec::zero;
					math::vec rotation = math::vec::zero;
					std::string file;
					const char* reference = nullptr;
					switch (type)
					{
					case C_MESH:
						file = c.FindMember("file")->value.GetString();
						reference = c.FindMember("reference")->value.GetString();
						App->resources->CheckFileLoaded(file.c_str(), reference, Resource_Type::R_MESH);
						mesh = new RE_CompMesh(new_go, reference);
						textures_val = &c.FindMember("textures")->value;
						if (textures_val->IsArray())
							for (auto& t : textures_val->GetArray())
							{
								file = t.FindMember("file")->value.GetString();
								reference = t.FindMember("reference")->value.GetString();
								App->resources->CheckFileLoaded(file.c_str(), reference, Resource_Type::R_TEXTURE);
								mesh->SetTexture(reference,file.c_str(), t.FindMember("type")->value.GetString());
							}
						new_go->AddCompMesh(mesh);
						break;
					case C_CAMERA:
						vector = &v.FindMember("position")->value;
						position.Set(vector->GetArray()[0].GetFloat(), vector->GetArray()[1].GetFloat(), vector->GetArray()[2].GetFloat());

						vector = &v.FindMember("scale")->value;
						scale.Set(vector->GetArray()[0].GetFloat(), vector->GetArray()[1].GetFloat(), vector->GetArray()[2].GetFloat());

						vector = &v.FindMember("rotation")->value;
						rotation.Set(vector->GetArray()[0].GetFloat(), vector->GetArray()[1].GetFloat(), vector->GetArray()[2].GetFloat());

						new_go->AddCompCamera(
							c.FindMember("isPrespective")->value.GetBool(),
							c.FindMember("near_plane")->value.GetFloat(),
							c.FindMember("far_plane")->value.GetFloat(),
							c.FindMember("pitch")->value.GetFloat(),
							c.FindMember("yaw")->value.GetFloat(),
							c.FindMember("roll")->value.GetFloat(),
							c.FindMember("h_fov_rads")->value.GetFloat(),
							c.FindMember("v_fov_rads")->value.GetFloat(),
							c.FindMember("h_fov_degrees")->value.GetFloat(),
							c.FindMember("v_fov_degrees")->value.GetFloat(),
							position,
							rotation,
							scale);
						break;
					}
				}
			}
		}
	}

	return go;
}

inline bool JSONNode::operator!() const
{
	return (config == nullptr) || pointerPath.empty();
}

const char * JSONNode::GetDocumentPath() const
{
	return pointerPath.c_str();
}

rapidjson::Document * JSONNode::GetDocument()
{
	return &config->document;
}

void JSONNode::SetArray()
{
	rapidjson::Pointer(pointerPath.c_str()).Get(config->document)->SetArray();
}

void JSONNode::SetObject()
{
	rapidjson::Pointer(pointerPath.c_str()).Get(config->document)->SetObject();
}
