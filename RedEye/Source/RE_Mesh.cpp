#include "RE_Mesh.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleScene.h"

#include "RE_FileSystem.h"
#include "RE_InternalResources.h"
#include "RE_ResourceManager.h"

#include "RE_ShaderImporter.h"
#include "RE_TextureImporter.h"

#include "RE_Material.h"
#include "RE_Texture.h"

#include "RE_GameObject.h"
#include "RE_Component.h"
#include "RE_CompTransform.h"

#include "OutputLog.h"

#include "ImGui\imgui.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "MathGeoLib\include\MathGeoLib.h"
#include "MathGeoLib\include\Math\Quat.h"
#include "md5.h"

#include "SDL2\include\SDL_assert.h"
#include "assimp/include/Importer.hpp"
#include "assimp/include/scene.h"
#include "assimp/include/postprocess.h"


RE_Mesh::RE_Mesh() { }

RE_Mesh::~RE_Mesh() { }

void RE_Mesh::LoadInMemory()
{
	if (App->fs->Exists(GetLibraryPath())) {
		LibraryLoad();
		SetupAABB();
		SetupMesh();
	}
	else {
		LOG_ERROR("Texture %s not found on project", GetName());
	}
}

void RE_Mesh::UnloadMemory()
{
	glDeleteVertexArrays(1, &VAO); VAO = 0;
	glDeleteBuffers(1, &VBO); VBO = 0;
	glDeleteBuffers(1, &EBO); EBO = 0;

	if (lVertexNormals) clearVertexNormals();
	if (lFaceNormals) clearFaceNormals();

	if (vertex) DEL_A(vertex);
	if (normals) DEL_A(normals);
	if (tangents) DEL_A(tangents);
	if (bitangents) DEL_A(bitangents);
	if (texturecoords) DEL_A(texturecoords);
	if (index) DEL_A(index);

	ResourceContainer::inMemory = false;
}

const char* RE_Mesh::CheckAndSave(bool* exists)
{
	size_t size = sizeof(uint);
	size += sizeof(bool) * 5;

	size += sizeof(float) * 3 * triangle_count; //vertex
	if (normals) size += sizeof(float) * 3 * triangle_count;
	if (tangents) size += sizeof(float) * 3 * triangle_count;
	if (bitangents) size += sizeof(float) * 3 * triangle_count;
	if (texturecoords) size += sizeof(float) * 2 * triangle_count;
	if (index) size += sizeof(uint) * 3 * triangle_count;

	char* buffer = new char[size + 1];
	char* cursor = buffer;

	size_t cSize = sizeof(uint);
	memcpy(cursor, &triangle_count, cSize);
	cursor += cSize;

	if (vertex) {
		size_t cSize = sizeof(float) * 3 * triangle_count;
		memcpy(cursor, vertex, cSize);
		cursor += cSize;
	}

	bool toFill = (normals) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;
	if (normals) {
		size_t cSize = sizeof(float) * 3 * triangle_count;
		memcpy(cursor, normals, cSize);
		cursor += cSize;
	}

	toFill = (tangents) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;
	if (tangents) {
		size_t cSize = sizeof(float) * 3 * triangle_count;
		memcpy(cursor, tangents, cSize);
		cursor += cSize;
	}

	toFill = (bitangents) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;
	if (bitangents) {
		size_t cSize = sizeof(float) * 3 * triangle_count;
		memcpy(cursor, bitangents, cSize);
		cursor += cSize;
	}

	toFill = (texturecoords) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;
	if (texturecoords) {
		size_t cSize = sizeof(float) * 2 * triangle_count;
		memcpy(cursor, texturecoords, cSize);
		cursor += cSize;
	}

	toFill = (index) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;
	if (index) {
		size_t cSize = sizeof(uint) * 3 * triangle_count;
		memcpy(cursor, index, cSize);
		cursor += cSize;
	}

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	std::string md5Generated = MD5(std::string(buffer, size + 1)).hexdigest();
	const char* existsMD5 = App->resources->IsReference(md5Generated.c_str());
	if (!existsMD5) {
		SetMD5(md5Generated.c_str());
		existsMD5 = GetMD5();

		std::string libraryPath("Library/Meshes/");
		libraryPath += existsMD5;
		SetLibraryPath(libraryPath.c_str());

		RE_FileIO toSave(GetLibraryPath(), App->fs->GetZipPath());
		toSave.Save(buffer, size + 1);
	}
	else
		*exists = true;

	if (vertex) DEL_A(vertex);
	if (normals) DEL_A(normals);
	if (tangents) DEL_A(tangents);
	if (bitangents) DEL_A(bitangents);
	if (texturecoords) DEL_A(texturecoords);
	if (index) DEL_A(index);
	DEL_A(buffer);
	return existsMD5;
}

void RE_Mesh::DrawMesh(const float* transform, unsigned int shader, const char* materialMD5, unsigned int checker, bool use_checkers)
{
	// Set Shader uniforms
	RE_ShaderImporter::use(shader);
	RE_ShaderImporter::setFloat4x4(shader, "model", transform);

	RE_Material* meshMaterial = nullptr;
	if(materialMD5) meshMaterial = (RE_Material*)App->resources->At(materialMD5);

	// Bind Textures
	if (use_checkers || !materialMD5 || meshMaterial->tDiffuse.empty())
	{
		RE_ShaderImporter::setFloat(shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(shader, "useTexture", 1.0f);

		use_checkers = true;
		glActiveTexture(GL_TEXTURE0);
		std::string name = "texture_diffuse0";
		RE_ShaderImporter::setUnsignedInt(shader, name.c_str(), 0);
		glBindTexture(GL_TEXTURE_2D, checker);
	}
	else if (materialMD5)
	{
		// Bind diffuse textures
		unsigned int diffuseNr = 1;
		if (!meshMaterial->tDiffuse.empty())
		{
			RE_ShaderImporter::setFloat(shader, "useColor", 0.0f);
			RE_ShaderImporter::setFloat(shader, "useTexture", 1.0f);

			for (unsigned int i = 0; i < meshMaterial->tDiffuse.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				std::string name = "texture_diffuse";
				name += std::to_string(diffuseNr++);
				RE_ShaderImporter::setUnsignedInt(shader, name.c_str(), i);
				((RE_Texture*)App->resources->At(meshMaterial->tDiffuse[i]))->use();
			}
		}
		/* TODO DRAW WITH MATERIAL

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

	// Draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, 0);

	// Release buffers
	glBindVertexArray(0);
	if (use_checkers || (materialMD5 && !meshMaterial->tDiffuse.empty()))
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// MESH DEBUG DRAWING
	if (lFaceNormals || lVertexNormals)
	{
		RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);

		if (lFaceNormals)
		{
			math::vec color(0.f, 0.f, 1.f);
			RE_ShaderImporter::setFloat(shader, "objectColor", color);

			glBindVertexArray(VAO_FaceNormals);
			glDrawArrays(GL_LINES, 0, triangle_count * 2);
			glBindVertexArray(0);

			color.Set(1.f, 1.f, 1.f);
			RE_ShaderImporter::setFloat(shader, "objectColor", color);

			glEnable(GL_PROGRAM_POINT_SIZE);
			glPointSize(10.0f);

			glBindVertexArray(VAO_FaceCenters);
			glDrawArrays(GL_POINTS, 0, triangle_count);
			glBindVertexArray(0);

			glPointSize(1.0f);
			glDisable(GL_PROGRAM_POINT_SIZE);
		}

		if (lVertexNormals)
		{
			math::vec color(0.f, 1.f, 0.f);
			RE_ShaderImporter::setFloat(shader, "objectColor", color);

			glBindVertexArray(VAO_VertexNormals);
			glDrawArrays(GL_LINES, 0, triangle_count * 3 * 2);
			glBindVertexArray(0);

			color.Set(1.f, 1.f, 1.f);
			RE_ShaderImporter::setFloat(shader, "objectColor", color);

			glEnable(GL_PROGRAM_POINT_SIZE);
			glPointSize(10.0f);

			glBindVertexArray(VAO_Vertex);
			glDrawArrays(GL_POINTS, 0, triangle_count * 3);
			glBindVertexArray(0);

			glPointSize(1.0f);
			glDisable(GL_PROGRAM_POINT_SIZE);
		}
	}

	RE_ShaderImporter::use(0);
}

void RE_Mesh::Draw()
{
	ImGui::Text("%u triangles.", &triangle_count);
	ImGui::Text("Bounding Box");
	ImGui::TextWrapped("Min: { %.2f, %.2f, %.2f}", bounding_box.minPoint.x, bounding_box.minPoint.y, bounding_box.minPoint.z);
	ImGui::TextWrapped("Max: { %.2f, %.2f, %.2f}", bounding_box.maxPoint.x, bounding_box.maxPoint.y, bounding_box.maxPoint.z);
	ImGui::Text((normals) ? "Has normals." : "Hasn't normals");
	ImGui::Text((tangents) ? "Has tangents." : "Hasn't tangents");
	ImGui::Text((bitangents) ? "Has bitangents." : "Hasn't bitangents");
	ImGui::Text((texturecoords) ? "Has texture coordinates." : "Hasn't texture coordinates");
}

void RE_Mesh::SetupAABB()
{
	std::vector<math::vec> vertex_pos;
	vertex_pos.resize(triangle_count * 3);

	for (int i = 0; i < triangle_count; i++)
		vertex_pos[i] = math::vec(&vertex[i * 3]);

	bounding_box.SetFrom(&vertex_pos[0], triangle_count * 3);
}

void RE_Mesh::SetupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	uint strideSize = 3 * sizeof(float);
	uint meshSize = triangle_count * 3 * sizeof(float);

	if (normals) {
		meshSize += triangle_count * 3 * sizeof(float);
		strideSize += 3 * sizeof(float);
	}
	if (tangents) {
		meshSize += triangle_count * 3 * sizeof(float);
		strideSize += 3 * sizeof(float);
	}
	if (bitangents) {
		meshSize += triangle_count * 3 * sizeof(float);
		strideSize += 3 * sizeof(float);
	}
	if (texturecoords) {
		meshSize += triangle_count * 2 * sizeof(float);
		strideSize += 2 * sizeof(float);
	}

	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;

	for (uint i = 0; i < triangle_count * 3; i++) {
		uint indexArray = i * 3;
		uint indexTexCoord = i * 2;

		size_t size = sizeof(float) * 3;
		memcpy(cursor, &vertex[indexArray], size);
		cursor += size;

		if (normals) {
			memcpy(cursor, &normals[indexArray], size);
			cursor += size;
		}

		if (tangents) {
			memcpy(cursor, &tangents[indexArray], size);
			cursor += size;
		}

		if (bitangents) {
			memcpy(cursor, &bitangents[indexArray], size);
			cursor += size;
		}

		if (texturecoords) {
			size = sizeof(float) * 2;
			memcpy(cursor, &texturecoords[indexTexCoord], size);
			cursor += size;
		}
	}

	glBufferData(GL_ARRAY_BUFFER, meshSize, meshBuffer, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_count * 3 * sizeof(unsigned int), index, GL_STATIC_DRAW);

	uint accumulativeOffset = 0;

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, strideSize, &accumulativeOffset);
	accumulativeOffset += sizeof(float) * 3;

	// vertex normals
	if (normals) {
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, strideSize, &accumulativeOffset);
		accumulativeOffset += sizeof(float) * 3;
	}

	// vertex tangents
	if (tangents) {
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, strideSize, &accumulativeOffset);
		accumulativeOffset += sizeof(float) * 3;
	}

	// vertex bitangents
	if (bitangents) {
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, strideSize, &accumulativeOffset);
		accumulativeOffset += sizeof(float) * 3;
	}

	// vertex texture coords
	if (texturecoords) {
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, strideSize, &accumulativeOffset);
	}

	glBindVertexArray(0);
}

math::AABB RE_Mesh::GetAABB() const
{
	return bounding_box;
}

void RE_Mesh::loadVertexNormals()
{
	if (normals) {
		LoadVertex();

		int line_length = 1.5f;

		vertexNormals = new float[2 * 3 * triangle_count];
		float* cursor = vertexNormals;


		std::vector<math::vec> lines;
		for (unsigned int i = 0; i < triangle_count; i++)
		{
			uint vertexIndex = index[i] * 3;

			size_t size = sizeof(float) * 3;
			memcpy(cursor, &vertex[vertexIndex], size);
			cursor += size;

			math::vec norm(&normals[vertexIndex]);
			math::vec vert(&vertex[vertexIndex]);
			vert = { vert.x + (norm.x * line_length),
				vert.y + (norm.y * line_length),
				vert.z + (norm.z * line_length) };

			memcpy(cursor, &vert, size);
			cursor += size;
		}

		glGenVertexArrays(1, &VAO_VertexNormals);
		glGenBuffers(1, &VBO_VertexNormals);

		glBindVertexArray(VAO_VertexNormals);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_VertexNormals);

		glBufferData(GL_ARRAY_BUFFER, 2 * 3 * triangle_count * sizeof(float), vertexNormals, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindVertexArray(0);

		lVertexNormals = true;
	}
}

void RE_Mesh::loadFaceNormals()
{
	int line_length = 1.5f;

	faceNormals = new float[2 * 3 * triangle_count];
	faceCenters = new float[3 * triangle_count];
	float* fNCursor = faceNormals;
	float* fCcursor = faceCenters;

	math::vec pos = math::vec::zero;
	math::vec v = math::vec::zero;
	math::vec w = math::vec::zero;
	math::vec normal = math::vec::zero;

	uint triangleA = 0;
	uint triangleB = 0;
	uint triangleC = 0;
	std::vector<math::vec> lines, face_centers;
	for (unsigned int i = 0; i < triangle_count; i += 3)
	{
		uint triangleA = index[i] * 3;
		uint triangleB = index[i + 1] * 3;
		uint triangleC = index[i + 2] * 3;

		pos = (math::vec(&vertex[triangleA]) + math::vec(&vertex[triangleB]) + math::vec(&vertex[triangleC]) / 3);
		v = math::vec(&vertex[triangleB]) - math::vec(&vertex[triangleA]);
		w = math::vec(&vertex[triangleC]) - math::vec(&vertex[triangleA]);
		normal = v.Cross(w).Normalized() * line_length;

		size_t size = sizeof(float) * 3;
		memcpy(fNCursor, &pos, size);
		fNCursor += size;

		memcpy(fNCursor, &math::vec(pos.x + normal.x, pos.y + normal.y, pos.z + normal.z), size);
		fNCursor += size;

		memcpy(fCcursor, &pos, size);
		fCcursor += size;
	}

	glGenVertexArrays(1, &VAO_FaceNormals);
	glGenBuffers(1, &VBO_FaceNormals);

	glBindVertexArray(VAO_FaceNormals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_FaceNormals);

	glBufferData(GL_ARRAY_BUFFER, 2 * 3 * triangle_count * sizeof(float), faceNormals, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	glGenVertexArrays(1, &VAO_FaceCenters);
	glGenBuffers(1, &VBO_FaceCenters);

	glBindVertexArray(VAO_FaceCenters);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_FaceCenters);

	glBufferData(GL_ARRAY_BUFFER, 3 * triangle_count * sizeof(float), faceCenters, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	lFaceNormals = true;
}

void RE_Mesh::clearVertexNormals()
{
	ClearVertex();

	DEL_A(vertexNormals);
	glDeleteVertexArrays(1, &VAO_VertexNormals);
	glDeleteBuffers(1, &VBO_VertexNormals);
	VAO_VertexNormals = VBO_VertexNormals = 0;

	lVertexNormals = false;
}

void RE_Mesh::clearFaceNormals()
{
	DEL_A(faceNormals);
	glDeleteVertexArrays(1, &VAO_FaceNormals);
	glDeleteBuffers(1, &VBO_FaceNormals);
	VAO_FaceNormals = VBO_FaceNormals = 0;

	DEL_A(faceCenters);
	glDeleteVertexArrays(1, &VAO_FaceCenters);
	glDeleteBuffers(1, &VBO_FaceCenters);
	VAO_FaceCenters = VBO_FaceCenters = 0;

	lFaceNormals = false;
}

void RE_Mesh::SetVerticesAndIndex(float* v, unsigned int* i, unsigned int triangleCount, float* tC, float* n, float* t, float* bT)
{
	vertex = v; index = i; triangle_count = triangleCount; texturecoords = tC; normals = n; tangents = t; bitangents = bT;
}

bool RE_Mesh::CheckFaceCollision(const math::Ray& local_ray, float& distance) const
{
	bool ret = false;
	float res_dist;
	math::Triangle face;

	for (int i = 0; i < triangle_count; i++)
	{
		face.a = math::vec(&vertex[index[3 * i]]);
		face.b = math::vec(&vertex[index[(3 * i) + 1]]);
		face.c = math::vec(&vertex[index[(3 * i) + 2]]);

		if (face.Intersects(local_ray, &res_dist) && (!ret || distance > res_dist))
		{
			distance = res_dist;
			ret = true;
		}
	}

	return ret;

}

void RE_Mesh::LoadVertex()
{
	glGenVertexArrays(1, &VAO_Vertex);
	glGenBuffers(1, &VBO_Vertex);

	glBindVertexArray(VAO_Vertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Vertex);

	glBufferData(GL_ARRAY_BUFFER, triangle_count * 3 * sizeof(float), vertex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
}

void RE_Mesh::ClearVertex()
{
	glDeleteVertexArrays(1, &VAO_Vertex);
	glDeleteBuffers(1, &VBO_Vertex);
	VAO_Vertex = VBO_Vertex = 0;
}

void RE_Mesh::LibraryLoad()
{
	RE_FileIO toLoad(GetLibraryPath());

	if (toLoad.Load()) {
		char* cursor = toLoad.GetBuffer();

		size_t cSize = sizeof(uint);
		memcpy(&triangle_count, cursor, cSize);
		cursor += cSize;

		vertex = new float[triangle_count * 3];
		cSize = sizeof(float) * 3 * triangle_count;
		memcpy(vertex, cursor, cSize);
		cursor += cSize;

		bool toFill = false;
		cSize = sizeof(bool);
		memcpy(&toFill, cursor, cSize);
		cursor += cSize;
		if (toFill) {
			normals = new float[triangle_count * 3];
			cSize = sizeof(float) * 3 * triangle_count;
			memcpy(normals, cursor, cSize);
			cursor += cSize;
		}

		toFill = false;
		cSize = sizeof(bool);
		memcpy(cursor, &toFill, cSize);
		cursor += cSize;
		if (toFill) {
			tangents = new float[triangle_count * 3];
			cSize = sizeof(float) * 3 * triangle_count;
			memcpy(cursor, tangents, cSize);
			cursor += cSize;
		}

		toFill = false;
		cSize = sizeof(bool);
		memcpy(cursor, &toFill, cSize);
		cursor += cSize;
		if (toFill) {
			bitangents = new float[triangle_count * 3];
			size_t cSize = sizeof(float) * 3 * triangle_count;
			memcpy(cursor, bitangents, cSize);
			cursor += cSize;
		}

		toFill = false;
		cSize = sizeof(bool);
		memcpy(cursor, &toFill, cSize);
		cursor += cSize;
		if (toFill) {
			texturecoords = new float[triangle_count * 2];
			size_t cSize = sizeof(float) * 2 * triangle_count;
			memcpy(cursor, texturecoords, cSize);
			cursor += cSize;
		}

		toFill = false;
		cSize = sizeof(bool);
		memcpy(cursor, &toFill, cSize);
		cursor += cSize;
		if (toFill) {
			index = new uint[triangle_count * 3];
			cSize = sizeof(uint) * 3 * triangle_count;
			memcpy(cursor, index, cSize);
			cursor += cSize;
		}
	}
	ResourceContainer::inMemory = true;
}
