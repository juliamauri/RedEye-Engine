#ifndef __RE_ECS_IMPORTER_H__
#define __RE_ECS_IMPORTER_H__

class RE_ECS_Pool;
class RE_Json;

namespace RE_ECS_Importer
{
	void JsonSerialize(RE_Json* node, RE_ECS_Pool* pool);
	char* BinarySerialize(RE_ECS_Pool* pool, unsigned int* bufferSize);

	RE_ECS_Pool* JsonDeserialize(RE_Json* node);
	RE_ECS_Pool* BinaryDeserialize(char*& cursor);

	bool JsonCheckResources(RE_Json* node);
	bool BinaryCheckResources(char*& cursor);
};

#endif // !__RE_RESOURCEANDGOIMPORTER_H__