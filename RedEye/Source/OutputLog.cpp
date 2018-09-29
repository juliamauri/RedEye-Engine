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
		App->Log(tmp_string2);
	}
}

void _RequestBrowser(const char* link)
{
	ShellExecute(NULL, "open", link, NULL, NULL, SW_SHOWNORMAL);
}
