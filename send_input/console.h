#ifndef _H_CONSOLE
#define _H_CONSOLE

#include <stddef.h>  // required for size_t

void RedirectIOToConsole(const size_t ulMinConsoleLines,
                         const size_t ulMaxConsoleLines);

#endif  // _H_CONSOLE

