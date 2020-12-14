#include "Particle.h"

#include "RE_CompParticleEmiter.h"

void Particle::Update()
{
	//lifetime += RE_TIME->GetDeltaTime();

	if (lifetime > 0)
	{
		/*speed += gravity * TimeManager::GetDeltaTime();
		position += speed * TimeManager::GetDeltaTime();
		transform.SetPosition(position);
		transform.Update();*/
	}
}

bool Particle::Alive()
{
	return current_lifetime < lifetime;
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
	//RE_Mesh* mesh = parent_emiter->GetMesh();

	/*if (mesh != nullptr)
	{
		math::float4x4 transform_matrix = math::float4x4(
			right.x, up.x, front.x, position.x,
			right.y, up.y, front.y, position.y,
			right.z, up.z, front.z, position.z,
			0.f, 0.f, 0.f, 1.f);
		transform_matrix.Transpose();

		if (parent_emiter->LocalEmission())
			transform_matrix = transform_matrix * parent_emiter->GetGOUID()->GetTransformPtr()->GetGlobalMatrix();

		RE_ShaderImporter::setFloat4x4(shader, "model", transform_matrix.ptr());
		//mesh->Draw(shader);
	}*/
}

void Particle::SetUp(RE_CompParticleEmitter * pe)
{
	parent_emiter = pe;
	lifetime = 0.0f;
}
