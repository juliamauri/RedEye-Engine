#include "RE_ECS_Importer.h"

#include "RE_Memory.h"
#include "RE_Json.h"
#include "Application.h"
#include "RE_ECS_Pool.h"
#include "RE_ResourceManager.h"
#include "Resource.h"

#include <EASTL/internal/char_traits.h>
#include <EASTL/bit.h>

void RE_ECS_Importer::JsonSerialize(RE_Json* node, RE_ECS_Pool* pool)
{
	//Get resources
	eastl::vector<const char*>  resGo = pool->GetAllResources();
	eastl::map<const char*, int> resourcesIndex;
	int count = 0;
	for (const char* res : resGo) resourcesIndex.insert(eastl::pair<const char*, int>(res, count++));

	//Resources Serialize
	RE_Json* resources = node->PushJObject("resources");
	resources->PushSizeT("resSize", resGo.size());
	for (int r = 0; r < static_cast<int>(resGo.size()); r++)
	{
		RE_Json* resN = resources->PushJObject(("r" + eastl::to_string(r)).c_str());
		ResourceContainer* res = RE_RES->At(resGo.at(static_cast<unsigned int>(r)));
		ResourceContainer::Type rtype = res->GetType();

		resN->Push("index", r);
		resN->Push("type", static_cast<uint>(rtype));
		resN->Push("mPath", (rtype == ResourceContainer::Type::MESH) ? res->GetLibraryPath() : res->GetMetaPath());

		DEL(resN)
	}
	DEL(resources)

	//GOs Serialize
	pool->SerializeJson(node, &resourcesIndex);
}

char* RE_ECS_Importer::BinarySerialize(RE_ECS_Pool* pool, size_t* bufferSize)
{
	//Get resources
	eastl::vector<const char*>  resGo = pool->GetAllResources();
	eastl::vector<ResourceContainer*>  resC;
	eastl::map<const char*, int> resourcesIndex;
	int count = 0;
	for (const char* res : resGo)
	{
		resourcesIndex.insert(eastl::pair<const char*, int>(res, count++));
		resC.push_back(RE_RES->At(res));
	}

	*bufferSize = sizeof(uint) + ((sizeof(int) + sizeof(ushort) + sizeof(uint)) * resGo.size());
	for (ResourceContainer* res : resC)
		*bufferSize += eastl::CharStrlen((res->GetType() == ResourceContainer::Type::MESH) ? res->GetLibraryPath() : res->GetMetaPath()) * sizeof(char);
	*bufferSize += pool->GetBinarySize();
	*bufferSize += 1;

	char* buffer = new char[*bufferSize];
	char* cursor = buffer;

	size_t size = sizeof(uint);
	eastl_size_t resSize = resGo.size();
	memcpy(eastl::bit_cast<void*>(cursor), &resSize, size);
	cursor += size;

	for (int r = 0; r < static_cast<int>(resGo.size()); r++)
	{
		ResourceContainer* res = resC.at(static_cast<unsigned int>(r));
		ResourceContainer::Type rtype = res->GetType();

		size = sizeof(int);
		memcpy(cursor, &r, size);
		cursor += size;

		size = sizeof(ushort);
		auto typeI = static_cast<ushort>(rtype);
		memcpy(cursor, &typeI, size);
		cursor += size;

		const char* toCopy = (rtype == ResourceContainer::Type::MESH) ? res->GetLibraryPath() : res->GetMetaPath();
		size_t strsize = eastl::CharStrlen(toCopy);
		size = sizeof(size_t);
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

RE_ECS_Pool* RE_ECS_Importer::JsonDeserialize(RE_Json* node)
{
	//Get resources
	RE_Json* resources = node->PullJObject("resources");
	eastl::map< int, const char*> resourcesIndex;

	size_t resSize = resources->PullSizeT("resSize", 0);
	for (uint r = 0; r < resSize; r++)
	{
		RE_Json* resN = resources->PullJObject(("r" + eastl::to_string(r)).c_str());

		int index = resN->PullInt("index", -1);
		auto type = static_cast<ResourceContainer::Type>(resN->PullUInt("type", static_cast<const uint>(ResourceContainer::Type::UNDEFINED)));
		eastl::string mPath = resN->PullString("mPath", "");

		const char* resMD5 = nullptr;
		resMD5 = (type == ResourceContainer::Type::MESH) ?
			RE_RES->CheckOrFindMeshOnLibrary(mPath.c_str()) :
			RE_RES->FindMD5ByMETAPath(mPath.c_str(), type);

		resourcesIndex.insert(eastl::pair< int, const char*>(r, resMD5));
		DEL(resN)
	}

	DEL(resources)
	RE_ECS_Pool* ret = new RE_ECS_Pool();
	ret->DeserializeJson(node, &resourcesIndex);
	return ret;
}

RE_ECS_Pool* RE_ECS_Importer::BinaryDeserialize(char*& cursor)
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

		size = sizeof(ushort);
		ushort typeI = 0;
		memcpy(&typeI, cursor, size);
		cursor += size;
		auto rType = static_cast<ResourceContainer::Type>(typeI);

		size = sizeof(size_t);
		size_t strsize = 0;
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
		(rType == ResourceContainer::Type::MESH) ?
			resMD5 = RE_RES->CheckOrFindMeshOnLibrary(str) :
			resMD5= RE_RES->FindMD5ByMETAPath(str, rType);

		resourcesIndex.insert(eastl::pair< int, const char*>(index, resMD5));
		DEL_A(str);
	}

	RE_ECS_Pool* ret = new RE_ECS_Pool();
	ret->DeserializeBinary(cursor, &resourcesIndex);
	return ret;
}

bool RE_ECS_Importer::JsonCheckResources(RE_Json* node)
{
	bool ret = true;
	RE_Json* resources = node->PullJObject("resources");
	uint resSize = resources->PullUInt("resSize", 0);
	eastl::string ref;

	for (uint r = 0; r < resSize && ret; r++)
	{
		ref = "r";
		ref += eastl::to_string(r);
		RE_Json* resN = resources->PullJObject(ref.c_str());

		int index = resN->PullInt("index", -1);
		auto type = static_cast<ResourceContainer::Type>(resN->PullUInt("type", static_cast<unsigned int>(ResourceContainer::Type::UNDEFINED)));
		eastl::string mPath = resN->PullString("mPath", "");

		const char* resMD5 = nullptr;
		resMD5 = (type == ResourceContainer::Type::MESH) ?
			RE_RES->CheckOrFindMeshOnLibrary(mPath.c_str()) :
			RE_RES->FindMD5ByMETAPath(mPath.c_str(), type);

		if (!resMD5) ret = false;
		DEL(resN)
	}

	DEL(resources)
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

		size = sizeof(ushort);
		ushort typeI = 0;
		memcpy(&typeI, cursor, size);
		cursor += size;
		auto rType = static_cast<ResourceContainer::Type>(typeI);

		size = sizeof(size_t);
		size_t strsize = 0;
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
		resMD5 = (rType == ResourceContainer::Type::MESH) ?
			RE_RES->CheckOrFindMeshOnLibrary(str) :
			RE_RES->FindMD5ByMETAPath(str, rType);

		if (!resMD5) ret = false;
		DEL_A(str);
	}
	return ret;
}
