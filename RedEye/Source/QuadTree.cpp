#include "QuadTree.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "RE_CompCamera.h"
#include "RE_GameObject.h"
#include "Globals.h"
#include "RE_ShaderImporter.h"
#include "SDL2\include\SDL_assert.h"
#include "Glew\include\glew.h"
#include <stack>

QTree::QTreeNode::QTreeNode()
{
	box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);

	for (int i = 0; i < 4; i++)
		nodes[i] = nullptr;
}

QTree::QTreeNode::QTreeNode(const AABB& box, QTreeNode* parent) :
	box(box), parent(parent)
{
	for (int i = 0; i < 4; i++)
		nodes[i] = nullptr;
}

QTree::QTreeNode::~QTreeNode()
{
	Clear();
}

void QTree::QTreeNode::Push(RE_GameObject* g_obj)
{
	if (is_leaf)
	{
		g_objs.push_back(g_obj);

		if (g_objs.size() >= 4)
		{
			AddNodes();
			Distribute();
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
			if (nodes[i]->box.Intersects(g_obj->GetGlobalBoundingBox()))
				nodes[i]->Push(g_obj);
	}
}

void QTree::QTreeNode::Pop(const RE_GameObject* g_obj)
{
	std::list<RE_GameObject*>::iterator it = std::find(g_objs.begin(), g_objs.end(), g_obj);
	if (it != g_objs.end())
		g_objs.erase(it);

	if (!is_leaf)
	{
		for (int i = 0; i < 4; i++)
			nodes[i]->Pop(g_obj);

		// delete nodes if node can be leaf
		int count = g_objs.size();
		if (count <= 4)
		{
			for (int i = 0; i < 4; i++)
				count += nodes[i]->g_objs.size();

			if (count <= 4)
			{
				for (int i = 0; i < 4; i++)
					for (auto objs : nodes[i]->g_objs)
						g_objs.push_back(objs);

				for (int i = 0; i < 4; i++)
					DEL(nodes[i]);

				is_leaf = true;
			}
		}
	}
}

void QTree::QTreeNode::Clear()
{
	for (int i = 0; i < 4; i++)
		DEL(nodes[i]);

	g_objs.clear();
	is_leaf = true;
}

void QTree::QTreeNode::Draw(const int* edges, int count) const
{
	for (int i = 0; i < count; i++)
	{
		glVertex3f(
			box.Edge(edges[i]).a.x,
			box.Edge(edges[i]).a.y,
			box.Edge(edges[i]).a.z);
		glVertex3f(
			box.Edge(edges[i]).b.x,
			box.Edge(edges[i]).b.y,
			box.Edge(edges[i]).b.z);
	}

	if (!is_leaf)
		for (int i = 0; i < 4; i++)
			nodes[i]->Draw(edges, count);
}

void QTree::QTreeNode::SetBox(const AABB & bounding_box)
{
	box = bounding_box;
}

const AABB& QTree::QTreeNode::GetBox() const
{
	return box;
}

void QTree::QTreeNode::AddNodes()
{
	AABB target_box;
	float3 center(box.CenterPoint());
	float3 target_size(box.Size());
	target_size.x *= 0.5f;
	target_size.z *= 0.5f;

	for (uint i = 0; i < 4; i++)
	{
		target_box.SetFromCenterAndSize( {
			i % 3 ? (center.x + target_size.x * 0.5f) : (center.x - target_size.x * 0.5f),
			center.y,
			i < 2 ? (center.z - target_size.z * 0.5f) : (center.z + target_size.z * 0.5f) },
			target_size);

		nodes[i] = new QTreeNode(target_box, this);
	}

	is_leaf = false;
}

void QTree::QTreeNode::Distribute()
{
	std::list<RE_GameObject*>::iterator it = g_objs.begin();
	
	while (it != g_objs.end())
	{
		bool intersecting[4];
		uint intersections_counter = 0;

		for (uint i = 0; i < 4; i++)
		{
			intersecting[i] = nodes[i]->box.Intersects((*it)->GetGlobalBoundingBox());

			if (intersecting[i])
				intersections_counter++; 
		}

		if (intersections_counter != 4)
		{
			for (uint i = 0; i < 4; i++)
			{
				if (intersecting[i]) nodes[i]->Push(*it);
			}

			it = g_objs.erase(it);
		}
		else
		{
			it++;
		}
	}
}

QTree::QTree()
{
	SetDrawMode(ALL);
	SetDrawMode(BOTTOM);
}

QTree::~QTree()
{}

void QTree::Build(RE_GameObject * root_g_obj)
{
	assert(root_g_obj != nullptr);

	root.Clear();

	PushWithChilds(root_g_obj);
}

void QTree::BuildFromList(const AABB& box, const std::list<RE_GameObject*>& gos)
{
	root.Clear();
	root.SetBox(box);

	for (auto go : gos)
		root.Push(go);
}

void QTree::Draw() const
{
	root.Draw(edges, count);
}

void QTree::SetDrawMode(short mode)
{
	switch (draw_mode = DrawMode(mode))
	{
	case QTree::DISABLED:
		count = 0;
	case QTree::TOP:
		count = 4;
		edges[0] = 5;
		edges[1] = 6;
		edges[2] = 7;
		edges[3] = 11;
		break;
	case QTree::BOTTOM:
		count = 4;
		edges[0] = 0;
		edges[1] = 2;
		edges[2] = 4;
		edges[3] = 8;
		break;
	case QTree::TOP_BOTTOM:
		count = 8;
		edges[0] = 0;
		edges[1] = 2;
		edges[2] = 4;
		edges[3] = 8;
		edges[4] = 0;
		edges[5] = 2;
		edges[6] = 4;
		edges[7] = 8;
		break;
	case QTree::ALL:
		count = 12;
		for (int i = 0; i < 12; i++)
			edges[i] = i;
		break;
	}
}

short QTree::GetDrawMode() const
{
	return draw_mode;
}

bool QTree::Contains(const math::AABB bounding_box) const
{
	return (root.GetBox().Contains(bounding_box.minPoint)
		&& root.GetBox().Contains(bounding_box.maxPoint));
}

bool QTree::TryPushing(RE_GameObject * g_obj)
{
	bool ret = Contains(g_obj->GetGlobalBoundingBox());

	if (ret)
		root.Push(g_obj);

	return ret;
}

bool QTree::TryPushingWithChilds(RE_GameObject * g_obj, std::list<RE_GameObject*>& out_childs)
{
	bool ret = TryPushing(g_obj);

	if (ret)
	{
		out_childs.push_back(g_obj);

		std::queue<RE_GameObject*> queue;
		for (auto child : g_obj->GetChilds())
			queue.push(child);

		while (!queue.empty())
		{
			RE_GameObject* obj = queue.front();
			queue.pop();

			if (obj->IsActiveStatic())
			{
				root.Push(obj);
				out_childs.push_back(obj);
			}

			for (auto child : obj->GetChilds())
				queue.push(child);
		}
	}

	return ret;
}

bool QTree::TryAdapting(RE_GameObject * g_obj)
{
	bool ret = Contains(g_obj->GetGlobalBoundingBox());

	if (ret)
	{
		root.Pop(g_obj);
		root.Push(g_obj);
	}

	return ret;
}

bool QTree::TryAdaptingWithChilds(RE_GameObject * g_obj, std::list<RE_GameObject*>& out_childs)
{
	bool ret = TryAdapting(g_obj);

	if (ret)
	{
		out_childs.push_back(g_obj);

		std::queue<RE_GameObject*> queue;
		for (auto child : g_obj->GetChilds())
			queue.push(child);

		while (!queue.empty())
		{
			RE_GameObject* obj = queue.front();
			queue.pop();

			if (obj->IsActiveStatic())
			{
				root.Pop(obj);
				root.Push(obj);
				out_childs.push_back(obj);
			}

			for (auto child : obj->GetChilds())
				queue.push(child);
		}
	}

	return ret;
}

bool QTree::TryAdaptingPushingChilds(RE_GameObject * g_obj, std::list<RE_GameObject*>& out_childs)
{
	bool ret = TryAdapting(g_obj);

	if (ret)
	{
		out_childs.push_back(g_obj);

		std::queue<RE_GameObject*> queue;
		for (auto child : g_obj->GetChilds())
			queue.push(child);

		while (!queue.empty())
		{
			RE_GameObject* obj = queue.front();
			queue.pop();

			if (obj->IsActiveStatic())
			{
				root.Push(obj);
				out_childs.push_back(obj);
			}

			for (auto child : obj->GetChilds())
				queue.push(child);
		}
	}

	return ret;
}

void QTree::Pop(const RE_GameObject * g_obj)
{
	root.Pop(g_obj);
}

void QTree::Push(RE_GameObject * g_obj)
{
	if (g_obj->GetGlobalBoundingBox().Intersects(root.GetBox()))
		root.Push(g_obj);
}

void QTree::Clear()
{
	root.Clear();
}

void QTree::PushWithChilds(RE_GameObject * g_obj)
{
	root.SetBox(g_obj->GetGlobalBoundingBox());

	std::stack<RE_GameObject*> objects;

	for (auto child : g_obj->GetChilds())
		objects.push(child);

	while (!objects.empty())
	{
		RE_GameObject* obj = objects.top();
		objects.pop();

		if (obj->IsActiveStatic())
		{
			root.Push(obj);

			for (auto child : obj->GetChilds())
				objects.push(child);
		}
	}
}


#define NullIndex -1

AABBDynamicTree::AABBDynamicTree() :  size(0), node_count(0), root_index(NullIndex)
{
	SetDrawMode(ALL);
	SetDrawMode(BOTTOM);
}

AABBDynamicTree::~AABBDynamicTree()
{
}

void AABBDynamicTree::PushNode(int index, AABB box, const int size_increment)
{
	// Stage 0: Allocate Leaf Node
	int leafIndex = AllocateLeafNode(box, index, size_increment);
	
	if (root_index != NullIndex)
	{
		// Stage 1: find the best sibling for the new leaf
		int best_sibling = root_index;
		float best_cost = Union(At(root_index).box, box).SurfaceArea();
		float box_sa = box.SurfaceArea();

		std::queue<int> potential_siblings;
		if (At(root_index).child1 != NullIndex) potential_siblings.push(At(root_index).child1);
		if (At(root_index).child2 != NullIndex) potential_siblings.push(At(root_index).child2);

		while (!potential_siblings.empty())
		{
			int current_sibling = potential_siblings.front();
			potential_siblings.pop();

			// C     = direct_cost                + inherited_cost
			// C     = SA (box U current_sibling) + SUM (Dif_SA (current_sibling parents))
			// Dif_SA (node) = SA (box U node) - SA (node)

			float direct_cost = Union(At(current_sibling).box, box).SurfaceArea();

			float inherited_cost = 0.f;
			for (int i = At(current_sibling).parent_index; i != NullIndex; i = At(i).parent_index)
				inherited_cost += Union(At(i).box, box).SurfaceArea() - At(i).box.SurfaceArea();

			if (direct_cost + inherited_cost < best_cost)
			{
				best_cost = direct_cost + inherited_cost;
				best_sibling = current_sibling;

				// C_low = SA (box) + direct_cost             + inherited_cost
				// C_low = SA (box) + Dif_SA(current_sibling) + SUM (Dif_SA (current_sibling parents))
				float lower_cost = box_sa + direct_cost - At(current_sibling).box.SurfaceArea();
				if (lower_cost + inherited_cost < best_cost)
				{
					if (At(current_sibling).child1 != NullIndex) potential_siblings.push(At(current_sibling).child1);
					if (At(current_sibling).child2 != NullIndex) potential_siblings.push(At(current_sibling).child2);
				}
			}
		}

		// Stage 2: create a new parent
		int new_parent = AllocateInternalNode(size_increment);
		int old_parent = At(best_sibling).parent_index;

		if (old_parent == NullIndex) // Sibling is root
			root_index = new_parent;
		else if (At(old_parent).child1 == best_sibling) // Sibling not root
			pool_[old_parent].child1 = new_parent;
		else
			pool_[old_parent].child2 = new_parent;

		pool_[new_parent].parent_index = old_parent;
		pool_[new_parent].child1 = best_sibling;
		pool_[new_parent].child2 = leafIndex;
		pool_[best_sibling].parent_index = new_parent;
		pool_[leafIndex].parent_index = new_parent;

		// Stage 3: walk back up the tree refitting AABBs
		int index = At(leafIndex).parent_index;
		while (index != NullIndex)
		{
			pool_[index].box = Union(
				At(At(index).child1).box,
				At(At(index).child2).box);

			index = At(index).parent_index;
		}
	}
	else
	{
		// Set as root
		root_index = leafIndex;
	}
}

void AABBDynamicTree::PopNode(int index)
{
	if (node_count > 0 && index < lastAvaibleIndex && At(index).is_leaf)
	{
		if (index == root_index)
		{
			root_index = NullIndex;
		}
		else
		{
			int parent_index = At(index).parent_index;
			AABBDynamicTreeNode& parent_node = pool_[parent_index];

			if (parent_index == root_index) // son of root
			{
				if (parent_node.child1 == index) // left child
					root_index = parent_node.child2;
				else // right child
					root_index = parent_node.child1;
			}
			else // has grand parent
			{
				AABBDynamicTreeNode& grand_parent_node = pool_[parent_index];

				if (parent_node.child1 == index) // left child
				{
					if (grand_parent_node.child1 == parent_index) // left grand child
						grand_parent_node.child1 = parent_node.child2;
					else // right grand child
						grand_parent_node.child2 = parent_node.child2;

					pool_[parent_node.child2].parent_index = parent_node.parent_index;
				}
				else // right child
				{
					if (grand_parent_node.child1 == parent_index)  // left grand child
						grand_parent_node.child1 = parent_node.child1;
					else // right grand child
						grand_parent_node.child2 = parent_node.child1;

					pool_[parent_node.child1].parent_index = parent_node.parent_index;
				}
			}
		}
		Pop(index);
		node_count--;
	}
}

void AABBDynamicTree::Clear()
{
	size = 0;
	node_count = 0;
	root_index = -1;

	lastAvaibleIndex = 0;
	poolmapped_.clear();
}

void AABBDynamicTree::CollectIntersections(Ray ray, std::stack<int>& indexes) const
{
	if (node_count > 0)
	{
		int index = NullIndex;
		std::stack<int> node_stack;
		node_stack.push(root_index);

		while (!node_stack.empty())
		{
			index = node_stack.top();
			node_stack.pop();

			if (ray.Intersects(At(index).box))
			{
				if (At(index).is_leaf)
				{
					indexes.push(At(index).object_index);
				}
				else
				{
					if (At(index).child1 != NullIndex) node_stack.push(At(index).child1);
					if (At(index).child2 != NullIndex) node_stack.push(At(index).child2);
				}
			}
		}
	}
}

void AABBDynamicTree::CollectIntersections(Frustum frustum, std::stack<int>& indexes) const
{
	if (node_count > 0)
	{
		int index = NullIndex;
		std::stack<int> node_stack;
		node_stack.push(root_index);

		while (!node_stack.empty())
		{
			index = node_stack.top();
			node_stack.pop();

			if (frustum.Intersects(At(index).box))
			{
				if (At(index).is_leaf)
				{
					indexes.push(At(index).object_index);
				}
				else
				{
					if (At(index).child1 != NullIndex) node_stack.push(At(index).child1);
					if (At(index).child2 != NullIndex) node_stack.push(At(index).child2);
				}
			}
		}
	}
}

void AABBDynamicTree::SetDrawMode(short mode)
{
	switch (draw_mode = DrawMode(mode))
	{
	case AABBDynamicTree::DISABLED:
		count = 0;
	case AABBDynamicTree::TOP:
		count = 4;
		edges[0] = 5;
		edges[1] = 6;
		edges[2] = 7;
		edges[3] = 11;
		break;
	case AABBDynamicTree::BOTTOM:
		count = 4;
		edges[0] = 0;
		edges[1] = 2;
		edges[2] = 4;
		edges[3] = 8;
		break;
	case AABBDynamicTree::TOP_BOTTOM:
		count = 8;
		edges[0] = 0;
		edges[1] = 2;
		edges[2] = 4;
		edges[3] = 8;
		edges[4] = 0;
		edges[5] = 2;
		edges[6] = 4;
		edges[7] = 8;
		break;
	case AABBDynamicTree::ALL:
		count = 12;
		for (int i = 0; i < 12; i++)
			edges[i] = i;
		break;
	}
}

short AABBDynamicTree::GetDrawMode() const
{
	return draw_mode;
}

void AABBDynamicTree::Draw() const
{
}

AABB AABBDynamicTree::Union(AABB box1, AABB box2)
{
	vec point_array[4];
	point_array[0] = box1.minPoint;
	point_array[1] = box1.maxPoint;
	point_array[2] = box2.minPoint;
	point_array[3] = box2.maxPoint;

	return AABB::MinimalEnclosingAABB(&point_array[0], 4);
}

int AABBDynamicTree::AllocateLeafNode(AABB box, int index, const int size_increment)
{
	AABBDynamicTreeNode newNode;
	SetLeaf(newNode, box, index);

	int node_index = lastAvaibleIndex;
	Push(newNode, node_index);

	node_count++;

	return node_index;
}

int AABBDynamicTree::AllocateInternalNode(const int size_increment)
{
	AABBDynamicTreeNode newNode;
	SetInternal(newNode);

	int node_index = lastAvaibleIndex;
	Push(newNode, node_index);

	node_count++;

	return node_index;
}

inline void AABBDynamicTree::SetLeaf(AABBDynamicTreeNode & node, AABB box, int index)
{
	node.box = box;
	node.object_index = index;
	node.parent_index = NullIndex;
	node.child1 = NullIndex;
	node.child2 = NullIndex;
	node.is_leaf = true;
}

inline void AABBDynamicTree::SetInternal(AABBDynamicTreeNode & node)
{
	node.box.SetNegativeInfinity();
	node.object_index = NullIndex;
	node.parent_index = NullIndex;
	node.child1 = NullIndex;
	node.child2 = NullIndex;
	node.is_leaf = false;
}
