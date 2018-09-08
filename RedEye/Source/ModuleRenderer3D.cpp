#include "ModuleRenderer3D.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include <Windows.h>
#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "RapidJson\include\filereadstream.h"
#include "TimeManager.h"
#include "Shader.h"
#include "ModuleInput.h"

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
	
	std::string fullPathvertex(SDL_GetBasePath());
	fullPathvertex += "Assets\\sinuscolor.vert";

	std::string fullPathfragment(SDL_GetBasePath());
	fullPathfragment += "Assets\\sinuscolor.frag";

	sinusColor = new Shader(fullPathvertex.c_str(), fullPathfragment.c_str());
	
	fullPathvertex = SDL_GetBasePath();
	fullPathvertex += "Assets\\vertexcolor.vert";

	fullPathfragment = SDL_GetBasePath();
	fullPathfragment += "Assets\\vertexcolor.frag";

	vertexColor = new Shader(fullPathvertex.c_str(), fullPathfragment.c_str());

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
		 0.5f,  0.5f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	return ret;
}

update_status ModuleRenderer3D::PreUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	//Set background with a clear color
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_A) == KEY_DOWN)
		v_color = !v_color;

	if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_S) == KEY_DOWN)
		B_EBO = !B_EBO;

	if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_D) == KEY_DOWN)
		isLine = !isLine;

	if (!v_color)
	{
		//gradually change color usingf uniform at fragmentshader
		sinusColor->use(); //for updating values, we need to useprogram
		timeValue += App->time->GetDeltaTime();
		float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		sinusColor->setFloat("vertexColor", 0.0f, greenValue, 0.0f, 1.0f);
	}

	return ret;
}

update_status ModuleRenderer3D::PostUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	if(isLine)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	(v_color) ? vertexColor->use() : sinusColor->use();
	// ..:: Drawing code (in render loop) :: .. EBO
	if (B_EBO)
	{
		glBindVertexArray(VAO_Square);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //EBO -> VAO
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
	
	//delete shaders
	delete sinusColor;
	delete vertexColor;

	//de-allocate all resources
	glDeleteVertexArrays(1, &VAO_Triangle);
	glDeleteVertexArrays(1, &VAO_Square);

	glDeleteBuffers(1, &VBO_Triangle);
	glDeleteBuffers(1, &VBO_Square);
	glDeleteBuffers(1, &EBO_Square);

	//Delete context
	SDL_GL_DeleteContext(mainContext);

	return ret;
}

unsigned int ModuleRenderer3D::GetMaxVertexAttributes()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	return nrAttributes;
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
