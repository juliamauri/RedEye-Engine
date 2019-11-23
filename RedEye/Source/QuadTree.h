#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "RE_GameObject.h"
#include "MathGeoLib\include\Geometry\AABB.h"
#include <list>

class QTreeNode
{
public:
	QTreeNode();
	QTreeNode(const AABB& box, QTreeNode* parent = nullptr);
	~QTreeNode();

	void Push(const RE_GameObject* g_obj);
	void Pop(const RE_GameObject* g_obj);
	void Clear();

	void Draw(const int* edges, int count) const;

	void SetBox(const AABB& bounding_box);
	const AABB& GetBox() const;
	QTreeNode* GetNode(uint index) const;

	template<typename TYPE>
	inline void CollectIntersections(std::vector<const RE_GameObject*>& objects, const TYPE & primitive) const;

private:

	void AddNodes();
	void Distribute();

private:

	QTreeNode* nodes[4];
	QTreeNode* parent = nullptr;

	std::list<const RE_GameObject*> g_objs;
	bool is_leaf = true;
	AABB box;
};

class QTree
{
public:
	QTree();
	~QTree();

	void Build(const RE_GameObject* root_g_obj);
	void Draw() const;

	void SetDrawMode(short mode);
	short GetDrawMode() const;

	template<typename TYPE>
	inline void CollectIntersections(std::vector<const RE_GameObject*>& objects, const TYPE & primitive) const;

private:

	void Push(const RE_GameObject* g_obj);
	void Pop(const RE_GameObject* g_obj);
	void Clear();

	void PushWithChilds(const RE_GameObject* g_obj);

private:

	QTreeNode root;

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
};

template<typename TYPE>
inline void QTreeNode::CollectIntersections(std::vector<const RE_GameObject*>& objects, const TYPE & primitive) const
{
	if (primitive.Intersects(box))
	{
		for (std::list<const RE_GameObject*>::const_iterator it = g_objs.begin(); it != g_objs.end(); ++it)
			if (primitive.Intersects((*it)->GetGlobalBoundingBox()))
				objects.push_back(*it);

		if (!is_leaf)
			for (int i = 0; i < 4; ++i)
				if (nodes[i] != nullptr) nodes[i]->CollectIntersections(objects, primitive);
	}
}

template<typename TYPE>
inline void QTree::CollectIntersections(std::vector<const RE_GameObject*>& objects, const TYPE & primitive) const
{
	root.CollectIntersections(objects, primitive);
}

#endif // !__QUADTREE_H__