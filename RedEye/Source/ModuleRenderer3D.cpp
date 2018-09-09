#include "ModuleRenderer3D.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include <Windows.h>
#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "TimeManager.h"
#include "Shader.h"
#include "ModuleInput.h"
#include "Texture2DManager.h"

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
	
	std::string fullPathvertex("Shaders/sinuscolor.vert");
	std::string fullPathfragment("Shaders/sinuscolor.frag");
	sinusColor = new Shader(fullPathvertex.c_str(), fullPathfragment.c_str());

	fullPathvertex = "Shaders/vertexcolor.vert";
	fullPathfragment = "Shaders/vertexcolor.frag";
	vertexColor = new Shader(fullPathvertex.c_str(), fullPathfragment.c_str());

	fullPathvertex = "Shaders/textureSquare.vert";
	fullPathfragment = "Shaders/textureSquare.frag";
 	textureSquare = new Shader(fullPathvertex.c_str(), fullPathfragment.c_str());


	fullPathvertex = "Shaders/twotextures.vert";
	fullPathfragment = "Shaders/twotextures.frag";

	twotextures = new Shader(fullPathvertex.c_str(), fullPathfragment.c_str());

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

	texture_manager = new Texture2DManager();
	texture_manager->Init();

	std::string imagepath(SDL_GetBasePath());
	imagepath += "Assets\\Images\\puppie1.jpg";
	puppie1 = texture_manager->LoadTexture2D(imagepath.c_str());

	imagepath = SDL_GetBasePath();
	imagepath += "Assets\\Images\\puppie2.jpg";
	puppie2 = texture_manager->LoadTexture2D(imagepath.c_str());

	imagepath = SDL_GetBasePath();
	imagepath += "Assets\\Images\\container.jpg";
	container = texture_manager->LoadTexture2D(imagepath.c_str());

	imagepath = SDL_GetBasePath();
	imagepath += "Assets\\Images\\awesomeface.png";
	awesomeface = texture_manager->LoadTexture2D(imagepath.c_str());

	return ret;
}

update_status ModuleRenderer3D::PreUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	//Set background with a clear color
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	//Shaders
	if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_Q) == KEY_DOWN)
		shaderenabled = SIN;

	if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_W) == KEY_DOWN)
		shaderenabled = VERTEX;

	if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_E) == KEY_DOWN)
		shaderenabled = TEXTURE;

	if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_Z) == KEY_DOWN)
	{
		if (shaderenabled == TEXTURE)
		{
			textureSquare->use();
			if (!printvertextcolor)
			{
				textureSquare->setBool("vertexcolor", true);
				printvertextcolor = !printvertextcolor;
			}
			else
			{
				textureSquare->setBool("vertexcolor", false);
				printvertextcolor = !printvertextcolor;
			}
		}
	}

	//Textures
	if (shaderenabled == TEXTURE)
	{
		if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_A) == KEY_DOWN)
		{
			textureEnabled = PUPPIE_1;
		}

		if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_S) == KEY_DOWN)
		{
			textureEnabled = PUPPIE_2;
		}

		if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_D) == KEY_DOWN)
		{
			textureEnabled = CONTAINER;
		}

		if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_F) == KEY_DOWN)
		{
			textureEnabled = MIX_AWESOMEFACE;
			twotextures->use();
			twotextures->setInt("texture1", 1);
			twotextures->setInt("texture2", 2);
		}
	}
	//change figure
	if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_1) == KEY_DOWN)
		B_EBO = !B_EBO;

	//wired
	if (App->input->GetKey(SDL_Scancode::SDL_SCANCODE_2) == KEY_DOWN)
		isLine = !isLine;

	if (shaderenabled == SIN)
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

	switch (shaderenabled)
	{
	case SIN:
		sinusColor->use();
		break;
	case VERTEX:
		vertexColor->use();
		break;
	case TEXTURE:
		if (textureEnabled == MIX_AWESOMEFACE)
			twotextures->use();
		else
			textureSquare->use();
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
				puppie1->use();
				break;
			case PUPPIE_2:
				glActiveTexture(GL_TEXTURE0);
				puppie2->use();
				break;
			case CONTAINER:
				glActiveTexture(GL_TEXTURE0);
				container->use();
				break;
			case MIX_AWESOMEFACE:
				glActiveTexture(GL_TEXTURE1);
				container->use();
				glActiveTexture(GL_TEXTURE2);
				awesomeface->use();
				break;
			}
		}
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
	delete textureSquare;
	delete twotextures;

	//de-allocate all resources
	glDeleteVertexArrays(1, &VAO_Triangle);
	glDeleteVertexArrays(1, &VAO_Square);

	glDeleteBuffers(1, &VBO_Triangle);
	glDeleteBuffers(1, &VBO_Square);
	glDeleteBuffers(1, &EBO_Square);

	//Delete context
	SDL_GL_DeleteContext(mainContext);

	//Delete textures
	delete texture_manager;

	return ret;
}

unsigned int ModuleRenderer3D::GetMaxVertexAttributes()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	return nrAttributes;
}