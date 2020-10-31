#ifndef __RE_GLCACHEMANAGER_H__
#define __RE_GLCACHEMANAGER_H__

class RE_GLCacheManager
{
public:
	static void ChangeShader(unsigned int ID);
	static void ChangeVAO(unsigned int VAO);
	static void ChangeTextureBind(unsigned int tID);
};

#endif // !__RE_GLCACHEMANAGER_H__
