#ifndef __RE_RESOURCEANDGOIMPORTER_H__
#define __RE_RESOURCEANDGOIMPORTER_H__

class RE_ECS_Manager;
class JSONNode;

class RE_ECS_Importer
{
public:
	static void JsonSerialize(JSONNode* node, RE_ECS_Manager* pool);
	static char* BinarySerialize(RE_ECS_Manager* pool, unsigned int* bufferSize);

	static RE_ECS_Manager* JsonDeserialize(JSONNode* node);
	static RE_ECS_Manager* BinaryDeserialize(char*& cursor);

	static bool JsonCheckResources(JSONNode* node);
	static bool BinaryCheckResources(char*& cursor);
};

#endif // !__RE_RESOURCEANDGOIMPORTER_H__