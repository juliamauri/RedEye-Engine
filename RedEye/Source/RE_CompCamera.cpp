#include "RE_CompCamera.h"

#include "Application.h"
#include "RE_GameObject.h"
#include "FileSystem.h"
#include "RE_CompTransform.h"
#include "ModuleWindow.h"
#include "SDL2\include\SDL_opengl.h"

RE_CompCamera::RE_CompCamera(RE_GameObject* go, bool toPerspective, float near_plane, float far_plane)
	: RE_Component(C_CAMERA, go), isPerspective(toPerspective)
{
	transform = (go == nullptr) ? new RE_CompTransform() : go->GetTransform();

	frustum.SetKind(
		math::FrustumProjectiveSpace::FrustumSpaceGL,
		math::FrustumHandedness::FrustumRightHanded);

	if (isPerspective)
		frustum.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	else
		frustum.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());

	frustum.SetWorldMatrix(math::float3x4::Translate(0.f, 0.f, 0.f));
	frustum.SetViewPlaneDistances(near_plane, far_plane);

	SetVerticalFOV(30.f);

	RecalculateMatrixes();
}

RE_CompCamera::~RE_CompCamera()
{
	if(go == nullptr)
		DEL(transform);
}

void RE_CompCamera::Update()
{
	if (go == nullptr)
	{
		transform->Update();

		if (transform->HasChanged())
			OnTransformModified();
	}
	
	if (need_recalculation)
		RecalculateMatrixes();
}

void RE_CompCamera::Draw()
{
	if (draw_frustum)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glLineWidth(2.0f);
		glColor4f(0.f, 1.0f, 0.2f, 1.0f);

		for (uint i = 0; i < 12; i++)
		{
			glVertex3f(frustum.Edge(i).a.x, frustum.Edge(i).a.y, frustum.Edge(i).a.z);
			glVertex3f(frustum.Edge(i).b.x, frustum.Edge(i).b.y, frustum.Edge(i).b.z);
		}

		glEnd();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void RE_CompCamera::DrawProperties()
{
}


void RE_CompCamera::SetEulerAngle(float p, float y)
{
	yaw += y;
	pitch += p;

	transform->SetLocalRot(math::vec(pitch,yaw,0.0f));
}

RE_CompTransform * RE_CompCamera::GetTransform() const
{
	return go != nullptr ? go->GetTransform() : transform;
}

void RE_CompCamera::OnTransformModified()
{
	math::float4x4 global_transform = transform->GetGlobalMatrix();
	/*frustum.SetFrame(
		global_transform.Col3(3),
		global_transform.Col3(2).Normalized(),
		global_transform.Col3(1).Normalized());*/

	frustum.SetWorldMatrix(global_transform.Float3x4Part());

	need_recalculation = true;
}

void RE_CompCamera::SetPlanesDistance(float near_plane, float far_plane)
{
	frustum.SetViewPlaneDistances(near_plane, far_plane);
}

void RE_CompCamera::ResetAspectRatio()
{
	if (isPerspective)
		frustum.SetVerticalFovAndAspectRatio(v_fov_rads, App->window->GetAspectRatio());
	else
		frustum.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());

	need_recalculation = true;
}

void RE_CompCamera::SwapCameraType()
{
	if (isPerspective)
		frustum.SetOrthographic((float)App->window->GetWidth(), (float)App->window->GetHeight());
	else
		frustum.SetPerspective(1.0f, (float)App->window->GetHeight() / (float)App->window->GetWidth());
	
	isPerspective = !isPerspective;

	need_recalculation = true;
}

void RE_CompCamera::SetVerticalFOV(float vertical_fov_degrees)
{
	if (isPerspective)
	{
		RE_CAPTO(vertical_fov_degrees, 180.0f);

		v_fov_rads = vertical_fov_degrees * DEGTORAD;
		h_fov_rads = 2.0f * math::Atan(math::Tan(v_fov_rads / 2.0f) * App->window->GetAspectRatio());

		h_fov_degrees = h_fov_rads * RADTODEG;
		v_fov_degrees = vertical_fov_degrees;

		frustum.SetPerspective(h_fov_rads, v_fov_rads);

		need_recalculation = true;
	}
}

math::float4x4 RE_CompCamera::GetView() const
{
	return calculated_view;
}

float* RE_CompCamera::GetViewPtr() const
{
	return (float*)calculated_view.v;
}

math::float4x4 RE_CompCamera::GetProjection() const
{
	return calculated_projection;
}

float* RE_CompCamera::GetProjectionPtr() const
{
	return (float*)calculated_projection.v;
}

void RE_CompCamera::RecalculateMatrixes()
{
	calculated_view = frustum.ViewMatrix();
	calculated_view.Transpose();

	calculated_projection = frustum.ProjectionMatrix();
	calculated_projection.Transpose();

	need_recalculation = false;
}

float RE_CompCamera::GetVFOVDegrees() const
{
	return v_fov_degrees;
}

void RE_CompCamera::Serialize(JSONNode * node, rapidjson::Value * comp_array)
{
	rapidjson::Value val(rapidjson::kObjectType);

	val.AddMember(rapidjson::Value::StringRefType("type"), rapidjson::Value().SetInt((int)type), node->GetDocument()->GetAllocator());
	
	val.AddMember(rapidjson::Value::StringRefType("isPrespective"), rapidjson::Value().SetBool(isPerspective), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("pitch"), rapidjson::Value().SetFloat(pitch), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("yaw"), rapidjson::Value().SetFloat(yaw), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("roll"), rapidjson::Value().SetFloat(roll), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("h_fov_rads"), rapidjson::Value().SetFloat(h_fov_rads), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("v_fov_rads"), rapidjson::Value().SetFloat(v_fov_rads), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("h_fov_degrees"), rapidjson::Value().SetFloat(h_fov_degrees), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("v_fov_degrees"), rapidjson::Value().SetFloat(v_fov_degrees), node->GetDocument()->GetAllocator());


	rapidjson::Value float_array(rapidjson::kArrayType);

	float_array.PushBack(GetTransform()->GetLocalPosition().x, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetGlobalPosition().y, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetGlobalPosition().z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("position"), float_array.Move(), node->GetDocument()->GetAllocator());

	float_array.SetArray();
	float_array.PushBack(GetTransform()->GetLocalRotXYZ().x, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalRot().y, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalRot().z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("rotation"), float_array.Move(), node->GetDocument()->GetAllocator());

	float_array.SetArray();
	float_array.PushBack(GetTransform()->GetLocalRot().x, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetGlobalScale().y, node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetGlobalScale().z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("scale"), float_array.Move(), node->GetDocument()->GetAllocator());

	comp_array->PushBack(val, node->GetDocument()->GetAllocator());
}
