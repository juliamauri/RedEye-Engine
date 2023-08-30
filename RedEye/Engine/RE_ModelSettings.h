#ifndef __RE_MODEL_SETTINGS_H__
#define __RE_MODEL_SETTINGS_H__

#include <EASTL/vector.h>

struct RE_ModelSettings
{
	eastl::vector<const char*> libraryMeshes;

	//presets
	int GetPresetSelected() const;
	bool presets[3] = { false, false, false };
	//bool Preset_TargetRealtime_Fast
	//bool Preset_TargetRealtime_MaxQuality
	//bool Preset_TargetRealtime_Quality

	unsigned int GetFlags() const;
	bool flags[25] = { true, true, true, false, true, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false };
	//0 CalcTangentSpace
	//1 JoinIdenticalVertices
	//2 Triangulate
	//3 RemoveComponent
	//4 GenNormals
	//5 GenSmoothNormals
	// SplitLargeMeshes
	// PreTransformVertices
	// LimitBoneWeights
	// ValidateDataStructure
	//10 ImproveCacheLocality
	// RemoveRedundantMaterials
	// FixInfacingNormals
	// SortByPType
	// FindDegenerates
	//15 FindInvalidData
	// GenUVCoords
	// TransformUVCoords
	// FindInstances
	// OptimizeMeshes
	//20 OptimizeGraph
	// FlipUVs
	// FlipWindingOrder
	// SplitByBoneCount
	//24 Debone

	inline bool operator==(const RE_ModelSettings& b)
	{
		bool ret = true;
		for (unsigned int i = 0; i < 3; i++)
		{
			if (presets[i] != b.presets[i])
			{
				ret = false;
				break;
			}
		}
		if (ret)
		{
			for (unsigned int i = 0; i < 25; i++)
			{
				if (flags[i] != b.flags[i])
				{
					ret = false;
					break;
				}
			}
		}
		return ret;
	}
	inline bool operator!=(const RE_ModelSettings& b)
	{
		bool ret = true;
		for (unsigned int i = 0; i < 3; i++)
		{
			if (presets[i] == b.presets[i])
			{
				ret = false;
				break;
			}
		}
		if (ret)
		{
			for (unsigned int i = 0; i < 25; i++)
			{
				if (flags[i] == b.flags[i])
				{
					ret = false;
					break;
				}
			}
		}
		return ret;
	}
};

#endif // !__RE_MODEL_SETTINGS_H__