#ifndef __RE_MESH_H__
#define __RE_MESH_H__

#include "RE_Math.h"

struct Vertex {
	// position
	math::vec Position;
	// normal
	math::vec Normal;
	// texCoords
	math::float2 TexCoords;
};


struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class RE_Mesh
{
public:
	/*  Mesh Data  */
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	unsigned int VAO;

	/*  Functions  */
	// constructor
	RE_Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	~RE_Mesh();

	// render the mesh
	void Draw(unsigned int shader_ID);

private:
	/*  Render data  */
	unsigned int VBO, EBO;
	bool useIndex = true;

	/*  Functions    */
	// initializes all the buffer objects/arrays
	void setupMesh();
};

#endif // !__RE_MESH_H__