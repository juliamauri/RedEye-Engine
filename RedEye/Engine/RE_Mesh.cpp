#include <MGL/Geometry/AABB.h>
#include <MGL/Geometry/Ray.h>
#include "Resource.h"

#include "RE_Mesh.h"

#include "RE_Memory.h"
#include "RE_ConsoleLog.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleScene.h"
#include "RE_ShaderImporter.h"
#include "RE_ResourceManager.h"
#include "RE_GLCache.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"

#include <ImGui/imgui.h>
#include <GL/glew.h>
#include <gl/GL.h>
#include <MGL/MathGeoLib.h>
#include <MGL/Math\Quat.h>
#include <MD5/md5.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


RE_Mesh::RE_Mesh()
{
	bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);
}

void RE_Mesh::SetLibraryPath(const char* path)
{
	ResourceContainer::SetLibraryPath(path);
	eastl::string md5(path);
	md5 = md5.substr(md5.find_last_of("/") + 1);
	SetMD5(md5.c_str());
}

void RE_Mesh::LoadInMemory()
{
	if (RE_FS->Exists(GetLibraryPath()))
	{
		LibraryLoad();
		SetupAABB();
		SetupMesh();
	}
	else RE_LOG_ERROR("Mesh %s not found in project", GetName());
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
	size_t size = (sizeof(uint) * 2)  + (sizeof(bool) * 5) + (sizeof(float) * 3 * vertex_count);
	if (normals) size += sizeof(float) * 3 * vertex_count;
	if (tangents) size += sizeof(float) * 3 * vertex_count;
	if (bitangents) size += sizeof(float) * 3 * vertex_count;
	if (texturecoords) size += sizeof(float) * 2 * vertex_count;
	if (index) size += sizeof(uint) * 3 * triangle_count;

	char* buffer = new char[size + 1];
	char* cursor = buffer;

	size_t cSize = sizeof(uint);
	memcpy(cursor, static_cast<const void*>(&triangle_count), cSize);
	cursor += cSize;

	memcpy(cursor, &vertex_count, cSize);
	cursor += cSize;

	cSize = sizeof(float) * 3 * vertex_count;
	if (vertex)
	{
		memcpy(cursor, vertex, cSize);
		cursor += cSize;
	}

	bool toFill = (normals) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;

	cSize = sizeof(float) * 3 * vertex_count;
	if (toFill)
	{
		memcpy(cursor, normals, cSize);
		cursor += cSize;
	}

	toFill = (tangents) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;

	cSize = sizeof(float) * 3 * vertex_count;
	if (toFill)
	{
		memcpy(cursor, tangents, cSize);
		cursor += cSize;
	}

	toFill = (bitangents) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;

	cSize = sizeof(float) * 3 * vertex_count;
	if (toFill)
	{
		memcpy(cursor, bitangents, cSize);
		cursor += cSize;
	}

	toFill = (texturecoords) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;

	cSize = sizeof(float) * 2 * vertex_count;
	if (toFill)
	{
		memcpy(cursor, texturecoords, cSize);
		cursor += cSize;
	}

	toFill = (index) ? true : false;
	cSize = sizeof(bool);
	memcpy(cursor, &toFill, cSize);
	cursor += cSize;

	cSize = sizeof(uint) * 3 * triangle_count;
	if (toFill)
	{
		memcpy(cursor, index, cSize);
		cursor += cSize;
	}

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	eastl::string md5Generated = MD5(eastl::string(buffer, size + 1)).hexdigest();
	const char* existsMD5 = RE_RES->IsReference(md5Generated.c_str());
	if (!existsMD5)
	{
		SetMD5(md5Generated.c_str());
		existsMD5 = GetMD5();

		eastl::string libraryPath("Library/Meshes/");
		libraryPath += existsMD5;
		ResourceContainer::SetLibraryPath(libraryPath.c_str());

		RE_FileBuffer toSave(GetLibraryPath());
		toSave.Save(buffer, size + 1);
	}
	else *exists = true;

	if (vertex) DEL_A(vertex);
	if (normals) DEL_A(normals);
	if (tangents) DEL_A(tangents);
	if (bitangents) DEL_A(bitangents);
	if (texturecoords) DEL_A(texturecoords);
	if (index) DEL_A(index);
	DEL_A(buffer);
	return existsMD5;
}

void RE_Mesh::DrawMesh(unsigned int shader)
{
	// Draw mesh
	RE_GLCache::ChangeVAO(VAO);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, nullptr);
	RE_GLCache::ChangeVAO(0);

	// MESH DEBUG DRAWING
	if (lFaceNormals || lVertexNormals)
	{
		RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);

		if (lFaceNormals)
		{
			// Draw lines
			math::vec color(0.f, 0.f, 1.f);
			RE_ShaderImporter::setFloat(shader, "objectColor", color);
			RE_GLCache::ChangeVAO(VAO_FaceNormals);
			glDrawArrays(GL_LINES, 0, triangle_count * 2);
			RE_GLCache::ChangeVAO(0);

			// Draw points
			color.Set(1.f, 1.f, 1.f);
			RE_ShaderImporter::setFloat(shader, "objectColor", color);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glPointSize(10.0f);
			RE_GLCache::ChangeVAO(VAO_FaceCenters);
			glDrawArrays(GL_POINTS, 0, triangle_count);
			RE_GLCache::ChangeVAO(0);

			// Reset Draw mode
			glPointSize(1.0f);
			glDisable(GL_PROGRAM_POINT_SIZE);
		}

		if (lVertexNormals)
		{
			// Draw lines
			math::vec color(0.f, 1.f, 0.f);
			RE_ShaderImporter::setFloat(shader, "objectColor", color);
			RE_GLCache::ChangeVAO(VAO_VertexNormals);
			glDrawArrays(GL_LINES, 0, triangle_count * 3 * 2);
			RE_GLCache::ChangeVAO(0);

			// Draw points
			color.Set(1.f, 1.f, 1.f);
			RE_ShaderImporter::setFloat(shader, "objectColor", color);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glPointSize(10.0f);
			RE_GLCache::ChangeVAO(VAO_Vertex);
			glDrawArrays(GL_POINTS, 0, triangle_count * 3);
			RE_GLCache::ChangeVAO(0);

			// Reset Draw mode
			glPointSize(1.0f);
			glDisable(GL_PROGRAM_POINT_SIZE);
		}
	}

	RE_GLCache::ChangeShader(0);
	RE_GLCache::ChangeTextureBind(0);
}

void RE_Mesh::Draw()
{
	ImGui::Text("%u triangles.", triangle_count);
	ImGui::Text("%u vertex.", vertex_count);
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
	eastl::vector<math::vec> vertex_pos;
	vertex_pos.resize(vertex_count * 3);
	for (unsigned int i = 0; i < vertex_count; i++) vertex_pos[i] = math::vec(&vertex[i * 3]);
	bounding_box.SetFrom(&vertex_pos[0], vertex_count);
}

void RE_Mesh::SetupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	RE_GLCache::ChangeVAO(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	int strideSize = 3;
	uint meshSize = 3;
	if (normals)
	{
		meshSize += 3;
		strideSize += 3;
	}
	if (tangents)
	{
		meshSize += 3;
		strideSize += 3;
	}
	if (bitangents)
	{
		meshSize +=  3;
		strideSize += 3;
	}
	if (texturecoords)
	{
		meshSize += 2;
		strideSize += 2;
	}

	strideSize *= sizeof(float);
	meshSize *= vertex_count;

	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;

	for (uint i = 0; i < vertex_count; i++)
	{
		uint indexArray = i * 3;
		uint indexTexCoord = i * 2;

		size_t size = 3;
		memcpy(cursor, &vertex[indexArray], size * sizeof(float));
		cursor += size;

		if (normals)
		{
			memcpy(cursor, &normals[indexArray], size * sizeof(float));
			cursor += size;
		}
		if (tangents)
		{
			memcpy(cursor, &tangents[indexArray], size * sizeof(float));
			cursor += size;
		}
		if (bitangents)
		{
			memcpy(cursor, &bitangents[indexArray], size * sizeof(float));
			cursor += size;
		}
		if (texturecoords)
		{
			size = 2;
			memcpy(cursor, &texturecoords[indexTexCoord], size * sizeof(float));
			cursor += size;
		}
	}

	glBufferData(GL_ARRAY_BUFFER, meshSize *  sizeof(float), &meshBuffer[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_count * 3 * sizeof(unsigned int), &index[0], GL_STATIC_DRAW);

	// vertex positions
	int accumulativeOffset = 0;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, strideSize, reinterpret_cast<void*>(accumulativeOffset));
	accumulativeOffset += sizeof(float) * 3;

	if (normals)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, strideSize, reinterpret_cast<void*>(accumulativeOffset));
		accumulativeOffset += sizeof(float) * 3;
	}
	if (tangents)
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, strideSize, reinterpret_cast<void*>(accumulativeOffset));
		accumulativeOffset += sizeof(float) * 3;
	}
	if (bitangents)
	{
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, strideSize, reinterpret_cast<void*>(accumulativeOffset));
		accumulativeOffset += sizeof(float) * 3;
	}
	if (texturecoords)
	{
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, strideSize, reinterpret_cast<void*>(accumulativeOffset));
	}
	RE_GLCache::ChangeVAO(0);
}

math::AABB RE_Mesh::GetAABB() const { return bounding_box; }

void RE_Mesh::loadVertexNormals()
{
	if (normals)
	{
		LoadVertex();

		vertexNormals = new float[2 * 3 * triangle_count];
		float* cursor = vertexNormals;

		float line_length = 1.5f;
		for (unsigned int i = 0; i < triangle_count; i++)
		{
			uint vertexIndex = index[i] * 3;

			size_t size = 3;
			memcpy(cursor, &vertex[vertexIndex], size * sizeof(float));
			cursor += size;

			math::vec norm(&normals[vertexIndex]);
			math::vec vert(&vertex[vertexIndex]);
			vert = { vert.x + (norm.x * line_length),
				vert.y + (norm.y * line_length),
				vert.z + (norm.z * line_length) };

			memcpy(cursor, &vert, size * sizeof(float));
			cursor += size;
		}

		glGenVertexArrays(1, &VAO_VertexNormals);
		glGenBuffers(1, &VBO_VertexNormals);

		RE_GLCache::ChangeVAO(VAO_VertexNormals);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_VertexNormals);

		glBufferData(GL_ARRAY_BUFFER, 2 * 3 * triangle_count * sizeof(float), vertexNormals, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));

		RE_GLCache::ChangeVAO(0);

		lVertexNormals = true;
	}
}

void RE_Mesh::loadFaceNormals()
{
	float* fNCursor = (faceNormals = new float[2 * 3 * triangle_count]);
	float* fCcursor = (faceCenters = new float[3 * triangle_count]);

	float line_length = 1.5f;
	uint triangleA = 0, triangleB = 0, triangleC = 0;
	math::vec pos = math::vec::zero, v = math::vec::zero, w = math::vec::zero, normal = math::vec::zero;
	eastl::vector<math::vec> lines, face_centers;
	for (unsigned int i = 0; i < triangle_count; i += 3)
	{
		uint triangleA = index[i    ] * 3;
		uint triangleB = index[i + 1] * 3;
		uint triangleC = index[i + 2] * 3;

		pos = (math::vec(&vertex[triangleA]) + math::vec(&vertex[triangleB]) + math::vec(&vertex[triangleC]) / 3);
		v = math::vec(&vertex[triangleB]) - math::vec(&vertex[triangleA]);
		w = math::vec(&vertex[triangleC]) - math::vec(&vertex[triangleA]);
		normal = v.Cross(w).Normalized() * line_length;

		size_t size = 3;
		memcpy(fNCursor, &pos, size * sizeof(float));
		fNCursor += size;

		memcpy(fNCursor, &math::vec(pos.x + normal.x, pos.y + normal.y, pos.z + normal.z), size * sizeof(float));
		fNCursor += size;

		memcpy(fCcursor, &pos, size * sizeof(float));
		fCcursor += size;
	}

	glGenVertexArrays(1, &VAO_FaceNormals);
	glGenBuffers(1, &VBO_FaceNormals);

	RE_GLCache::ChangeVAO(VAO_FaceNormals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_FaceNormals);

	glBufferData(GL_ARRAY_BUFFER, 2 * 3 * triangle_count * sizeof(float), faceNormals, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));

	glGenVertexArrays(1, &VAO_FaceCenters);
	glGenBuffers(1, &VBO_FaceCenters);

	RE_GLCache::ChangeVAO(0);

	RE_GLCache::ChangeVAO(VAO_FaceCenters);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_FaceCenters);

	glBufferData(GL_ARRAY_BUFFER, 3 * triangle_count * sizeof(float), faceCenters, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));

	RE_GLCache::ChangeVAO(0);

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

void RE_Mesh::SetVerticesAndIndex(float* v, unsigned int* i,unsigned int vertexCount, unsigned int triangleCount, float* tC, float* n, float* t, float* bT)
{
	vertex = v; index = i; vertex_count = vertexCount; triangle_count = triangleCount; texturecoords = tC; normals = n; tangents = t; bitangents = bT;
}

bool RE_Mesh::CheckFaceCollision(const math::Ray& local_ray, float& distance) const
{
	bool ret = false;
	float res_dist;
	for (unsigned int i = 0; i < triangle_count; i++)
	{
		math::Triangle face;
		face.a = math::vec(&vertex[3 * index[3 * i]]);
		face.b = math::vec(&vertex[3 * index[(3 * i) + 1]]);
		face.c = math::vec(&vertex[3 * index[(3 * i) + 2]]);

		if (face.Intersects(local_ray, &res_dist) && (!ret || res_dist < distance))
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

	RE_GLCache::ChangeVAO(VAO_Vertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Vertex);

	glBufferData(GL_ARRAY_BUFFER, triangle_count * 3 * sizeof(float), vertex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));

	RE_GLCache::ChangeVAO(0);
}

void RE_Mesh::ClearVertex()
{
	glDeleteVertexArrays(1, &VAO_Vertex);
	glDeleteBuffers(1, &VBO_Vertex);
	VAO_Vertex = VBO_Vertex = 0;
}

void RE_Mesh::LibraryLoad()
{
	RE_FileBuffer toLoad(GetLibraryPath());

	if (toLoad.Load())
	{
		char* cursor = toLoad.GetBuffer();

		size_t cSize = sizeof(uint);
		memcpy(&triangle_count, cursor, cSize);
		cursor += cSize;

		memcpy(&vertex_count, cursor, cSize);
		cursor += cSize;

		vertex = new float[vertex_count * 3];
		cSize = sizeof(float) * 3 * vertex_count;
		memcpy(vertex, cursor, cSize);
		cursor += cSize;

		bool toFill = false;
		cSize = sizeof(bool);
		memcpy(&toFill, cursor, cSize);
		cursor += cSize;
		if (toFill)
		{
			normals = new float[vertex_count * 3];
			cSize = sizeof(float) * 3 * vertex_count;
			memcpy(normals, cursor, cSize);
			cursor += cSize;
		}

		toFill = false;
		cSize = sizeof(bool);
		memcpy(&toFill, cursor, cSize);
		cursor += cSize;
		if (toFill)
		{
			tangents = new float[vertex_count * 3];
			cSize = sizeof(float) * 3 * vertex_count;
			memcpy(tangents, cursor, cSize);
			cursor += cSize;
		}

		toFill = false;
		cSize = sizeof(bool);
		memcpy(&toFill, cursor, cSize);
		cursor += cSize;
		if (toFill)
		{
			bitangents = new float[vertex_count * 3];
			size_t cSize = sizeof(float) * 3 * vertex_count;
			memcpy(bitangents, cursor, cSize);
			cursor += cSize;
		}

		toFill = false;
		cSize = sizeof(bool);
		memcpy( &toFill, cursor, cSize);
		cursor += cSize;
		if (toFill)
		{
			texturecoords = new float[vertex_count * 2];
			size_t cSize = sizeof(float) * 2 * vertex_count;
			memcpy( texturecoords, cursor, cSize);
			cursor += cSize;
		}

		toFill = false;
		cSize = sizeof(bool);
		memcpy(&toFill, cursor, cSize);
		cursor += cSize;
		if (toFill)
		{
			index = new uint[triangle_count * 3];
			cSize = sizeof(uint) * 3 * triangle_count;
			memcpy( index, cursor, cSize);
			cursor += cSize;
		}
	}
	ResourceContainer::inMemory = true;
}
