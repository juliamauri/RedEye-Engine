#ifndef __LOG__
#define __LOG__

#define LOG(format, ...) _log(__FILE__, __LINE__, format, __VA_ARGS__)

#define BROWSER(link) _RequestBrowser(link)

void _log(const char file[], int line, const char* format, ...);

void _RequestBrowser(const char* link);

#endif // !__LOG__