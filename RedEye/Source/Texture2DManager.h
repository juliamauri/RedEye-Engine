#ifndef __TEXTURE2DMANAGER_H__
#define __TEXTURE2DMANAGER_H__

#include <list>

struct Texture2D
{
private:
	unsigned int ID = 0;
	int width, height;

public:
	Texture2D(const char* path);

	const unsigned char* GetData();
};

class Texture2DManager 
{
public:
	~Texture2DManager();

	bool Init();

	//&width, &height, &nrChannels
	Texture2D* LoadTexture2D(const char* path);

	bool CleanUp();

private:
	std::list<Texture2D*> textures2d;
};




#endif // !__TEXTURE2DMANAGER_H__