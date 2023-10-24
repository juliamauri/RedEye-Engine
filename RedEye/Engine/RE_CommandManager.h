#ifndef __RE_COMMAND_MANAGER_H__
#define __RE_COMMAND_MANAGER_H__

class RE_Command;

namespace RE_CommandManager
{
	void Redo();
	void Undo();

	void PushCommand(RE_Command* newCommand);
	void Clear();

	bool CanRedo();
	bool CanUndo();
};

#endif // !__RE_COMMAND_MANAGER_H__