#include "RE_ParticleRender.h"

#include "Application.h"
#include "RE_ResourceManager.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"


void RE_ParticleRender::LoadInMemory()
{
	if (RE_FS->Exists(GetLibraryPath())) BinaryDeserialize();
	else if (RE_FS->Exists(GetAssetPath())) { JsonDeserialize(); BinarySerialize(); }
	else if (isInternal()) ResourceContainer::inMemory = true;
	else RE_LOG_ERROR("Particle Render %s not found on project", GetName());
}

void RE_ParticleRender::UnloadMemory()
{
	color = {};
	opacity = {};
	light = {};
	primCmp = C_POINT;
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
		+ color.GetBinarySize() 
		+ opacity.GetBinarySize() 
		+ light.GetBinarySize();
}
