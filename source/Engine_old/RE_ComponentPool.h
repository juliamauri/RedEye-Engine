#ifndef __RE_COMPONENT_POOL_H__
#define __RE_COMPONENT_POOL_H__

#include "RE_Memory.h"
#include "RE_Random.h"
#include "Application.h"
#include "RE_Json.h"
#include "RE_HashMap.h"

class GameObjectsPool;

template<class COMPCLASS, unsigned int size, unsigned int increment>
class ComponentPool : public RE_HashMap<COMPCLASS, COMP_UID, size, increment>
{
public:
	ComponentPool() { }
	~ComponentPool() { }

	void SetName(const char* name)
	{
		cName = name;
	}

	void Update()
	{
		for (int i = 0; i < lastAvaibleIndex; ++i) pool_[i].Update();
	}

	void Clear()
	{
		key_map.clear();
		lastAvaibleIndex = 0;
	}

	COMP_UID Push(COMPCLASS val) final
	{
		COMP_UID ret = RANDOM_UID;
		RE_HashMap::Push(val, ret);
		RE_HashMap::pool_[RE_HashMap::key_map.at(ret)].SetPoolID(ret);
		return ret;
	}

	COMP_UID GetNewCompUID()
	{
		return Push({});
	}

	eastl::vector<const char*> GetAllResources()
	{
		eastl::vector<const char*> ret;
		for (int i = 0; i < lastAvaibleIndex; i++) {
			eastl::vector<const char*> cmpres = pool_[i].GetAllResources();
			if (!cmpres.empty()) ret.insert(ret.end(), cmpres.begin(), cmpres.end());
		}
		return ret;
	}

	void UseResources()
	{
		for (int i = 0; i < lastAvaibleIndex; i++)
			pool_[i].UseResources();
	}

	void UnUseResources()
	{
		for (int i = 0; i < lastAvaibleIndex; i++)
			pool_[i].UnUseResources();
	}

	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources)
	{
		RE_Json* compPool = node->PushJObject(cName.c_str());

		size_t count = GetCount();
		compPool->PushSizeT("poolSize", count);

		for (size_t i = 0; i < count; i++)
		{
			RE_Json* comp = compPool->PushJObject(eastl::to_string(i).c_str());
			comp->Push("parentPoolID", pool_[i].GetGOUID());
			pool_[i].SerializeJson(comp, resources);
			DEL(comp)
		}
		DEL(compPool)
	}

	void DeserializeJson(GameObjectsPool* goPool, RE_Json* node, eastl::map<int, const char*>* resources)
	{
		RE_Json* comp_objs = node->PullJObject(cName.c_str());
		auto cmpSize = comp_objs->PullSizeT("poolSize", 0);
		for (size_t i = 0; i < cmpSize; i++)
		{
			COMPCLASS* comp_ptr = AtPtr(Push({}));
			RE_Json* comp_obj = comp_objs->PullJObject(eastl::to_string(i).c_str());
			comp_ptr->PoolSetUp(goPool, comp_obj->PullUnsignedLongLong("parentPoolID", 0), true);
			comp_ptr->DeserializeJson(comp_obj, resources);

			DEL(comp_obj)
		}
		DEL(comp_objs)
	}

	size_t GetBinarySize() const
	{
		// Count
		size_t size = sizeof(size_t);

		// GO_UID & Component Size
		for (size_t i = 0; i < GetCount(); i++)
			size += sizeof(GO_UID) + pool_[i].GetBinarySize();

		return size;
	}

	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
	{
		// Count
		size_t size = sizeof(size_t);
		size_t count = GetCount();
		memcpy(cursor, &count, size);
		cursor += size;

		// Serialize each GO_UID & Component data
		size = sizeof(GO_UID);
		for (size_t i = 0; i < count; i++)
		{
			GO_UID uid = pool_[i].GetGOUID();
			memcpy(cursor, &uid, size);
			cursor += size;

			pool_[i].SerializeBinary(cursor, resources);
		}
	}

	void DeserializeBinary(GameObjectsPool* goPool, char*& cursor, eastl::map<int, const char*>* resources)
	{
		// Count
		size_t size = sizeof(size_t);
		size_t totalComps;
		memcpy(&totalComps, cursor, size);
		cursor += size;

		// Deserialize each GO_UID & Component data
		size = sizeof(GO_UID);
		for (size_t i = 0; i < totalComps; i++)
		{
			GO_UID goID;
			memcpy(&goID, cursor, size);
			cursor += size;

			COMPCLASS* comp_ptr = AtPtr(Push({}));
			comp_ptr->PoolSetUp(goPool, goID, true);
			comp_ptr->DeserializeBinary(cursor, resources);
		}
	}

	eastl::vector<COMP_UID> GetAllKeys() const final
	{
		eastl::vector<COMP_UID> ret;
		for (const auto &cmp : key_map) ret.push_back(cmp.first);
		return ret;
	}

private:

	eastl::string cName;
};

//Components Pools
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CompMesh.h"
#include "RE_CompLight.h"
#include "RE_CompWater.h"
#include "RE_CompParticleEmitter.h"
typedef ComponentPool<RE_CompTransform, 1024, 512> TransformsPool;
typedef ComponentPool<RE_CompCamera, 128, 64> CamerasPool;
typedef ComponentPool<RE_CompMesh, 128, 64> MeshesPool;
typedef ComponentPool<RE_CompLight, 128, 64> LightPool;
typedef ComponentPool<RE_CompWater, 8, 8> WaterPool;
typedef ComponentPool<RE_CompParticleEmitter, 32, 16> ParticleSystemPool;

//Primitives
#include "RE_CompPrimitive.h"
typedef ComponentPool<RE_CompGrid, 128, 64> GridPool;
typedef ComponentPool<RE_CompRock, 128, 64> RockPool;
typedef ComponentPool<RE_CompCube, 128, 64> CubePool;
typedef ComponentPool<RE_CompDodecahedron, 128, 64> DodecahedronPool;
typedef ComponentPool<RE_CompTetrahedron, 128, 64> TetrahedronPool;
typedef ComponentPool<RE_CompOctohedron, 128, 64> OctohedronPool;
typedef ComponentPool<RE_CompIcosahedron, 128, 64> IcosahedronPool;
typedef ComponentPool<RE_CompPoint, 128, 64> PointPool;
typedef ComponentPool<RE_CompPlane, 128, 64> PlanePool;
typedef ComponentPool<RE_CompSphere, 128, 64> SpherePool;
typedef ComponentPool<RE_CompCylinder, 128, 64> CylinderPool;
typedef ComponentPool<RE_CompHemiSphere, 128, 64> HemiSpherePool;
typedef ComponentPool<RE_CompTorus, 128, 64> TorusPool;
typedef ComponentPool<RE_CompTrefoiKnot, 128, 64> TrefoiKnotPool;

#endif // !__RE_COMPONENT_POOL_H__