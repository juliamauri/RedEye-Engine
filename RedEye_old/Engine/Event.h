#ifndef __EVENT__
#define __EVENT__

#include "RE_DataTypes.h"
#include "RE_Cvar.h"

class EventListener;

enum class RE_EventType : ushort
{
	// APP
	PLAY,
	PAUSE,
	TICK,
	STOP,

	REQUEST_LOAD,
	REQUEST_SAVE,
	REQUEST_QUIT,

	// Window
	WINDOW_MOVED,
	WINDOW_SIZE_CHANGED,

	// Scene
	GO_CHANGED_TO_ACTIVE,
	GO_CHANGED_TO_INACTIVE,
	GO_CHANGED_TO_STATIC,
	GO_CHANGED_TO_NON_STATIC,
	GO_HAS_NEW_CHILD,
	DESTROY_GO,
	TRANSFORM_MODIFIED,
	PLANE_CHANGE_TO_MESH,

	// Renderer
	SET_VSYNC,
	SET_DEPTH_TEST,
	SET_FACE_CULLING,
	SET_LIGHTNING,
	SET_TEXTURE_TWO_D,
	SET_COLOR_MATERIAL,
	SET_WIRE_FRAME,
	PARTRICLEEDITORWINDOWCHANGED,
	EDITORWINDOWCHANGED,
	GAMEWINDOWCHANGED,

	//Editor
	UPDATE_SCENE_WINDOWS,
	EDITOR_SCENE_RAYCAST,

	SCOPE_PROCEDURE_START,
	SCOPE_PROCEDURE_END,

	CONSOLE_LOG_MIN,
	CONSOLE_LOG_SEPARATOR,
	CONSOLE_LOG_GLOBAL,
	CONSOLE_LOG_SECONDARY,
	CONSOLE_LOG_TERCIARY,
	CONSOLE_LOG_SOFTWARE,
	CONSOLE_LOG_ERROR,
	CONSOLE_LOG_WARNING,
	CONSOLE_LOG_SOLUTION,
	CONSOLE_LOG_SAVE_ERROR,
	CONSOLE_LOG_SAVE_WARNING,
	CONSOLE_LOG_SAVE_SOLUTION,
	CONSOLE_LOG_MAX,

	// Resources
	RESOURCE_CHANGED,

	MAX_EVENT_TYPES
};

struct Event
{
	RE_EventType type;
	class EventListener* listener;
	RE_Cvar data1;
	RE_Cvar data2;
	const unsigned int timestamp;

	Event(RE_EventType t, EventListener* lis, RE_Cvar data = RE_Cvar(), RE_Cvar data2 = RE_Cvar());
	Event(Event& e);
	~Event() {}
	bool operator==(const Event& other) const;
};

#endif // !__EVENT__