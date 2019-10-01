#ifndef __RE_PREFAB_H__
#define __RE_PREFAB_H__

class RE_GameObject;

class RE_Prefab
{
public:
	RE_Prefab();
	~RE_Prefab();

public:

	void load();
	void unload();
	RE_GameObject* GetRoot();

private:
	char* md5;
	 //uuid

	RE_GameObject* loaded = nullptr;

	bool isInternal = false;
	bool isInMemory = false;

	char* filePath = false;
};

#endif // !__RE_PREFAB_H__