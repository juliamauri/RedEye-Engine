#include "RE_Mesh.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ShaderManager.h"
#include "RE_TextureImporter.h"
#include "OutputLog.h"
#include "ModuleScene.h"
#include "RE_GameObject.h"
#include "RE_Component.h"
#include "RE_CompTransform.h"
#include "RE_PrimitiveManager.h"
#include "ResourceManager.h"
#include "FileSystem.h"

#include "RE_Material.h"

#include "ImGui\imgui.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "MathGeoLib\include\MathGeoLib.h"
#include "MathGeoLib\include\Math\Quat.h"

#include "SDL2\include\SDL_assert.h"
#include "assimp/include/Importer.hpp"
#include "assimp/include/scene.h"
#include "assimp/include/postprocess.h"

void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

void _CheckGLError(const char* file, int line)
{
	GLenum err(glGetError());

	while (err != GL_NO_ERROR)
	{
		std::string error;
		switch (err)
		{
		case GL_INVALID_OPERATION:  error = "INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:       error = "INVALID_ENUM";           break;
		case GL_INVALID_VALUE:      error = "INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:      error = "OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		}
		std::cout << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
		err = glGetError();
	}

	return;
}



RE_Mesh::RE_Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const char* materialMD5, unsigned int triangles)
{
	this->vertices = vertices;
	this->indices = indices;
	this->materialMD5 = materialMD5;

	triangle_count = triangles;

	SetupAABB();

	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	setupMesh();
}

RE_Mesh::~RE_Mesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	if (lVertexNormals) clearVertexNormals();
	if (lFaceNormals) clearFaceNormals();
}

void RE_Mesh::Draw(unsigned int shader_ID)
{
	ShaderManager::use(shader_ID);
	// bind appropriate textures
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;

	if (materialMD5)
	{
		RE_Material* meshMaterial = (RE_Material*)App->resources->At(materialMD5);

		if (!meshMaterial->tDiffuse.empty())
		{
			for (unsigned int i = 0; i < meshMaterial->tDiffuse.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
												  // retrieve texture number (the N in diffuse_textureN)
				std::string name = "texture_diffuse";
				name += std::to_string(diffuseNr++);
				// now set the sampler to the correct texture unit
				ShaderManager::setUnsignedInt(shader_ID, name.c_str(), i);

				// and finally bind the texture
				((Texture2D*)App->resources->At(meshMaterial->tDiffuse[i]))->use();
			}
		}
	}
	/*
			if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++); // transfer unsigned int to stream
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); // transfer unsigned int to stream
		else if (name == "texture_height")
			number = std::to_string(heightNr++); // transfer unsigned int to stream
	*/

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);

	ShaderManager::use(0);

	// MESH DEBUG DRAWING
	/*
	if (f_normals || v_normals)
	{
		ShaderManager::use(App->primitives->shaderPrimitive);
		ShaderManager::setFloat4x4(App->primitives->shaderPrimitive, "model", App->scene->drop->GetTransform()->GetGlobalMatrix().ptr());
		ShaderManager::setFloat4x4(App->primitives->shaderPrimitive, "view", App->renderer3d->camera->GetView().ptr());
		ShaderManager::setFloat4x4(App->primitives->shaderPrimitive, "projection", App->renderer3d->camera->GetProjection().ptr());
	}

	if (f_normals)
	{
		math::vec color(0.f, 0.f, 1.f);
		ShaderManager::setFloat(App->primitives->shaderPrimitive, "objectColor", color);

		glBindVertexArray(VAO_FaceNormals);
		glDrawArrays(GL_LINES, 0, indices.size() / 3 * 2);
		glBindVertexArray(0);
	}

	if (v_normals)
	{
		math::vec color(0.f, 1.f, 0.f);
		ShaderManager::setFloat(App->primitives->shaderPrimitive, "objectColor", color);

		glBindVertexArray(VAO_VertexNormals);
		glDrawArrays(GL_LINES, 0, indices.size() * 2);
		glBindVertexArray(0);
	}

	if (f_normals || v_normals)
	{
		math::vec color(1.f, 1.f, 1.f);
		ShaderManager::setFloat(App->primitives->shaderPrimitive, "objectColor", color);

		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointSize(10.0f);

		glBindVertexArray(VAO_Vertex);
		glDrawArrays(GL_POINTS, 0, vertices.size());
		glBindVertexArray(0);

		glPointSize(1.0f);
		glDisable(GL_PROGRAM_POINT_SIZE);

		ShaderManager::use(0);
	}*/
}

void RE_Mesh::SetupAABB()
{
	// reset size to minimum
	bounding_box.SetFromCenterAndSize(vertices[0].Position, vertices[0].Position);

	// adapt box to each vertex position
	for (auto vertex : vertices)
	{
		// X
		if (vertex.Position.x > bounding_box.maxPoint.x)
			bounding_box.maxPoint.x = vertex.Position.x;
		else if (vertex.Position.x < bounding_box.minPoint.x)
			bounding_box.minPoint.x = vertex.Position.x;

		// Y
		if (vertex.Position.y > bounding_box.maxPoint.y)
			bounding_box.maxPoint.y = vertex.Position.y;
		else if (vertex.Position.y < bounding_box.minPoint.y)
			bounding_box.minPoint.y = vertex.Position.y;

		// Z
		if (vertex.Position.z > bounding_box.maxPoint.z)
			bounding_box.maxPoint.z = vertex.Position.z;
		else if (vertex.Position.z < bounding_box.minPoint.z)
			bounding_box.minPoint.z = vertex.Position.z;
	}
}

void RE_Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	glBindVertexArray(0);
}

math::AABB RE_Mesh::GetAABB() const
{
	return bounding_box;
}

void RE_Mesh::loadVertexNormals()
{
	if (!lFaceNormals) loadVertex();

	glGenVertexArrays(1, &VAO_VertexNormals);
	glGenBuffers(1, &VBO_VertexNormals);

	glBindVertexArray(VAO_VertexNormals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_VertexNormals);

	int line_length = 1.5f;

	std::vector<math::vec> lines;
	for (unsigned int i = 0; i < indices.size(); i++)
	{
		lines.push_back(math::vec(vertices[i].Position.x, vertices[i].Position.y, vertices[i].Position.z));
		lines.push_back(math::vec(vertices[i].Position.x + (vertices[i].Normal.x * line_length),
			vertices[i].Position.y + (vertices[i].Normal.y * line_length),
			vertices[i].Position.z + (vertices[i].Normal.z * line_length)));
	}

	glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(math::vec), &lines[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(math::vec), (void*)0);

	glBindVertexArray(0);

	lVertexNormals = true;
}

void RE_Mesh::loadFaceNormals()
{
	if (!lVertexNormals) loadVertex();

	glGenVertexArrays(1, &VAO_FaceNormals);
	glGenBuffers(1, &VBO_FaceNormals);

	glBindVertexArray(VAO_FaceNormals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_FaceNormals);

	math::vec pos = math::vec::zero;
	math::vec v = math::vec::zero;
	math::vec w = math::vec::zero;
	math::vec normal = math::vec::zero;
	int line_length = 1.5f;

	std::vector<math::vec> lines;
	for (unsigned int i = 0; i < indices.size(); i += 3)
	{
		pos = (vertices[i].Position + vertices[i + 1].Position + vertices[i + 2].Position) / 3;
		v = vertices[i + 1].Position - vertices[i].Position;
		w = vertices[i + 2].Position - vertices[i].Position;
		normal = v.Cross(w).Normalized() * line_length;
		;
		lines.push_back(math::vec(pos.x, pos.y, pos.z));
		lines.push_back(math::vec(pos.x + normal.x, pos.y + normal.y, pos.z + normal.z));
	}

	glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(math::vec), &lines[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(math::vec), (void*)0);

	glBindVertexArray(0);

	lFaceNormals = true;
}

void RE_Mesh::clearVertexNormals()
{
	if (!lFaceNormals) clearVertex();

	glDeleteVertexArrays(1, &VAO_VertexNormals);
	glDeleteBuffers(1, &VBO_VertexNormals);
	VAO_VertexNormals = VBO_VertexNormals = 0;

	lVertexNormals = false;
}

void RE_Mesh::clearFaceNormals()
{
	if (!lVertexNormals) clearVertex();

	glDeleteVertexArrays(1, &VAO_FaceNormals);
	glDeleteBuffers(1, &VBO_FaceNormals);
	VAO_FaceNormals = VBO_FaceNormals = 0;

	lFaceNormals = false;
}

void RE_Mesh::loadVertex()
{
	glGenVertexArrays(1, &VAO_Vertex);
	glGenBuffers(1, &VBO_Vertex);

	glBindVertexArray(VAO_Vertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Vertex);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glBindVertexArray(0);
}

void RE_Mesh::clearVertex()
{
	glDeleteVertexArrays(1, &VAO_Vertex);
	glDeleteBuffers(1, &VBO_Vertex);
	VAO_Vertex = VBO_Vertex = 0;
}