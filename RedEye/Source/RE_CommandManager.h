#ifndef __RE_COMMAND_MANAGER__
#define __RE_COMMAND_MANAGER__

#include <EASTL/stack.h>

class RE_Command;

namespace RE_CommandManager
{
	bool CanRedo();
	bool CanUndo();
	void Redo();
	void Undo();
	void PushCommand(RE_Command* newCommand);
	void Clear();
	
	namespace Internal
	{
		static unsigned int max_commands = 100u;
		static eastl::stack<RE_Command*> undoCommands, redoCommands;
	};
};

#endif // !__RE_COMMAND_MANAGER__