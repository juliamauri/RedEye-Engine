#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "RE_GameObject.h"
#include "MathGeoLib\include\Geometry\AABB.h"

#include "PoolMapped.h"

#include <list>
#include <iterator>
#include <stack>

class QTree
{
public:
	QTree();
	~QTree();

	void	Build(RE_GameObject* root_g_obj);
	void	BuildFromList(const AABB& box, const std::list<RE_GameObject*>& gos);
	void	Draw() const;

	void	SetDrawMode(short mode);
	short	GetDrawMode() const;

	void	Pop(const RE_GameObject* g_obj);

	template<typename TYPE>
	inline void CollectIntersections(std::vector<RE_GameObject*>& objects, const TYPE & primitive) const;

private:

	void Push(RE_GameObject* g_obj);
	void Clear();
	void PushWithChilds(RE_GameObject* g_obj);

private:

	class QTreeNode
	{
	public:
		QTreeNode();
		QTreeNode(const AABB& box, QTreeNode* parent = nullptr);
		~QTreeNode();

		void Push(RE_GameObject* g_obj);
		void Pop(const RE_GameObject* g_obj);
		void Clear();

		void Draw(const int* edges, int count) const;

		void SetBox(const AABB& bounding_box);
		const AABB& GetBox() const;

		template<typename TYPE>
		inline void CollectIntersections(std::vector<RE_GameObject*>& objects, const TYPE & primitive) const;

	private:

		void AddNodes();
		void Distribute();

	private:

		QTreeNode* nodes[4];
		QTreeNode* parent = nullptr;

		std::list<RE_GameObject*> g_objs;
		bool is_leaf = true;
		AABB box;
	} root;

	enum DrawMode : short
	{
		DISABLED,
		TOP,
		BOTTOM,
		TOP_BOTTOM,
		ALL
	} draw_mode;

	int edges[12];
	int count = 0;

	std::list<RE_GameObject*>::iterator it;
};

template<typename TYPE>
inline void QTree::QTreeNode::CollectIntersections(std::vector<RE_GameObject*>& objects, const TYPE & primitive) const
{
	if (primitive.Intersects(box))
	{
		for (auto go : g_objs)
			if (primitive.Intersects(go->GetGlobalBoundingBox()))
				objects.push_back(go);

		if (!is_leaf)
			for (int i = 0; i < 4; ++i)
				if (nodes[i] != nullptr) nodes[i]->CollectIntersections(objects, primitive);
	}
}

template<typename TYPE>
inline void QTree::CollectIntersections(std::vector<RE_GameObject*>& objects, const TYPE & primitive) const
{
	root.CollectIntersections(objects, primitive);
}

struct AABBDynamicTreeNode
{
	AABB box;
	int object_index;
	int parent_index;
	int child1;
	int child2;
	bool is_leaf;
};

class AABBDynamicTree : public PoolMapped<AABBDynamicTreeNode,int>
{
private:
	int size;
	int node_count;
	int root_index;

public:

	AABBDynamicTree();
	~AABBDynamicTree();

	void PushNode(int index, AABB box, const int size_increment = 10);
	void PopNode(int index);
	void Clear();
	void CollectIntersections(Ray ray, std::stack<int>& indexes) const;
	void CollectIntersections(Frustum frustum, std::stack<int>& indexes) const;

	void Draw()const;

private:

	int AllocateLeafNode(AABB box, int index, const int size_increment = 10);
	int AllocateInternalNode(const int size_increment = 10);

	static inline AABB Union(AABB box1, AABB box2);
	static inline void SetLeaf(AABBDynamicTreeNode& node, AABB box, int index);
	static inline void SetInternal(AABBDynamicTreeNode& node);
};

#endif // !__QUADTREE_H__