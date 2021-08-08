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

class RE_ConsoleLog
{
public:
	RE_ConsoleLog() {}
	~RE_ConsoleLog() {}

	void Log(int category, const char file[], int line, const char* format, ...);
	void ReportSoftware(const char file[], int line, const char* name, const char* version = nullptr, const char* website = nullptr);
	
	void RequestBrowser(const char* link) const;
	 
	void ScopeProcedureLogging();
	void EndScope();
	bool ScopedErrors();

private:

	bool scoping_procedure = false;
	bool error_scoped = false;
};

#define RE_LOG(format, ...) App->log.Log(L_GLOBAL, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_INTERNAL(format, ...) App->log.Log(-1, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SEPARATOR(format, ...) App->log.Log(L_SEPARATOR, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SECONDARY(format, ...) App->log.Log(L_SECONDARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_TERCIARY(format, ...) App->log.Log(L_TERCIARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_ERROR(format, ...) App->log.Log(L_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_WARNING(format, ...) App->log.Log(L_WARNING, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SOLUTION(format, ...) App->log.Log(L_SOLUTION, __FILE__, __LINE__, format, __VA_ARGS__)

#define RE_SOFT_N(name) App->log.ReportSoftware(__FILE__, __LINE__, name, nullptr, nullptr)
#define RE_SOFT_NV(name, version) App->log.ReportSoftware(__FILE__, __LINE__, name, version, nullptr)
#define RE_SOFT_NS(name, website) App->log.ReportSoftware(__FILE__, __LINE__, name, nullptr, website)
#define RE_SOFT_NVS(name, version, website) App->log.ReportSoftware(__FILE__, __LINE__, name, version, website)

#define BROWSER(link) App->log.RequestBrowser(link)

#endif // !__RE_CONSOLE_LOG__