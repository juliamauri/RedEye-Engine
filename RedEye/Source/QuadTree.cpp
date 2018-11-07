#include "QuadTree.h"

#include "RE_GameObject.h"
#include "Globals.h"
#include "SDL2\include\SDL_assert.h"
#include "Glew\include\glew.h"

QTreeNode::QTreeNode(const AABB& box, QTreeNode* parent) : 
	box(box), parent(parent)
{
	for (auto node : nodes) node = nullptr;
}

QTreeNode::~QTreeNode()
{
	Clear();
}

void QTreeNode::Push(RE_GameObject* g_obj)
{
	if (IsLeaf())
	{
		if (g_objs.size() < 4 || box.HalfSize().LengthSq() <= MIN_BOX_RADIUS)
		{
			g_objs.push_back(g_obj);
		}
		else
		{
			AddNodes();
			g_objs.push_back(g_obj);
			Distribute();
		}
	}
	else
	{
		for (auto node : nodes)
		{
			if (node->box.Intersects(g_obj->GetBoundingBox()))
				node->Push(g_obj);
		}
	}
}

void QTreeNode::Pop(RE_GameObject* g_obj)
{
	std::list<RE_GameObject*>::iterator it = std::find(g_objs.begin(), g_objs.end(), g_obj);
	if (it != g_objs.end())
		g_objs.erase(it);

	if (nodes[0] != nullptr)
		for (auto node : nodes)
			node->Pop(g_obj);
}

void QTreeNode::Clear()
{
	if (nodes[0] != nullptr)
	{
		for (auto node : nodes)
		{
			node->Clear();
			DEL(node);
		}
	}

	g_objs.clear();
}

void QTreeNode::Draw()
{
	for (uint i = 0; i < 12; i++)
	{
		glVertex3f(box.Edge(i).a.x, box.Edge(i).a.y, box.Edge(i).a.z);
		glVertex3f(box.Edge(i).b.x, box.Edge(i).b.y, box.Edge(i).b.z);
	}

	if (nodes[0] != nullptr)
		for (auto node : nodes)
			node->Draw();
}

const AABB& QTreeNode::GetBox() const
{
	return box;
}

bool QTreeNode::IsLeaf() const
{
	return (nodes[0] == nullptr);
}

void QTreeNode::AddNodes()
{
	AABB target_box;
	float3 target_size(box.Size() * 0.5f);
	float3 center(box.CenterPoint());

	for (uint i = 0; i < 4; i++)
	{
		target_box.SetFromCenterAndSize( {
			i % 3 ? (center.x + target_size.x * 0.5f) : (center.x - target_size.x * 0.5f),
			center.y,
			i < 2 ? (center.z - target_size.z * 0.5f) : (center.z + target_size.z * 0.5f) },
			target_size);

		nodes[i] = new QTreeNode(target_box, this);
	}
}

void QTreeNode::Distribute()
{
	std::list<RE_GameObject*>::iterator it = g_objs.begin();
	
	while (it != g_objs.end())
	{
		bool intersecting[4];
		uint intersections_counter = 0;

		for (uint i = 0; i < 4; i++)
		{
			intersecting[i] = nodes[i]->box.Intersects((*it)->GetBoundingBox());
			if (intersecting[i]) intersections_counter++; 
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
{}

QTree::~QTree()
{
	DEL(root);
}

void QTree::Build(RE_GameObject * root_g_obj)
{
	assert(root_g_obj != nullptr);

	if (root != nullptr) root->Clear();

	root = new QTreeNode(box = root_g_obj->GetBoundingBox());

	RecursiveBuildFromRoot(root_g_obj);
}

void QTree::Draw()
{
	if (root != nullptr)
	{
		glBegin(GL_LINES);
		glLineWidth(3.0f);
		glColor4f(0.00f, 0.00f, 0.80f, 1.00f);

		root->Draw();

		glEnd();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void QTree::Push(RE_GameObject * g_obj)
{
	assert(g_obj != nullptr);

	if (root != nullptr)
	{
		if (g_obj->GetBoundingBox().IsFinite())
		{
			if (g_obj->GetBoundingBox().Intersects(root->GetBox()))
			{
				root->Push(g_obj);
			}
			else
			{
				//LOG("GameObject %s could not be added to QuadTree: Invalid AABB - no intersection with root AABB", g_obj->GetName());
			}
		}
		else
		{
			//LOG("GameObject %s could not be added to QuadTree: Invalid AABB - infinite", g_obj->GetName());
		}
	}
}

void QTree::Pop(RE_GameObject * g_obj)
{
	assert(g_obj != nullptr);

	root->Pop(g_obj);
}

void QTree::Clear()
{
	if (root != nullptr)
	{
		root->Clear();
		DEL(root);
	}
}

void QTree::RecursiveBuildFromRoot(RE_GameObject * g_obj)
{
	std::list<RE_GameObject*> childs = g_obj->GetChilds();
	std::list<RE_GameObject*>::iterator it = childs.begin();

	while (it != childs.end())
	{
		if ((*it)->IsActive())
		{
			if ((*it)->IsStatic())
				Push(*it);

			if ((*it)->ChildCount() > 0)
				RecursiveBuildFromRoot(*it);
		}

		it++;
	}
}
