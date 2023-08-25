#ifndef __RE_SKYBOX_H__
#define __RE_SKYBOX_H__

#include "RE_SkyBoxSettings.h"

class RE_SkyBox : public ResourceContainer
{
public:
	RE_SkyBox() {}
	RE_SkyBox(const char* metaPath) : ResourceContainer(metaPath) {}
	~RE_SkyBox() {}

	void LoadInMemory() override;
	void UnloadMemory() override;

	void Import(bool keepInMemory = true) override;

	void use();
	unsigned int GetID()const { return ID; }
	unsigned int GetVAO()const { return VAO; }

	void SetAsInternal();

	void AddTexture(RE_TextureFace face, const char* textureMD5);
	void AddTexturePath(RE_TextureFace face, const char* path);
	void AssetSave();

	void DrawSkybox() const;

	void DrawEditSkyBox();

	bool isFacesFilled() const;

private:

	void Draw() override;
	void SaveResourceMeta(RE_Json* metaNode) override;
	void LoadResourceMeta(RE_Json* metaNode) override;

	bool NeededResourcesReferenced(RE_Json* metaNode) override;

	void AssetLoad(bool generateLibraryPath = false);
	void LibraryLoad();
	void LibrarySave();

	void TexParameteri(unsigned int pname, int param);

	void LoadSkyBoxSphere();

private:

	unsigned int ID = 0, VAO = 0, VBO = 0, EBO = 0, triangle_count = 0;
	bool applySize = false, applyTextures = false, applySave = false;
	RE_SkyBoxSettings skyBoxSettings, restoreSettings;

	static const char* texturesname[6];
};

#endif // !__RE_SKYBOX_H__