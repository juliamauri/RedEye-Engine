#include "QuadTree.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "RE_CompCamera.h"
#include "RE_GameObject.h"
#include "Globals.h"
#include "ShaderManager.h"
#include "SDL2\include\SDL_assert.h"
#include "Glew\include\glew.h"
#include <stack>

QTreeNode::QTreeNode()
{
	box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);

	for (int i = 0; i < 4; i++)
		nodes[i] = nullptr;
}

QTreeNode::QTreeNode(const AABB& box, QTreeNode* parent) :
	box(box), parent(parent)
{
	for (int i = 0; i < 4; i++)
		nodes[i] = nullptr;
}

QTreeNode::~QTreeNode()
{
	Clear();
}

void QTreeNode::Push(const RE_GameObject* g_obj)
{
	if (is_leaf)
	{
		if (g_objs.size() < 4)
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
		for (int i = 0; i < 4; i++)
			if (nodes[i]->box.Intersects(g_obj->GetGlobalBoundingBox()))
				nodes[i]->Push(g_obj);
	}
}

void QTreeNode::Pop(const RE_GameObject* g_obj)
{
	std::list<const RE_GameObject*>::iterator it = std::find(g_objs.begin(), g_objs.end(), g_obj);
	if (it != g_objs.end())
		g_objs.erase(it);

	if (!is_leaf)
		for (int i = 0; i < 4; i++)
			nodes[i]->Pop(g_obj);
}

void QTreeNode::Clear()
{
	for (int i = 0; i < 4; i++)
		DEL(nodes[i]);

	g_objs.clear();
	is_leaf = true;
}

void QTreeNode::Draw(const int* edges, int count) const
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

void QTreeNode::SetBox(const AABB & bounding_box)
{
	box = bounding_box;
}

const AABB& QTreeNode::GetBox() const
{
	return box;
}

QTreeNode * QTreeNode::GetNode(uint index) const
{
	return nodes[index];
}

void QTreeNode::AddNodes()
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

void QTreeNode::Distribute()
{
	std::list<const RE_GameObject*>::iterator it = g_objs.begin();
	
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

void QTree::Build(const RE_GameObject * root_g_obj)
{
	assert(root_g_obj != nullptr);

	root.Clear();

	PushWithChilds(root_g_obj);
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

void QTree::Push(const RE_GameObject * g_obj)
{
	if (g_obj->GetGlobalBoundingBox().Intersects(root.GetBox()))
		root.Push(g_obj);
}

void QTree::Pop(const RE_GameObject * g_obj)
{
	root.Pop(g_obj);
}

void QTree::Clear()
{
	root.Clear();
}

void QTree::PushWithChilds(const RE_GameObject * g_obj)
{
	root.SetBox(g_obj->GetGlobalBoundingBox());

	std::stack<const RE_GameObject*> objects;

	for (auto child : g_obj->GetChilds())
		objects.push(child);

	while (!objects.empty())
	{
		const RE_GameObject* obj = objects.top();
		objects.pop();

		if (obj->IsActiveStatic())
		{
			root.Push(obj);

			for (auto child : obj->GetChilds())
				objects.push(child);
		}
	}
}
