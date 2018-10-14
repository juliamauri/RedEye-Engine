#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "FileSystem.h"
#include "OutputLog.h"
#include "Texture2DManager.h"
#include "ShaderManager.h"
#include "RE_CompMesh.h"
#include "RE_Camera.h"
#include "RE_PrimitiveManager.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include <string>
#include <algorithm>

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Start()
{
	bool ret = true;

	//Loading Shaders
	ret = App->shaders->Load("texture", &modelloading);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	//Loading textures

	//Loading meshes
	/*
	Vertex vert;
	Texture tex;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> index;
	std::vector<Texture> textures;

	math::vec vPositionTriangle[] = {
		// positions       
		math::vec(1.0f, -1.0f, 0.0f),   // bottom right
		math::vec(-1.0f, -1.0f, 0.0f),   // bottom left
		math::vec(0.0f,  1.0f, 0.0f)    // top 
	};
	math::vec vColorsTriangle[] = {
		math::vec(1.0f, 0.0f, 0.0f),
		math::vec(0.0f, 1.0f, 0.0f),
		math::vec(0.0f, 0.0f, 1.0f)
	};

	for (unsigned int i = 0; i < 3; i++)
	{
		vert.Position = vPositionTriangle[i];
		vert.Normal = vColorsTriangle[i]; //using normal like vertex color
		vertices.push_back(vert);
		index.push_back(i);
	}

	triangle = new RE_Mesh(vertices, index, textures);

	vertices.clear();
	index.clear();
	textures.clear();
	vert.Position = math::float3::nan;
	vert.Normal = math::float3::nan;
	vert.TexCoords = math::float2::nan;
	tex.id = 0;
	tex.path.clear();
	tex.type.clear();

	math::vec vPositionSquare[] = {
		// positions          
		 math::vec(1.0f,  1.0f, 0.0f),  // top right
		 math::vec(1.0f, -1.0f, 0.0f),  // bottom right
		 math::vec(-1.0f, -1.0f, 0.0f), // bottom left
		 math::vec(-1.0f,  1.0f, 0.0f)  // top left 
	};

	math::vec vColorsSquare[] = {
		math::vec(1.0f, 0.0f, 0.0f),
		math::vec(0.0f, 1.0f, 0.0f),
		math::vec(0.0f, 0.0f, 1.0f),
		math::vec(1.0f, 1.0f, 0.0f)
	};

	math::float2 TextureCoordsSquare[] = {
		math::float2(1.0f, 1.0f),
		math::float2(1.0f, 0.0f),
		math::float2(0.0f, 0.0f),
		math::float2(0.0f, 1.0f)
	};

	unsigned int indicesSquare[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	for (unsigned int i = 0; i < 4; i++)
	{
		vert.Position = vPositionSquare[i];
		vert.Normal = vColorsSquare[i]; //using normal like vertex color
		vert.TexCoords = TextureCoordsSquare[i]; //using normal like vertex color
		vertices.push_back(vert);
	}
	for (unsigned int i = 0; i < 6; i++)
		index.push_back(indicesSquare[i]);

	square = new RE_Mesh(vertices, index, textures);

	vertices.clear();
	index.clear();
	textures.clear();
	vert.Position = math::float3::nan;
	vert.Normal = math::float3::nan;
	vert.TexCoords = math::float2::nan;
	tex.id = 0;
	tex.path.clear();
	tex.type.clear();

	//Cube without index
	math::vec vPositionCube[] = {
		math::vec(-1.0f, -1.0f, -1.0f),
		math::vec(1.0f, -1.0f, -1.0f),
		math::vec(1.0f,  1.0f, -1.0f),
		math::vec(1.0f,  1.0f, -1.0f),
		math::vec(-1.0f,  1.0f, -1.0f),
		math::vec(-1.0f, -1.0f, -1.0f),

		math::vec(-1.0f, -1.0f,  1.0f),
		math::vec(1.0f, -1.0f,  1.0f),
		math::vec(1.0f,  1.0f,  1.0f),
		math::vec(1.0f,  1.0f,  1.0f),
		math::vec(-1.0f,  1.0f,  1.0f),
		math::vec(-1.0f, -1.0f,  1.0f),

		math::vec(-1.0f,  1.0f,  1.0f),
		math::vec(-1.0f,  1.0f, -1.0f),
		math::vec(-1.0f, -1.0f, -1.0f),
		math::vec(-1.0f, -1.0f, -1.0f),
		math::vec(-1.0f, -1.0f,  1.0f),
		math::vec(-1.0f,  1.0f,  1.0f),

		math::vec(1.0f,  1.0f,  1.0f),
		math::vec(1.0f,  1.0f, -1.0f),
		math::vec(1.0f, -1.0f, -1.0f),
		math::vec(1.0f, -1.0f, -1.0f),
		math::vec(1.0f, -1.0f,  1.0f),
		math::vec(1.0f,  1.0f,  1.0f),

		math::vec(-1.0f, -1.0f, -1.0f),
		math::vec(1.0f, -1.0f, -1.0f),
		math::vec(1.0f, -1.0f,  1.0f),
		math::vec(1.0f, -1.0f,  1.0f),
		math::vec(-1.0f, -1.0f,  1.0f),
		math::vec(-1.0f, -1.0f, -1.0f),

		math::vec(-1.0f,  1.0f, -1.0f),
		math::vec(1.0f,  1.0f, -1.0f),
		math::vec(1.0f,  1.0f,  1.0f),
		math::vec(1.0f,  1.0f,  1.0f),
		math::vec(-1.0f,  1.0f,  1.0f),
		math::vec(-1.0f,  1.0f, -1.0f)
	};

	for (unsigned int i = 0; i < 36; i++)
	{
		vert.Position = vPositionCube[i];
		vertices.push_back(vert);
	}

	cube_array = new RE_Mesh(vertices, index, textures);

	vertices.clear();
	index.clear();
	textures.clear();
	vert.Position = math::float3::nan;
	vert.Normal = math::float3::nan;
	vert.TexCoords = math::float2::nan;
	tex.id = 0;
	tex.path.clear();
	tex.type.clear();

	//Cube with index
	math::vec vPositionCubeArray[] = {
		//vertecies        
		math::vec(1.0f,  1.0f, 1.0f),  //Top Right Back - Vert 0
		math::vec(1.0f, -1.0f, 1.0f),  //Bottom Right Back - Vert 1
		math::vec(-1.0f, -1.0f, 1.0f),  //Bottom Left Back - Vert 2
		math::vec(-1.0f,  1.0f, 1.0f),  //Top Left Back - Vert 3
		math::vec(1.0f,  1.0f, -1.0f),  //Top Right Front - Vert 4
		math::vec(1.0f, -1.0f, -1.0f),  //Bottom Right Front - Vert 5
		math::vec(-1.0f, -1.0f, -1.0f),  //Bottom Left Front - Vert 6
		math::vec(-1.0f,  1.0f, -1.0f), //Top Left Front - Vert 7
	};

	unsigned int indicesCube[] = {  //Tell OpenGL What triangle uses what Vertecies
		0, 1, 3,   //Back Quad
		1, 2, 3,
		0, 1, 4,     //Right Quad
		1, 5, 4,
		2, 3, 7,   //Left Quad
		2, 6, 7,
		4, 5, 7,   //Front Quad
		5, 6, 7,
		0, 3, 4,   //Top Quad
		3, 4, 7,
		1, 2, 5,   //Bottom Quad
		2, 5, 6
	};

	for (unsigned int i = 0; i < 8; i++)
	{
		vert.Position = vPositionCubeArray[i];
		vertices.push_back(vert);
	}

	for (unsigned int i = 0; i < 36; i++)
		index.push_back(indicesCube[i]);

	cube_index = new RE_Mesh(vertices, index, textures);

	vertices.clear();
	index.clear();
	textures.clear();
	vert.Position = math::float3::nan;
	vert.Normal = math::float3::nan;
	vert.TexCoords = math::float2::nan;
	tex.id = 0;
	tex.path.clear();
	tex.type.clear();


	//Testing primitives
	compcube = App->primitives->CreateCube(nullptr);
	comppoint = App->primitives->CreatePoint(nullptr, math::vec(2.0f,0.0f,0.0f));
	compline = App->primitives->CreateLine(nullptr, math::vec(-5.0f,0.0f,0.0f),math::vec(0.0f,2.0f,0.0f));
	comptriangle = App->primitives->CreateTriangle(nullptr);
	*/

	//Setting Camera
	//App->renderer3d->camera->SetPos(math::vec(0.0f, 0.0f, -10.0f));

	root = new RE_GameObject();
	//root->AddComponent(C_POINT);
	//root->AddComponent(C_CUBE);
	root->AddComponent(C_PLANE);
	//root->AddComponent(C_SPHERE);
	drop = new RE_GameObject();

	mesh_droped = nullptr;
	
	return ret;
}

update_status ModuleScene::PreUpdate()
{
	//Setting Shaders
	//ShaderManager::use(ShaderPrimitive);
	//ShaderManager::setFloat4x4(ShaderPrimitive, "view", App->renderer3d->camera->GetView().ptr());
	//ShaderManager::setFloat4x4(ShaderPrimitive, "projection", App->renderer3d->camera->GetProjection().ptr());
	//ShaderManager::setFloat(ShaderPrimitive, "objectColor", math::vec(1.0f, 0.0f, 0.0f));

	return UPDATE_CONTINUE;
}

update_status ModuleScene::Update()
{
	//math::vec t = root->transform->GetPosition();

	//root->transform->SetPos(t);

	root->Update();
	drop->Update();

	return UPDATE_CONTINUE;
}

update_status ModuleScene::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	if (mesh_droped)
		DEL(mesh_droped);
	if (root)
		DEL(root);
	if (drop)
		DEL(drop);

	return true;
}

void ModuleScene::FileDrop(const char * file)
{
	RE_FileIO* holder = App->fs->QuickBufferFromPDPath(file);

	std::string full_path(file);
	std::string file_name = full_path.substr(full_path.find_last_of("\\") + 1);
	std::string directory = full_path.substr(0, full_path.find_last_of('\\'));
	std::string ext = full_path.substr(full_path.find_last_of(".") + 1);

	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext.compare("fbx") == 0)
	{
		/*RE_CompMesh* c_mesh = (RE_CompMesh*)drop->GetComponent(C_MESH);

		if (c_mesh == nullptr)
			drop->AddComponent(C_MESH, (char*)file, true);
		else
			c_mesh->LoadMesh(file, true);*/

		if (mesh_droped != nullptr)
			DEL(mesh_droped);

		drop->GetComponent(C_TRANSFORM)->Reset();

		mesh_droped = new RE_CompUnregisteredMesh((char*)file, holder->GetBuffer(), holder->GetSize());
	}
	else if (ext.compare("jpg") == 0 || ext.compare("png") == 0 || ext.compare("dds") == 0)
	{
		App->textures->LoadTexture2D(directory.c_str(), file_name.c_str(), true);
	}

	DEL(holder);
}

void ModuleScene::RecieveEvent(const Event * e)
{
}

void ModuleScene::DrawScene()
{
	//triangle->Draw(ShaderPrimitive);

	/*ShaderManager::use(0);
	//App->renderer3d->DirectDrawCube(math::vec(-3.0f,0.0f,0.0f), math::vec(1.0f,0.0f,0.0f));

	ShaderManager::use(ShaderPrimitive);
	math::float4x4 model = math::float4x4::Translate(math::float3(0.0f, 0.0f, 0.0f).Neg());
	model.InverseTranspose();
	ShaderManager::setFloat4x4(ShaderPrimitive, "model", model.ptr());
	ShaderManager::setFloat(ShaderPrimitive, "objectColor", math::vec(0.0f, 1.0f, 0.0f));
	//cube_array->Draw(ShaderPrimitive);

	model = math::float4x4::Translate(math::float3(3.0f, 0.0f, 0.0f).Neg());
	model.InverseTranspose();
	ShaderManager::setFloat4x4(ShaderPrimitive, "model", model.ptr());
	ShaderManager::setFloat(ShaderPrimitive, "objectColor", math::vec(0.0f, 0.0f, 1.0f));
	//cube_index->Draw(ShaderPrimitive);*/

	//primitives
	/*
	compcube->Draw();
	comppoint->Draw();
	compline->Draw();
	comptriangle->Draw();
	*/

	root->Draw();

	if (mesh_droped)
	{
		ShaderManager::use(modelloading);
		ShaderManager::setFloat4x4(modelloading, "model", drop->GetTransform()->GetGlobalMatrix().ptr());
		ShaderManager::setFloat4x4(modelloading, "view", App->renderer3d->camera->GetView().ptr());
		ShaderManager::setFloat4x4(modelloading, "projection", App->renderer3d->camera->GetProjection().ptr());
		mesh_droped->Draw(modelloading);

		/*ShaderManager::use(modelloading);
		ShaderManager::setFloat4x4(modelloading, "model", root->transform->GetGlobalMatrix().ptr());
		ShaderManager::setFloat4x4(modelloading, "view", App->renderer3d->camera->GetView().ptr());
		ShaderManager::setFloat4x4(modelloading, "projection", App->renderer3d->camera->GetProjection().ptr());
		ShaderManager::setFloat(modelloading, "viewPos", math::vec(.0f, 0.0f, 10.0f));
		math::float3x3 modelNormal(root->transform->GetGlobalMatrix().InverseTransposed().Float3x3Part());
		ShaderManager::setFloat3x3(modelloading, "modelNormal", modelNormal.ptr());
		mesh_droped->Draw(modelloading);*/
	}
	

	//drop->Draw();
}

void ModuleScene::DrawFocusedProperties()
{
	if (mesh_droped)
	{
		// root->DrawProperties();

		RE_CompTransform* transform = (RE_CompTransform*)drop->GetComponent(C_TRANSFORM);
		transform->DrawProperties();
		mesh_droped->DrawProperties();
	}
}

//INIT
/*

	ret = App->shaders->Load("sinuscolor", &sinusColor);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	ret = App->shaders->Load("vertexcolor", &vertexColor);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	ret = App->shaders->Load("textureSquare", &textureSquare);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	ret = App->shaders->Load("twotextures", &twotextures);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	ret = App->shaders->Load("cube", &shader_cube);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	ret = App->shaders->Load("light", &lightingShader);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	ret = App->shaders->Load("lamp", &lampShader);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	ret = App->shaders->Load("lightmaps", &lightingmapShader);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	ret = App->shaders->Load("modelloading", &modelloading);
	if (!ret)
		LOG("%s\n", App->shaders->GetShaderError());

	//puppie1 = App->textures->LoadTexture2D("puppie1", ImageExtensionType::JPG);
	//puppie2 = App->textures->LoadTexture2D("puppie2", ImageExtensionType::JPG);
	//container = App->textures->LoadTexture2D("container", ImageExtensionType::JPG);
	//awesomeface = App->textures->LoadTexture2D("awesomeface", ImageExtensionType::PNG);
	//container2 = App->textures->LoadTexture2D("container2", ImageExtensionType::PNG);
	//container2_specular = App->textures->LoadTexture2D("container2_specular", ImageExtensionType::PNG);

//Lighting
	float verticesCubeWNormal[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,    0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,    1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,    1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,    0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,    0.0f, 1.0f
	};
	glGenBuffers(1, &VBO_Light);
	glGenVertexArrays(1, &VAO_Light);

	glBindVertexArray(VAO_Light);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Light);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCubeWNormal), verticesCubeWNormal, GL_STATIC_DRAW);
	// we only need to bind to the VBO, the container's VBO's data already contains the correct data.
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Light);
	// set the vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glEnableVertexAttribArray(0);

	//Defining GL_TEXTUREX for shaders
	ShaderManager::use(twotextures);
	ShaderManager::setInt(twotextures, "texture1", 1);
	ShaderManager::setInt(twotextures, "texture2", 2);
	ShaderManager::use(shader_cube);
	ShaderManager::setInt(shader_cube, "texture1", 1);
	ShaderManager::setInt(shader_cube, "texture2", 2);

	lastX = App->window->GetWidth() / 2;
	lastY = App->window->GetHeight() / 2;

	ShaderManager::use(lightingmapShader);
	ShaderManager::setFloat4x4(lightingmapShader, "model", model_light.ptr());
	math::float3x3 modelNormal(model_light.InverseTransposed().Float3x3Part());
	ShaderManager::setFloat3x3(lightingmapShader, "modelNormal", modelNormal.ptr());
	ShaderManager::setInt(lightingmapShader, "material.diffuse", 0);
	ShaderManager::setInt(lightingmapShader, "material.specular", 1);
	ShaderManager::setFloat(lightingmapShader, "light.direction", lightPos.x, lightPos.y, lightPos.z, 1.0f);

	math::float4x4 model_lamp(math::float4x4::Translate(math::vec::zero));
	model_lamp = model_lamp * math::float4x4::Scale(math::vec(5.0f));
	model_lamp = model_lamp * math::float4x4::Translate(lightPos.Neg());
	model_lamp.InverseTranspose();

	ShaderManager::use(lampShader);
	ShaderManager::setFloat4x4(lampShader, "model", model_lamp.ptr());

*/

//PREUPDATE
/*
if (false)
	{
		//color change for Sinuscolor shader
		if (shaderenabled == SIN)
		{
			//gradually change color usingf uniform at fragmentshader
			ShaderManager::use(sinusColor); //for updating values, we need to useprogram
			timeValue += App->time->GetDeltaTime();
			float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
			ShaderManager::setFloat(sinusColor, "vertexColor", 0.0f, greenValue, 0.0f, 1.0f);
		}

		if (textureEnabled == MIX_AWESOMEFACE)
		{
			if (objectEnabled == PLANE)
			{
				ShaderManager::use(twotextures);
				if (!isRotated)
				{
					camera->SetPos(math::float3(0.0f, 0.0f, -3.0f));

					math::float4x4 model = RE_Math::Rotate(math::float3(1.0f, 0.0f, 0.0f), 55.0f * DEGTORAD);
					model.InverseTranspose();

					ShaderManager::setFloat4x4(twotextures, "model", model.ptr());
					ShaderManager::setFloat4x4(twotextures, "view", camera->GetView().ptr());
					ShaderManager::setFloat4x4(twotextures, "projection", camera->GetProjection().ptr());
				}
				else
				{
					timerotateValue += App->time->GetDeltaTime();

					math::float4x4 trans;
					//All values needed to be inversed, because the InverseTranspose()
					trans = RE_Math::Rotate(math::float3(0.0f, 0.0f, -1.0f), timerotateValue);

					//scale the container
					if (isScaled)
						trans = trans * trans.Scale(math::float3(2.0f, 2.0f, 2.0f));

					//we translate its rotated version to the bottom - right corner of the screen.
					trans = trans * trans.Translate(math::float3(0.5f, -0.5f, 0.0f).Neg());

					//For OpenGL redeable
					trans.InverseTranspose();

					//send the matrix to the shaders
					ShaderManager::setFloat4x4(twotextures, "transform", trans.ptr());
				}
			}
			else
			{
				timeCuberotateValue += App->time->GetDeltaTime();
				ShaderManager::use(shader_cube);

				ShaderManager::setFloat4x4(shader_cube, "projection", camera->GetProjection().ptr());

				if (!isCubes)
				{
					camera->SetPos(math::float3(0.0f, 0.0f, -3.0f));

					math::float4x4 model = RE_Math::Rotate(math::float3(0.5f, 1.0f, 0.0f), timeCuberotateValue * 50.0f * DEGTORAD);
					model.InverseTranspose();

					ShaderManager::setFloat4x4(shader_cube, "model", model.ptr());
					ShaderManager::setFloat4x4(shader_cube, "view", camera->GetView().ptr());
				}
				else
				{
					if (!isMove)
					{
						float radius = 10.0f;
						float camX = sin(timeCuberotateValue) * radius;
						float camZ = cos(timeCuberotateValue) * radius;
						camera->SetPos(math::float3(camX, 0.0f, camZ));
						camera->LookAt(math::float3(0.0f, 0.0f, 0.0f));
						ShaderManager::setFloat4x4(shader_cube, "view", camera->RealView());
					}
					else
					{
						float cameraSpeed = 2.5f * App->time->GetDeltaTime();

						App->input->SetMouseAtCenter();


						const MouseData* mouse = App->input->GetMouse();

						float xoffset = mouse->mouse_x - lastX;
						float yoffset = lastY - mouse->mouse_y;

						float sensitivity = 0.002f;
						xoffset *= sensitivity;
						yoffset *= sensitivity;

						yaw = xoffset;
						pitch = yoffset;

						camera->SetEulerAngle(pitch, yaw);

						yaw = pitch = 0;

						if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
						{
							isMove = false;
							ResetCamera();
						}

						if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
							camera->Move(CameraMovement::FORWARD, cameraSpeed);
						if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
							camera->Move(CameraMovement::BACKWARD, cameraSpeed);
						if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
							camera->Move(CameraMovement::LEFT, cameraSpeed);
						if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
							camera->Move(CameraMovement::RIGHT, cameraSpeed);

						ShaderManager::setFloat4x4(shader_cube, "view", camera->RealView());
					}
				}
			}
		}
	}

	{
	float cameraSpeed = 2.5f * App->time->GetDeltaTime();

	//App->input->SetMouseAtCenter();


	const MouseData* mouse = App->input->GetMouse();

	float xoffset = mouse->mouse_x - lastX;
	float yoffset = lastY - mouse->mouse_y;

	float sensitivity = 0.002f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw = xoffset;
	pitch = yoffset;

	camera->SetEulerAngle(pitch, yaw);

	yaw = pitch = 0;

	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
	{
		isMove = false;
		ResetCamera();
	}

	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		camera->Move(CameraMovement::FORWARD, cameraSpeed);
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		camera->Move(CameraMovement::BACKWARD, cameraSpeed);
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		camera->Move(CameraMovement::LEFT, cameraSpeed);
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		camera->Move(CameraMovement::RIGHT, cameraSpeed);

timeLight += App->time->GetDeltaTime();

	math::vec lightColor;
	lightColor.x = sin(timeLight * 2.0f);
	lightColor.y = sin(timeLight * 0.7f);
	lightColor.z = sin(timeLight * 1.3f);

	math::vec diffuseColor = lightColor.Mul(2.0f); // decrease the influence
	math::vec ambientColor = diffuseColor.Mul(0.2f); // low influence


	math::float4x4 model = math::float4x4::Scale(math::vec(5.0f)) * math::float4x4::Translate(math::vec(0.0f));
	model.InverseTranspose();

	ShaderManager::use(modelloading);
	ShaderManager::setFloat4x4(modelloading, "model", model.ptr());
	ShaderManager::setFloat4x4(modelloading, "view", camera->RealView());
	ShaderManager::setFloat4x4(modelloading, "projection", camera->GetProjection().ptr());
	ShaderManager::setFloat(modelloading, "viewPos", camera->GetPos(true));
	math::float3x3 modelNormal(model.InverseTransposed().Float3x3Part());
	ShaderManager::setFloat3x3(modelloading, "modelNormal", modelNormal.ptr());

		//light propieties
	math::vec pointLightPositions[] = {
		math::vec(1.2f, 1.0f, -1.0f),
	};
	for (unsigned int i = 0; i < 1; i++)
	{
		char tmp[16];
		sprintf_s(tmp, 16, "pointLights[%u].", i);
		std::string pLarray(tmp);

		pLarray += "position";
		ShaderManager::setFloat(modelloading, pLarray.c_str(), pointLightPositions[i]);

		pLarray = tmp;
		pLarray += "constant";
		ShaderManager::setFloat(modelloading, pLarray.c_str(), 1.0f);

		pLarray = tmp;
		pLarray += "linear";
		ShaderManager::setFloat(modelloading, pLarray.c_str(), 0.09f);

		pLarray = tmp;
		pLarray += "quadratic";
		ShaderManager::setFloat(modelloading, pLarray.c_str(), 0.032f);

		pLarray = tmp;
		pLarray += "ambient";
		ShaderManager::setFloat(modelloading, pLarray.c_str(), 0.2f, 0.2f, 0.2f);

		pLarray = tmp;
		pLarray += "diffuse";
		ShaderManager::setFloat(modelloading, pLarray.c_str(), 0.5f, 0.5f, 0.5f);
	}

	ShaderManager::setFloat(lightingmapShader, "light.SPosition", camera->GetPos(true));
	ShaderManager::setFloat(lightingmapShader, "light.SDirection", camera->GetFront());
	ShaderManager::setFloat(lightingmapShader, "light.cutOff", cos(12.5f * DEGTORAD));
	ShaderManager::setFloat(lightingmapShader, "light.outerCutOff", cos(17.5f * DEGTORAD));

	ShaderManager::setFloat(modelloading, "light.ambient", 0.2f, 0.2f, 0.2f);
	ShaderManager::setFloat(modelloading, "light.diffuse", 0.5f, 0.5f, 0.5f);
	ShaderManager::setFloat(modelloading, "light.specular", 1.0f, 1.0f, 1.0f);
	ShaderManager::setFloat(modelloading, "light.constant", 1.0f);
	ShaderManager::setFloat(modelloading, "light.linear", 0.09f);
	ShaderManager::setFloat(modelloading, "light.quadratic", 0.032f);

	ShaderManager::setFloat(lightingmapShader, "light.direction", -0.2f, -1.0f, -0.3f, 0.0f);

			//material propieties
	//ShaderManager::setFloat(lightingmapShader, "material.shininess", 32.0f);
	ShaderManager::use(lampShader);
	ShaderManager::setFloat4x4(lampShader, "view", camera->RealView());
	ShaderManager::setFloat4x4(lampShader, "projection", camera->GetProjection().ptr());
*/

//POSTUPDATE
/*
		ShaderManager::use(lightingmapShader);

		glActiveTexture(GL_TEXTURE0);
		texture_manager->use(container2);
		glActiveTexture(GL_TEXTURE1);
		texture_manager->use(container2_specular);

		math::vec cubePositions[] = {
			math::vec(0.0f,  0.0f,  0.0f),
			math::vec(2.0f,  5.0f, -15.0f),
			math::vec(-1.5f, -2.2f, -2.5f),
			math::vec(-3.8f, -2.0f, -12.3f),
			math::vec(2.4f, -0.4f, -3.5f),
			math::vec(-1.7f,  3.0f, -7.5f),
			math::vec(1.3f, -2.0f, -2.5f),
			math::vec(1.5f,  2.0f, -2.5f),
			math::vec(1.5f,  0.2f, -1.5f),
			math::vec(-1.3f,  1.0f, -1.5f)
		};

		glBindVertexArray(VAO_Light);
		for (unsigned int i = 0; i < 10; i++)
		{
			float angle = 20.0f * i;
			math::float4x4 model = RE_Math::Rotate(math::vec(1.0f, 0.3f, 0.5f), angle * DEGTORAD) * math::float4x4::Translate(cubePositions[i].Neg());
			model.InverseTranspose();
			math::float3x3 modelNormal(model.InverseTransposed().Float3x3Part());

			ShaderManager::setFloat3x3(lightingmapShader, "modelNormal", modelNormal.ptr());
			ShaderManager::setFloat4x4(lightingmapShader, "model", model.ptr());

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		ShaderManager::use(lampShader);
		glBindVertexArray(VAO_Light);

		math::vec pointLightPositions[] = {
			math::vec(0.7f,  0.2f,  2.0f),
			math::vec(2.3f, -3.3f, -4.0f),
			math::vec(-4.0f,  2.0f, -12.0f),
			math::vec(0.0f,  0.0f, -3.0f)
		};

		for (unsigned int i = 0; i < 4; i++)
		{
			math::float4x4 model = math::float4x4::Scale(math::vec(5.0f)) * math::float4x4::Translate(pointLightPositions[i].Neg());
			model.InverseTranspose();
			ShaderManager::setFloat4x4(lampShader, "model", model.ptr());

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}



	ShaderManager::use(lampShader);
	glBindVertexArray(VAO_Light);

	math::vec pointLightPositions[] = {
		math::vec(1.2f, 1.0f, -1.0f),
	};
	for (unsigned int i = 0; i < 1; i++)
	{
		math::float4x4 model = math::float4x4::Scale(math::vec(5.0f)) * math::float4x4::Translate(pointLightPositions[i].Neg());
		model.InverseTranspose();
		ShaderManager::setFloat4x4(lampShader, "model", model.ptr());

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);


*/