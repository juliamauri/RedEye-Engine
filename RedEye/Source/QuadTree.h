#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "RE_GameObject.h"
#include "MathGeoLib\include\Geometry\AABB.h"

// Depricated class. GOs now use UIDs

class QTree
{
public:
	QTree();
	~QTree();

	void	Build(RE_GameObject* root_g_obj);
	void	BuildFromList(const AABB& box, const eastl::list<RE_GameObject*>& gos);
	void	Draw() const;

	void	SetDrawMode(short mode);
	short	GetDrawMode() const;

	void	Pop(const RE_GameObject* g_obj);

	template<typename TYPE>
	inline void CollectIntersections(eastl::vector<RE_GameObject*>& objects, const TYPE & primitive) const;

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
		inline void CollectIntersections(eastl::vector<RE_GameObject*>& objects, const TYPE & primitive) const;

	private:

		void AddNodes();
		void Distribute();

	private:

		QTreeNode* nodes[4] = { nullptr, nullptr, nullptr, nullptr };
		QTreeNode* parent = nullptr;

		eastl::list<RE_GameObject*> g_objs;
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
	} draw_mode = DISABLED;

	int edges[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int count = 0;
};

template<typename TYPE>
inline void QTree::QTreeNode::CollectIntersections(eastl::vector<RE_GameObject*>& objects, const TYPE & primitive) const
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
inline void QTree::CollectIntersections(eastl::vector<RE_GameObject*>& objects, const TYPE & primitive) const
{
	root.CollectIntersections(objects, primitive);
}


#endif // !__QUADTREE_H__