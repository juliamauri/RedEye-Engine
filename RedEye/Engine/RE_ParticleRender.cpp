#include "Resource.h"
#include "RE_Component.h"

#include "RE_ParticleRender.h"

#include "RE_Memory.h"
#include "Application.h"
#include "ModuleScene.h"
#include "RE_ResourceManager.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"

#include "RE_PrimitiveManager.h"
#include "RE_CompPrimitive.h"

void RE_ParticleRender::LoadInMemory()
{
	if (RE_FS->Exists(GetLibraryPath())) BinaryDeserialize();
	else if (RE_FS->Exists(GetAssetPath())) { JsonDeserialize(); BinarySerialize(); }
	else if (isInternal()) ResourceContainer::inMemory = true;
	else RE_LOG_ERROR("Particle Render %s not found on project", GetName());

	if (ResourceContainer::inMemory && meshMD5) RE_RES->Use(meshMD5);
}

void RE_ParticleRender::UnloadMemory()
{
	color = {};
	opacity = {};
	light = {};

	if (primType != RE_Component::Type::EMPTY)
	{
		primCmp->UnUseResources();
		DEL(primCmp)
	}
	primType = RE_Component::Type::EMPTY;

	if (meshMD5) RE_RES->UnUse(meshMD5);
	scale = { 0.5f,0.5f,0.1f };
	particleDir = RE_ParticleEmitter::ParticleDir::Billboard;
	direction = { -1.0f,1.0f,0.5f };

	ResourceContainer::inMemory = false;
}

void RE_ParticleRender::Import(bool keepInMemory)
{
	JsonDeserialize(true);
	BinarySerialize();
	if (!keepInMemory) UnloadMemory();
}

void RE_ParticleRender::Save()
{
	JsonSerialize();
	BinarySerialize();
	SaveMeta();
}

void RE_ParticleRender::ProcessMD5()
{
	JsonSerialize(true);
}

void RE_ParticleRender::FillEmitter(RE_ParticleEmitter* to_fill)
{
	to_fill->color = color;
	to_fill->opacity = opacity;
	to_fill->light = light;
	to_fill->meshMD5 = meshMD5;

	if (primType != RE_Component::Type::EMPTY)
	{
		switch (primType)
		{
		case RE_Component::Type::CUBE: to_fill->primCmp = new RE_CompCube(); break;
		case RE_Component::Type::POINT: to_fill->primCmp = new RE_CompPoint(); break;
		case RE_Component::Type::DODECAHEDRON: to_fill->primCmp = new RE_CompDodecahedron(); break;
		case RE_Component::Type::TETRAHEDRON: to_fill->primCmp = new RE_CompTetrahedron(); break;
		case RE_Component::Type::OCTOHEDRON: to_fill->primCmp = new RE_CompOctohedron(); break;
		case RE_Component::Type::ICOSAHEDRON: to_fill->primCmp = new RE_CompIcosahedron(); break;
		case RE_Component::Type::PLANE: to_fill->primCmp = new RE_CompPlane(); break;
		case RE_Component::Type::SPHERE: to_fill->primCmp = new RE_CompSphere(); break;
		case RE_Component::Type::CYLINDER: to_fill->primCmp = new RE_CompCylinder(); break;
		case RE_Component::Type::HEMISHPERE: to_fill->primCmp = new RE_CompHemiSphere(); break;
		case RE_Component::Type::TORUS: to_fill->primCmp = new RE_CompTorus(); break;
		case RE_Component::Type::TREFOILKNOT: to_fill->primCmp = new RE_CompTrefoiKnot(); break;
		case RE_Component::Type::ROCK: to_fill->primCmp = new RE_CompRock(); break;
		}

		if (primType >= RE_Component::Type::CUBE && primType <= RE_Component::Type::ICOSAHEDRON)
			RE_SCENE->primitives->SetUpComponentPrimitive(to_fill->primCmp);
		else
		{
			char* prim_buffer = new char[primCmp->GetParticleBinarySize()];
			char* cursor = prim_buffer;
			primCmp->ParticleBinarySerialize(cursor);
			cursor = prim_buffer;
			to_fill->primCmp->ParticleBinaryDeserialize(cursor);
			DEL_A(prim_buffer);
		}
	}
	else to_fill->primCmp = nullptr;

	to_fill->scale = scale;
	to_fill->orientation = particleDir;
	to_fill->direction = direction;
}

void RE_ParticleRender::FillResouce(RE_ParticleEmitter* from)
{
	color = from->color;
	opacity = from->opacity;
	light = from->light;
	meshMD5 = from->meshMD5;
	if (from->primCmp)
	{
		DEL(primCmp)
		primType = from->primCmp->GetType();
		primCmp = from->primCmp;
	}
	else
	{
		primType = RE_Component::Type::EMPTY;
		primCmp = nullptr;
	}
	scale = from->scale;
	particleDir = from->orientation;
	direction = from->direction;
}

void RE_ParticleRender::SaveResourceMeta(RE_Json* metaNode) const
{
	metaNode->Push("isMesh", (meshMD5 != nullptr));
	if (meshMD5) metaNode->Push("meshPath", RE_RES->At(meshMD5)->GetLibraryPath());
}

void RE_ParticleRender::LoadResourceMeta(RE_Json* metaNode)
{
	if (metaNode->PullBool("isMesh", false))
		meshMD5 = RE_RES->CheckOrFindMeshOnLibrary(metaNode->PullString("meshPath", ""));
}

void RE_ParticleRender::JsonDeserialize(bool generateLibraryPath)
{
	Config render(GetAssetPath());
	if (!render.Load()) return;

	math::float2 tmp;
	RE_Json* node = render.GetRootNode("Render");

	color.JsonDeserialize(node->PullJObject("Color"));
	opacity.JsonDeserialize(node->PullJObject("Opacity"));
	light.JsonDeserialize(node->PullJObject("Light"));

	scale = node->PullFloatVector("Scale", { 0.5f,0.5f,0.1f });
	particleDir = static_cast<RE_ParticleEmitter::ParticleDir>(node->PullInt("particleDir", 0));
	direction = node->PullFloatVector("Direction", { -1.0f,1.0f,0.5f });

	primType = static_cast<RE_Component::Type>(node->PullUInt("primitiveType", static_cast<uint>(RE_Component::Type(0))));
	if (primType != RE_Component::Type::EMPTY)
	{
		switch (primType)
		{
		case RE_Component::Type::CUBE: primCmp = new RE_CompCube(); break;
		case RE_Component::Type::POINT: primCmp = new RE_CompPoint(); break;
		case RE_Component::Type::DODECAHEDRON: primCmp = new RE_CompDodecahedron(); break;
		case RE_Component::Type::TETRAHEDRON: primCmp = new RE_CompTetrahedron(); break;
		case RE_Component::Type::OCTOHEDRON: primCmp = new RE_CompOctohedron(); break;
		case RE_Component::Type::ICOSAHEDRON: primCmp = new RE_CompIcosahedron(); break;
		case RE_Component::Type::PLANE: primCmp = new RE_CompPlane(); break;
		case RE_Component::Type::SPHERE: primCmp = new RE_CompSphere(); break;
		case RE_Component::Type::CYLINDER: primCmp = new RE_CompCylinder(); break;
		case RE_Component::Type::HEMISHPERE: primCmp = new RE_CompHemiSphere(); break;
		case RE_Component::Type::TORUS: primCmp = new RE_CompTorus(); break;
		case RE_Component::Type::TREFOILKNOT: primCmp = new RE_CompTrefoiKnot(); break;
		case RE_Component::Type::ROCK: primCmp = new RE_CompRock(); break;
		}

		if (primType >= RE_Component::Type::CUBE && primType <= RE_Component::Type::ICOSAHEDRON)
			RE_SCENE->primitives->SetUpComponentPrimitive(primCmp);
		else primCmp->ParticleJsonSerialize(node->PullJObject("primitive"));
	}

	if (generateLibraryPath)
	{
		SetMD5(render.GetMd5().c_str());
		eastl::string libraryPath("Library/Particles/");
		libraryPath += GetMD5();
		SetLibraryPath(libraryPath.c_str());
	}

	ResourceContainer::inMemory = true;
}

void RE_ParticleRender::JsonSerialize(bool onlyMD5)
{
	Config render(GetAssetPath());
	RE_Json* node = render.GetRootNode("Render");
	
	color.JsonSerialize(node->PushJObject("Color"));
	opacity.JsonSerialize(node->PushJObject("Opacity"));
	light.JsonSerialize(node->PushJObject("Light"));

	node->PushFloatVector("Scale", scale);
	node->Push("particleDir", static_cast<int>(particleDir));
	node->PushFloatVector("Direction", direction);

	node->Push("primitiveType", static_cast<int>(primType));
	if (primType != RE_Component::Type::EMPTY)
		primCmp->ParticleJsonSerialize(node->PushJObject("primitive"));

	if (!onlyMD5) render.Save();
	SetMD5(render.GetMd5().c_str());

	eastl::string libraryPath("Library/Particles/");
	libraryPath += GetMD5();
	SetLibraryPath(libraryPath.c_str());
}

void RE_ParticleRender::BinaryDeserialize()
{
	RE_FileBuffer libraryFile(GetLibraryPath());
	if (!libraryFile.Load()) return;

	char* cursor = libraryFile.GetBuffer();
	color.BinaryDeserialize(cursor);
	opacity.BinaryDeserialize(cursor);
	light.BinaryDeserialize(cursor);

	size_t size = sizeof(float) * 3u;
	memcpy(scale.ptr(), cursor, size);
	cursor += size;

	size = sizeof(RE_ParticleEmitter::ParticleDir);
	memcpy(&particleDir, cursor, size);
	cursor += size;

	size = sizeof(float) * 3u;
	memcpy(direction.ptr(), cursor, size);
	cursor += size;

	size = sizeof(RE_Component::Type);
	memcpy(&primType, cursor, size);
	cursor += size;

	if (primType != RE_Component::Type::EMPTY)
	{
		switch (primType)
		{
		case RE_Component::Type::CUBE: primCmp = new RE_CompCube(); break;
		case RE_Component::Type::POINT: primCmp = new RE_CompPoint(); break;
		case RE_Component::Type::DODECAHEDRON: primCmp = new RE_CompDodecahedron(); break;
		case RE_Component::Type::TETRAHEDRON: primCmp = new RE_CompTetrahedron(); break;
		case RE_Component::Type::OCTOHEDRON: primCmp = new RE_CompOctohedron(); break;
		case RE_Component::Type::ICOSAHEDRON: primCmp = new RE_CompIcosahedron(); break;
		case RE_Component::Type::PLANE: primCmp = new RE_CompPlane(); break;
		case RE_Component::Type::SPHERE: primCmp = new RE_CompSphere(); break;
		case RE_Component::Type::CYLINDER: primCmp = new RE_CompCylinder(); break;
		case RE_Component::Type::HEMISHPERE: primCmp = new RE_CompHemiSphere(); break;
		case RE_Component::Type::TORUS: primCmp = new RE_CompTorus(); break;
		case RE_Component::Type::TREFOILKNOT: primCmp = new RE_CompTrefoiKnot(); break;
		case RE_Component::Type::ROCK: primCmp = new RE_CompRock(); break;
		}

		primCmp->ParticleBinaryDeserialize(cursor);

		if (primType >= RE_Component::Type::CUBE && primType <= RE_Component::Type::ICOSAHEDRON)
			RE_SCENE->primitives->SetUpComponentPrimitive(primCmp);
	}

	ResourceContainer::inMemory = true;
}

void RE_ParticleRender::BinarySerialize() const
{
	RE_FileBuffer libraryFile(GetLibraryPath());

	auto bufferSize = GetBinarySize() + 1;

	char* buffer = new char[bufferSize];
	char* cursor = buffer;

	color.BinarySerialize(cursor);
	opacity.BinarySerialize(cursor);
	light.BinarySerialize(cursor);

	size_t size = sizeof(float) * 3u;
	memcpy(cursor, scale.ptr(), size);
	cursor += size;

	size = sizeof(RE_ParticleEmitter::ParticleDir);
	memcpy(cursor, &particleDir, size);
	cursor += size;

	size = sizeof(float) * 3u;
	memcpy(cursor, direction.ptr(), size);
	cursor += size;

	size = sizeof(RE_Component::Type);
	memcpy(cursor, &primType, size);
	cursor += size;

	if (primType != RE_Component::Type::EMPTY) primCmp->ParticleBinarySerialize(cursor);

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	libraryFile.Save(buffer, bufferSize);
	DEL_A(buffer);
}

size_t RE_ParticleRender::GetBinarySize() const
{
	return sizeof(RE_ParticleEmitter::ParticleDir)
		+ (sizeof(float) * 6)
		+ sizeof(RE_Component::Type)
		+ ((primType != RE_Component::Type::EMPTY) ? primCmp->GetParticleBinarySize() : 0)
		+ color.GetBinarySize() 
		+ opacity.GetBinarySize() 
		+ light.GetBinarySize();
}
