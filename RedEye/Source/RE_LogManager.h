#ifndef __LOG__
#define __LOG__

#include <EASTL/list.h>
#include <EASTL/string.h>
#include <EASTL/map.h>

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

class RE_LogManager
{
public:

	static void _ReportSoftware(const char file[], int line, const char* name, const char* version = nullptr, const char* website = nullptr);
	static void _Log(int category, const char file[], int line, const char* format, ...);
	static void _RequestBrowser(const char* link);

	static void ScopeProcedureLogging();
	static void EndScope();
	static bool ScopedErrors();

private:

	static bool scoping_procedure;
	static bool error_scoped;
};

#define RE_LOG(format, ...) RE_LogManager::_Log(L_GLOBAL, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_INTERNAL(format, ...) RE_LogManager::_Log(-1, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SEPARATOR(format, ...) RE_LogManager::_Log(L_SEPARATOR, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SECONDARY(format, ...) RE_LogManager::_Log(L_SECONDARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_TERCIARY(format, ...) RE_LogManager::_Log(L_TERCIARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_ERROR(format, ...) RE_LogManager::_Log(L_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_WARNING(format, ...) RE_LogManager::_Log(L_WARNING, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SOLUTION(format, ...) RE_LogManager::_Log(L_SOLUTION, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_CATEGORY(category, format, ...) RE_LogManager::_Log(category, __FILE__, __LINE__, format, __VA_ARGS__)

#define RE_SOFT_N(name) RE_LogManager::_ReportSoftware(__FILE__, __LINE__, name, nullptr, nullptr)
#define RE_SOFT_NV(name, version) RE_LogManager::_ReportSoftware(__FILE__, __LINE__, name, version, nullptr)
#define RE_SOFT_NS(name, website) RE_LogManager::_ReportSoftware(__FILE__, __LINE__, name, nullptr, website)
#define RE_SOFT_NVS(name, version, website) RE_LogManager::_ReportSoftware(__FILE__, __LINE__, name, version, website)

#define BROWSER(link) RE_LogManager::_RequestBrowser(link)

#endif // !__LOG__