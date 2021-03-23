#include "ModulePhysics.h"

ModulePhysics::ModulePhysics() : Module("Physics") {}
ModulePhysics::~ModulePhysics() {}

bool ModulePhysics::Init()
{
	return true;
}

bool ModulePhysics::Start()
{
	return true;
}

void ModulePhysics::Update()
{
}

void ModulePhysics::CleanUp()
{
	particles.Clear();
}
