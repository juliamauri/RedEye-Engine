#include "RE_TextureImporter.h"

#include "Application.h"
#include "ResourceManager.h"
#include "FileSystem.h"

#include "Globals.h"
#include "OutputLog.h"

#include "md5.h"

#include "ImGui\imgui.h"

#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"

#pragma comment(lib, "IL/libx86/DevIL.lib")
#pragma comment(lib, "IL/libx86/ILU.lib")
#pragma comment(lib, "IL/libx86/ILUT.lib")



RE_TextureImporter::RE_TextureImporter(const char* folderPath) : folderPath(folderPath) {}

RE_TextureImporter::~RE_TextureImporter() {}

bool RE_TextureImporter::Init()
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

const char * RE_TextureImporter::LoadTextureAssets(const char * assetsPath)
{
	const char* exists = nullptr;
	std::string file_path(assetsPath);
	std::string filename = file_path.substr(file_path.find_last_of("/") + 1);
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_FileIO file(assetsPath);
	if (file.Load())
	{
		std::string md5Generated = md5(assetsPath);
		exists = App->resources->IsReference(md5Generated.c_str());
		if (exists == nullptr) {
			Texture2D * newTexture = ProcessTexture(&file, GetExtensionIL(extension.c_str()), true, md5Generated.c_str());
			newTexture->SetFilePath(assetsPath);
			newTexture->SetMD5(md5Generated.c_str());
			newTexture->SetType(Resource_Type::R_TEXTURE);
			exists = App->resources->Reference(newTexture);
		}
	}
	return exists;
}

const char * RE_TextureImporter::LoadTextureLibrary(const char * libraryPath, const char * assetsPath)
{
	const char* exists = nullptr;

	RE_FileIO file(libraryPath);
	if (file.Load())
	{
		std::string md5Generated = md5(assetsPath);
		exists = App->resources->IsReference(md5Generated.c_str());
		if (exists == nullptr) {
			Texture2D * newTexture = ProcessTexture(&file, IL_DDS);
			newTexture->SetFilePath(assetsPath);
			newTexture->SetMD5(md5Generated.c_str());
			newTexture->SetType(Resource_Type::R_TEXTURE);
			exists = App->resources->Reference(newTexture);
		}
	}

	return exists;
}

Texture2D * RE_TextureImporter::ProcessTexture(RE_FileIO * fileTexture, int ILextension, bool generateOwnFormat, const char* md5Generated)
{
	uint imageID = 0;
	uint ID = 0;
	int width = 0;
	int height = 0;

	ilGenImages(1, &imageID);
	ilBindImage(imageID);
	
	ilLoadL(ILextension, fileTexture->GetBuffer(), fileTexture->GetSize());
	
	if (generateOwnFormat)
	{
		if (ILextension == IL_TGA)
			ilFlipSurfaceDxtcData();

		//Save into dds
		ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
		ILubyte *data = new ILubyte[size];
	
		ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
		std::string save_path("Library/Images/");
		save_path += md5Generated;
		save_path += ".eye";
		RE_FileIO save(save_path.c_str(), App->fs->GetZipPath());
		save.Save((char*)data, size);
	}
	
	if (ILextension == IL_TGA)
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
	
	return new Texture2D(ID, width, height);
}

int RE_TextureImporter::GetExtensionIL(const char * extension)
{
	if (std::strcmp(extension,"dds") == 0 || std::strcmp(extension, "eye") == 0)
		return IL_DDS;
	else if (std::strcmp(extension, "png") == 0)
		return IL_PNG;
	else if (std::strcmp(extension, "jpg") == 0)
		return IL_JPG;
	else if (std::strcmp(extension, "tga") == 0)
		return IL_TGA;
	return -1;
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