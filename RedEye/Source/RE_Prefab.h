#ifndef __RE_PREFAB_H__
#define __RE_PREFAB_H__

class RE_GameObject;
#include "Resource.h"

#include <string>

//Internal is named .efab
//external is named .refab
class RE_Prefab : public ResourceContainer
{
public:
	RE_Prefab(RE_GameObject* toBePrefab, bool isInternal = false);
	~RE_Prefab();

public:
	//returns a new, needed destroy after use.
	RE_GameObject* GetRoot();

private:
	void Load();
	void Unload();

private:
	RE_GameObject* loaded = nullptr;
	bool isInternal = false;
};

#endif // !__RE_PREFAB_H__