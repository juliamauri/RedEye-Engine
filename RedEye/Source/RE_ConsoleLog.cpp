#include "RE_ConsoleLog.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include <EAStdC/EASprintf.h>
#include <windows.h> // TODO Julius: Destruir Windows. Reventarlo quitandote la camiseta. Que no lo reconozca ni Billy el Puertas.

#define LOG_STATEMENT_MAX_LENGTH 512

void RE_ConsoleLog::Log(int category, const char file[], int line, const char* format, ...)
{
	static char base[LOG_STATEMENT_MAX_LENGTH];
	static va_list  ap;

	// Construct the string from variable arguments
	va_start(ap, format);
	EA::StdC::Vsnprintf(base, LOG_STATEMENT_MAX_LENGTH, format, ap);
	va_end(ap);

	// Extract file's name only
	eastl::string file_name = file;
	file_name = file_name.substr(file_name.find_last_of("\\") + 1);

	// Log to Console - file(line) : log
	OutputDebugString(eastl::string(file_name + "(" + eastl::to_string(line) + ") : " + base + "\n").c_str());

	// Create event for Editor Console
	eastl::string edited;
	switch (category) {
	case L_SEPARATOR: edited = "\n============\t"; edited += base; edited += "\t============\n"; break;
	case L_GLOBAL: edited = base; edited.push_back('\n'); break;
	case L_SECONDARY: edited = "\t- "; edited += base; edited.push_back('\n'); break;
	case L_TERCIARY: edited = "\t\t+ "; edited += base; edited.push_back('\n'); break;
	case L_SOFTWARE: edited = "\t* "; edited += base; edited.push_back('\n'); break;
	case L_ERROR: edited = "ERROR: "; edited += base; edited.push_back('\n'); error_scoped = scoping_procedure; break;
	case L_WARNING: edited = "Warning: "; edited += base; edited.push_back('\n'); break;
	case L_SOLUTION: edited = "Solution: "; edited += base; edited.push_back('\n'); break;
	default: return; }

	if (scoping_procedure && category >= L_ERROR) category += 3;
	RE_INPUT->PushForced(static_cast<RE_EventType>(CONSOLE_LOG_SEPARATOR + category), RE_EDITOR, edited, file_name);
}

void RE_ConsoleLog::ReportSoftware(const char file[], int line, const char* name, const char* version, const char* website)
{
	RE_EDITOR->ReportSoftawe(name, version, website);

	if (version != nullptr)
	{
		if (website != nullptr) Log(L_SOFTWARE, file, line, "3rd party software report: %s v%s (%s)", name, version, website);
		else Log(L_SOFTWARE, file, line, "3rd party software report: %s v%s", name, version);
	}
	else if (website != nullptr) Log(L_SOFTWARE, file, line, "3rd party software report: %s (%s)", name, website);
	else Log(L_SOFTWARE, file, line, "3rd party software report: %s", name);
}

void RE_ConsoleLog::RequestBrowser(const char* link) const
{
	ShellExecute(NULL, "open", link, NULL, NULL, SW_SHOWNORMAL);
}

void RE_ConsoleLog::ScopeProcedureLogging()
{
	scoping_procedure = true;
	error_scoped = false;
}
void RE_ConsoleLog::EndScope()
{
	if (scoping_procedure)
	{
		scoping_procedure = false;
		RE_INPUT->PushForced(SCOPE_PROCEDURE_END, RE_EDITOR, error_scoped);
	}
}

bool RE_ConsoleLog::ScopedErrors() { return error_scoped; }
