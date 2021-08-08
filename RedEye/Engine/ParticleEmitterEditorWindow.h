#ifndef __PARTICLE_EMITTER_EDITOR_WINDOW__
#define __PARTICLE_EMITTER_EDITOR_WINDOW__

#include "EditorWindow.h"
#include <EASTL/string.h>
#include <MGL/Math/float4.h>

class RE_ParticleEmitter;
class RE_ParticleEmitterBase;

class ParticleEmitterEditorWindow : public EditorWindow
{
public:

	ParticleEmitterEditorWindow() : EditorWindow("Particle Emitter Workspace", false) {}
	~ParticleEmitterEditorWindow() {}

	void StartEditing(RE_ParticleEmitter* sim, const char* md5);
	const RE_ParticleEmitter* GetEdittingParticleEmitter()const { return simulation; }

	void SaveEmitter(bool close = false, const char* emitter_name = nullptr, const char* emissor_base = nullptr, const char* renderer_base = nullptr);
	void NextOrClose();
	void CloseEditor();
	void LoadNextEmitter();

	unsigned int GetSceneWidht()const { return (width == 0) ? 500 : width; }
	unsigned int GetSceneHeight()const { return (heigth == 0) ? 500 : heigth; }

	bool isSelected()const { return isWindowSelected; }

	void UpdateViewPort();
	void Recalc() { recalc = true; }

private:

	void Draw(bool secondary = false) override;

private:

	const char* emiter_md5 = nullptr;
	RE_ParticleEmitter* simulation = nullptr;
	RE_ParticleEmitterBase* new_emitter = nullptr;

	bool load_next = false;
	const char* next_emiter_md5 = nullptr;
	RE_ParticleEmitter* next_simulation = nullptr;

	math::float4 viewport = math::float4::zero;
	int width = 0;
	int heigth = 0;

	bool isWindowSelected = false;
	bool recalc = false;

	bool docking = false;
	bool need_save = false;
};

#endif // !__PARTICLE_EMITTER_EDITOR_WINDOW__