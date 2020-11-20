#include "QuadTree.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "RE_CompCamera.h"
#include "RE_GameObject.h"
#include "Globals.h"
#include "RE_ShaderImporter.h"
#include "SDL2\include\SDL_assert.h"
#include "Glew\include\glew.h"

#include <EASTL/queue.h>

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
	eastl::list<RE_GameObject*>::iterator it = eastl::find(g_objs.begin(), g_objs.end(), g_obj);
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
	eastl::list<RE_GameObject*>::iterator it = g_objs.begin();
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

QTree::QTree() { SetDrawMode(BOTTOM); }

QTree::~QTree()
{}

void QTree::Build(RE_GameObject * root_g_obj)
{
	assert(root_g_obj != nullptr);

	root.Clear();

	PushWithChilds(root_g_obj);
}

void QTree::BuildFromList(const AABB& box, const eastl::list<RE_GameObject*>& gos)
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

	eastl::stack<RE_GameObject*> objects;

	for (auto child : g_obj->GetChildsPtr())
		objects.push(child);

	while (!objects.empty())
	{
		RE_GameObject* obj = objects.top();
		objects.pop();

		if (obj->IsActiveStatic())
		{
			root.Push(obj);

			for (auto child : obj->GetChildsPtr())
				objects.push(child);
		}
	}
}

AABBDynamicTree::AABBDynamicTree() {}
AABBDynamicTree::~AABBDynamicTree() {}

void AABBDynamicTree::PushNode(UID go_index, AABB box)
{
	// Stage 0: Allocate Leaf Node
	int leafIndex = AllocateLeafNode(box, go_index);
	
	if (root_index != -1)
	{
		// Stage 1: find the best sibling for the new leaf
		AABBDynamicTreeNode rootNode = At(root_index);
		int best_sibling_index = root_index;
		float best_cost = Union(rootNode.box, box).SurfaceArea();
		float box_sa = box.SurfaceArea();

		if (!rootNode.is_leaf)
		{
			eastl::queue<int> potential_siblings;
			potential_siblings.push(rootNode.child1);
			potential_siblings.push(rootNode.child2);
			
			while (!potential_siblings.empty())
			{
				int current_sibling = potential_siblings.front();
				potential_siblings.pop();

				AABBDynamicTreeNode currentSNode = At(current_sibling);

				// C     = direct_cost                + inherited_cost
				// C     = SA (box U current_sibling) + SUM (Dif_SA (current_sibling parents))
				// Dif_SA (node) = SA (box U node) - SA (node)

				float direct_cost = Union(currentSNode.box, box).SurfaceArea();
				float inherited_cost = 0.f;

				AABBDynamicTreeNode iNode;
				for (int i = currentSNode.parent_index; i != -1; i = iNode.parent_index)
				{
					iNode = At(i);
					inherited_cost += Union(iNode.box, box).SurfaceArea() - iNode.box.SurfaceArea();
				}

				if (direct_cost + inherited_cost < best_cost)
				{
					best_cost = direct_cost + inherited_cost;
					best_sibling_index = current_sibling;

					// C_low = SA (box) + direct_cost             + inherited_cost
					// C_low = SA (box) + Dif_SA(current_sibling) + SUM (Dif_SA (current_sibling parents))
					float lower_cost = box_sa + direct_cost - currentSNode.box.SurfaceArea();
					if (lower_cost + inherited_cost < best_cost && !currentSNode.is_leaf)
					{
						potential_siblings.push(currentSNode.child1);
						potential_siblings.push(currentSNode.child2);
					}
				}
			}
		}

		// Stage 2: create a new parent
		int new_parent_index = AllocateInternalNode();
		int old_parent = At(best_sibling_index).parent_index;

		if (old_parent != -1)
		{
			// Connect old parent's child new parent
			AABBDynamicTreeNode* oldParentNode = AtPtr(old_parent);
			if (oldParentNode->child1 == best_sibling_index)
				oldParentNode->child1 = new_parent_index;
			else
				oldParentNode->child2 = new_parent_index;
		}
		else
		{
			// Sibling is root and has no parent
			root_index = new_parent_index;
		}

		// Connect new parent
		AABBDynamicTreeNode* newParentNode = AtPtr(new_parent_index);
		newParentNode->parent_index = old_parent;
		newParentNode->child1 = best_sibling_index;
		newParentNode->child2 = leafIndex;

		// Connect sibling & new node to new parent
		AtPtr(best_sibling_index)->parent_index = new_parent_index;
		AtPtr(leafIndex)->parent_index = new_parent_index;

		// Stage 3: walk back up the tree refitting AABBs
		int parent_index = At(leafIndex).parent_index;
		while (parent_index != -1)
		{
			AABBDynamicTreeNode& iN = At(parent_index);
			iN.box = Union(
				At(iN.child1).box,
				At(iN.child2).box);

			int next = iN.parent_index;
			Rotate(iN, parent_index);
			parent_index = next;
		}
	}
	else
	{
		// Set as root
		root_index = leafIndex;
	}
}

void AABBDynamicTree::PopNode(UID objectIndex)
{
	int index = objectToNode.at(objectIndex);

	if (node_count > 0 && index < randomCount && At(index).is_leaf)
	{
		if (index == root_index)
		{
			root_index = -1;
		}
		else
		{
			int parent_index = At(index).parent_index;
			AABBDynamicTreeNode* parent_node = AtPtr(parent_index);

			if (parent_index == root_index) // son of root
			{
				if (parent_node->child1 == index) // left child
					root_index = parent_node->child2;
				else // right child
					root_index = parent_node->child1;
				AtPtr(root_index)->parent_index = -1;
			}
			else // has grand parent
			{
				AABBDynamicTreeNode* grand_parent_node =  AtPtr(parent_node->parent_index);

				if (parent_node->child1 == index) // left child
				{
					if (grand_parent_node->child1 == parent_index) // left grand child
						grand_parent_node->child1 = parent_node->child2;
					else // right grand child
						grand_parent_node->child2 = parent_node->child2;

					AtPtr(parent_node->child2)->parent_index = parent_node->parent_index;
				}
				else // right child
				{
					if (grand_parent_node->child1 == parent_index)  // left grand child
						grand_parent_node->child1 = parent_node->child1;
					else // right grand child
						grand_parent_node->child2 = parent_node->child1;

					AtPtr(parent_node->child1)->parent_index = parent_node->parent_index;
				}
			}

			Pop(parent_index);
			node_count--;

		}

		Pop(index);
		node_count--;
		objectToNode.erase(objectIndex);
	}
}

eastl::vector<int> AABBDynamicTree::GetAllKeys() const
{
	eastl::vector<int> ret;
	for (auto go : poolmapped_) ret.push_back(go.first);
	return ret;
}

void AABBDynamicTree::Clear()
{
	size = 0;
	node_count = 0;
	root_index = -1;
	randomCount = 0;
	lastAvaibleIndex = 0;
	objectToNode.clear();
	poolmapped_.clear();
}

void AABBDynamicTree::CollectIntersections(Ray ray, eastl::stack<UID>& indexes) const
{
	if (node_count > 0)
	{
		AABBDynamicTreeNode node;
		eastl::stack<int> node_stack;
		node_stack.push(root_index);

		while (!node_stack.empty())
		{
			node = At(node_stack.top());
			node_stack.pop();

			if (ray.Intersects(node.box))
			{
				if (node.is_leaf)
				{
					indexes.push(node.object_index);
				}
				else
				{
					node_stack.push(node.child1);
					node_stack.push(node.child2);
				}
			}
		}
	}
}

void AABBDynamicTree::CollectIntersections(const Frustum frustum, eastl::stack<UID>& indexes) const
{
	if (node_count > 0)
	{
		AABBDynamicTreeNode node;
		eastl::stack<int> node_stack;
		node_stack.push(root_index);

		while (!node_stack.empty())
		{
			node = At(node_stack.top());
			node_stack.pop();

			if (frustum.Intersects(node.box))
			{
				if (node.is_leaf)
				{
					indexes.push(node.object_index);
				}
				else
				{
					node_stack.push(node.child1);
					node_stack.push(node.child2);
				}
			}
		}
	}
}

void AABBDynamicTree::Draw() const
{
	for (auto pm_ : poolmapped_) {
		AABBDynamicTreeNode node = At(pm_.first);
		if (!node.is_leaf && (node.parent_index != -1 || pm_.first == root_index))
		{
			for (int a = 0; a < 12; a++)
			{
				glVertex3f(
					node.box.Edge(a).a.x,
					node.box.Edge(a).a.y,
					node.box.Edge(a).a.z);
				glVertex3f(
					node.box.Edge(a).b.x,
					node.box.Edge(a).b.y,
					node.box.Edge(a).b.z);
			}
		}
	}
}

int AABBDynamicTree::GetCount() const
{
	return node_count;
}

AABB AABBDynamicTree::Union(AABB box1, AABB box2)
{
	vec point_array[4] = { box1.minPoint, box1.maxPoint, box2.minPoint, box2.maxPoint };
	return AABB::MinimalEnclosingAABB(&point_array[0], 4);
}

void AABBDynamicTree::Rotate(AABBDynamicTreeNode& node, int index)
{
	if (node.parent_index != -1)
	{
		AABBDynamicTreeNode& parent = At(node.parent_index);

		bool node_is_child1 = (parent.child1 == index);
		AABBDynamicTreeNode& sibling = At(node_is_child1 ? parent.child2 : parent.child1);

		// rotation[0] = sibling <-> child1;
		// rotation[1] = sibling <-> child2;
		// rotation[2] = current <-> sibling child1;
		// rotation[3] = current <-> sibling child2;

		float rotation_sa[4];
		int count = 2;

		rotation_sa[0] = Union(sibling.box, At(node.child2).box).SurfaceArea();
		rotation_sa[1] = Union(sibling.box, At(node.child1).box).SurfaceArea();

		if (!sibling.is_leaf)
		{
			count = 4;
			rotation_sa[2] = Union(node.box, At(sibling.child2).box).SurfaceArea();
			rotation_sa[3] = Union(node.box, At(sibling.child1).box).SurfaceArea();
		}

		int rotation_index = -1;
		float lowest = node.box.SurfaceArea();
		for (int i = 0; i < count; ++i)
		{
			if (rotation_sa[i] < lowest)
			{
				lowest = rotation_sa[i];
				rotation_index = i;
			}
		}

		if (rotation_index >= 0)
		{
			switch (rotation_index)
			{
			case 0: // sibling <-> child1;
			{
				sibling.parent_index = index;
				At(node.child1).parent_index = node.parent_index;

				int child_node_index = node.child1;

				if (node_is_child1)
				{
					node.child1 = parent.child2;
					parent.child2 = child_node_index;
				}
				else
				{
					node.child1 = parent.child1;
					parent.child1 = child_node_index;
				}

				break;
			}
			case 1: // sibling <-> child2;
			{
				sibling.parent_index = index;
				At(node.child2).parent_index = node.parent_index;

				int child_node_index = node.child2;

				if (node_is_child1)
				{
					node.child2 = parent.child2;
					parent.child2 = child_node_index;
				}
				else
				{
					node.child2 = parent.child1;
					parent.child1 = child_node_index;
				}

				break;
			}
			case 2: // current <-> sibling child1;
			{
				int parent_node_index = node.parent_index;

				if (node_is_child1)
				{
					node.parent_index = parent.child2;
					parent.child1 = sibling.child1;
				}
				else
				{
					node.parent_index = parent.child1;
					parent.child2 = sibling.child1;
				}

				At(sibling.child1).parent_index = parent_node_index;
				sibling.child1 = index;

				break;
			}
			case 3: // current <-> sibling child2;
			{
				int parent_node_index = node.parent_index;

				if (node_is_child1)
				{
					node.parent_index = parent.child2;
					parent.child1 = sibling.child2;
				}
				else
				{
					node.parent_index = parent.child1;
					parent.child2 = sibling.child2;
				}

				At(sibling.child2).parent_index = parent_node_index;
				sibling.child2 = index;

				break;
			}
			}
		}
	}
}

int AABBDynamicTree::AllocateLeafNode(AABB box, UID index)
{
	AABBDynamicTreeNode newNode;
	SetLeaf(newNode, box, index);

	int node_index = randomCount++;
	Push(newNode, node_index);
	objectToNode.insert(eastl::pair<UID, int>(index, node_index));

	node_count++;

	return node_index;
}

int AABBDynamicTree::AllocateInternalNode()
{
	AABBDynamicTreeNode newNode;
	SetInternal(newNode);

	int node_index = randomCount++;
	Push(newNode, node_index);

	node_count++;

	return node_index;
}

inline void AABBDynamicTree::SetLeaf(AABBDynamicTreeNode & node, AABB box, UID index)
{
	node.box = box;
	node.object_index = index;
	node.parent_index = -1;
	node.child1 = -1;
	node.child2 = -1;
	node.is_leaf = true;
}

inline void AABBDynamicTree::SetInternal(AABBDynamicTreeNode & node)
{
	node.box.SetNegativeInfinity();
	node.object_index = -1;
	node.parent_index = -1;
	node.child1 = -1;
	node.child2 = -1;
	node.is_leaf = false;
}
