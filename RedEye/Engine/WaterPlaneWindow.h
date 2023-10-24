#ifndef __WATER_PLANE_WINDOW__
#define __WATER_PLANE_WINDOW__

class WaterPlaneWindow : public EditorWindow
{
private:

	eastl::string waterResouceName = "WaterMaterial";
	bool deferred = false;

public:

	WaterPlaneWindow() : EditorWindow("Water As Resource", false) {}
	~WaterPlaneWindow() = default;

private:

	void Draw(bool secondary = false) override;
};

#endif // !__WATER_PLANE_WINDOW__