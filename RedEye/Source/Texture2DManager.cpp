#include "Texture2DManager.h"

#include "Application.h"
#include "ResourceManager.h"
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

#include "md5.h"

Texture2DManager::Texture2DManager(const char * folderPath) : folderPath(folderPath) 
{}

Texture2DManager::~Texture2DManager()
{
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

const char* Texture2DManager::LoadTexture2D(const char * name, ImageExtensionType ext)
{
	std::string extension = GetExtensionStr(ext);
	std::string path(folderPath);
	path += name;
	path += extension;
	std::string file_name = name;
	file_name  += ".";
	file_name += extension;
	Texture2D* new_image = ProcessTexture(path.c_str(), GetExtensionIL(extension.c_str()), file_name.c_str());

	if (new_image)
	{
		ResourceContainer* texure_resource = (ResourceContainer*)new_image;
		texure_resource->SetType(Resource_Type::R_TEXTURE);
		texure_resource->SetMD5(md5_genereted.c_str());
		return App->resources->Reference(texure_resource);
	}
	else
		return App->resources->At(exists_md5.c_str())->GetMD5();
}

const char* Texture2DManager::LoadTexture2D(const char * path, const char* file_name, bool droped)
{
	Texture2D* new_image = nullptr;
	std::string filename = file_name;
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	if (filename.find("\\") > 0 && !droped)
	{
		filename = filename.substr(filename.find_last_of("\\") + 1);
		new_image = ProcessTexture(std::string(path + std::string("/") + filename).c_str(), GetExtensionIL(extension.c_str()), file_name);
	}
	else
	{
		if (droped)
			new_image = ProcessTexture(std::string(path + std::string("\\") + file_name).c_str(), GetExtensionIL(extension.c_str()), file_name, droped);
		else
			new_image = ProcessTexture(std::string(path + std::string("/") + file_name).c_str(), GetExtensionIL(extension.c_str()), file_name);
	}

	if (new_image)
	{
		ResourceContainer* texure_resource = (ResourceContainer*)new_image;
		texure_resource->SetType(Resource_Type::R_TEXTURE);
		texure_resource->SetMD5(md5_genereted.c_str());
		texure_resource->SetFilePath(std::string(path + std::string("/") + filename).c_str());
		return App->resources->Reference(texure_resource);
	}
	else
		return App->resources->At(exists_md5.c_str())->GetMD5();
}

void Texture2DManager::LoadTexture2D(const char * path, bool from_Library, const char* assets_file)
{
	Texture2D* new_image = nullptr;
	std::string file_path(path);
	std::string filename = file_path.substr(file_path.find_last_of("/") + 1);;
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	new_image = ProcessTexture(file_path.c_str(), (from_Library) ? IL_DDS : GetExtensionIL(extension.c_str()), filename.c_str(), false, from_Library);

	if (new_image)
	{
		ResourceContainer* texure_resource = (ResourceContainer*)new_image;
		texure_resource->SetType(Resource_Type::R_TEXTURE);
		texure_resource->SetMD5(md5_genereted.c_str());
		texure_resource->SetFilePath((!from_Library) ? path : assets_file);
		App->resources->Reference(texure_resource);
	}
}

void Texture2DManager::use(const char* TextureID)
{
	((Texture2D*)App->resources->At(TextureID))->use();
}

void Texture2DManager::use(Texture2D * TextureID)
{
	TextureID->use();
}

void Texture2DManager::drawTexture(const char* TextureID)
{
	((Texture2D*)App->resources->At(TextureID))->DrawTextureImGui();
}

void Texture2DManager::drawTexture(Texture2D * TextureID)
{
	TextureID->DrawTextureImGui();
}

void Texture2DManager::GetWithHeight(const char* TextureID, int * w, int * h)
{
	((Texture2D*)App->resources->At(TextureID))->GetWithHeight(w, h);
}

void Texture2DManager::DeleteTexture2D(const char* TextureID)
{
	//App->resources->UnReference(TextureID);
}

Texture2D * Texture2DManager::ProcessTexture(const char * path, int extension, const char * name, bool droped, bool from_Library)
{
	uint ID = 0;
	int width = 0;
	int height = 0;

	const char* is_reference = nullptr;
	Texture2D* texture = nullptr;

	unsigned int imageID = 0;

	if (droped)
	{
		RE_FileIO* image = App->fs->QuickBufferFromPDPath(path);
		if (image)
		{
			md5_genereted = md5(path);
			is_reference = App->resources->IsReference(md5_genereted.c_str());
			if (is_reference)
				exists_md5 = is_reference;
			else
			{
				ilGenImages(1, &imageID);
				ilBindImage(imageID);

				ilLoadL(extension, image->GetBuffer(), image->GetSize());
				DEL(image);
			}
		}
	}
	else
	{
		RE_FileIO image(path);
		if (image.Load())
		{
			if (!from_Library)
				md5_genereted = md5(path);
			else
			{
				std::string fullname(name);
				size_t lastindex = fullname.find_last_of(".");
				md5_genereted = fullname.substr(0, lastindex);
			}
			is_reference = App->resources->IsReference(md5_genereted.c_str());
			if (is_reference)
				exists_md5 = is_reference;
			else
			{
				ilGenImages(1, &imageID);
				ilBindImage(imageID);

				ilLoadL(extension, image.GetBuffer(), image.GetSize());

				if (!from_Library)
				{
					if (extension == IL_TGA)
						ilFlipSurfaceDxtcData();

					//Save into dds
					ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
					ILubyte *data = new ILubyte[size];

					ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
					std::string save_path("Library/Images/");
					save_path += md5_genereted;
					save_path += ".eye";
					RE_FileIO save(save_path.c_str(), App->fs->GetZipPath());
					save.Save((char*)data, size);
				}
			}
		}
	}

	if (!is_reference)
	{
		if (extension == IL_TGA)
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

		texture = new Texture2D(ID, width, height);
	}

	return texture;
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
	if (exttension.compare("dds") == 0 || exttension.compare("eye") == 0)
		IL_Extension = IL_DDS;
	else if (exttension.compare("png") == 0)
		IL_Extension = IL_PNG;
	else if (exttension.compare("jpg") == 0)
		IL_Extension = IL_JPG;
	else if (exttension.compare("tga") == 0)
		IL_Extension = IL_TGA;

	return IL_Extension;
}

Texture2D::Texture2D(unsigned int ID, int widht, int height)
{
	this->ID = ID;
	this->width = widht;
	this->height = height;
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