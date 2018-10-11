#include "RE_CompMesh.h"

#include "Application.h"
#include "MeshManager.h"
#include "OutputLog.h"
#include "Globals.h"

RE_CompMesh::RE_CompMesh(RE_GameObject * go, const char * path, const bool file_dropped, const bool start_active) : RE_Component(C_MESH, go, start_active)
{
	LoadMesh(path, file_dropped);
}

RE_CompMesh::~RE_CompMesh()
{
	((ResourceManager*)App->meshes)->UnReference(reference);
}

unsigned int RE_CompMesh::LoadMesh(const char * path, bool dropped)
{
	((ResourceManager*)App->meshes)->UnReference(reference);

	if (path != nullptr)
		reference = App->meshes->LoadMesh(path, dropped);

	return reference;
}

void RE_CompMesh::Draw()
{
	if(reference) App->meshes->DrawMesh(reference);
}





































#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ShaderManager.h"
#include "Texture2DManager.h"
#include "ModuleScene.h"
#include "RE_GameObject.h"
#include "RE_Component.h"
#include "RE_CompTransform.h"
#include "RE_PrimitiveManager.h"
#include "FileSystem.h"
#include "RE_Camera.h"
#include "ImGui\imgui.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "MathGeoLib\include\MathGeoLib.h"
#include "MathGeoLib\include\Math\Quat.h"

#include "SDL2\include\SDL_assert.h"
#include "assimp/include/Importer.hpp"
#include "assimp/include/scene.h"
#include "assimp/include/postprocess.h"


RE_UnregisteredMesh::RE_UnregisteredMesh(std::vector<_Vertex> vertices, std::vector<unsigned int> indices, std::vector<_Texture> textures, unsigned int triangles)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	triangle_count = triangles;

	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	_setupMesh();
}
/*
RE_UnregisteredMesh::~RE_UnregisteredMesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	if (lVertexNormals) clearVertexNormals();
	if (lFaceNormals) clearFaceNormals();
}
*/
void RE_UnregisteredMesh::Draw(unsigned int shader_ID, bool f_normals, bool v_normals)
{
	ShaderManager::use(shader_ID);
	// bind appropriate textures
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
										  // retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++); // transfer unsigned int to stream
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); // transfer unsigned int to stream
		else if (name == "texture_height")
			number = std::to_string(heightNr++); // transfer unsigned int to stream

												 // now set the sampler to the correct texture unit
		ShaderManager::setUnsignedInt(shader_ID, (name + number).c_str(), i);

		// and finally bind the texture
		App->textures->use(textures[i].id);
	}

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);

	ShaderManager::use(0);

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
	}
}

void RE_UnregisteredMesh::_setupMesh()
{
	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(_Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(_Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(_Vertex), (void*)offsetof(_Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(_Vertex), (void*)offsetof(_Vertex, TexCoords));
	// vertex tangent
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	//glEnableVertexAttribArray(4);
	//glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	glBindVertexArray(0);
}

void RE_UnregisteredMesh::loadVertexNormals()
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

void RE_UnregisteredMesh::loadFaceNormals()
{
	if(!lVertexNormals) loadVertex();

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

void RE_UnregisteredMesh::clearVertexNormals()
{
	if (!lFaceNormals) clearVertex();

	glDeleteVertexArrays(1, &VAO_VertexNormals);
	glDeleteBuffers(1, &VBO_VertexNormals);
	VAO_VertexNormals = VBO_VertexNormals = 0;

	lVertexNormals = false;
}

void RE_UnregisteredMesh::clearFaceNormals()
{
	if (!lVertexNormals) clearVertex();

	glDeleteVertexArrays(1, &VAO_FaceNormals);
	glDeleteBuffers(1, &VBO_FaceNormals);
	VAO_FaceNormals = VBO_FaceNormals = 0;

	lFaceNormals = false;
}

void RE_UnregisteredMesh::loadVertex()
{
	glGenVertexArrays(1, &VAO_Vertex);
	glGenBuffers(1, &VBO_Vertex);

	glBindVertexArray(VAO_Vertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Vertex);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(_Vertex), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(_Vertex), (void*)0);

	glBindVertexArray(0);
}

void RE_UnregisteredMesh::clearVertex()
{
	glDeleteVertexArrays(1, &VAO_Vertex);
	glDeleteBuffers(1, &VBO_Vertex);
	VAO_Vertex = VBO_Vertex = 0;
}


RE_CompUnregisteredMesh::RE_CompUnregisteredMesh(char * path)
{
	loadModel(path);
	bounding_box.minPoint = bounding_box.maxPoint = math::vec::zero;
}

RE_CompUnregisteredMesh::RE_CompUnregisteredMesh(char * path, const char * buffer, unsigned int size)
{
	buffer_size = size;
	buffer_file = buffer;
	dropped = true;
	loadModel(path);
}

void RE_CompUnregisteredMesh::Draw(unsigned int shader)
{
	if (!error_loading)
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader, show_f_normals, show_v_normals);
}

void RE_CompUnregisteredMesh::loadModel(std::string path)
{
	if (buffer_file != nullptr)
	{
		LOG("Loading Model: %s", buffer_file);

		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFileFromMemory(buffer_file, buffer_size, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
			return;
		}

		name = buffer_file;
		directory = path.substr(0, path.find_last_of('\\'));

		processNode(scene->mRootNode, scene);
	}
	else
	{
		LOG("Loading Model from: %s", path.c_str());

		RE_FileIO modelFBX(path.c_str());

		if (modelFBX.Load())
		{
			Assimp::Importer importer;
			const aiScene *scene = importer.ReadFileFromMemory(modelFBX.GetBuffer(), modelFBX.GetSize(), aiProcess_Triangulate | aiProcess_FlipUVs);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
				return;
			}

			name = modelFBX.GetBuffer();
			directory = path.substr(0, path.find_last_of('/'));

			processNode(scene->mRootNode, scene);
		}
	}
}

void RE_CompUnregisteredMesh::processNode(aiNode * node, const aiScene * scene)
{
	LOG_SECONDARY("%s Node: %s (%u meshes | %u children)",
		node->mParent ? "SON" : "PARENT",
		node->mName.C_Str(),
		node->mNumMeshes,
		node->mNumChildren);

	unsigned int i = 0;

	for (; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene, i + 1));
		meshes.rbegin()->name = node->mName.C_Str();
		total_triangle_count += meshes.rbegin()->triangle_count;
	}

	for (i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}

RE_UnregisteredMesh RE_CompUnregisteredMesh::processMesh(aiMesh * mesh, const aiScene * scene, const unsigned int pos)
{
	std::vector<_Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<_Texture> textures;

	LOG_TERCIARY("Mesh %u: %s (%u vertices | %u faces | %u texture indexes | %u bones)",
		pos,
		mesh->mName.C_Str(),
		mesh->mNumVertices,
		mesh->mNumFaces,
		mesh->mMaterialIndex,
		mesh->mNumBones);

	// process vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		// process vertex positions, normals and texture coordinates
		_Vertex vertex;
		if		((vertex.Position.x = mesh->mVertices[i].x) < bounding_box.minPoint.x) bounding_box.minPoint.x = vertex.Position.x;
		else if ((vertex.Position.x = mesh->mVertices[i].x) > bounding_box.maxPoint.x) bounding_box.maxPoint.x = vertex.Position.x;
		if		((vertex.Position.y = mesh->mVertices[i].y) < bounding_box.minPoint.y) bounding_box.minPoint.y = vertex.Position.y;
		else if ((vertex.Position.y = mesh->mVertices[i].y) > bounding_box.maxPoint.y) bounding_box.maxPoint.y = vertex.Position.y;
		if		((vertex.Position.z = mesh->mVertices[i].z) < bounding_box.minPoint.z) bounding_box.minPoint.z = vertex.Position.z;
		else if ((vertex.Position.z = mesh->mVertices[i].z) > bounding_box.maxPoint.z) bounding_box.maxPoint.z = vertex.Position.x;

		vertex.Normal.x = mesh->mNormals[i].x;
		vertex.Normal.y = mesh->mNormals[i].y;
		vertex.Normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.TexCoords = math::float2::zero;

		vertices.push_back(vertex);
	}
	
	// process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		
		if (face.mNumIndices != 3)
		{
			error_loading = true;
			LOG_WARNING("Loading geometry face with %u indexes (instead of 3)", face.mNumIndices);
		}

		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// process material
	aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<_Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	//std::vector<_Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	//textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	return RE_UnregisteredMesh(vertices, indices, textures, mesh->mNumFaces);
}

std::vector<_Texture> RE_CompUnregisteredMesh::loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName)
{
	std::vector<_Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{   // if texture hasn't been loaded already, load it
			_Texture texture;
			texture.id = App->textures->LoadTexture2D(directory.c_str(), str.C_Str(), dropped);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture); // add to loaded textures
		}
	}
	return textures;
}

void RE_CompUnregisteredMesh::DrawProperties()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		ImGui::Text("Name: %s%s", name.c_str(), dropped ? " (dropped)" : " ");

		ImGui::TextWrapped("Directory: %s", directory.c_str());
		ImGui::Text("Triangle Count: %u", total_triangle_count);

		if (ImGui::Button(show_f_normals ? "Hide Face Normals" : "Show Face Normals")) show_f_normals = !show_f_normals;

		if (ImGui::Button(show_v_normals ? "Hide Vertex Normals" : "Show Vertex Normals")) show_v_normals = !show_v_normals;


		int width = 0;
		int height = 0;
		std::vector<RE_UnregisteredMesh>::iterator it = meshes.begin();
		for (; it != meshes.end(); it++)
		{
			if (show_f_normals && !it->lFaceNormals)	it->loadFaceNormals();
			if (show_v_normals && !it->lVertexNormals)	it->loadVertexNormals();

			if (!show_f_normals && it->lFaceNormals) it->clearFaceNormals();
			if (!show_v_normals && it->lVertexNormals) it->clearVertexNormals();

			if (ImGui::TreeNode(it->name.c_str()))
			{
				ImGui::Text("Vertex count: %u", it->vertices.size());
				ImGui::Text("Triangle Face count: %u", it->triangle_count);
				ImGui::Text("VAO: %u", it->VAO);

				std::vector<_Texture>::iterator it2 = it->textures.begin();
				for (unsigned int i = 1; it2 != it->textures.end(); it2++, i++)
				{
					App->textures->GetWithHeight(it2->id, &width, &height);

					if (ImGui::TreeNode("Texture"))
					{
						ImGui::Text("\t- ID: %u", it2->id);
						ImGui::Text("\t- Size: %ux%u", width, height);
						ImGui::Text("\t- Path: %s", it2->path.c_str());
						ImGui::Text("\t- Type: %s", it2->type.c_str());

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
		}
	}
}
