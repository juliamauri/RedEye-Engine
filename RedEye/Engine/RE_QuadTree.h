#ifndef __QUADTREE_H__
#define __QUADTREE_H__

enum QTreeDrawMode : short { DISABLED, TOP, BOTTOM, TOP_BOTTOM, ALL };

template <class TYPE> class QTreeNode;

template <class TYPE> class RE_QuadTree
{
public:

	RE_QuadTree(math::AABB& max_size);
	~RE_QuadTree() {}

	void Push(TYPE in, math::AABB& in_box);
	void Pop(TYPE to_remove);
	void Clear();

	void BuildFromList(const eastl::vector<eastl::pair<TYPE, math::AABB>>& items, math::AABB& max_scope);

	void GetDrawVertices(eastl::vector<math::vec>& out) const;
	short GetDrawMode() const;
	void SetDrawMode(short mode);

	template<typename COLLISION_GEO>
	inline void RecursiveIntersections(eastl::vector<TYPE>& out, const COLLISION_GEO& geometry) const
	{
		root.RecursiveIntersections(out, geometry);
	}

private:

	QTreeNode<TYPE> root;
	short draw_mode = DISABLED;
	int edges[12] = {}, count = 0;
};

template <class TYPE> class QTreeNode
{
public:

	QTreeNode() {}
	QTreeNode(const AABB& box, QTreeNode* parent = nullptr);
	~QTreeNode();

	void RecursivePush(TYPE item, math::AABB& _box);
	void RecursivePop(const TYPE to_remove);
	void Clear();

	void SetBox(const math::AABB& new_box);
	const math::AABB& GetBox() const;

	void GetDrawVertices(const int* edges, int count, eastl::vector<math::vec>& out) const;

	template<typename COLLISION_GEO>
	inline void RecursiveIntersections(eastl::vector<TYPE>& out, const COLLISION_GEO& geometry) const
	{
		if (box.Intersects(geometry))
		{
			for (auto item : contained_items)
				if (item.first.Intersects(geometry))
					out.push_back(item.second);

			if (!is_leaf)
				for (int i = 0; i < 4; ++i)
					if (nodes[i] != nullptr) nodes[i]->CollectIntersections(out, geometry);
		}
	}

private:

	void AddNodes();
	void FreeNodes();
	void Distribute();

private:

	math::AABB box;
	bool is_leaf = true;

	typedef eastl::vector<eastl::pair<TYPE, math::AABB>> item_storage;
	item_storage contained_items;

	QTreeNode* nodes[4] = {};
	QTreeNode* parent = nullptr;
};

#endif // !__QUADTREE_H__
