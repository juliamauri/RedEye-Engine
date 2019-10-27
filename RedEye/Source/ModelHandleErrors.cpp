#include "ModelHandleErrors.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "EditorWindows.h"

#include "OutputLog.h"

ModelHandleErrors::ModelHandleErrors() : Module("ModuleHandleErrors")
{
}


ModelHandleErrors::~ModelHandleErrors()
{
}

void ModelHandleErrors::StartHandling()
{
	recLogs = true;
}

void ModelHandleErrors::StopHandling()
{
	recLogs = false;
}

void ModelHandleErrors::HandleLog(const char * _log, LogCategory category)
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

bool ModelHandleErrors::AnyErrorHandled() const
{
	return !errors.empty();
}

bool ModelHandleErrors::AnyWarningHandled() const
{
	return !warnings.empty();
}

bool ModelHandleErrors::AnySolutionHandled() const
{
	return !solutions.empty();
}

const char * ModelHandleErrors::GetErrors()
{
	return errors.c_str();
}

const char * ModelHandleErrors::GetSolutions()
{
	return solutions.c_str();
}

const char * ModelHandleErrors::GetLogs()
{
	return logs.c_str();
}

const char * ModelHandleErrors::GetWarnings()
{
	return warnings.c_str();
}

void ModelHandleErrors::ClearAll()
{
	logs.clear();
	errors.clear();
	warnings.clear();
	solutions.clear();
}

void ModelHandleErrors::ActivatePopUp()
{
	App->editor->popupWindow->PopUpError();
}
