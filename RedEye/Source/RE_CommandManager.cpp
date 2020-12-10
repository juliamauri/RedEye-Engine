#include "RE_CommandManager.h"

#include "Globals.h"
#include "RE_Command.h"

using namespace RE_CommandManager::Internal;

bool RE_CommandManager::CanRedo()
{
	return !redoCommands.empty();
}

bool RE_CommandManager::CanUndo()
{
	return !undoCommands.empty();
}

void RE_CommandManager::Redo()
{
	if (!redoCommands.empty()) {
		RE_Command* cmd = redoCommands.top();
		cmd->execute();
		undoCommands.push(cmd);
		redoCommands.pop();
	}
}

void RE_CommandManager::Undo()
{
	if (!undoCommands.empty()) {
		RE_Command* cmd = undoCommands.top();
		cmd->Undo();
		redoCommands.push(cmd);
		undoCommands.pop();
	}
}

void RE_CommandManager::PushCommand(RE_Command* newCommand)
{
	while (!redoCommands.empty()) {
		DEL(redoCommands.top());
		redoCommands.pop();
	}

	if (undoCommands.size() == max_commands) {
		DEL(*undoCommands.c.begin());
		undoCommands.c.erase(undoCommands.c.begin());
	}
	
	undoCommands.push(newCommand);
}

void RE_CommandManager::Clear()
{
	while (!undoCommands.empty()) {
		DEL(undoCommands.top());
		undoCommands.pop();
	}

	while (!redoCommands.empty()) {
		DEL(redoCommands.top());
		redoCommands.pop();
	}
}
