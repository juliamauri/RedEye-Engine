#ifndef __GLCACHE_H__
#define __GLCACHE_H__

class RE_GLCache
{
public:
	static void ChangeShader(unsigned int ID);
	static void ChangeVAO(unsigned int VAO);
	static void ChangeTextureBind(unsigned int tID);
};

#endif // !__GLCACHE_H__
