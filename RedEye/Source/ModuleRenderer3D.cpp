#include "ModuleRenderer3D.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include <Windows.h>
#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "RapidJson\include\filereadstream.h"

#pragma comment(lib, "Glew/lib/glew32.lib")
#pragma comment(lib, "opengl32.lib")

ModuleRenderer3D::ModuleRenderer3D(const char * name, bool start_enabled) : Module(name, start_enabled)
{
}

ModuleRenderer3D::~ModuleRenderer3D()
{
}

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

	//Creating vertex and fragment shader

//compiling vertex shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//load source
	std::string fullPath(SDL_GetBasePath());
	fullPath += "Assets\\VertexShader.vert";

	char* vs_buffer = new char[65536];
	LoadBuffer(fullPath.c_str(), &vs_buffer, 65536);
	glShaderSource(vertexShader, 1, &vs_buffer, NULL);
	glCompileShader(vertexShader);
	delete[] vs_buffer;

	//check
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		LOG("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n", infoLog);
		ret = false;
	}

	//compiling fragment shader
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//load source
	fullPath = SDL_GetBasePath();
	fullPath += "Assets\\FragmentShader.frag";

	char* fs_buffer = new char[65536];
	LoadBuffer(fullPath.c_str(), &fs_buffer, 65536);
	glShaderSource(fragmentShader, 1, &fs_buffer, NULL);
	glCompileShader(fragmentShader);
	delete[] fs_buffer;

	//check
	int  success2;
	char infoLog2[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success2);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog2);
		LOG("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n", infoLog2);
		ret = false;
	}

	//creating Shader program, link the once shaders types
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//check
	int success3;
	char infoLog3[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success3);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog3);
		LOG("ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n", infoLog3);
		ret = false;
	}

	//deleting shaders, no needed after link
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//Print a 2d triangle3d https://learnopengl.com/Getting-started/Hello-Triangle
//2d traingle defined with 3d coordenates
	float vertices[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
	};

	//VBO
	//Generating vertex buffer objects
	unsigned int VBO;
	glGenBuffers(1, &VBO); //(size, direction of memory)

	// 0. copy our vertices array in a buffer for OpenGL to use
	//Setting type of buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //(type, buffer) GL_ARRAY_BUFFER -> for arrays

	//copies vertex tado into buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //GL_ARRAY_BUFFER target is VBO value. GL_STATIC_DRAW: the data will most likely not change at all or very rarely.
	// 1. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	//VAO
	if (!B_EBO)
	{
		glGenVertexArrays(1, &VAO);

		// ..:: Initialization code (done once (unless your object frequently changes)) :: ..
		// 1. bind Vertex Array Object
		glBindVertexArray(VAO);
		// 2. copy our vertices array in a buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// 3. then set our vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	else
	{
		//EBO
		float verticesS[] = {
			 0.5f,  0.5f, 0.0f,  // top right
			 0.5f, -0.5f, 0.0f,  // bottom right
			-0.5f, -0.5f, 0.0f,  // bottom left
			-0.5f,  0.5f, 0.0f   // top left 
		};
		unsigned int indices[] = {  // note that we start from 0!
			0, 1, 3,   // first triangle
			1, 2, 3    // second triangle
		};

		glGenBuffers(1, &EBO);

		// ..:: Initialization code :: ..
		// 1. bind Vertex Array Object
		glBindVertexArray(VAO);
		// 2. copy our vertices array in a vertex buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verticesS), verticesS, GL_STATIC_DRAW);
		// 3. copy our index array in a element buffer for OpenGL to use
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		// 4. then set the vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	return ret;
}

update_status ModuleRenderer3D::PreUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	//Set background with a clear color
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	return ret;
}

update_status ModuleRenderer3D::PostUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// ..:: Drawing code (in render loop) :: .. EBO
	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	(B_EBO) ? glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0)/*EBO -> BAO*/ : glDrawArrays(GL_TRIANGLES, 0, 3); //VBO -> VAO
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
	
	//delete shared program(Vertex and fragment shared link)
	glDeleteProgram(shaderProgram);

	//Delete context
	SDL_GL_DeleteContext(mainContext);

	return ret;
}

void ModuleRenderer3D::LoadBuffer(const char* path, char ** buffer, unsigned int size)
{
	FILE* fp;
	if (fopen_s(&fp, path, "rb") == 0);// non-Windows use "r"
	{
		// Read File
		rapidjson::FileReadStream is(fp, *buffer, size);
		fclose(fp);
	}
}
