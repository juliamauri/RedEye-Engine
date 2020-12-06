#include "RE_LogManager.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "EditorWindows.h"

#include <EAStdC/EASprintf.h>

#include <windows.h> // TODO Julius: Destruir Windows. Reventarlo quitandote la camiseta. Que no lo reconozca ni Billy el Puertas.

#define LOG_STATEMENT_MAX_LENGTH 512

bool RE_LogManager::scoping_procedure = false;
bool RE_LogManager::error_scoped = false;

void RE_LogManager::_RequestBrowser(const char* link)
{
	ShellExecute(NULL, "open", link, NULL, NULL, SW_SHOWNORMAL);
}

void RE_LogManager::_Log(int category, const char file[], int line, const char* format, ...)
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
	Event::PushForced(static_cast<RE_EventType>(CONSOLE_LOG_SEPARATOR + category), App::editor, edited, file_name);
}

void RE_LogManager::_ReportSoftware(const char file[], int line, const char* name, const char* version, const char* website)
{
	App::editor->ReportSoftawe(name, version, website);

	if (version != nullptr)
	{
		if (website != nullptr) _Log(L_SOFTWARE, file, line, "3rd party software report: %s v%s (%s)", name, version, website);
		else _Log(L_SOFTWARE, file, line, "3rd party software report: %s v%s", name, version);
	}
	else if (website != nullptr) _Log(L_SOFTWARE, file, line, "3rd party software report: %s (%s)", name, website);
	else _Log(L_SOFTWARE, file, line, "3rd party software report: %s", name);
}

void RE_LogManager::ScopeProcedureLogging()
{
	scoping_procedure = true;
	error_scoped = false;
}
void RE_LogManager::EndScope()
{
	if (scoping_procedure)
	{
		scoping_procedure = false;
		Event::PushForced(SCOPE_PROCEDURE_END, App::editor, error_scoped);
	}
}

bool RE_LogManager::ScopedErrors() { return error_scoped; }
