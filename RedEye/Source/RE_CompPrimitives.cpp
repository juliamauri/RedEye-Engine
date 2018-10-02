#include "RE_CompPrimitives.h"

#include "RE_Mesh.h"

#define SMALL_INFINITY 2000

RE_CompPrimitives::RE_CompPrimitives() : 
	transform(math::float4x4::identity), type(POINT)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Vertex vex;
	vex.Position = math::vec::zero;

	vertices.push_back(vex);

	meshes.push_back(RE_Mesh(vertices, indices, textures));
}

RE_CompPrimitives::RE_CompPrimitives(PrimitiveType t) :
	transform(math::float4x4::identity), type(t)
{}

PrimitiveType RE_CompPrimitives::GetType() const
{
	return type;
}

RE_CompLine::RE_CompLine(math::vec o, math::vec d) : RE_CompPrimitives(LINE), origin(o), dest(d)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Vertex vex;
	vex.Position = origin;
	vertices.push_back(vex);

	vex.Position = dest;
	vertices.push_back(vex);

	meshes.push_back(RE_Mesh(vertices, indices, textures));
}

RE_CompRay::RE_CompRay(math::vec o, math::vec d) : RE_CompPrimitives(RAY), origin(o), dir(d)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Vertex vex;
	vex.Position = origin;
	vertices.push_back(vex);

	vex.Position = dir * SMALL_INFINITY;
	vertices.push_back(vex);

	meshes.push_back(RE_Mesh(vertices, indices, textures));
}

RE_CompTriangle::RE_CompTriangle()
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
	math::vec vColorsTriangle[] = {
		math::vec(1.0f, 0.0f, 0.0f),
		math::vec(0.0f, 1.0f, 0.0f),
		math::vec(0.0f, 0.0f, 1.0f)
	};

	for (unsigned int i = 0; i < 3; i++)
	{
		vert.Position = vPositionTriangle[i];
		vert.Normal = vColorsTriangle[i]; //using normal like vertex color
		vertices.push_back(vert);
		index.push_back(i);
	}

	meshes.push_back(RE_Mesh(vertices, index, textures));
}
