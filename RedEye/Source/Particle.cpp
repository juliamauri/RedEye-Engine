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
		/*speed += gravity * App->time->GetDeltaTime();
		position += speed * App->time->GetDeltaTime();
		transform.SetPosition(position);
		transform.Update();*/
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
	/*position = spawn;
	speed = s;
	gravity = g;
	lifetime = lt;
	
	transform.SetPosition(position);
	transform.Update();*/

	isEmitted_flag = true;
}

void Particle::Draw(unsigned int shader)
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

void Particle::SetUp(RE_CompParticleEmitter * pe, RE_Mesh * m)
{
	parent_emiter = pe;
	mesh = m;
}
