#include "RE_ECS_Importer.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "Resource.h"
#include "RE_ECS_Manager.h"

#include <EASTL/internal/char_traits.h>

void RE_ECS_Importer::JsonSerialize(JSONNode* node, RE_ECS_Manager* pool)
{
	//Get resources
	eastl::vector<const char*>  resGo = pool->GetAllResources();
	eastl::map<const char*, int> resourcesIndex;
	int count = 0;
	for (const char* res : resGo) resourcesIndex.insert(eastl::pair<const char*, int>(res, count++));

	//Resources Serialize
	JSONNode* resources = node->PushJObject("resources");
	resources->PushUInt("resSize", resGo.size());
	for (int r = 0; r < static_cast<int>(resGo.size()); r++)
	{
		JSONNode* resN = resources->PushJObject(("r" + eastl::to_string(r)).c_str());
		ResourceContainer* res = App::resources->At(resGo.at(static_cast<unsigned int>(r)));
		Resource_Type rtype = res->GetType();

		resN->PushInt("index", r);
		resN->PushInt("type", rtype);
		resN->PushString("mPath", (rtype == Resource_Type::R_MESH) ? res->GetLibraryPath() : res->GetMetaPath());

		DEL(resN);
	}
	DEL(resources);

	//GOs Serialize
	pool->SerializeJson(node, &resourcesIndex);
}

char* RE_ECS_Importer::BinarySerialize(RE_ECS_Manager* pool, unsigned int* bufferSize)
{
	//Get resources
	eastl::vector<const char*>  resGo = pool->GetAllResources();
	eastl::vector<ResourceContainer*>  resC;
	eastl::map<const char*, int> resourcesIndex;
	int count = 0;
	for (const char* res : resGo)
	{
		resourcesIndex.insert(eastl::pair<const char*, int>(res, count++));
		resC.push_back(App::resources->At(res));
	}

	*bufferSize = sizeof(uint) + ((sizeof(int) + sizeof(unsigned int) + sizeof(uint)) * resGo.size());
	for (ResourceContainer* res : resC)
		*bufferSize += eastl::CharStrlen((res->GetType() == Resource_Type::R_MESH) ? res->GetLibraryPath() : res->GetMetaPath()) * sizeof(char);
	*bufferSize += pool->GetBinarySize();
	*bufferSize += 1;
	char* buffer = new char[*bufferSize];
	char* cursor = buffer;

	size_t size = sizeof(uint);
	uint resSize = resGo.size();
	memcpy(cursor, &resSize, size);
	cursor += size;

	for (int r = 0; r < static_cast<int>(resGo.size()); r++)
	{
		ResourceContainer* res = resC.at(static_cast<unsigned int>(r));
		Resource_Type rtype = res->GetType();

		size = sizeof(int);
		memcpy(cursor, &r, size);
		cursor += size;

		size = sizeof(unsigned int);
		unsigned int typeI = rtype;
		memcpy(cursor, &typeI, size);
		cursor += size;

		const char* toCopy = (rtype == Resource_Type::R_MESH) ? res->GetLibraryPath() : res->GetMetaPath();
		uint strsize = eastl::CharStrlen(toCopy);
		size = sizeof(uint);
		memcpy(cursor, &strsize, size);
		cursor += size;

		size = sizeof(char) * strsize;
		memcpy(cursor, toCopy, size);
		cursor += size;
	}


	//GOs Serialize
	pool->SerializeBinary(cursor, &resourcesIndex);
	

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	return buffer;
}

RE_ECS_Manager* RE_ECS_Importer::JsonDeserialize(JSONNode* node)
{
	//Get resources
	JSONNode* resources = node->PullJObject("resources");
	eastl::map< int, const char*> resourcesIndex;

	uint resSize = resources->PullUInt("resSize", 0);
	for (uint r = 0; r < resSize; r++)
	{
		JSONNode* resN = resources->PullJObject(("r" + eastl::to_string(r)).c_str());

		int index = resN->PullInt("index", -1);
		Resource_Type type = (Resource_Type)resN->PullInt("type", Resource_Type::R_UNDEFINED);
		eastl::string mPath = resN->PullString("mPath", "");

		const char* resMD5 = nullptr;
		resMD5 = (type == Resource_Type::R_MESH) ?
			App::resources->CheckOrFindMeshOnLibrary(mPath.c_str()) :
			App::resources->FindMD5ByMETAPath(mPath.c_str(), type);

		resourcesIndex.insert(eastl::pair< int, const char*>(r, resMD5));
		DEL(resN);
	}

	DEL(resources);
	RE_ECS_Manager* ret = new RE_ECS_Manager();
	ret->DeserializeJson(node, &resourcesIndex);
	return ret;
}

RE_ECS_Manager* RE_ECS_Importer::BinaryDeserialize(char*& cursor)
{
	//Get resources
	eastl::map< int, const char*> resourcesIndex;
	size_t size = sizeof(uint);
	uint resSize = 0;
	memcpy(&resSize, cursor, size);
	cursor += size;

	for (uint r = 0; r < resSize; r++)
	{
		size = sizeof(int);
		int index = 0;
		memcpy(&index, cursor, size);
		cursor += size;

		size = sizeof(unsigned int);
		unsigned int typeI = 0;
		memcpy(&typeI, cursor, size);
		cursor += size;
		Resource_Type rType = (Resource_Type)typeI;

		size = sizeof(uint);
		uint strsize = 0;
		memcpy(&strsize, cursor, size);
		cursor += size;

		char* str = new char[strsize + 1];
		char* strCursor = str;
		size = strsize * sizeof(char);
		memcpy(str, cursor, size);
		cursor += size;
		strCursor += size;
		char nullchar = '\0';
		memcpy(strCursor, &nullchar, sizeof(char));

		const char* resMD5 = nullptr;
		(rType == Resource_Type::R_MESH) ?
			resMD5 = App::resources->CheckOrFindMeshOnLibrary(str) :
			resMD5= App::resources->FindMD5ByMETAPath(str, rType);

		resourcesIndex.insert(eastl::pair< int, const char*>(index, resMD5));
		DEL_A(str);
	}

	RE_ECS_Manager* ret = new RE_ECS_Manager();
	ret->DeserializeBinary(cursor, &resourcesIndex);
	return ret;
}

bool RE_ECS_Importer::JsonCheckResources(JSONNode* node)
{
	bool ret = true;
	JSONNode* resources = node->PullJObject("resources");
	uint resSize = resources->PullUInt("resSize", 0);
	eastl::string ref;

	for (uint r = 0; r < resSize && ret; r++)
	{
		ref = "r";
		ref += eastl::to_string(r);
		JSONNode* resN = resources->PullJObject(ref.c_str());

		int index = resN->PullInt("index", -1);
		Resource_Type type = (Resource_Type)resN->PullInt("type", Resource_Type::R_UNDEFINED);
		eastl::string mPath = resN->PullString("mPath", "");

		const char* resMD5 = nullptr;
		resMD5 = (type == Resource_Type::R_MESH) ?
			App::resources->CheckOrFindMeshOnLibrary(mPath.c_str()) :
			App::resources->FindMD5ByMETAPath(mPath.c_str(), type);

		if (!resMD5) ret = false;
		DEL(resN);
	}

	DEL(resources);
	return ret;
}

bool RE_ECS_Importer::BinaryCheckResources(char*& cursor)
{
	bool ret = true;

	size_t size = sizeof(uint);
	uint resSize = 0;
	memcpy(&resSize, cursor, size);
	cursor += size;

	for (uint r = 0; r < resSize && ret; r++)
	{
		size = sizeof(int);
		int index = 0;
		memcpy(&index, cursor, size);
		cursor += size;

		size = sizeof(unsigned int);
		unsigned int typeI = 0;
		memcpy(&typeI, cursor, size);
		cursor += size;
		Resource_Type rType = (Resource_Type)typeI;

		size = sizeof(uint);
		uint strsize = 0;
		memcpy(&strsize, cursor, size);
		cursor += size;

		char* str = new char[strsize + 1];
		char* strCursor = str;
		size = strsize * sizeof(char);
		memcpy(str, cursor, size);
		cursor += size;
		strCursor += size;
		char nullchar = '\0';
		memcpy(strCursor, &nullchar, sizeof(char));

		const char* resMD5 = nullptr;
		resMD5 = (rType == Resource_Type::R_MESH) ?
			App::resources->CheckOrFindMeshOnLibrary(str) :
			App::resources->FindMD5ByMETAPath(str, rType);

		if (!resMD5) ret = false;
		DEL_A(str);
	}
	return ret;
}
