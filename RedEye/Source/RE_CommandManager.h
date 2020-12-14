#ifndef __RE_COMMAND_MANAGER_H__
#define __RE_COMMAND_MANAGER_H__

#include <EASTL/stack.h>

class RE_Command;

class RE_CommandManager
{
public:
	RE_CommandManager() {}
	~RE_CommandManager() { Clear(); }

	void Redo();
	void Undo();

	void PushCommand(RE_Command* newCommand);
	void Clear();

	bool CanRedo() const { return !redoCommands.empty(); }
	bool CanUndo() const { return !undoCommands.empty(); }

private:

	eastl::stack<RE_Command*> undoCommands;
	eastl::stack<RE_Command*> redoCommands;
};

#endif // !__RE_COMMAND_MANAGER_H__