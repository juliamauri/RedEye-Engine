#include "RE_ParticleRender.h"

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
	primType = C_EMPTY;
	if (primType != C_EMPTY)
	{
		primCmp->UnUseResources();
		DEL(primCmp);
	}
	if (meshMD5) RE_RES->UnUse(meshMD5);
	scale = { 0.5f,0.5f,0.1f };
	particleDir = RE_ParticleEmitter::Particle_Dir::PS_Billboard;
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

	if (primType != C_EMPTY)
	{
		switch (primType)
		{
		case C_CUBE: to_fill->primCmp = new RE_CompCube(); break;
		case C_POINT: to_fill->primCmp = new RE_CompPoint(); break;
		case C_DODECAHEDRON: to_fill->primCmp = new RE_CompDodecahedron(); break;
		case C_TETRAHEDRON: to_fill->primCmp = new RE_CompTetrahedron(); break;
		case C_OCTOHEDRON: to_fill->primCmp = new RE_CompOctohedron(); break;
		case C_ICOSAHEDRON: to_fill->primCmp = new RE_CompIcosahedron(); break;
		case C_PLANE: to_fill->primCmp = new RE_CompPlane(); break;
		case C_SPHERE: to_fill->primCmp = new RE_CompSphere(); break;
		case C_CYLINDER: to_fill->primCmp = new RE_CompCylinder(); break;
		case C_HEMISHPERE: to_fill->primCmp = new RE_CompHemiSphere(); break;
		case C_TORUS: to_fill->primCmp = new RE_CompTorus(); break;
		case C_TREFOILKNOT: to_fill->primCmp = new RE_CompTrefoiKnot(); break;
		case C_ROCK: to_fill->primCmp = new RE_CompRock(); break;
		}

		if (primType >= C_CUBE && primType <= C_ICOSAHEDRON)
			RE_SCENE->primitives->SetUpComponentPrimitive(to_fill->primCmp);
		else {

			uint p_size = primCmp->GetParticleBinarySize();

			char* prim_buffer = new char[p_size];
			char* cursor = prim_buffer;
			primCmp->SerializeParticleBinary(cursor);
			to_fill->primCmp->DeserializeParticleBinary(cursor);

			DEL_A(prim_buffer);
		}
	}
	else to_fill->primCmp = nullptr;

	to_fill->scale = scale;
	to_fill->particleDir = particleDir;
	to_fill->direction = direction;
}

void RE_ParticleRender::FillResouce(RE_ParticleEmitter* from)
{
	color = from->color;
	opacity = from->opacity;
	light = from->light;
	meshMD5 = from->meshMD5;
	if (from->primCmp) {
		if (primCmp) DEL(primCmp);

		primType = from->primCmp->GetType();
		primCmp = from->primCmp;
	}
	else
	{
		primType = C_EMPTY;
		primCmp = nullptr;
	}
	scale = from->scale;
	particleDir = from->particleDir;
	direction = from->direction;
}

void RE_ParticleRender::SaveResourceMeta(RE_Json* metaNode)
{
	metaNode->PushBool("isMesh", (meshMD5));
	if (meshMD5)
		metaNode->PushString("meshPath", RE_RES->At(meshMD5)->GetLibraryPath());
}

void RE_ParticleRender::LoadResourceMeta(RE_Json* metaNode)
{
	bool isMesh = metaNode->PullBool("isMesh", false);
	if (isMesh) {
		eastl::string libraryMesh = metaNode->PullString("meshPath", "");
		meshMD5 = RE_RES->CheckOrFindMeshOnLibrary(libraryMesh.c_str());
	}
}

void RE_ParticleRender::JsonDeserialize(bool generateLibraryPath)
{
	Config render(GetAssetPath(), RE_FS->GetZipPath());
	if (render.Load())
	{
		math::float2 tmp;
		RE_Json* node = render.GetRootNode("Render");

		color.JsonDeserialize(node->PullJObject("Color"));
		opacity.JsonDeserialize(node->PullJObject("Opacity"));
		light.JsonDeserialize(node->PullJObject("Light"));

		scale = node->PullFloatVector("Scale", { 0.5f,0.5f,0.1f });
		particleDir = static_cast<RE_ParticleEmitter::Particle_Dir>(node->PullInt("particleDir", 0));
		direction = node->PullFloatVector("Direction", { -1.0f,1.0f,0.5f });

		primType = static_cast<ComponentType>(node->PullInt("primitiveType", static_cast<int>(C_EMPTY)));
		if (primType != C_EMPTY) {
			switch (primType)
			{
			case C_CUBE: primCmp = new RE_CompCube(); break;
			case C_POINT: primCmp = new RE_CompPoint(); break;
			case C_DODECAHEDRON: primCmp = new RE_CompDodecahedron(); break;
			case C_TETRAHEDRON: primCmp = new RE_CompTetrahedron(); break;
			case C_OCTOHEDRON: primCmp = new RE_CompOctohedron(); break;
			case C_ICOSAHEDRON: primCmp = new RE_CompIcosahedron(); break;
			case C_PLANE: primCmp = new RE_CompPlane(); break;
			case C_SPHERE: primCmp = new RE_CompSphere(); break;
			case C_CYLINDER: primCmp = new RE_CompCylinder(); break;
			case C_HEMISHPERE: primCmp = new RE_CompHemiSphere(); break;
			case C_TORUS: primCmp = new RE_CompTorus(); break;
			case C_TREFOILKNOT: primCmp = new RE_CompTrefoiKnot(); break;
			case C_ROCK: primCmp = new RE_CompRock(); break;
			}

			if (primType >= C_CUBE && primType <= C_ICOSAHEDRON)
				RE_SCENE->primitives->SetUpComponentPrimitive(primCmp);
			else primCmp->SerializeParticleJson(node->PullJObject("primitive"));
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
}

void RE_ParticleRender::JsonSerialize(bool onlyMD5)
{
	Config render(GetAssetPath(), RE_FS->GetZipPath());
	RE_Json* node = render.GetRootNode("Render");
	
	color.JsonSerialize(node->PushJObject("Color"));
	opacity.JsonSerialize(node->PushJObject("Opacity"));
	light.JsonSerialize(node->PushJObject("Light"));

	node->PushFloatVector("Scale", scale);
	node->PushInt("particleDir", static_cast<int>(particleDir));
	node->PushFloatVector("Direction", direction);

	node->PushInt("primitiveType", static_cast<int>(primType));
	if (primType != C_EMPTY) primCmp->SerializeParticleJson(node->PushJObject("primitive"));

	if (!onlyMD5) render.Save();
	SetMD5(render.GetMd5().c_str());

	eastl::string libraryPath("Library/Particles/");
	libraryPath += GetMD5();
	SetLibraryPath(libraryPath.c_str());
}

void RE_ParticleRender::BinaryDeserialize()
{
	RE_FileBuffer libraryFile(GetLibraryPath());
	if (libraryFile.Load())
	{
		char* cursor = libraryFile.GetBuffer();

		color.BinaryDeserialize(cursor);
		opacity.BinaryDeserialize(cursor);
		light.BinaryDeserialize(cursor);

		size_t size = sizeof(float) * 3u;
		memcpy(scale.ptr(), cursor, size);
		cursor += size;

		size = sizeof(RE_ParticleEmitter::Particle_Dir);
		memcpy(&particleDir, cursor, size);
		cursor += size;

		size = sizeof(float) * 3u;
		memcpy(direction.ptr(), cursor, size);
		cursor += size;

		size = sizeof(ComponentType);
		memcpy(&primType, cursor, size);
		cursor += size;

		if (primType != C_EMPTY) {
			switch (primType)
			{
			case C_CUBE: primCmp = new RE_CompCube(); break;
			case C_POINT: primCmp = new RE_CompPoint(); break;
			case C_DODECAHEDRON: primCmp = new RE_CompDodecahedron(); break;
			case C_TETRAHEDRON: primCmp = new RE_CompTetrahedron(); break;
			case C_OCTOHEDRON: primCmp = new RE_CompOctohedron(); break;
			case C_ICOSAHEDRON: primCmp = new RE_CompIcosahedron(); break;
			case C_PLANE: primCmp = new RE_CompPlane(); break;
			case C_SPHERE: primCmp = new RE_CompSphere(); break;
			case C_CYLINDER: primCmp = new RE_CompCylinder(); break;
			case C_HEMISHPERE: primCmp = new RE_CompHemiSphere(); break;
			case C_TORUS: primCmp = new RE_CompTorus(); break;
			case C_TREFOILKNOT: primCmp = new RE_CompTrefoiKnot(); break;
			case C_ROCK: primCmp = new RE_CompRock(); break;
			}

			primCmp->DeserializeParticleBinary(cursor);

			if(primType >= C_CUBE && primType <= C_ICOSAHEDRON)
				RE_SCENE->primitives->SetUpComponentPrimitive(primCmp);
		}
	}
}

void RE_ParticleRender::BinarySerialize()
{
	RE_FileBuffer libraryFile(GetLibraryPath(), RE_FS->GetZipPath());

	uint bufferSize = GetBinarySize() + 1;

	char* buffer = new char[bufferSize];
	char* cursor = buffer;

	color.BinarySerialize(cursor);
	opacity.BinarySerialize(cursor);
	light.BinarySerialize(cursor);

	size_t size = sizeof(float) * 3u;
	memcpy(cursor, scale.ptr(), size);
	cursor += size;

	size = sizeof(RE_ParticleEmitter::Particle_Dir);
	memcpy(cursor, &particleDir, size);
	cursor += size;

	size = sizeof(float) * 3u;
	memcpy(cursor, direction.ptr(), size);
	cursor += size;

	size = sizeof(ComponentType);
	memcpy(cursor, &primType, size);
	cursor += size;

	if (primType != C_EMPTY) primCmp->SerializeParticleBinary(cursor);

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	libraryFile.Save(buffer, bufferSize);
	DEL_A(buffer);
}

unsigned int RE_ParticleRender::GetBinarySize() const
{
	return sizeof(RE_ParticleEmitter::Particle_Dir)
		+ (sizeof(float) * 6u)
		+ sizeof(ComponentType)
		+ ((primType != C_EMPTY) ? primCmp->GetParticleBinarySize() : 0)
		+ color.GetBinarySize() 
		+ opacity.GetBinarySize() 
		+ light.GetBinarySize();
}
