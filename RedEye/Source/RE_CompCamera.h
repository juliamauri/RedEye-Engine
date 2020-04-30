#ifndef __RE_COMPCAMERA_H__
#define __RE_COMPCAMERA_H__

#include "RE_Component.h"
#include "RE_Math.h"
#include "Globals.h"

class RE_CompTransform;

class RE_CompCamera : public RE_Component
{
public:

	enum AspectRatioTYPE : short
	{
		Fit_Window = 0,
		Square_1x1,
		TraditionalTV_4x3,
		Movietone_16x9,
		Personalized
	} target_ar = Fit_Window;

public:
	RE_CompCamera(
		RE_GameObject* go = nullptr,
		bool toPerspective = true,
		float near_plane = 1.0f,
		float far_plane = 5000.0f,
		float v_fov = 0.523599f,
		short aspect_ratio_t = 0, 
		bool draw_frustum = true,
		bool usingSkybox =  true,
		const char* skyboxMD5 = nullptr);
	RE_CompCamera(const RE_CompCamera& cmpCamera, RE_GameObject* go);
	~RE_CompCamera();
	
	void Update() override;
	void DrawProperties() override;
	void DrawAsEditorProperties();
	void DrawFrustum() const;

	RE_CompTransform* GetTransform() const;
	void OnTransformModified() override;

	void SetPlanesDistance(float near_plane, float far_plane);
	void SetFOV(float vertical_fov_degrees);
	void SetAspectRatio(AspectRatioTYPE aspect_ratio);
	void SetBounds(float width, float height);

	void ForceFOV(float vertical_fov_degrees, float horizontal_fov_degrees);

	float GetTargetWidth() const;
	float GetTargetHeight() const;
	void GetTargetWidthHeight(int &width, int &height) const;
	void GetTargetWidthHeight(float &width, float &height) const;
	void GetTargetViewPort(math::float4 &viewPort) const;

	void SetPerspective();
	void SetOrthographic();
	void SwapCameraType();

	const math::Frustum GetFrustum() const;

	float GetVFOVDegrees() const;
	float GetHFOVDegrees() const;

	math::float4x4 GetView() const;
	float* GetViewPtr() const;
	
	math::float4x4 GetProjection() const;
	float* GetProjectionPtr() const;

	bool OverridesCulling() const;

	// Camera Controls
	void LocalRotate(float dx, float dy);
	void LocalMove(Dir dir, float speed);
	void Orbit(float dx, float dy, const RE_GameObject& focus);
	void Focus(const RE_GameObject* focus, float min_dist = 3.0f);
	void Focus(math::vec center, float radius = 1, float min_dist = 3.0f);

	// local camera Axis
	math::vec GetRight() const;
	math::vec GetUp() const;
	math::vec GetFront() const;

	eastl::vector<const char*> GetAllResources() override;

	unsigned int GetBinarySize()const override;
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) override;

	//Skybox
	bool isUsingSkybox()const;
	void DrawSkybox()const;

	void UseResources()override;
	void UnUseResources()override;

private:
	void RecalculateMatrixes();
	void DrawItsProperties();

private:

	// Transform
	RE_CompTransform* transform = nullptr;

	// Axis
	math::vec right = math::vec::zero;
	math::vec up = math::vec::zero;
	math::vec front = math::vec::zero;

	// Focus
	math::vec focus_global_pos = math::vec::zero;

	// Camera frustum
	math::Frustum frustum;

	// Panes
	float near_plane = 0.0f;
	float far_plane = 0.0f;

	// Field of View
	bool isPerspective = true;
	float h_fov_rads = 0.0f;
	float v_fov_rads = 30.0f;
	float h_fov_degrees = 0.0f;
	float v_fov_degrees = 0.0f;

	// Aspect Ratio
	float width = 0.f;
	float height = 0.f;

	// View & Projection
	bool need_recalculation = false;
	math::float4x4 calculated_view;
	math::float4x4 calculated_projection;

	// Debug Drawing
	bool draw_frustum = true;
	bool override_cull = false;

	//skybox
	bool usingSkybox = true;
	const char* skyboxMD5 = nullptr;
};

#endif // !__RE_CCOMPAMERA_H__