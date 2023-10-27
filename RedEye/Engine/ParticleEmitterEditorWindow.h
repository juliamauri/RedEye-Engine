#ifndef __PARTICLE_EMITTER_EDITOR_WINDOW__
#define __PARTICLE_EMITTER_EDITOR_WINDOW__

#include "RenderedWindow.h"

class RE_ParticleEmitterBase;

class ParticleEmitterEditorWindow : public OwnCameraRenderedWindow
{
private:

	const char* emiter_md5 = nullptr;
	P_UID simulation = 0;
	RE_ParticleEmitterBase* new_emitter = nullptr;

	bool load_next = false;
	const char* next_emiter_md5 = nullptr;
	P_UID next_simulation = 0;

	bool docking = false;
	bool need_save = false;

public:

	ParticleEmitterEditorWindow();
	~ParticleEmitterEditorWindow() final = default;

	void Orbit(float delta_x, float delta_y) final;
	void Focus() final;

	void StartEditing(P_UID sim, const char* md5);
	const P_UID GetEdittingParticleEmitter() const { return simulation; }

	void SaveEmitter(
		bool close = false,
		const char* emitter_name = nullptr,
		const char* emissor_base = nullptr,
		const char* renderer_base = nullptr);
	
	void NextOrClose();
	void CloseEditor();
	void LoadNextEmitter();

private:

	void Draw(bool secondary = false) final;
};

#endif // !__PARTICLE_EMITTER_EDITOR_WINDOW__