#ifndef __RE_HANDLEERRORS_H__
#define __RE_HANDLEERRORS_H__

#include "Module.h"

#include <EASTL/string.h>

enum LogCategory;

class RE_HandleErrors {
public:
	RE_HandleErrors();
	~RE_HandleErrors();

	void StartHandling();
	void StopHandling();
	void StopAndPresent();

	void HandleLog(const char* log, LogCategory category);

	bool AnyErrorHandled()const;
	bool AnyWarningHandled()const;
	bool AnySolutionHandled()const;

	const char* GetLogs();
	const char* GetWarnings();
	const char* GetErrors();
	const char* GetSolutions();

	void ClearAll();

	void ActivatePopUp();

private:
	eastl::string logs;
	eastl::string warnings;
	eastl::string errors;
	eastl::string solutions;
	bool recLogs = false;
};

#endif // !__RE_HANDLEERRORS_H__