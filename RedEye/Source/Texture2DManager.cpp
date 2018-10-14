#include "Texture2DManager.h"

#include "Application.h"
#include "FileSystem.h"
#include "ModuleEditor.h"
#include "OutputLog.h"

#include "Glew/include/glew.h"
#include <string>

#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"
#pragma comment(lib, "IL/libx86/DevIL.lib")
#pragma comment(lib, "IL/libx86/ILU.lib")
#pragma comment(lib, "IL/libx86/ILUT.lib")

Texture2DManager::Texture2DManager(const char * folderPath) : folderPath(folderPath) 
{}

Texture2DManager::~Texture2DManager()
{
	Texture2D* toDelete = nullptr;
	for (unsigned int TextureID : textureIDContainer) {
		toDelete = textures2D.find(TextureID)->second;
		delete toDelete;
	}
}

bool Texture2DManager::Init()
{
	LOG("Initializing Texture Manager");

	ilInit();
	iluInit();
	ilutInit();

	ILenum error = ilGetError();
	bool ret = (error == IL_NO_ERROR);
	if (ret)
		ilutRenderer(ILUT_OPENGL);
	else
		LOG_ERROR("DevIL could not initialice! DevIL Error %d - %s", error, iluErrorString(error));
	
	if (folderPath == nullptr)
	{
		LOG_ERROR("Texure Manager could not read folder path");
		ret = false;
	}

	char tmp[8];
	sprintf_s(tmp, 8, "%u.%u.%u", IL_VERSION / 100, (IL_VERSION % 100) / 10, IL_VERSION % 10);
	App->ReportSoftware("DevIL", tmp, "http://openil.sourceforge.net/");

	return ret;
}

unsigned int Texture2DManager::LoadTexture2D(const char * name, ImageExtensionType ext)
{
	std::string extension = GetExtensionStr(ext);
	std::string path(folderPath);
	path += name;
	path += extension;
	std::string file_name = name;
	file_name  += ".";
	file_name += extension;
	Texture2D* new_image = new Texture2D(path.c_str(), GetExtensionIL(extension.c_str()), file_name.c_str());

	textures2D.insert(std::pair<unsigned int, Texture2D*>(ID_count, new_image));

	textureIDContainer.push_back(ID_count);

	return ID_count++;
}

unsigned int Texture2DManager::LoadTexture2D(const char * path, const char* file_name, bool droped)
{
	Texture2D* new_image = nullptr;
	std::string filename = file_name;
	std::string extension = filename.substr(filename.find_last_of(".") + 1);
	if (droped)
		new_image = new Texture2D(std::string(path + std::string("\\") + file_name).c_str(), GetExtensionIL(extension.c_str()), file_name, droped);
	else
		new_image = new Texture2D(std::string(path + std::string("/") + file_name).c_str(), GetExtensionIL(extension.c_str()), file_name);
	
	textures2D.insert(std::pair<unsigned int, Texture2D*>(ID_count, new_image));

	textureIDContainer.push_back(ID_count);

	texturesmodified = true;

	return ID_count++;
}

void Texture2DManager::use(unsigned int TextureID)
{
	textures2D.at(TextureID)->use();
}

void Texture2DManager::drawTexture(unsigned int TextureID)
{
	textures2D.at(TextureID)->DrawTextureImGui();
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

std::vector<Texture2D*>* Texture2DManager::GetTextures()
{
	if (texturesmodified)
	{
		actualTextures.clear();
		for (unsigned int TextureID : textureIDContainer) {
			actualTextures.push_back(textures2D.find(TextureID)->second);
		}
		texturesmodified = false;
	}
	return &actualTextures;
}

unsigned int Texture2DManager::FindTMID(Texture2D * tex)
{
	unsigned int ret = 0;

	for (unsigned int TextureID : textureIDContainer) {
		if (tex == textures2D.find(TextureID)->second)
			ret = TextureID;
	}
	return ret;
}

const char * Texture2DManager::GetExtensionStr(ImageExtensionType imageType)
{
	switch (imageType)
	{
	case PNG:
		return "png";
		break;
	case JPG:
		return "jpg";
	case DDS:
		return "dds";
		break;
	}
}

int Texture2DManager::GetExtensionIL(const char* ext)
{
	int IL_Extension = 0;
	std::string exttension(ext);
	if (exttension.compare("dds") == 0)
		IL_Extension = IL_DDS;
	else if (exttension.compare("png") == 0)
		IL_Extension = IL_PNG;
	else if (exttension.compare("jpg") == 0)
		IL_Extension = IL_JPG;

	return IL_Extension;
}

Texture2D::Texture2D(const char* path, int extension, const char* name, bool droped)
{
	this->name.append(name);
	unsigned int imageID = 0;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (droped)
	{
		RE_FileIO* image = App->fs->QuickBufferFromPDPath(path);
		if (image)
		{
			ilLoadL(extension, image->GetBuffer(), image->GetSize());
			DEL(image);
		}
	}
	else
	{
		RE_FileIO image(path);
		if (image.Load())
			ilLoadL(extension, image.GetBuffer(), image.GetSize());
	}

	//iluFlipImage();

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

void Texture2D::DrawTextureImGui()
{
	ImGui::Image((void *)ID, ImVec2(200, 200));
}

const char * Texture2D::GetName()
{
	return name.c_str();
}

const unsigned int Texture2D::GetID()
{
	return ID;
}
