#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "MathGeoLib\include\Geometry\AABB.h"
#include <list>

#define MIN_BOX_RADIUS 4

class RE_GameObject;

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