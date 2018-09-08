#include "Texture2DManager.h"

#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"
#pragma comment(lib, "IL/libx86/DevIL.lib")
#pragma comment(lib, "IL/libx86/ILU.lib")
#pragma comment(lib, "IL/libx86/ILUT.lib")

Texture2DManager::~Texture2DManager()
{
	CleanUp();
}

bool Texture2DManager::Init()
{
	ilInit();
	iluInit();
	ilutInit();
	//before any use ilut functions
	ilutRenderer(ILUT_OPENGL);

	return true;
}

Texture2D * Texture2DManager::LoadTexture2D(const char * path)
{
	Texture2D* new_image = new Texture2D(path);
	textures2d.push_back(new_image);
	return new_image;
}

bool Texture2DManager::CleanUp()
{
	while(!textures2d.empty())
	{
		delete textures2d.back();
		textures2d.pop_back();
	}
	return true;
}

Texture2D::Texture2D(const char * path)
{
	ilGenImages(1, &ID);
	ilBindImage(ID);

	ilLoadImage(path);

	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);

	ilBindImage(0);
}

const unsigned char * Texture2D::GetData()
{
	ilBindImage(ID);
	unsigned char* data = ilGetData();
	ilBindImage(0);
	return data;
}
