#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

#include "RE_Component.h"

class RE_CompMesh : public RE_Component
{
public:
	RE_CompMesh(RE_GameObject* go = nullptr, const char *path = nullptr, const bool file_dropped = false, const bool start_active = true);
	RE_CompMesh(RE_GameObject* go = nullptr, unsigned int reference = 0u, const bool start_active = true);
	~RE_CompMesh();

	unsigned int LoadMesh(const char* path, const bool dropped = false);

	void Draw() override;

	void DrawProperties() override;

protected:

	unsigned int reference = 0u;


};


#endif // !__RE_COMPMESH_H__