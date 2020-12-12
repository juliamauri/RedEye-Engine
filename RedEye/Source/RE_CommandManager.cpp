#include "RE_CommandManager.h"

#include "RE_Command.h"

#include "Globals.h"

#define MAX_COMMANDS 100

RE_CommandManager::RE_CommandManager()
{
}

RE_CommandManager::~RE_CommandManager()
{
	Clear();
}

bool RE_CommandManager::canRedo() const
{
	return !redoCommands.empty();
}

bool RE_CommandManager::canUndo() const
{
	return !undoCommands.empty();
}

void RE_CommandManager::redo()
{
	if (canRedo()) {
		RE_Command* cmd = redoCommands.top();
		cmd->execute();
		undoCommands.push(cmd);
		redoCommands.pop();
	}
}

void RE_CommandManager::undo()
{
	if (canUndo()) {
		RE_Command* cmd = undoCommands.top();
		cmd->undo();
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

	if (undoCommands.size() == MAX_COMMANDS) {
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
