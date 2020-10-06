#ifndef __RE_RESOURCEANDGOIMPORTER_H__
#define __RE_RESOURCEANDGOIMPORTER_H__

class RE_GameObject;
class RE_GOManager;
class JSONNode;

class RE_ResouceAndGOImporter
{
public:
	static void JsonSerialize(JSONNode* node, RE_GameObject* toSerialize);
	static char* BinarySerialize(RE_GameObject* toSerialize, unsigned int* bufferSize);

	static RE_GOManager* JsonDeserialize(JSONNode* node);
	static RE_GOManager* BinaryDeserialize(char*& cursor);

	static bool JsonCheckResources(JSONNode* node);
	static bool BinaryCheckResources(char*& cursor);
};

#endif // !__RE_RESOURCEANDGOIMPORTER_H__