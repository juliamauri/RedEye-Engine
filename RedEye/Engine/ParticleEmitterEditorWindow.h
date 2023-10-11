#ifndef __PARTICLE_EMITTER_EDITOR_WINDOW__
#define __PARTICLE_EMITTER_EDITOR_WINDOW__

#include "RenderedWindow.h"

class RE_ParticleEmitter;
class RE_ParticleEmitterBase;

class ParticleEmitterEditorWindow : public RenderedWindow
{
public:

	ParticleEmitterEditorWindow() : RenderedWindow("Particle Emitter Workspace", false) {}
	~ParticleEmitterEditorWindow() final = default;

	void StartEditing(RE_ParticleEmitter* sim, const char* md5);
	const RE_ParticleEmitter* GetEdittingParticleEmitter() const { return simulation; }

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

private:

	const char* emiter_md5 = nullptr;
	RE_ParticleEmitter* simulation = nullptr;
	RE_ParticleEmitterBase* new_emitter = nullptr;

	bool load_next = false;
	const char* next_emiter_md5 = nullptr;
	RE_ParticleEmitter* next_simulation = nullptr;

	bool docking = false;
	bool need_save = false;
};

#endif // !__PARTICLE_EMITTER_EDITOR_WINDOW__