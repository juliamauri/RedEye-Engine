#ifndef __RE_CONSOLE_LOG__
#define __RE_CONSOLE_LOG__

#include "RE_DataTypes.h"

namespace RE_ConsoleLog
{
	enum class Category : ushort
	{
		SEPARATOR = 0,
		GLOBAL,
		SECONDARY,
		TERCIARY,
		SOFTWARE,
		ERROR_,
		WARNING,
		SOLUTION,
		TOTAL_CATEGORIES
	};

	void Log(RE_ConsoleLog::Category category, const char file[], int line, const char* format, ...);
	void ReportSoftware(const char file[], int line, const char* name, const char* version = nullptr, const char* website = nullptr);
	
	void RequestBrowser(const char* link);
	 
	void ScopeProcedureLogging();
	bool ScopedErrors();
	void EndScope();
};

#define RE_LOGGER RE_ConsoleLog
#define RE_LOG(format, ...) RE_ConsoleLog::Log(RE_ConsoleLog::Category::GLOBAL, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_INTERNAL(format, ...) RE_ConsoleLog::Log(-1, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SEPARATOR(format, ...) RE_ConsoleLog::Log(RE_ConsoleLog::Category::SEPARATOR, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SECONDARY(format, ...) RE_ConsoleLog::Log(RE_ConsoleLog::Category::SECONDARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_TERCIARY(format, ...) RE_ConsoleLog::Log(RE_ConsoleLog::Category::TERCIARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_ERROR(format, ...) RE_ConsoleLog::Log(RE_ConsoleLog::Category::ERROR_, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_WARNING(format, ...) RE_ConsoleLog::Log(RE_ConsoleLog::Category::WARNING, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SOLUTION(format, ...) RE_ConsoleLog::Log(RE_ConsoleLog::Category::SOLUTION, __FILE__, __LINE__, format, __VA_ARGS__)

#define RE_SOFT_N(name) RE_ConsoleLog::ReportSoftware(__FILE__, __LINE__, name, nullptr, nullptr)
#define RE_SOFT_NV(name, version) RE_ConsoleLog::ReportSoftware(__FILE__, __LINE__, name, version, nullptr)
#define RE_SOFT_NS(name, website) RE_ConsoleLog::ReportSoftware(__FILE__, __LINE__, name, nullptr, website)
#define RE_SOFT_NVS(name, version, website) RE_ConsoleLog::ReportSoftware(__FILE__, __LINE__, name, version, website)

#define BROWSER(link) RE_ConsoleLog::RequestBrowser(link)

#endif // !__RE_CONSOLE_LOG__