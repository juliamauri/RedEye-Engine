#ifndef __RE_SKYBOX_H__
#define __RE_SKYBOX_H__

#include "Resource.h"
#include "RE_SkyBoxSettings.h"

class RE_SkyBox : public ResourceContainer
{
public:
	RE_SkyBox() = default;
	RE_SkyBox(const char* metaPath) : ResourceContainer(metaPath) {}
	~RE_SkyBox() final = default;

	void LoadInMemory() final;
	void UnloadMemory() final;

	void Import(bool keepInMemory = true) final;

	void use();
	unsigned int GetID()const { return ID; }
	unsigned int GetVAO()const { return VAO; }

	void SetAsInternal();

	void AddTexture(SkyBoxTexture::Face face, const char* textureMD5);
	void AddTexturePath(SkyBoxTexture::Face face, const char* path);
	void AssetSave();

	void DrawSkybox() const;

	void DrawEditSkyBox();

	bool isFacesFilled() const;

private:

	void Draw() final;
	void SaveResourceMeta(RE_Json* metaNode) const final;
	void LoadResourceMeta(RE_Json* metaNode) final;

	bool NeededResourcesReferenced(RE_Json* metaNode) final;

	void AssetLoad(bool generateLibraryPath = false);
	void LibraryLoad();
	void LibrarySave();

	void TexParameteri(unsigned int pname, int param);

	void LoadSkyBoxSphere();

private:

	uint ID = 0;
	uint VAO = 0;
	uint VBO = 0;
	uint EBO = 0;
	uint triangle_count = 0;

	bool applySize = false;
	bool applyTextures = false;
	bool applySave = false;

	RE_SkyBoxSettings skyBoxSettings;
	RE_SkyBoxSettings restoreSettings;

	static const char* texturesname[6];
};

#endif // !__RE_SKYBOX_H__