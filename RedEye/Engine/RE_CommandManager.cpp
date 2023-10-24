#include "RE_CommandManager.h"

#include "RE_Memory.h"
#include "RE_Command.h"

#include <EASTL/stack.h>

constexpr size_t MAX_COMMANDS = 100;

eastl::stack<RE_Command*> undoCommands;
eastl::stack<RE_Command*> redoCommands;

void RE_CommandManager::Redo()
{
	if (redoCommands.empty()) return;

	RE_Command* cmd = redoCommands.top();
	cmd->execute();
	undoCommands.push(cmd);
	redoCommands.pop();
}

void RE_CommandManager::Undo()
{
	if (undoCommands.empty()) return;

	RE_Command* cmd = undoCommands.top();
	cmd->Undo();
	redoCommands.push(cmd);
	undoCommands.pop();
}

void RE_CommandManager::PushCommand(RE_Command* newCommand)
{
	while (!redoCommands.empty())
	{
		DEL(redoCommands.top())
		redoCommands.pop();
	}

	if (undoCommands.size() == MAX_COMMANDS)
	{
		DEL(*undoCommands.c.begin())
		undoCommands.c.erase(undoCommands.c.begin());
	}
	
	undoCommands.push(newCommand);
}

void RE_CommandManager::Clear()
{
	while (!undoCommands.empty())
	{
		DEL(undoCommands.top())
		undoCommands.pop();
	}

	while (!redoCommands.empty())
	{
		DEL(redoCommands.top())
		redoCommands.pop();
	}
}

bool RE_CommandManager::CanRedo() { return !redoCommands.empty(); }
bool RE_CommandManager::CanUndo() { return !undoCommands.empty(); }