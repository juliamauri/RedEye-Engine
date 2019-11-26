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
