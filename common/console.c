#include "console.h"
#include "win32.h"
#include "min_max.h"
#include "error_exit.h"
#include "log.h"
#include <assert.h>
#include <io.h>     // required for _open_osfhandle()
#include <fcntl.h>  // required for _O_TEXT
#include <stdio.h>  // required for _snwprintf_s() and _fdopen()

static char *StaticGetStdHandleName(_In_ const DWORD nStdHandle)
{
    switch (nStdHandle)
    {
        case STD_INPUT_HANDLE : return "STD_INPUT_HANDLE";
        case STD_OUTPUT_HANDLE: return "STD_OUTPUT_HANDLE";
        case STD_ERROR_HANDLE : return "STD_ERROR_HANDLE";
        default:
        {
            assert(FALSE);
            return NULL;  // unreachable
        }
    }
}

static HANDLE StaticGetStdHandle(_In_ const DWORD nStdHandle)
{
    // Ref: https://docs.microsoft.com/en-us/windows/console/getstdhandle
    const HANDLE h = GetStdHandle(nStdHandle);
    if (INVALID_HANDLE_VALUE == h)
    {
        ErrorExitF("INVALID_HANDLE_VALUE == GetStdHandle(%s)", StaticGetStdHandleName(nStdHandle));
    }
    else if (NULL == h)
    {
        ErrorExitF("NULL == GetStdHandle(%s)", StaticGetStdHandleName(nStdHandle));
    }
    return h;
}

static void StaticRedirect(_In_ const DWORD nStdHandle)
{
    char *mode = NULL;
    FILE *fpStd = NULL;
    switch (nStdHandle)
    {
        case STD_INPUT_HANDLE:
        {
            mode = "r";
            fpStd = stdin;
            break;
        }
        case STD_OUTPUT_HANDLE:
        {
            mode = "w";
            fpStd = stdout;
            break;
        }
        case STD_ERROR_HANDLE:
        {
            mode = "w";
            fpStd = stderr;
            break;
        }
        default:
        {
            assert(FALSE);
            return;  // unreachable
        }
    }

    const HANDLE hStd = StaticGetStdHandle(nStdHandle);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/open-osfhandle?view=msvc-170
    const int fd = _open_osfhandle((intptr_t) hStd, _O_TEXT);
    if (-1 == fd)
    {
        ErrorExitF("-1 == _open_osfhandle(%s, _O_TEXT)", StaticGetStdHandleName(nStdHandle));
    }

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=msvc-170
    const FILE *fpStdNew = _fdopen(fd, mode);
    if (NULL == fpStdNew)
    {
        ErrorExitF("NULL == _fdopen(%s, \"w\")", StaticGetStdHandleName(nStdHandle));
    }

    *fpStd = *fpStdNew;

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setvbuf?view=msvc-170
    if (0 != setvbuf(stdout,  // FILE *stream
                     NULL,    // char *buffer
                     _IONBF,  // int mode
                     0))      // size_t size
    {
        ErrorExitF("0 != setvbuf(%s, ...)", StaticGetStdHandleName(nStdHandle));
    }
}

// Ref: http://dslweb.nwnexus.com/~ast/dload/guicon.htm
// Ref: https://www.asawicki.info/news_1326_redirecting_standard_io_to_windows_console
// Ref: https://gist.github.com/rhoot/2893136
void RedirectIOToConsole(const size_t ulMinConsoleLines,
                         const size_t ulMaxConsoleLines)
{
    // Intentional: ...
    assert(ulMinConsoleLines <= SHRT_MAX);
    assert(ulMaxConsoleLines <= SHRT_MAX);

    const SHORT sMinConsoleLines = ulMinConsoleLines;
    const SHORT sMaxConsoleLines = ulMaxConsoleLines;

    // Ref: https://docs.microsoft.com/en-us/windows/console/allocconsole
    const BOOL bAllocConsoleRef = AllocConsole();
    if (!bAllocConsoleRef)
    {
        // Intentional: Linux/Wine has some weird behaviour!
        #if defined(__WINE__)
            LogF(stderr, "ERROR: AllocConsole() return non-TRUE result: %d (TRUE=%d, FALSE=%d)", bAllocConsoleRef, TRUE, FALSE);
        #else
            ErrorExit("AllocConsole()");
        #endif  // __WINE__
    }

    const HANDLE hStdout = StaticGetStdHandle(STD_OUTPUT_HANDLE);

    // Ref: https://docs.microsoft.com/en-us/windows/console/getconsolescreenbufferinfo
    CONSOLE_SCREEN_BUFFER_INFO coninfo = {};
    if (!GetConsoleScreenBufferInfo(hStdout, &coninfo))
    {
        ErrorExit("GetConsoleScreenBufferInfo(hStdout, &CONSOLE_SCREEN_BUFFER_INFO)");
    }

    const SHORT sConsoleLines = MinShort(sMaxConsoleLines,
                                         MaxShort(sMinConsoleLines, coninfo.dwSize.Y));

    if (sConsoleLines != coninfo.dwSize.Y)
    {
        // Ref: https://docs.microsoft.com/en-us/windows/console/setconsolescreenbuffersize
        const COORD dwSize = {.X = coninfo.dwSize.X, .Y = sConsoleLines};
        if (!SetConsoleScreenBufferSize(hStdout, dwSize))
        {
            ErrorExit("SetConsoleScreenBufferSize(hStdout, &coninfo)");
        }
    }

    StaticRedirect(STD_INPUT_HANDLE);
    StaticRedirect(STD_OUTPUT_HANDLE);
    StaticRedirect(STD_ERROR_HANDLE);
}

