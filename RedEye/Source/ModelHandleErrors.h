#ifndef __MODULEHANDLEERRORS_H__
#define __MODULEHANDLEERRORS_H__

#include "Module.h"

#include <string>

enum LogCategory;

class ModelHandleErrors :
	public Module
{
public:
	ModelHandleErrors();
	~ModelHandleErrors();

	void StartHandling();
	void StopHandling();

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
	std::string logs;
	std::string warnings;
	std::string errors;
	std::string solutions;
	bool recLogs = false;
};

#endif // !__MODULEHANDLEERRORS_H__