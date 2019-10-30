#include "RE_HandleErrors.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "EditorWindows.h"

#include "OutputLog.h"

RE_HandleErrors::RE_HandleErrors()
{
}


RE_HandleErrors::~RE_HandleErrors()
{
}

void RE_HandleErrors::StartHandling()
{
	ClearAll();
	recLogs = true;
}

void RE_HandleErrors::StopHandling()
{
	recLogs = false;
}

void RE_HandleErrors::HandleLog(const char * _log, LogCategory category)
{
	if (recLogs) {
		logs += _log;
		switch (LogCategory(category))
		{
		case L_ERROR: errors += _log; break;
		case L_WARNING: warnings += _log; break;
		case L_SOLUTION: solutions += _log; break;
		}

	}
}

bool RE_HandleErrors::AnyErrorHandled() const
{
	return !errors.empty();
}

bool RE_HandleErrors::AnyWarningHandled() const
{
	return !warnings.empty();
}

bool RE_HandleErrors::AnySolutionHandled() const
{
	return !solutions.empty();
}

const char * RE_HandleErrors::GetErrors()
{
	return errors.c_str();
}

const char * RE_HandleErrors::GetSolutions()
{
	return solutions.c_str();
}

const char * RE_HandleErrors::GetLogs()
{
	return logs.c_str();
}

const char * RE_HandleErrors::GetWarnings()
{
	return warnings.c_str();
}

void RE_HandleErrors::ClearAll()
{
	if (!App->editor->popupWindow->IsActive()) {
		logs.clear();
		errors.clear();
		warnings.clear();
		solutions.clear();
	}
}

void RE_HandleErrors::ActivatePopUp()
{
	App->editor->popupWindow->PopUpError();
}
