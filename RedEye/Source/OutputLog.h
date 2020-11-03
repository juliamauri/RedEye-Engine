#ifndef __LOG__
#define __LOG__

enum LogCategory : int
{
	L_SEPARATOR = 0x00,
	L_GLOBAL,
	L_SECONDARY,
	L_TERCIARY,
	L_ERROR,
	L_WARNING,
	L_SOLUTION,
	L_SOFTWARE,
	L_TOTAL_CATEGORIES
};

#define RE_LOG(format, ...) _log(L_GLOBAL, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SEPARATOR(format, ...) _log(L_SEPARATOR, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SECONDARY(format, ...) _log(L_SECONDARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_TERCIARY(format, ...) _log(L_TERCIARY, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_ERROR(format, ...) _log(L_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_WARNING(format, ...) _log(L_WARNING, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SOLUTION(format, ...) _log(L_SOLUTION, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_SOFTWARE(format, ...) _log(L_SOFTWARE, __FILE__, __LINE__, format, __VA_ARGS__)
#define RE_LOG_CATEGORY(category, format, ...) _log(LogCategory(category), __FILE__, __LINE__, format, __VA_ARGS__)

#define BROWSER(link) _RequestBrowser(link)

void _log(const int category, const char file[], int line, const char* format, ...);

void _RequestBrowser(const char* link);

#include <EASTL/list.h>
#include <EASTL/string.h>
#include <EASTL/map.h>

struct RE_Log
{
	RE_Log(unsigned int caller_id, LogCategory category, const char* data);
	unsigned int caller_id;
	LogCategory category;
	eastl::string data;
};

class OutputLogHolder
{
public:

	OutputLogHolder(const char * log_dir = nullptr);
	~OutputLogHolder();

	void Add(int category, const char * text, const char* file);
	
	// TODO Rub: void SaveLogs();

	eastl::list<RE_Log> logHistory;
	eastl::map<eastl::string, unsigned int> callers;

private:

	const char * dir = nullptr;
	unsigned int next_caller_id = 0u;
};

#endif // !__LOG__