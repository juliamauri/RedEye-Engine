#ifndef __REMODELIMPORTER_H__
#define __REMODELIMPORTER_H__

struct aiMesh;
struct aiMaterial;
struct RE_ModelSettings;
class RE_ECS_Pool;

namespace RE_ModelImporter
{
	eastl::vector<eastl::string> GetOutsideResourcesAssetsPath(const char * path);
	RE_ECS_Pool* ProcessModel(const char* buffer, size_t size, const char* assetPayh, RE_ModelSettings* mSettings);

	struct CurrentlyImporting
	{
		RE_ModelSettings* settings = nullptr;
		eastl::map<aiMesh*, const char*> meshesLoaded;
		eastl::map<aiMaterial*, const char*> materialsLoaded;
		eastl::string workingfilepath;
		eastl::string name;
	};
};

#endif // !__REMODELIMPORTER_H__