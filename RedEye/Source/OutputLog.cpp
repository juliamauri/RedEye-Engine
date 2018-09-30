#include "OutputLog.h"

#include "Application.h"

#include <windows.h>
#include <stdio.h>

#define WIN32_MEAN_AND_LEAN
#define LOG_STATEMENT_MAX_LENGTH 4096

void _log(const char file[], int line, const char* format, ...)
{
	static char tmp_string[4096];
	static char tmp_string2[4096];
	static va_list  ap;

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmp_string, 4096, format, ap);
	va_end(ap);
	sprintf_s(tmp_string2, 4096, "\n%s(%d) : %s", file, line, tmp_string);
	OutputDebugString(tmp_string2);

	if (App != nullptr) {
		sprintf_s(tmp_string2, 4096, "\n%s", tmp_string);
		App->Log(tmp_string2, file);
	}
}

void _RequestBrowser(const char* link)
{
	ShellExecute(NULL, "open", link, NULL, NULL, SW_SHOWNORMAL);
}

OutputLogHolder::OutputLogHolder(const char * log_dir) : dir(log_dir)
{}

void OutputLogHolder::Add(const char * text, const char* file)
{
	std::string file_path = file;
	std::string file_name = file_path.substr(file_path.find_last_of("\\") + 1);

	std::map<std::string, unsigned int>::iterator caller_id = callers.find(file_name);

	if (caller_id != callers.end())
	{
		logHistory.push_back({ caller_id->second, text });
	}
	else
	{
		callers.insert({ file_name, next_caller_id });
		logHistory.push_back({ next_caller_id, text });
		next_caller_id++;
	}
}

void OutputLogHolder::SaveLogs()
{
	// TODO save log to file
}
