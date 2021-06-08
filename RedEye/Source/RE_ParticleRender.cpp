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
	emitlight = false;
	lightColor = math::vec::one;
	randomLightColor = false;
	specular = 0.2f;
	sClamp[1] = 0.f;
	sClamp[2] = 1.f;
	randomSpecular = false;
	particleLColor = false;
	iClamp[1] = 0.0f;
	iClamp[2] = 50.0f;
	intensity = 1.0f;
	randomIntensity = false;
	constant = 1.0f;
    linear = 0.091f;
	quadratic = 0.011f;
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

		emitlight = node->PullBool("emitlight", false);
		lightColor = node->PullFloatVector("lightColor", math::vec::one);
		randomLightColor = node->PullBool("randomLightColor", false);
		specular = node->PullFloat("Specular", 0.2f);
		tmp = node->PullFloat("SpecularClamp", { 0.f, 1.f });
		memcpy(sClamp, tmp.ptr(), sizeof(float) * 2);
		randomSpecular = node->PullBool("randomSpecular", false);
		particleLColor = node->PullBool("particleLightColor", false);
		tmp = node->PullFloat("IntensityClamp", { 0.0f, 50.0f });
		memcpy(iClamp, tmp.ptr(), sizeof(float) * 2);
		intensity = node->PullFloat("Intensity", 1.0f);
		randomIntensity = node->PullBool("randomIntensity", false);
		constant = node->PullFloat("Constant", 1.0f);
		linear = node->PullFloat("Linear", 0.091f);
		quadratic = node->PullFloat("Quadratic", 0.011f);
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

	node->PushBool("emitlight", emitlight);
	node->PushFloatVector("lightColor", lightColor);
	node->PushBool("randomLightColor", randomLightColor);
	node->PushFloat("Specular", specular);
	node->PushFloat("SpecularClamp", math::float2(sClamp));
	node->PushBool("randomSpecular", randomSpecular);
	node->PushBool("particleLightColor", particleLColor);
	node->PushFloat("IntensityClamp", math::float2(iClamp));
	node->PushFloat("Intensity", intensity);
	node->PushBool("randomIntensity", randomIntensity);
	node->PushFloat("Constant", constant);
	node->PushFloat("Linear", linear);
	node->PushFloat("Quadratic", quadratic);
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

		size_t size = sizeof(bool);
		memcpy(&emitlight, cursor, size);
		cursor += size;

		size = sizeof(float) * 3;
		memcpy(lightColor.ptr(), cursor, size);
		cursor += size;

		size = sizeof(bool);
		memcpy(&randomLightColor, cursor, size);
		cursor += size;

		size = sizeof(float);
		memcpy(&specular, cursor, size);
		cursor += size;

		size = sizeof(float) * 2;
		memcpy(&sClamp, cursor, size);
		cursor += size;

		size = sizeof(bool);
		memcpy(&randomSpecular, cursor, size);
		cursor += size;

		memcpy(&particleLColor, cursor, size);
		cursor += size;

		size = sizeof(float) * 2;
		memcpy(&iClamp, cursor, size);
		cursor += size;

		size = sizeof(float);
		memcpy(&intensity, cursor, size);
		cursor += size;

		size = sizeof(bool);
		memcpy(&randomIntensity, cursor, size);
		cursor += size;

		size = sizeof(float);
		memcpy(&constant, cursor, size);
		cursor += size;

		memcpy(&linear, cursor, size);
		cursor += size;

		memcpy(&quadratic, cursor, size);
		cursor += size;

		size = sizeof(float) * 3;
		memcpy(scale.ptr(), cursor, size);
		cursor += size;

		size = sizeof(RE_ParticleEmitter::Particle_Dir);
		memcpy(&particleDir, cursor, size);
		cursor += size;

		size = sizeof(float) * 3;
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

	size_t size = sizeof(bool);
	memcpy(cursor, &emitlight, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, lightColor.ptr(), size);
	cursor += size;

	size = sizeof(bool);
	memcpy(cursor, &randomLightColor, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &specular, size);
	cursor += size;

	size = sizeof(float) * 2;
	memcpy(cursor, &sClamp, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(cursor, &randomSpecular, size);
	cursor += size;

	memcpy(cursor, &particleLColor, size);
	cursor += size;

	size = sizeof(float) * 2;
	memcpy(cursor, &iClamp, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &intensity, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(cursor, &randomIntensity, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &constant, size);
	cursor += size;

	memcpy(cursor, &linear, size);
	cursor += size;

	memcpy(cursor, &quadratic, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, scale.ptr(), size);
	cursor += size;

	size = sizeof(RE_ParticleEmitter::Particle_Dir);
	memcpy(cursor, &particleDir, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, direction.ptr(), size);
	cursor += size;

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	libraryFile.Save(buffer, bufferSize);
	DEL_A(buffer);
}

unsigned int RE_ParticleRender::GetBinarySize() const
{
	return color.GetBinarySize() + opacity.GetBinarySize() + sizeof(bool) * 5 + sizeof(RE_ParticleEmitter::Particle_Dir)
		+ sizeof(float) * 18 + sizeof(ComponentType);
}
