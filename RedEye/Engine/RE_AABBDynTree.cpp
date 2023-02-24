#include <MGL/Geometry/AABB.h>

#include "RE_AABBDynTree.h"

#include <MGL/Geometry/Frustum.h>
#include <MGL/Geometry/LineSegment.h>
#include <SDL2/SDL_assert.h>
#include <GL/glew.h>

void RE_AABBDynTree::PushNode(Object_UID go_index, AABB box)
{
	// Stage 0: Allocate Leaf Node
	int leafIndex = AllocateLeafNode(box, go_index);

	if (root_index != -1)
	{
		// Stage 1: find the best sibling for the new leaf
		int best_sibling_index = root_index;
		{
			RE_AABBDynTreeNode rootNode = At(root_index);
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

					RE_AABBDynTreeNode& currentSNode = At(current_sibling);

					// C     = direct_cost                + inherited_cost
					// C     = SA (box U current_sibling) + SUM (Dif_SA (current_sibling parents))
					// Dif_SA (node) = SA (box U node) - SA (node)

					float direct_cost = Union(currentSNode.box, box).SurfaceArea();
					float inherited_cost = 0.f;

					RE_AABBDynTreeNode* iNode = nullptr;
					for (int i = currentSNode.parent_index; i != -1; i = iNode->parent_index)
					{
						iNode = AtPtr(i);
						inherited_cost += Union(iNode->box, box).SurfaceArea() - iNode->box.SurfaceArea();
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
		}

		// Stage 2: create a new parent
		int new_parent_index = AllocateInternalNode();
		RE_AABBDynTreeNode& best_sibling = At(best_sibling_index);
		int old_parent = best_sibling.parent_index;

		if (old_parent != -1)
		{
			// Connect old parent's child new parent
			RE_AABBDynTreeNode& oldParentNode = At(old_parent);
			if (oldParentNode.child1 == best_sibling_index) oldParentNode.child1 = new_parent_index;
			else oldParentNode.child2 = new_parent_index;
		}
		else
		{
			// Sibling is root and has no parent
			root_index = new_parent_index;
		}

		// Connect new parent
		RE_AABBDynTreeNode& newParentNode = At(new_parent_index);
		newParentNode.parent_index = old_parent;
		newParentNode.child1 = best_sibling_index;
		newParentNode.child2 = leafIndex;

		// Connect sibling & new node to new parent
		best_sibling.parent_index = new_parent_index;
		AtPtr(leafIndex)->parent_index = new_parent_index;

		// Stage 3: walk back up the tree refitting AABBs
		while (new_parent_index != -1)
		{
			RE_AABBDynTreeNode& iN = At(new_parent_index);
			iN.box = Union(AtPtr(iN.child1)->box, AtPtr(iN.child2)->box);
			int next = iN.parent_index;
			Rotate(iN, new_parent_index);
			new_parent_index = next;
		}
	}
	else
	{
		// Set as root
		root_index = leafIndex;
	}
}

void RE_AABBDynTree::PopNode(Object_UID go_index)
{
	SDL_assert(node_count > 0);
	int index = objectToNode.at(go_index);
	SDL_assert(index < randomCount);

	if (index == root_index)
	{
		root_index = -1;
	}
	else
	{
		int parent_index = At(index).parent_index;
		RE_AABBDynTreeNode& parent_node = At(parent_index);

		if (parent_index == root_index) // son of root
		{
			root_index = (parent_node.child1 == index) ? parent_node.child2 : parent_node.child1;
		}
		else // has grand parent
		{
			RE_AABBDynTreeNode& grand_parent_node = At(parent_node.parent_index);
			if (parent_node.child1 == index) // left child
			{
				if (grand_parent_node.child1 == parent_index) // left grand child
					grand_parent_node.child1 = parent_node.child2;
				else // right grand child
					grand_parent_node.child2 = parent_node.child2;

				AtPtr(parent_node.child2)->parent_index = parent_node.parent_index;
			}
			else // right child
			{
				if (grand_parent_node.child1 == parent_index)  // left grand child
					grand_parent_node.child1 = parent_node.child1;
				else // right grand child
					grand_parent_node.child2 = parent_node.child1;

				AtPtr(parent_node.child1)->parent_index = parent_node.parent_index;
			}
		}

		Pop(parent_index);
		node_count--;
	}

	Pop(index);
	node_count--;
	objectToNode.erase(go_index);
}

void RE_AABBDynTree::UpdateNode(Object_UID go_index, AABB box)
{
	SDL_assert(node_count > 0);
	int index = objectToNode.at(go_index);
	SDL_assert(index < randomCount);

	if (index == root_index) // is root
	{
		At(index).box = box;
	}
	else
	{
		RE_AABBDynTreeNode& current_node = At(index);
		current_node.box = box;
		int parent_index = current_node.parent_index;
		RE_AABBDynTreeNode& parent_node = At(parent_index);
		if (parent_index == root_index) // son of root
		{
			parent_node.box = Union(box, At((parent_node.child1 == index) ? parent_node.child2 : parent_node.child1).box);
		}
		else // has grand parent
		{
			// Stage 0: Remove parent from hierarchy
			{
				RE_AABBDynTreeNode& grand_parent_node = At(parent_node.parent_index);
				if (parent_node.child1 == index) // left child
				{
					if (grand_parent_node.child1 == parent_index) grand_parent_node.child1 = parent_node.child2;
					else grand_parent_node.child2 = parent_node.child2;
					AtPtr(parent_node.child2)->parent_index = parent_node.parent_index;
				}
				else // right child
				{
					if (grand_parent_node.child1 == parent_index) grand_parent_node.child1 = parent_node.child1;
					else grand_parent_node.child2 = parent_node.child1;
					AtPtr(parent_node.child1)->parent_index = parent_node.parent_index;
				}
			}

			// Stage 1: find best sibling
			int best_sibling_index = root_index;
			{
				RE_AABBDynTreeNode& rootNode = At(root_index);
				float best_cost = Union(rootNode.box, box).SurfaceArea();
				float box_sa = box.SurfaceArea();
				eastl::queue<int> potential_siblings;
				potential_siblings.push(rootNode.child1);
				potential_siblings.push(rootNode.child2);
				while (!potential_siblings.empty())
				{
					int current_sibling = potential_siblings.front();
					potential_siblings.pop();

					RE_AABBDynTreeNode currentSNode = At(current_sibling);

					// C     = direct_cost                + inherited_cost
					// C     = SA (box U current_sibling) + SUM (Dif_SA (current_sibling parents))
					// Dif_SA (node) = SA (box U node) - SA (node)

					float direct_cost = Union(currentSNode.box, box).SurfaceArea();
					float inherited_cost = 0.f;

					RE_AABBDynTreeNode* iNode = nullptr;
					for (int i = currentSNode.parent_index; i != -1; i = iNode->parent_index)
					{
						iNode = AtPtr(i);
						inherited_cost += Union(iNode->box, box).SurfaceArea() - iNode->box.SurfaceArea();
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

			// Stage 2: conect parent and best sibling
			{
				RE_AABBDynTreeNode& best_sibling = At(best_sibling_index);
				int best_grand_parent = best_sibling.parent_index;
				if (best_grand_parent == -1)
				{
					// Sibling is root and has no parent
					root_index = parent_index;
				}
				else
				{
					// Connect current parent to sibling's grand parent
					RE_AABBDynTreeNode& best_grand_parent_node = At(best_grand_parent);
					(best_grand_parent_node.child1 == best_sibling_index ? best_grand_parent_node.child1 : best_grand_parent_node.child2) = parent_index;
				}

				// Connect parent & sibling
				parent_node.parent_index = best_grand_parent;
				parent_node.child1 = best_sibling_index;
				parent_node.child2 = index;
				best_sibling.parent_index = parent_index; }

			// Stage 3: walk back up the tree refitting AABBs
			{
				while (parent_index != -1)
				{
					RE_AABBDynTreeNode& iN = At(parent_index);
					iN.box = Union(AtPtr(iN.child1)->box, AtPtr(iN.child2)->box);

					int next = iN.parent_index;
					Rotate(iN, parent_index);
					parent_index = next;
				}
			}
		}
	}
}

eastl::vector<int> RE_AABBDynTree::GetAllKeys() const
{
	eastl::vector<int> ret;
	for (auto go : poolmapped_) ret.push_back(go.first);
	return ret;
}

void RE_AABBDynTree::Clear()
{
	root_index = -1;
	size = node_count = randomCount = lastAvaibleIndex = 0;
	objectToNode.clear();
	poolmapped_.clear();
}

void RE_AABBDynTree::CollectIntersections(Ray ray, eastl::queue<Object_UID>& indexes) const
{
	if (node_count > 0)
	{
		RE_AABBDynTreeNode node;
		eastl::queue<int> node_stack;
		node_stack.push(root_index);

		while (!node_stack.empty())
		{
			node = At(node_stack.front());
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

void RE_AABBDynTree::CollectIntersections(const Frustum frustum, eastl::queue<Object_UID>& indexes) const
{
	if (node_count > 0)
	{
		RE_AABBDynTreeNode node;
		eastl::queue<int> node_stack;
		node_stack.push(root_index);

		while (!node_stack.empty())
		{
			node = At(node_stack.front());
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

void RE_AABBDynTree::Draw() const
{
	for (auto pm_ : poolmapped_) {
		RE_AABBDynTreeNode node = At(pm_.first);
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

int RE_AABBDynTree::GetCount() const
{
	return node_count;
}

AABB RE_AABBDynTree::Union(AABB box1, AABB box2)
{
	vec point_array[4] = { box1.minPoint, box1.maxPoint, box2.minPoint, box2.maxPoint };
	return AABB::MinimalEnclosingAABB(&point_array[0], 4);
}

void RE_AABBDynTree::Rotate(RE_AABBDynTreeNode& node, int index)
{
	if (node.parent_index != -1)
	{
		RE_AABBDynTreeNode& parent = At(node.parent_index);

		bool node_is_child1 = (parent.child1 == index);
		RE_AABBDynTreeNode& sibling = At(node_is_child1 ? parent.child2 : parent.child1);

		// rotation[0] = sibling <-> child1;
		// rotation[1] = sibling <-> child2;
		// rotation[2] = current <-> sibling child1;
		// rotation[3] = current <-> sibling child2;

		float rotation_sa[4] = {};
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

int RE_AABBDynTree::AllocateLeafNode(AABB box, Object_UID index)
{
	RE_AABBDynTreeNode newNode;
	SetLeaf(newNode, box, index);

	int node_index = randomCount++;
	Push(newNode, node_index);
	objectToNode.insert({ index, node_index });

	node_count++;

	return node_index;
}

int RE_AABBDynTree::AllocateInternalNode()
{
	RE_AABBDynTreeNode newNode;
	SetInternal(newNode);

	int node_index = randomCount++;
	Push(newNode, node_index);

	node_count++;

	return node_index;
}

inline void RE_AABBDynTree::SetLeaf(RE_AABBDynTreeNode& node, AABB box, Object_UID index)
{
	node.box = box;
	node.object_index = index;
	node.parent_index = -1;
	node.child1 = -1;
	node.child2 = -1;
	node.is_leaf = true;
}

inline void RE_AABBDynTree::SetInternal(RE_AABBDynTreeNode& node)
{
	node.box.SetNegativeInfinity();
	node.object_index = -1;
	node.parent_index = -1;
	node.child1 = -1;
	node.child2 = -1;
	node.is_leaf = false;
}