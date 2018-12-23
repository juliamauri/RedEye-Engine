#include "Particle.h"

#include "Application.h"
#include "TimeManager.h"
#include "RE_CompParticleEmiter.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "ShaderManager.h"
#include "RE_Mesh.h"

void Particle::Update()
{
	lifetime -= App->time->GetDeltaTime();

	if (lifetime > 0)
	{
		speed += gravity * App->time->GetDeltaTime();
		position += speed * App->time->GetDeltaTime();
		transform.SetPosition(position);
		transform.Update();
	}

	if (lifetime <= 0)
		isEmitted_flag = false;
}

bool Particle::isEmitted()
{
	return isEmitted_flag;
}

void Particle::Emit(math::vec spawn, math::vec s, math::vec g, float lt)
{
	position = spawn;
	speed = s;
	gravity = g;
	lifetime = lt;
	
	transform.SetPosition(position);
	transform.Update();

	isEmitted_flag = true;
}

void Particle::Draw(unsigned int shader)
{
	math::float4x4 model = parent_emiter->GetGO()->GetTransform()->GetMatrixModel() * transform.GetMatrixModel();
	ShaderManager::setFloat4x4(shader, "model", model.ptr());
	mesh->Draw(shader);
}

void Particle::SetUp(RE_CompParticleEmitter * pe, RE_Mesh * m)
{
	parent_emiter = pe;
	mesh = m;
}
