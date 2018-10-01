#include "Texture2DManager.h"

#include "Application.h"
#include "FileSystem.h"

#include "Glew/include/glew.h"
#include <string>

#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"
#pragma comment(lib, "IL/libx86/DevIL.lib")
#pragma comment(lib, "IL/libx86/ILU.lib")
#pragma comment(lib, "IL/libx86/ILUT.lib")

Texture2DManager::Texture2DManager(const char * folderPath) : folderPath(folderPath) 
{
	ilutRenderer(ILUT_OPENGL);
	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);
}

Texture2DManager::~Texture2DManager()
{
	Texture2D* toDelete = nullptr;
	for (unsigned int TextureID : textureIDContainer) {
		toDelete = textures2D.find(TextureID)->second;
		delete toDelete;
	}
}

unsigned int Texture2DManager::LoadTexture2D(const char * name, ImageExtensionType extension)
{
	std::string path(folderPath);
	path += name;
	path += GetExtensionStr(extension);

	Texture2D* new_image = new Texture2D(path.c_str(), GetExtensionIL(extension));

	textures2D.insert(std::pair<unsigned int, Texture2D*>(ID_count, new_image));

	textureIDContainer.push_back(ID_count);

	return ID_count++;
}

unsigned int Texture2DManager::LoadTexture2D(const char * path, const char* file_name, bool droped)
{
	Texture2D* new_image = nullptr;
	if (droped)
		new_image = new Texture2D(std::string(path + std::string("\\") + file_name).c_str(), GetExtensionIL(ImageExtensionType::PNG), droped);
	else
		new_image = new Texture2D(std::string(path + std::string("/") + file_name).c_str(), GetExtensionIL(ImageExtensionType::PNG));
	
	textures2D.insert(std::pair<unsigned int, Texture2D*>(ID_count, new_image));

	textureIDContainer.push_back(ID_count);

	return ID_count++;
}

void Texture2DManager::use(unsigned int TextureID)
{
	textures2D.at(TextureID)->use();
}

void Texture2DManager::GetWithHeight(unsigned int TextureID, int * w, int * h)
{
	textures2D.at(TextureID)->GetWithHeight(w, h);
}

void Texture2DManager::DeleteTexture2D(unsigned int TextureID)
{
	Texture2D* toDelete = textures2D.find(TextureID)->second;
	delete toDelete;
	textureIDContainer.remove(TextureID);
}

const char * Texture2DManager::GetExtensionStr(ImageExtensionType imageType)
{
	switch (imageType)
	{
	case PNG:
		return ".png";
		break;
	case JPG:
		return ".jpg";
		break;
	}
}

int Texture2DManager::GetExtensionIL(ImageExtensionType imageType)
{
	int IL_Extension = 0;
	switch (imageType)
	{
	case PNG:
		IL_Extension = IL_PNG;
		break;
	case JPG:
		IL_Extension = IL_JPG;
		break;
	}
	return IL_Extension;
}

Texture2D::Texture2D(const char* path, int extension, bool droped)
{

	unsigned int imageID = 0;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (droped)
	{
		RE_FileIO* image = App->fs->QuickBufferFromPDPath(path);
		ilLoadL(extension, image->GetBuffer(), image->GetSize());
	}
	else
	{
		RE_FileIO image(path);
		if (image.Load())
			ilLoadL(extension, image.GetBuffer(), image.GetSize());
	}

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

void Texture2D::GetWithHeight(int * w, int * h)
{
	*w = width;
	*h = height;
}
