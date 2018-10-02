#include "RE_CompPrimitive.h"

#include "RE_Mesh.h"

#define SMALL_INFINITY 2000

RE_CompPrimitive::RE_CompPrimitive(PrimitiveType t) :
	transform(math::float4x4::identity), type(t)
{
	if (type == 0)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		Vertex vex;
		vex.Position = math::vec::zero;

		vertices.push_back(vex);

		//meshes.push_back(RE_Mesh(vertices, indices, textures));
	}
}

PrimitiveType RE_CompPrimitive::GetType() const
{
	return type;
}

RE_CompLine::RE_CompLine(math::vec o, math::vec d) : RE_CompPrimitive(LINE), origin(o), dest(d)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Vertex vex;
	vex.Position = origin;
	vertices.push_back(vex);

	vex.Position = dest;
	vertices.push_back(vex);

	//meshes.push_back(RE_Mesh(vertices, indices, textures));
}

RE_CompRay::RE_CompRay(math::vec o, math::vec d) : RE_CompPrimitive(RAY), origin(o), dir(d)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Vertex vex;
	vex.Position = origin;
	vertices.push_back(vex);

	vex.Position = dir * SMALL_INFINITY;
	vertices.push_back(vex);

	//meshes.push_back(RE_Mesh(vertices, indices, textures));
}

RE_CompTriangle::RE_CompTriangle() : RE_CompPrimitive(TRIANGLE)
{
	Vertex vert;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> index;
	std::vector<Texture> textures;

	math::vec vPositionTriangle[] = {
		// positions       
		{ 0.5f, -0.5f, 0.0f },// bottom right
		{-0.5f, -0.5f, 0.0f },   // bottom left
		{ 0.0f,  0.5f, 0.0f }    // top 
	};

	for (unsigned int i = 0; i < 3; i++)
	{
		vert.Position = vPositionTriangle[i];
		vertices.push_back(vert);
	}

	//meshes.push_back(RE_Mesh(vertices, index, textures));
}
