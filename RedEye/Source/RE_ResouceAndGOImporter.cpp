#include "RE_ResouceAndGOImporter.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "Resource.h"

#include "RE_GameObject.h"


void RE_ResouceAndGOImporter::JsonSerialize(JSONNode* node, RE_GameObject* toSerialize)
{
	//Get resources
	std::vector<const char*>  resGo = toSerialize->GetAllResources();
	std::map<const char*, int> resourcesIndex;
	int count = 0;
	for (const char* res : resGo) resourcesIndex.insert(std::pair<const char*, int>(res, count++));

	//Resources Serialize
	JSONNode* resources = node->PushJObject("resources");
	resources->PushUInt("resSize", resGo.size());
	std::string ref;
	for (int r = 0; r < resGo.size(); r++) {
		ref = "r";
		ref += std::to_string(r);
		JSONNode* resN = resources->PushJObject(ref.c_str());
		ResourceContainer* res = App->resources->At(resGo.at(r));
		Resource_Type rtype = res->GetType();

		resN->PushInt("index", r);
		resN->PushInt("type", rtype);
		resN->PushString("mPath", (rtype == Resource_Type::R_MESH) ? res->GetLibraryPath() : res->GetMetaPath());

		DEL(resN);
	}
	DEL(resources);

	//GOs Serialize
	toSerialize->SerializeJson(node, &resourcesIndex);
}

char* RE_ResouceAndGOImporter::BinarySerialize(RE_GameObject* toSerialize, unsigned int* bufferSize)
{
	//Get resources
	std::vector<const char*>  resGo = toSerialize->GetAllResources();
	std::vector<ResourceContainer*>  resC;
	std::map<const char*, int> resourcesIndex;
	int count = 0;
	for (const char* res : resGo) {
		resourcesIndex.insert(std::pair<const char*, int>(res, count++));
		resC.push_back(App->resources->At(res));
	}

	*bufferSize = sizeof(uint) + ((sizeof(int) + sizeof(unsigned int) + sizeof(uint)) * resGo.size());
	for (ResourceContainer* res : resC) *bufferSize += std::strlen((res->GetType() == Resource_Type::R_MESH) ? res->GetLibraryPath() : res->GetMetaPath()) * sizeof(char);
	*bufferSize += toSerialize->GetBinarySize();
	*bufferSize += 1;
	char* buffer = new char[*bufferSize + 1];
	char* cursor = buffer;

	size_t size = sizeof(uint);
	uint resSize = resGo.size();
	memcpy(cursor, &resSize, size);
	cursor += size;

	for (int r = 0; r < resGo.size(); r++) {
		ResourceContainer* res = resC.at(r);
		Resource_Type rtype = res->GetType();

		size = sizeof(int);
		memcpy(cursor, &r, size);
		cursor += size;

		size = sizeof(unsigned int);
		unsigned int typeI = rtype;
		memcpy(cursor, &typeI, size);
		cursor += size;

		const char* toCopy = (rtype == Resource_Type::R_MESH) ? res->GetLibraryPath() : res->GetMetaPath();
		uint strsize = std::strlen(toCopy);
		size = sizeof(uint);
		memcpy(cursor, &strsize, size);
		cursor += size;

		size = sizeof(char) * strsize;
		memcpy(cursor, toCopy, size);
		cursor += size;
	}

	//GOs Serialize
	toSerialize->SerializeBinary(cursor, &resourcesIndex);

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	return buffer;
}

RE_GameObject* RE_ResouceAndGOImporter::JsonDeserialize(JSONNode* node)
{
	//Get resources
	JSONNode* resources = node->PullJObject("resources");

	std::map< int, const char*> resourcesIndex;

	uint resSize = resources->PullUInt("resSize", 0);
	std::string ref;
	for (uint r = 0; r < resSize; r++) {
		ref = "r";
		ref += std::to_string(r);
		JSONNode* resN = resources->PullJObject(ref.c_str());

		int index = resN->PullInt("index", -1);
		Resource_Type type = (Resource_Type)resN->PullInt("type", Resource_Type::R_UNDEFINED);
		std::string mPath = resN->PullString("mPath", "");

		const char* resMD5 = nullptr;
		(type == Resource_Type::R_MESH) ?
			resMD5 = App->resources->CheckOrFindMeshOnLibrary(mPath.c_str()) :
			resMD5 = App->resources->FindMD5ByMETAPath(mPath.c_str(), type);

		if (resMD5) resourcesIndex.insert(std::pair< int, const char*>(r, resMD5));

		DEL(resN);
	}
	DEL(resources);

	return RE_GameObject::DeserializeJSON(node, &resourcesIndex);;
}

RE_GameObject* RE_ResouceAndGOImporter::BinaryDeserialize(char*& cursor)
{
	std::map< int, const char*> resourcesIndex;
	//Get resources
	size_t size = sizeof(uint);
	uint resSize = 0;
	memcpy(&resSize, cursor, size);
	cursor += size;

	for (uint r = 0; r < resSize; r++) {

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
			resMD5 = App->resources->CheckOrFindMeshOnLibrary(str) :
			resMD5= App->resources->FindMD5ByMETAPath(str, rType);

		if (resMD5) resourcesIndex.insert(std::pair< int, const char*>(index, resMD5));

		DEL_A(str);
	}
	return RE_GameObject::DeserializeBinary(cursor, &resourcesIndex);;
}
