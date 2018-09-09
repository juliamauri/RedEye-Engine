#include "Texture2DManager.h"

#include "Glew/include/glew.h"

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
	unsigned int imageID = 0;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	ilLoadImage(path);
	iluFlipImage();

	/* OpenGL texture binding of the image loaded by DevIL  */
	glGenTextures(1, &ID); /* Texture name generation */
	glBindTexture(GL_TEXTURE_2D, ID); /* Binding of texture name */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear interpolation for magnification filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear interpolation for minifying filter */
	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), width = ilGetInteger(IL_IMAGE_WIDTH), height = ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */

	ilBindImage(0);
	/* Delete used resources*/
	ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &ID);
}

void Texture2D::use()
{
	glBindTexture(GL_TEXTURE_2D, ID);
}
