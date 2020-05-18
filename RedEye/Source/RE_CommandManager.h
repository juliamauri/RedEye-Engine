#pragma once
#include <EASTL/stack.h>

class RE_Command;

class RE_CommandManager
{
public:
	RE_CommandManager();
	~RE_CommandManager();

	bool canRedo()const;
	bool canUndo()const;
	void redo();
	void undo();

	void PushCommand(RE_Command* newCommand);

	void Clear();

private:
	eastl::stack<RE_Command*> undoCommands;
	eastl::stack<RE_Command*> redoCommands;

};

