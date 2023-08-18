#include <MGL\Math\float3.h>
#include <MGL\Geometry\AABB.h>
#include <EASTL\vector.h>

#include "RE_QuadTree.h"

template<class TYPE>
RE_QuadTree<TYPE>::RE_QuadTree(math::AABB& max_size)
{
	root.SetBox(max_size);
}

template<class TYPE>
void RE_QuadTree<TYPE>::Push(TYPE in, math::AABB& in_box)
{
	if (root.GetBox().Intersects(in_box))
		root.RecursivePush(in, in_box);
}

template<class TYPE>
void RE_QuadTree<TYPE>::Pop(TYPE to_remove)
{
	root.RecursivePop(to_remove);
}

template<class TYPE>
void RE_QuadTree<TYPE>::Clear()
{
	root.Clear();
}

template<class TYPE>
void RE_QuadTree<TYPE>::BuildFromList(const eastl::vector<eastl::pair<TYPE, math::AABB>>& items, math::AABB& max_scope)
{
	for (auto item : items) root.RecursivePush(item.first, item.second);
}

template<class TYPE>
void RE_QuadTree<TYPE>::GetDrawVertices(eastl::vector<math::vec>& out) const
{
	root.GetDrawVertices(edges, count, out);
}

template<class TYPE>
short RE_QuadTree<TYPE>::GetDrawMode() const
{
	return draw_mode;
}

template<class TYPE>
void RE_QuadTree<TYPE>::SetDrawMode(short mode)
{
	switch (draw_mode = mode) {
	case QTreeDrawMode::DISABLED: count = 0;
	case QTreeDrawMode::TOP: count = 4; edges[0] = 5; edges[1] = 6; edges[2] = 7; edges[3] = 11; break;
	case QTreeDrawMode::BOTTOM: count = 4; edges[0] = 0; edges[1] = 2; edges[2] = 4; edges[3] = 8; break;
	case QTreeDrawMode::TOP_BOTTOM: count = 8; edges[0] = edges[4] = 0; edges[1] = edges[5] = 2; edges[2] = edges[6] = 4; edges[3] = edges[7] = 8; break;
	case QTreeDrawMode::ALL: count = 12; for (int i = 0; i < 12; i++) { edges[i] = i; } break; }
}

template<class TYPE>
QTreeNode<TYPE>::QTreeNode(const AABB& box, QTreeNode* parent) : box(box), parent(parent) {}

template<class TYPE>
QTreeNode<TYPE>::~QTreeNode() { FreeNodes(); }

template<class TYPE>
void QTreeNode<TYPE>::RecursivePush(TYPE item, math::AABB& _box)
{
	if (is_leaf)
	{
		contained_items.push_back({ item, _box });
		if (contained_items.size() >= 4) { AddNodes(); Distribute(); }
	}
	else
		for (int i = 0; i < 4; i++)
			if (nodes[i]->box.Intersects(_box))
				nodes[i]->RecursivePush(item, _box);
}

template<class TYPE>
void QTreeNode<TYPE>::RecursivePop(const TYPE to_remove)
{
	item_storage::iterator it = contained_items.begin();
	for (auto item : contained_items)
	{
		if (item.first == to_remove)
		{
			contained_items.erase(it);
			break;
		}
		else it++;
	}

	if (!is_leaf)
	{
		for (int i = 0; i < 4; i++) nodes[i]->RecursivePop(to_remove);

		int count = contained_items.size();
		if (count <= 4)
		{
			for (int i = 0; i < 4; i++) count += nodes[i]->contained_items.size();
			if (count <= 4) // check if node can be set to leaf with 4 or less items
			{
				is_leaf = true;
				for (int i = 0; i < 4; i++)
				{
					for (auto& item : nodes[i]->contained_items) contained_items.push_back(item);
					delete nodes[i];
				}
			}
		}
	}
}

template<class TYPE>
void QTreeNode<TYPE>::Clear()
{
	is_leaf = true;
	contained_items.clear();
	FreeNodes();
}

template<class TYPE>
void QTreeNode<TYPE>::FreeNodes()
{
	for (int i = 0; i < 4; i++) if (nodes[i] != nullptr) delete nodes[i];
}

template<class TYPE>
void QTreeNode<TYPE>::SetBox(const math::AABB& new_box) { box = new_box; }

template<class TYPE>
const math::AABB& QTreeNode<TYPE>::GetBox() const { return box; }

template<class TYPE>
void QTreeNode<TYPE>::AddNodes()
{
	math::AABB target;
	math::vec center(box.CenterPoint());
	math::vec target_size(box.Size());
	target_size.x *= 0.5f;
	target_size.z *= 0.5f;

	for (int i = 0; i < 4; i++)
	{
		target.SetFromCenterAndSize({
			i % 3 ? (center.x + target_size.x * 0.5f) : (center.x - target_size.x * 0.5f),
			center.y,
			i < 2 ? (center.z - target_size.z * 0.5f) : (center.z + target_size.z * 0.5f) },
			target_size);

		nodes[i] = new QTreeNode(target, this);
	}

	is_leaf = false;
}

template<class TYPE>
void QTreeNode<TYPE>::Distribute()
{
	item_storage::iterator it = contained_items.begin();
	while (it != contained_items.end())
	{
		bool intersecting[4] = {};
		int intersections_counter = 0;

		for (int i = 0; i < 4; i++)
			intersections_counter += (intersecting[i] = nodes[i]->box.Intersects(it->second));

		if (intersections_counter != 4)
		{
			for (int i = 0; i < 4; i++) if (intersecting[i]) nodes[i]->RecursivePush(it->first, it->second);
			it = contained_items.erase(it);
		}
		else it++;
	}
}

template<class TYPE>
void QTreeNode<TYPE>::GetDrawVertices(const int* edges, int count, eastl::vector<math::vec>& out) const
{
	for (int i = 0; i < count; i++)
	{
		out.push_back(box.Edge(edges[i]).a);
		out.push_back(box.Edge(edges[i]).b);
	}

	if (!is_leaf)
		for (int i = 0; i < 4; i++)
			nodes[i]->GetDrawVertices(edges, count, out);
}
