#ifndef H_COMMON_WIN32_CONSOLE
#define H_COMMON_WIN32_CONSOLE

#include <stddef.h>  // required for size_t

void
Win32RedirectIOToConsole(const size_t ulMinConsoleLines,
                         const size_t ulMaxConsoleLines);

#endif  // H_COMMON_WIN32_CONSOLE

