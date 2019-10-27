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


RE_Mesh::RE_Mesh(std::vector<Vertex> _vertices, std::vector<unsigned int> _indices, const char* _materialMD5, unsigned int triangles)
{
	vertices = _vertices;
	indices = _indices;
	materialMD5 = _materialMD5;
	triangle_count = triangles;

	SetupAABB();
	SetupMesh();
}

RE_Mesh::~RE_Mesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	if (lVertexNormals) clearVertexNormals();
	if (lFaceNormals) clearFaceNormals();
}

void RE_Mesh::Draw(const float* transform, bool use_checkers)
{
	// Set Shader uniforms
	ShaderManager::use(App->scene->modelloading);
	ShaderManager::setFloat4x4(App->scene->modelloading, "model", transform);

	// Bind Textures
	if (use_checkers)
	{
		DrawCheckerTexture();
	}
	else if (materialMD5)
	{
		RE_Material* meshMaterial = (RE_Material*)App->resources->At(materialMD5);

		// Bind diffuse textures
		unsigned int diffuseNr = 1;
		if (!meshMaterial->tDiffuse.empty())
		{
			for (unsigned int i = 0; i < meshMaterial->tDiffuse.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				std::string name = "texture_diffuse";
				name += std::to_string(diffuseNr++);
				ShaderManager::setUnsignedInt(App->scene->modelloading, name.c_str(), i);
				((Texture2D*)App->resources->At(meshMaterial->tDiffuse[i]))->use();
			}
		}
		else {
			use_checkers = true;
			DrawCheckerTexture();
		}
		/*
			unsigned int specularNr = 1;
			unsigned int normalNr = 1;
			unsigned int heightNr = 1;

				if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++); // transfer unsigned int to stream
			else if (name == "texture_normal")
				number = std::to_string(normalNr++); // transfer unsigned int to stream
			else if (name == "texture_height")
				number = std::to_string(heightNr++); // transfer unsigned int to stream
		*/
	}
	else {
		use_checkers = true;
		DrawCheckerTexture();
	}

	// Draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// Release buffers
	glBindVertexArray(0);
	if (use_checkers || materialMD5)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// MESH DEBUG DRAWING
	if (lFaceNormals || lVertexNormals)
	{
		ShaderManager::use(App->primitives->shaderPrimitive);
		ShaderManager::setFloat4x4(App->primitives->shaderPrimitive, "model", transform);

		if (lFaceNormals)
		{
			math::vec color(0.f, 0.f, 1.f);
			ShaderManager::setFloat(App->primitives->shaderPrimitive, "objectColor", color);

			glBindVertexArray(VAO_FaceNormals);
			glDrawArrays(GL_LINES, 0, indices.size() / 3 * 2);
			glBindVertexArray(0);

			color.Set(1.f, 1.f, 1.f);
			ShaderManager::setFloat(App->primitives->shaderPrimitive, "objectColor", color);

			glEnable(GL_PROGRAM_POINT_SIZE);
			glPointSize(10.0f);

			glBindVertexArray(VAO_FaceCenters);
			glDrawArrays(GL_POINTS, 0, indices.size() / 3);
			glBindVertexArray(0);

			glPointSize(1.0f);
			glDisable(GL_PROGRAM_POINT_SIZE);
		}

		if (lVertexNormals)
		{
			math::vec color(0.f, 1.f, 0.f);
			ShaderManager::setFloat(App->primitives->shaderPrimitive, "objectColor", color);

			glBindVertexArray(VAO_VertexNormals);
			glDrawArrays(GL_LINES, 0, vertices.size() * 2);
			glBindVertexArray(0);

			color.Set(1.f, 1.f, 1.f);
			ShaderManager::setFloat(App->primitives->shaderPrimitive, "objectColor", color);

			glEnable(GL_PROGRAM_POINT_SIZE);
			glPointSize(10.0f);

			glBindVertexArray(VAO_Vertex);
			glDrawArrays(GL_POINTS, 0, vertices.size());
			glBindVertexArray(0);

			glPointSize(1.0f);
			glDisable(GL_PROGRAM_POINT_SIZE);
		}
	}

	ShaderManager::use(0);
}

void RE_Mesh::DrawCheckerTexture()
{
	glActiveTexture(GL_TEXTURE0);
	std::string name = "texture_diffuse0";
	ShaderManager::setUnsignedInt(App->scene->modelloading, name.c_str(), 0);
	glBindTexture(GL_TEXTURE_2D, App->scene->checkers_texture);
}

void RE_Mesh::SetupAABB()
{
	std::vector<math::vec> vertex_pos;
	vertex_pos.resize(vertices.size());

	for (int i = 0; i < vertices.size(); i++)
		vertex_pos[i].Set(vertices[i].Position.x, vertices[i].Position.y, vertices[i].Position.z);

	bounding_box.SetFrom(&vertex_pos[0], vertices.size());
}

void RE_Mesh::SetupMesh()
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
	if (!lFaceNormals)
		LoadVertex();

	glGenVertexArrays(1, &VAO_VertexNormals);
	glGenBuffers(1, &VBO_VertexNormals);

	glBindVertexArray(VAO_VertexNormals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_VertexNormals);

	int line_length = 1.5f;

	std::vector<math::vec> lines;
	for (unsigned int i = 0; i < indices.size(); i++)
	{
		lines.push_back(math::vec(vertices[indices[i]].Position.x, vertices[indices[i]].Position.y, vertices[indices[i]].Position.z));
		lines.push_back(math::vec(
			vertices[indices[i]].Position.x + (vertices[indices[i]].Normal.x * line_length),
			vertices[indices[i]].Position.y + (vertices[indices[i]].Normal.y * line_length),
			vertices[indices[i]].Position.z + (vertices[indices[i]].Normal.z * line_length)));
	}

	glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(math::vec), &lines[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(math::vec), (void*)0);

	glBindVertexArray(0);

	lVertexNormals = true;
}

void RE_Mesh::loadFaceNormals()
{
	if (!lVertexNormals) LoadVertex();

	glGenVertexArrays(1, &VAO_FaceNormals);
	glGenBuffers(1, &VBO_FaceNormals);

	glBindVertexArray(VAO_FaceNormals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_FaceNormals);

	math::vec pos = math::vec::zero;
	math::vec v = math::vec::zero;
	math::vec w = math::vec::zero;
	math::vec normal = math::vec::zero;
	int line_length = 1.5f;

	std::vector<math::vec> lines, face_centers;
	for (unsigned int i = 0; i < indices.size(); i += 3)
	{
		pos = (vertices[indices[i]].Position + vertices[indices[i + 1]].Position + vertices[indices[i + 2]].Position) / 3;
		v = vertices[indices[i + 1]].Position - vertices[indices[i]].Position;
		w = vertices[indices[i + 2]].Position - vertices[indices[i]].Position;
		normal = v.Cross(w).Normalized() * line_length;

		lines.push_back(math::vec(pos.x, pos.y, pos.z));
		lines.push_back(math::vec(pos.x + normal.x, pos.y + normal.y, pos.z + normal.z));
		face_centers.push_back(math::vec(pos.x, pos.y, pos.z));
	}

	glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(math::vec), &lines[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(math::vec), (void*)0);

	glBindVertexArray(0);

	glGenVertexArrays(1, &VAO_FaceCenters);
	glGenBuffers(1, &VBO_FaceCenters);

	glBindVertexArray(VAO_FaceCenters);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_FaceCenters);

	glBufferData(GL_ARRAY_BUFFER, face_centers.size() * sizeof(math::vec), &face_centers[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(math::vec), (void*)0);

	glBindVertexArray(0);

	lFaceNormals = true;
}

void RE_Mesh::clearVertexNormals()
{
	if (!lFaceNormals) ClearVertex();

	glDeleteVertexArrays(1, &VAO_VertexNormals);
	glDeleteBuffers(1, &VBO_VertexNormals);
	VAO_VertexNormals = VBO_VertexNormals = 0;

	lVertexNormals = false;
}

void RE_Mesh::clearFaceNormals()
{
	if (!lVertexNormals) ClearVertex();

	glDeleteVertexArrays(1, &VAO_FaceNormals);
	glDeleteBuffers(1, &VBO_FaceNormals);
	VAO_FaceNormals = VBO_FaceNormals = 0;

	glDeleteVertexArrays(1, &VAO_FaceCenters);
	glDeleteBuffers(1, &VBO_FaceCenters);
	VAO_FaceCenters = VBO_FaceCenters = 0;

	lFaceNormals = false;
}

void RE_Mesh::LoadVertex()
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

void RE_Mesh::ClearVertex()
{
	glDeleteVertexArrays(1, &VAO_Vertex);
	glDeleteBuffers(1, &VBO_Vertex);
	VAO_Vertex = VBO_Vertex = 0;
}