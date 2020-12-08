#ifndef __RE_ECS_IMPORTER_H__
#define __RE_ECS_IMPORTER_H__

class RE_ECS_Manager;
class JSONNode;

namespace RE_ECS_Importer
{
	void JsonSerialize(JSONNode* node, RE_ECS_Manager* pool);
	char* BinarySerialize(RE_ECS_Manager* pool, unsigned int* bufferSize);

	RE_ECS_Manager* JsonDeserialize(JSONNode* node);
	RE_ECS_Manager* BinaryDeserialize(char*& cursor);

	bool JsonCheckResources(JSONNode* node);
	bool BinaryCheckResources(char*& cursor);
};

#endif // !__RE_RESOURCEANDGOIMPORTER_H__