#ifndef __AABB_DYNAMIC_TREE_H__
#define __AABB_DYNAMIC_TREE_H__

#include "RE_HashMap.h"
#include <EASTL/queue.h>

typedef unsigned long long Object_UID;

struct RE_AABBDynTreeNode
{
	AABB box;
	Object_UID object_index = 0;
	size_t parent_index = -1, child1 = -1, child2 = -1;
	bool is_leaf = true;
};

class RE_AABBDynTree : public RE_HashMap<RE_AABBDynTreeNode, size_t, 1024, 512>
{
public:

	RE_AABBDynTree() = default;
	~RE_AABBDynTree() final = default;

	void PushNode(Object_UID id, AABB box);
	void PopNode(Object_UID index);
	void UpdateNode(Object_UID index, AABB box);

	eastl::vector<size_t> GetAllKeys() const final;
	void Clear();
	void CollectIntersections(Ray ray, eastl::queue<Object_UID>& indexes) const;
	void CollectIntersections(const Frustum frustum, eastl::queue<Object_UID>& indexes) const;

	void Draw() const;
	size_t GetCount() const;

	eastl::map<Object_UID, size_t> objectToNode;

private:

	void Rotate(RE_AABBDynTreeNode& node, size_t index);

	size_t AllocateLeafNode(AABB box, Object_UID index);
	size_t AllocateInternalNode();

	inline AABB Union(AABB box1, AABB box2);
	inline void SetLeaf(RE_AABBDynTreeNode& node, AABB box, Object_UID index);
	inline void SetInternal(RE_AABBDynTreeNode& node);

private:

	size_t randomCount = 0;
	size_t size = 0;
	size_t node_count = 0;
	size_t root_index = -1;
};

#endif // !__AABB_DYNAMIC_TREE_H__