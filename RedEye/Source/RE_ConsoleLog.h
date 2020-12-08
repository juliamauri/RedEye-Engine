#ifndef __RE_CONSOLE_LOG__
#define __RE_CONSOLE_LOG__

enum LogCategory : unsigned int
{
	L_SEPARATOR = 0,
	L_GLOBAL,
	L_SECONDARY,
	L_TERCIARY,
	L_SOFTWARE,
	L_ERROR,
	L_WARNING,
	L_SOLUTION,
	L_TOTAL_CATEGORIES
};

namespace RE_ConsoleLog
{
	void _Log(int category, const char file[], int line, const char* format, ...);
	void _ReportSoftware(const char file[], int line, const char* name, const char* version = nullptr, const char* website = nullptr);
	void _RequestBrowser(const char* link);

	void ScopeProcedureLogging();
	void EndScope();
	bool ScopedErrors();

	static bool scoping_procedure = false;
	static bool error_scoped = false;
};

#define RE_LOG(format, ...) RE_ConsoleLog::_Log(L_GLOBAL, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_INTERNAL(format, ...) RE_ConsoleLog::_Log(-1, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SEPARATOR(format, ...) RE_ConsoleLog::_Log(L_SEPARATOR, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SECONDARY(format, ...) RE_ConsoleLog::_Log(L_SECONDARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_TERCIARY(format, ...) RE_ConsoleLog::_Log(L_TERCIARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_ERROR(format, ...) RE_ConsoleLog::_Log(L_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_WARNING(format, ...) RE_ConsoleLog::_Log(L_WARNING, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SOLUTION(format, ...) RE_ConsoleLog::_Log(L_SOLUTION, __FILE__, __LINE__, format, __VA_ARGS__)

#define RE_SOFT_N(name) RE_ConsoleLog::_ReportSoftware(__FILE__, __LINE__, name, nullptr, nullptr)
#define RE_SOFT_NV(name, version) RE_ConsoleLog::_ReportSoftware(__FILE__, __LINE__, name, version, nullptr)
#define RE_SOFT_NS(name, website) RE_ConsoleLog::_ReportSoftware(__FILE__, __LINE__, name, nullptr, website)
#define RE_SOFT_NVS(name, version, website) RE_ConsoleLog::_ReportSoftware(__FILE__, __LINE__, name, version, website)

#define BROWSER(link) RE_ConsoleLog::_RequestBrowser(link)

#endif // !__RE_CONSOLE_LOG__