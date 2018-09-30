#ifndef __LOG__
#define __LOG__

#define LOG(format, ...) _log(__FILE__, __LINE__, format, __VA_ARGS__)

#define BROWSER(link) _RequestBrowser(link)

void _log(const char file[], int line, const char* format, ...);

void _RequestBrowser(const char* link);

#include <list>
#include <string>
#include <map>

class OutputLogHolder
{
public:

	OutputLogHolder(const char * log_dir = nullptr);

	void Add(const char * text, const char* file);
	void SaveLogs();

	std::list<std::pair<unsigned int, std::string>> logHistory;
	std::map<std::string, unsigned int> callers;

private:

	const char * dir = nullptr;
	unsigned int next_caller_id = 0u;
};

#endif // !__LOG__