#include "ModuleRenderer3D.h"

#include <Windows.h>
#include "MathGeoLib/include/MathGeoLib.h"
#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "TimeManager.h"
#include "ShaderManager.h"
#include "ModuleInput.h"
#include "Texture2DManager.h"
#include "FileSystem.h"

#pragma comment(lib, "Glew/lib/glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "MathGeoLib/lib/MathGeoLib_debug.lib")

ModuleRenderer3D::ModuleRenderer3D(const char * name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleRenderer3D::~ModuleRenderer3D()
{}

bool ModuleRenderer3D::Init(JSONNode * config_module)
{
	bool ret = true;

	SDL_GLContext t;

	mainContext = SDL_GL_CreateContext(App->window->GetWindow());
	if (mainContext == nullptr)
	{
		//Error
		ret = false;
	}

	//Glew inicialitzation
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		//fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		ret = false;
	}

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	//Loading Shaders
	shader_manager = new ShaderManager("Shaders/");
	ret = shader_manager->Load("sinuscolor", &sinusColor);
	if (!ret)
		LOG("%s\n", shader_manager->GetShaderError());

	ret = shader_manager->Load("vertexcolor", &vertexColor);
	if(!ret)
		LOG("%s\n", shader_manager->GetShaderError());

	ret = shader_manager->Load("textureSquare", &textureSquare);
	if (!ret)
		LOG("%s\n", shader_manager->GetShaderError());

	ret = shader_manager->Load("twotextures", &twotextures);
	if (!ret)
		LOG("%s\n", shader_manager->GetShaderError());

	ret = shader_manager->Load("cube", &shader_cube);
	if (!ret)
		LOG("%s\n", shader_manager->GetShaderError());

	//Print a 2d triangle3d https://learnopengl.com/Getting-started/Hello-Triangle
	//2d traingle defined with 3d coordenates
	//VBO with VAO
	float vertices[] = {
		// positions         // colors
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
		 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
	};
	glGenVertexArrays(1, &VAO_Triangle);
	glGenBuffers(1, &VBO_Triangle);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO_Triangle);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	
	//EBO
	float verticesS[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
	};

	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};
	glGenBuffers(1, &VBO_Square);
	glGenBuffers(1, &EBO_Square);
	glGenVertexArrays(1, &VAO_Square);
	// ..:: Initialization code :: ..
	// 1. bind Vertex Array Object
	glBindVertexArray(VAO_Square);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Square);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesS), verticesS, GL_STATIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Square);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// 4. then set the vertex attributes pointers
	//position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//vertex colors
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//Texture coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	//Cube
	float verticesCube[] = {
	//Vertex pos		  Texture Coords
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	glGenBuffers(1, &VBO_Cube);
	glGenVertexArrays(1, &VAO_Cube);

	glBindVertexArray(VAO_Cube);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCube), verticesCube, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	texture_manager = new Texture2DManager();
	texture_manager->Init("Images/");

	puppie1 = texture_manager->LoadTexture2D("puppie1", ImageExtensionType::JPG);
	puppie2 = texture_manager->LoadTexture2D("puppie2", ImageExtensionType::JPG);
	container = texture_manager->LoadTexture2D("container", ImageExtensionType::JPG);
	awesomeface = texture_manager->LoadTexture2D("awesomeface", ImageExtensionType::PNG);

	//Defining GL_TEXTUREX for shaders
	shader_manager->use(twotextures);
	shader_manager->setInt(twotextures, "texture1", 1);
	shader_manager->setInt(twotextures, "texture2", 2);
	shader_manager->use(shader_cube);
	shader_manager->setInt(shader_cube, "texture1", 1);
	shader_manager->setInt(shader_cube, "texture2", 2);

	enableVSync(vsync = config_module->PullBool("vsync", false));

	return ret;
}

update_status ModuleRenderer3D::PreUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	//Set background with a clear color
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//color change for Sinuscolor shader
	if (shaderenabled == SIN)
	{
		//gradually change color usingf uniform at fragmentshader
		shader_manager->use(sinusColor); //for updating values, we need to useprogram
		timeValue += App->time->GetDeltaTime();
		float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		shader_manager->setFloat(sinusColor, "vertexColor", 0.0f, greenValue, 0.0f, 1.0f);
	}

	/*
	math::float4 vec(1.0f, 0.0f, 0.0f,1.0f);
	math::float4x4 trans;
	trans.Translate(math::float3(1.0f,1.0f,0.0f));
	vec = trans * vec;
	*/

	if (textureEnabled == MIX_AWESOMEFACE)
	{
		if (objectEnabled == PLANE)
		{
			shader_manager->use(twotextures);
			if (!isRotated)
			{
				math::float4x4 model;
				model = model.Translate(math::float3(0.0f, 0.0f, 0.0f));
				model = model * model.RotateAxisAngle(math::float3(-1.0f, 0.0f, 0.0f).Neg(), math::DegToRad(55.0f));

				math::float4x4 view;
				// note that we're translating the scene in the reverse direction of where we want to move
				view = view.Translate(math::float3(0.0f, 0.0f, -3.0f).Neg());
				view.InverseTranspose();

				float horitzontalFOV = 1;
				float verticalFOV = horitzontalFOV * (float)App->window->GetHeight() / (float)App->window->GetWidth();

				math::Frustum camera;
				camera.SetViewPlaneDistances(0.1f, 100.0f);
				camera.SetPerspective(horitzontalFOV, verticalFOV);
				camera.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);


				shader_manager->setFloat4x4(twotextures, "model", model.ptr());
				shader_manager->setFloat4x4(twotextures, "view", view.ptr());
				shader_manager->setFloat4x4(twotextures, "projection", camera.ProjectionMatrix().Transposed().ptr());
			}
			else
			{
				timerotateValue += App->time->GetDeltaTime();

				math::float4x4 trans;
				//All values needed to be inversed, because the InverseTranspose()
				//first rotate the container around the origin (0,0,0)
				trans = trans.Translate(math::float3(0.0f, 0.0f, 0.0f));
				trans = trans * trans.RotateAxisAngle(math::float3(0.0f, 0.0f, 1.0f).Neg(), timerotateValue);

				//scale the container
				if (isScaled)
					trans = trans * trans.Scale(math::float3(2.0f, 2.0f, 2.0f));

				//we translate its rotated version to the bottom - right corner of the screen.
				trans = trans * trans.Translate(math::float3(0.5f, -0.5f, 0.0f).Neg());

				//For OpenGL redeable
				trans.InverseTranspose();

				//send the matrix to the shaders
				shader_manager->setFloat4x4(twotextures, "transform", trans.ptr());
			}
		}
		else
		{
			if (!isCubes)
			{
				shader_manager->use(shader_cube);

				timeCuberotateValue += App->time->GetDeltaTime();

				math::float4x4 model;
				model = model.RotateAxisAngle(math::float3(0.5f, 1.0f, 0.0f).Neg(), timeCuberotateValue * math::DegToRad(50.0f));

				math::float4x4 view;
				// note that we're translating the scene in the reverse direction of where we want to move
				view = view.Translate(math::float3(0.0f, 0.0f, -3.0f).Neg());
				view.InverseTranspose();

				float horitzontalFOV = 1;
				float verticalFOV = horitzontalFOV * (float)App->window->GetHeight() / (float)App->window->GetWidth();

				math::Frustum camera;
				camera.SetViewPlaneDistances(0.1f, 100.0f);
				camera.SetPerspective(horitzontalFOV, verticalFOV);
				camera.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

				shader_manager->setFloat4x4(shader_cube, "model", model.ptr());
				shader_manager->setFloat4x4(shader_cube, "view", view.ptr());
				shader_manager->setFloat4x4(shader_cube, "projection", camera.ProjectionMatrix().Transposed().ptr());
			}
			else
			{
				shader_manager->use(shader_cube);

				timeCuberotateValue += App->time->GetDeltaTime();

				math::float4x4 view;
				// note that we're translating the scene in the reverse direction of where we want to move
				view = view.Translate(math::float3(0.0f, 0.0f, -3.0f).Neg());
				view.InverseTranspose();

				float horitzontalFOV = 1;
				float verticalFOV = horitzontalFOV * (float)App->window->GetHeight() / (float)App->window->GetWidth();

				math::Frustum camera;
				camera.SetViewPlaneDistances(0.1f, 100.0f);
				camera.SetPerspective(horitzontalFOV, verticalFOV);
				camera.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);

				shader_manager->setFloat4x4(shader_cube, "view", view.ptr());
				shader_manager->setFloat4x4(shader_cube, "projection", camera.ProjectionMatrix().Transposed().ptr());
			}
		}
	}
	return ret;
}

update_status ModuleRenderer3D::PostUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	if(isLine)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	switch (shaderenabled)
	{
	case SIN:
		shader_manager->use(sinusColor);
		break;
	case VERTEX:
		shader_manager->use(vertexColor);
		break;
	case TEXTURE:
		if (textureEnabled == MIX_AWESOMEFACE)
			if (objectEnabled == PLANE)
				shader_manager->use(twotextures);
			else
				shader_manager->use(shader_cube);
		else
			shader_manager->use(textureSquare);
		break;
	}

	// ..:: Drawing code (in render loop) :: .. EBO
	if (B_EBO)
	{
		if (shaderenabled == TEXTURE)
		{
			switch (textureEnabled)
			{
			case PUPPIE_1:
				glActiveTexture(GL_TEXTURE0);
				texture_manager->use(puppie1);
				break;
			case PUPPIE_2:
				glActiveTexture(GL_TEXTURE0);
				texture_manager->use(puppie2);
				break;
			case CONTAINER:
				glActiveTexture(GL_TEXTURE0);
				texture_manager->use(container);
				break;
			case MIX_AWESOMEFACE:
				glActiveTexture(GL_TEXTURE1);
				texture_manager->use(container);
				glActiveTexture(GL_TEXTURE2);
				texture_manager->use(awesomeface);
				break;
			}
		}
		if (objectEnabled == PLANE)
		{
			glBindVertexArray(VAO_Square);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //EBO -> VAO
		}
		else
		{
			glBindVertexArray(VAO_Cube);
			if (!isCubes)
				glDrawArrays(GL_TRIANGLES, 0, 36);
			else
			{
				math::float3 cubePositions[] = {
				  math::float3(0.0f,  0.0f,  0.0f),
				  math::float3(2.0f,  5.0f, -15.0f),
				  math::float3(-1.5f, -2.2f, -2.5f),
				  math::float3(-3.8f, -2.0f, -12.3f),
				  math::float3(2.4f, -0.4f, -3.5f),
				  math::float3(-1.7f,  3.0f, -7.5f),
				  math::float3(1.3f, -2.0f, -2.5f),
				  math::float3(1.5f,  2.0f, -2.5f),
				  math::float3(1.5f,  0.2f, -1.5f),
				  math::float3(-1.3f,  1.0f, -1.5f)
				};

				for (unsigned int i = 0; i < 10; i++)
				{
					math::float4x4 model;
					float angle = 20.0f * i;
					model = model.RotateAxisAngle(math::float3(1.0f, 0.3f, 0.5f).Neg(), math::DegToRad(angle));
					model = model * model.Translate(cubePositions[i].Neg());
					model.InverseTranspose();
					shader_manager->setFloat4x4(shader_cube, "model", model.ptr());

					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			}
		}
	}
	else
	{
		glBindVertexArray(VAO_Triangle);
		glDrawArrays(GL_TRIANGLES, 0, 3); //VBO with vertex attributes (VAO)
	}
	glBindVertexArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// Draw Editor
	if(App->editor != nullptr)
		App->editor->Draw();

	//Swap buffers
	SDL_GL_SwapWindow(App->window->GetWindow());

	return ret;
}

bool ModuleRenderer3D::CleanUp()
{
	bool ret = true;

	//de-allocate all resources
	glDeleteVertexArrays(1, &VAO_Triangle);
	glDeleteVertexArrays(1, &VAO_Square);
	glDeleteVertexArrays(1, &VAO_Cube);

	glDeleteBuffers(1, &VBO_Triangle);
	glDeleteBuffers(1, &VBO_Square);
	glDeleteBuffers(1, &EBO_Square);
	glDeleteBuffers(1, &VBO_Cube);

	//Delete context
	SDL_GL_DeleteContext(mainContext);

	//Delete textures
	delete texture_manager;

	//delete shaders
	delete shader_manager;

	return ret;
}

void ModuleRenderer3D::DrawEditor()
{
}

void ModuleRenderer3D::RecieveEvent(const Event * e)
{
}

void ModuleRenderer3D::enableVSync(const bool enable)
{
	SDL_GL_SetSwapInterval(enable ? 1 : 0);
}

unsigned int ModuleRenderer3D::GetMaxVertexAttributes()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	return nrAttributes;
}

ShaderType ModuleRenderer3D::GetShaderEnabled() const
{
	return shaderenabled;
}

Texture2DType ModuleRenderer3D::GetTexture2DEnabled() const
{
	return textureEnabled;
}

ObjectType ModuleRenderer3D::GetObjectEnabled() const
{
	return objectEnabled;
}

void ModuleRenderer3D::SetShaderEnabled(ShaderType shader_enabled)
{
	shaderenabled = shader_enabled;
}

void ModuleRenderer3D::SetTexture2DEnabled(Texture2DType texture2d_enabled)
{
	textureEnabled = texture2d_enabled;
}

void ModuleRenderer3D::SetObjectEnabled(ObjectType object_enabled)
{
	objectEnabled = object_enabled;
}

void ModuleRenderer3D::UseShader(ShaderType shader_enabled)
{
	switch (shader_enabled)
	{
	case SIN:
		shader_manager->use(sinusColor);
		break;
	case VERTEX:
		shader_manager->use(vertexColor);
		break;
	case TEXTURE:
		if (objectEnabled == PLANE)
			if (textureEnabled == MIX_AWESOMEFACE)
				shader_manager->use(twotextures);
			else
				shader_manager->use(textureSquare);
		else
			shader_manager->use(shader_cube);
		break;
	}
}

void ModuleRenderer3D::SetShaderBool(const char * name, bool value)
{
	if (objectEnabled == PLANE)
		if (textureEnabled == MIX_AWESOMEFACE)
			shader_manager->setBool(twotextures, name, value);
		else
			shader_manager->setBool(textureSquare, name, value);
}

bool * ModuleRenderer3D::GetVsync()
{
	return &vsync;
}

bool * ModuleRenderer3D::GetB_EBO()
{
	return &B_EBO;
}

bool * ModuleRenderer3D::GetisLine()
{
	return &isLine;
}

bool * ModuleRenderer3D::Getprintvertextcolor()
{
	return &printvertextcolor;
}

bool * ModuleRenderer3D::GetisRotated()
{
	return &isRotated;
}

bool * ModuleRenderer3D::GetisScaled()
{
	return &isScaled;
}

bool * ModuleRenderer3D::GetisCubes()
{
	return &isCubes;
}
