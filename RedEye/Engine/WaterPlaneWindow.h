#ifndef __WATER_PLANE_WINDOW__
#define __WATER_PLANE_WINDOW__

class WaterPlaneWindow : public EditorWindow
{
public:

	WaterPlaneWindow();
	~WaterPlaneWindow() {}

private:

	void Draw(bool secondary = false) override;

private:

	eastl::string waterResouceName = "WaterMaterial";
	bool deferred = false;
};

#endif // !__WATER_PLANE_WINDOW__