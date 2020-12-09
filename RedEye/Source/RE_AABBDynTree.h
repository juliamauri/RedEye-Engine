#ifndef __AABB_DYNAMIC_TREE_H__
#define __AABB_DYNAMIC_TREE_H__

#include "RE_HashMap.h"
#include "Globals.h"
#include "MathGeoLib\include\Geometry\AABB.h"
#include <EASTL/queue.h>

struct RE_AABBDynTreeNode
{
	AABB box;
	UID object_index = 0;
	int parent_index = -1, child1 = -1, child2 = -1;
	bool is_leaf = true;
};

class RE_AABBDynTree : public RE_HashMap<RE_AABBDynTreeNode, int, 1024, 512>
{
public:

	RE_AABBDynTree();
	~RE_AABBDynTree();

	void PushNode(UID goUID, AABB box);
	void PopNode(UID index);
	void UpdateNode(UID index, AABB box);

	eastl::vector<int> GetAllKeys() const override;
	void Clear();
	void CollectIntersections(Ray ray, eastl::queue<UID>& indexes) const;
	void CollectIntersections(const Frustum frustum, eastl::queue<UID>& indexes) const;

	void Draw()const;
	int GetCount() const;

	eastl::map<UID, int> objectToNode;

private:

	void Rotate(RE_AABBDynTreeNode& node, int index);

	int AllocateLeafNode(AABB box, UID index);
	int AllocateInternalNode();

	static inline AABB Union(AABB box1, AABB box2);
	static inline void SetLeaf(RE_AABBDynTreeNode& node, AABB box, UID index);
	static inline void SetInternal(RE_AABBDynTreeNode& node);

private:

	int randomCount = 0, size = 0, node_count = 0, root_index = -1;
};

#endif // !__AABB_DYNAMIC_TREE_H__