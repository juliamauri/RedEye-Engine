#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "RE_GameObject.h"
#include "MathGeoLib\include\Geometry\AABB.h"
#include <list>

#define MIN_BOX_RADIUS 4


class QTreeNode
{
public:
	QTreeNode(const AABB& box, QTreeNode* parent = nullptr);
	~QTreeNode();

	void Push(RE_GameObject* g_obj);
	void Pop(RE_GameObject* g_obj);
	void Clear();

	void Draw();

	const AABB& GetBox() const;
	bool IsLeaf() const;

	template<typename TYPE>
	inline void CollectIntersections(std::vector<RE_GameObject*>& objects, const TYPE & primitive) const;

private:

	void AddNodes();
	void Distribute();

private:

	std::list<RE_GameObject*> g_objs;
	QTreeNode* parent = nullptr;
	QTreeNode* nodes[4];
	AABB box;
};

class QTree
{
public:
	QTree();
	~QTree();

	void Build(RE_GameObject* root_g_obj);
	void Draw();

	template<typename TYPE>
	inline void CollectIntersections(std::vector<RE_GameObject*>& objects, const TYPE & primitive) const;

private:

	void Push(RE_GameObject* g_obj);
	void Pop(RE_GameObject* g_obj);
	void Clear();

	void RecursiveBuildFromRoot(RE_GameObject* g_obj);

private:

	AABB box;
	QTreeNode* root = nullptr;

};

#endif // !__QUADTREE_H__

template<typename TYPE>
inline void QTreeNode::CollectIntersections(std::vector<RE_GameObject*>& objects, const TYPE & primitive) const
{
	if (primitive.Intersects(box))
	{
		for (std::list<RE_GameObject*>::const_iterator it = g_objs.begin(); it != g_objs.end(); ++it)
		{
			if (primitive.Intersects((*it)->GetGlobalBoundingBox()))
				objects.push_back(*it);
		}

		for (int i = 0; i < 4; ++i)
			if (nodes[i] != nullptr) nodes[i]->CollectIntersections(objects, primitive);
	}


}

template<typename TYPE>
inline void QTree::CollectIntersections(std::vector<RE_GameObject*>& objects, const TYPE & primitive) const
{
	root->CollectIntersections(objects, primitive);
}
