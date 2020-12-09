#ifndef __RE_SKYBOX_IMPORTER_H__
#define __RE_SKYBOX_IMPORTER_H__

#include "RE_SkyBoxSettings.h"

class RE_FileBuffer;

namespace RE_SkyboxImporter
{
	void LoadSkyBoxInMemory(RE_SkyBoxSettings& settings, unsigned int* ID, bool isDDS = false);
};

#endif // !__RE_SKYBOX_IMPORTER_H__