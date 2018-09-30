#include "ModuleRenderer3D.h"

#include "RE_Math.h"
#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>

#include "OutputLog.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "TimeManager.h"
#include "ShaderManager.h"
#include "ModuleInput.h"
#include "Texture2DManager.h"
#include "FileSystem.h"
#include "RE_Camera.h"

#pragma comment(lib, "Glew/lib/glew32.lib")
#pragma comment(lib, "opengl32.lib")

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

	App->ReportSoftware("OpenGL", (char*)glGetString(GL_VERSION), "https://www.opengl.org/");
	App->ReportSoftware("GLslang", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION), "https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/glsl_overview.php");
	App->ReportSoftware("Glew", (char*)glewGetString(GLEW_VERSION), "http://glew.sourceforge.net/");

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	//Loading Shaders
	shader_manager = new ShaderManager("Shaders/");
	if (false)
	{
		ret = shader_manager->Load("sinuscolor", &sinusColor);
		if (!ret)
			LOG("%s\n", shader_manager->GetShaderError());

		ret = shader_manager->Load("vertexcolor", &vertexColor);
		if (!ret)
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
	}
	ret = shader_manager->Load("light", &lightingShader);
	if (!ret)
		LOG("%s\n", shader_manager->GetShaderError());

	ret = shader_manager->Load("lamp", &lampShader);
	if (!ret)
		LOG("%s\n", shader_manager->GetShaderError());

	ret = shader_manager->Load("lightmaps", &lightingmapShader);
	if (!ret)
		LOG("%s\n", shader_manager->GetShaderError());
	
	if (false)
	{
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
	}

	//Lighting
	float verticesCubeWNormal[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
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

	texture_manager = new Texture2DManager();
	texture_manager->Init("Images/");
	if (false)
	{
		puppie1 = texture_manager->LoadTexture2D("puppie1", ImageExtensionType::JPG);
		puppie2 = texture_manager->LoadTexture2D("puppie2", ImageExtensionType::JPG);
		container = texture_manager->LoadTexture2D("container", ImageExtensionType::JPG);
		awesomeface = texture_manager->LoadTexture2D("awesomeface", ImageExtensionType::PNG);

		//Defining GL_TEXTUREX for shaders
		ShaderManager::use(twotextures);
		ShaderManager::setInt(twotextures, "texture1", 1);
		ShaderManager::setInt(twotextures, "texture2", 2);
		ShaderManager::use(shader_cube);
		ShaderManager::setInt(shader_cube, "texture1", 1);
		ShaderManager::setInt(shader_cube, "texture2", 2);
	}
	container2 = texture_manager->LoadTexture2D("container2", ImageExtensionType::PNG);
	container2_specular = texture_manager->LoadTexture2D("container2_specular", ImageExtensionType::PNG);
	enableVSync(vsync = config_module->PullBool("vsync", false));

	camera = new RE_Camera(true);
	camera->SetPos(math::vec(0.0f, 0.0f, -3.0f));

	lastX = App->window->GetWidth() / 2;
	lastY = App->window->GetHeight() / 2;

	math::float4x4 model_light;

	//model_light = math::float4x4::Translate(math::vec(0.0f));
	//model_light.InverseTranspose();

	math::vec lightPos(1.2f, 1.0f, 2.0f);

	ShaderManager::use(lightingmapShader);
	//ShaderManager::setFloat4x4(lightingmapShader, "model", model_light.ptr());
	//math::float3x3 modelNormal(model_light.InverseTransposed().Float3x3Part());
	//ShaderManager::setFloat3x3(lightingmapShader, "modelNormal", modelNormal.ptr());
	ShaderManager::setInt(lightingmapShader, "material.diffuse", 0);
	ShaderManager::setInt(lightingmapShader, "material.specular", 1);
	//ShaderManager::setFloat(lightingmapShader, "light.direction", lightPos.x, lightPos.y, lightPos.z, 1.0f);

	/*
	math::float4x4 model_lamp(math::float4x4::Translate(math::vec::zero));
	model_lamp = model_lamp * math::float4x4::Scale(math::vec(5.0f));
	model_lamp = model_lamp * math::float4x4::Translate(lightPos.Neg());
	model_lamp.InverseTranspose();

	ShaderManager::use(lampShader);
	ShaderManager::setFloat4x4(lampShader, "model", model_lamp.ptr());
	*/

	return ret;
}

update_status ModuleRenderer3D::PreUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	//Set background with a clear color
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
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

	/*
	timeLight += App->time->GetDeltaTime();

	math::vec lightColor;
	lightColor.x = sin(timeLight * 2.0f);
	lightColor.y = sin(timeLight * 0.7f);
	lightColor.z = sin(timeLight * 1.3f);

	math::vec diffuseColor = lightColor.Mul(2.0f); // decrease the influence
	math::vec ambientColor = diffuseColor.Mul(0.2f); // low influence
	*/

	ShaderManager::use(lightingmapShader);
	ShaderManager::setFloat4x4(lightingmapShader, "view", camera->RealView());
	ShaderManager::setFloat4x4(lightingmapShader, "projection", camera->GetProjection().ptr());
	ShaderManager::setFloat(lightingmapShader, "viewPos", camera->GetPos(true));

		//light propieties

	math::vec pointLightPositions[] = {
		math::vec(0.7f,  0.2f,  2.0f),
		math::vec(2.3f, -3.3f, -4.0f),
		math::vec(-4.0f,  2.0f, -12.0f),
		math::vec(0.0f,  0.0f, -3.0f)
	};
	for (unsigned int i = 0; i < 4; i++)
	{
		char tmp[16];
		sprintf_s(tmp, 16, "pointLights[%u].", i);
		std::string pLarray(tmp);

		pLarray += "position";
		ShaderManager::setFloat(lightingmapShader, pLarray.c_str(), pointLightPositions[i]);

		pLarray = tmp;
		pLarray += "constant";
		ShaderManager::setFloat(lightingmapShader, pLarray.c_str(), 1.0f);

		pLarray = tmp;
		pLarray += "linear";
		ShaderManager::setFloat(lightingmapShader, pLarray.c_str(), 0.09f);

		pLarray = tmp;
		pLarray += "quadratic";
		ShaderManager::setFloat(lightingmapShader, pLarray.c_str(), 0.032f);

		pLarray = tmp;
		pLarray += "ambient";
		ShaderManager::setFloat(lightingmapShader, pLarray.c_str(), 0.2f, 0.2f, 0.2f);

		pLarray = tmp;
		pLarray += "diffuse";
		ShaderManager::setFloat(lightingmapShader, pLarray.c_str(), 0.5f, 0.5f, 0.5f);

		pLarray = tmp;
		pLarray += "specular";
		ShaderManager::setFloat(lightingmapShader, pLarray.c_str(), 1.0f, 1.0f, 1.0f);
	}
	/*
	ShaderManager::setFloat(lightingmapShader, "light.SPosition", camera->GetPos(true));
	ShaderManager::setFloat(lightingmapShader, "light.SDirection", camera->GetFront());
	ShaderManager::setFloat(lightingmapShader, "light.cutOff", cos(12.5f * DEGTORAD));
	ShaderManager::setFloat(lightingmapShader, "light.outerCutOff", cos(17.5f * DEGTORAD));

	ShaderManager::setFloat(lightingmapShader, "light.ambient", 0.2f, 0.2f, 0.2f);
	ShaderManager::setFloat(lightingmapShader, "light.diffuse", 0.5f, 0.5f, 0.5f);
	ShaderManager::setFloat(lightingmapShader, "light.specular", 1.0f, 1.0f, 1.0f);
	ShaderManager::setFloat(lightingmapShader, "light.constant", 1.0f);
	ShaderManager::setFloat(lightingmapShader, "light.linear", 0.09f);
	ShaderManager::setFloat(lightingmapShader, "light.quadratic", 0.032f);
	//ShaderManager::setFloat(lightingmapShader, "light.direction", -0.2f, -1.0f, -0.3f, 0.0f);
	*/

		//material propieties
	//ShaderManager::setFloat(lightingShader, "material.ambient", 1.0f, 0.5f, 0.31f);
	//ShaderManager::setFloat(lightingShader, "material.diffuse", 1.0f, 0.5f, 0.31f);
	//ShaderManager::setFloat(lightingmapShader, "material.specular", 0.5f, 0.5f, 0.5f);
	ShaderManager::setFloat(lightingmapShader, "material.shininess", 32.0f);

	ShaderManager::use(lampShader);
	ShaderManager::setFloat4x4(lampShader, "view", camera->RealView());
	ShaderManager::setFloat4x4(lampShader, "projection", camera->GetProjection().ptr());
	}
	return ret;
}

update_status ModuleRenderer3D::PostUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	if(isLine)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (false)
	{
		switch (shaderenabled)
		{
		case SIN:
			ShaderManager::use(sinusColor);
			break;
		case VERTEX:
			ShaderManager::use(vertexColor);
			break;
		case TEXTURE:
			if (textureEnabled == MIX_AWESOMEFACE)
				if (objectEnabled == PLANE)
					ShaderManager::use(twotextures);
				else
					ShaderManager::use(shader_cube);
			else
				ShaderManager::use(textureSquare);
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
						float angle = 20.0f * i;

						math::float4x4 model = RE_Math::Rotate(math::float3(1.0f, 0.3f, 0.5f), angle * DEGTORAD) * math::float4x4::Translate(math::float3(cubePositions[i]).Neg());
						model.InverseTranspose();

						ShaderManager::setFloat4x4(shader_cube, "model", model.ptr());

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
	}
	else
	{
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
	glDeleteVertexArrays(1, &VAO_Light);

	glDeleteBuffers(1, &VBO_Triangle);
	glDeleteBuffers(1, &VBO_Square);
	glDeleteBuffers(1, &EBO_Square);
	glDeleteBuffers(1, &VBO_Cube);
	glDeleteBuffers(1, &VBO_Light);

	//Delete textures
	delete texture_manager;

	//delete shaders
	delete shader_manager;

	delete camera;

	//Delete context
	SDL_GL_DeleteContext(mainContext);

	return ret;
}

void ModuleRenderer3D::DrawEditor()
{
	if(ImGui::CollapsingHeader("Renderer 3D"))
	{
		if (ImGui::Checkbox((vsync) ? "Disable vsync" : "Enable vsync", &vsync))
			enableVSync(vsync);

		ImGui::Checkbox((isLine) ? "Disable Wireframe" : "Enable Wireframe", &isLine);
	}
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
		ShaderManager::use(sinusColor);
		break;
	case VERTEX:
		ShaderManager::use(vertexColor);
		break;
	case TEXTURE:
		if (objectEnabled == PLANE)
			if (textureEnabled == MIX_AWESOMEFACE)
				ShaderManager::use(twotextures);
			else
				ShaderManager::use(textureSquare);
		else
			ShaderManager::use(shader_cube);
		break;
	}
}

void ModuleRenderer3D::SetShaderBool(const char * name, bool value)
{
	if (objectEnabled == PLANE)
		if (textureEnabled == MIX_AWESOMEFACE)
			ShaderManager::setBool(twotextures, name, value);
		else
			ShaderManager::setBool(textureSquare, name, value);
}

void ModuleRenderer3D::ResetCamera()
{
	camera->ResetCameraQuat();
	camera->SetPos(math::vec(0.0f, 0.0f, -3.0f));
}

bool * ModuleRenderer3D::GetB_EBO()
{
	return &B_EBO;
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

bool * ModuleRenderer3D::GetisMove()
{
	return &isMove;
}

void ModuleRenderer3D::ResetAspectRatio()
{
	glViewport(0, 0, App->window->GetWidth(), App->window->GetHeight());
	camera->ResetFov();
}
