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
	lifetime += App->time->GetDeltaTime();

	if (lifetime > 0)
	{
		/*speed += gravity * App->time->GetDeltaTime();
		position += speed * App->time->GetDeltaTime();
		transform.SetPosition(position);
		transform.Update();*/

	}
}

bool Particle::Alive()
{
	return current_lifetime < max_lifetime;
}

void Particle::Emit()
{
	lifetime = 100.0f;
	current_lifetime = 0.0f;

	right = math::vec(1.f, 0.f, 0.f);
	up = math::vec(0.f, 1.f, 0.f);
	front = math::vec(0.f, 0.f, 1.f);

	position = math::vec::zero;
}

void Particle::Draw(unsigned int shader)
{
	RE_Mesh* mesh = parent_emiter->GetMesh();

	if (mesh != nullptr)
	{
		math::float4x4 transform_matrix = math::float4x4(
			right.x, up.x, front.x, position.x,
			right.y, up.y, front.y, position.y,
			right.z, up.z, front.z, position.z,
			0.f, 0.f, 0.f, 1.f);

		if (!parent_emiter->LocalEmission())
			transform_matrix = transform_matrix * parent_emiter->GetGO()->GetTransform()->GetMatrixModel();

		ShaderManager::setFloat4x4(shader, "model", transform_matrix.ptr());
		mesh->Draw(shader);
	}
}

void Particle::SetUp(RE_CompParticleEmitter * pe)
{
	parent_emiter = pe;
}
