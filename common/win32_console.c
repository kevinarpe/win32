#include "win32_console.h"
#include "win32.h"
#include "min_max.h"
#include "win32_last_error.h"
#include "log.h"
#include <assert.h>
#include <io.h>     // required for _open_osfhandle()
#include <fcntl.h>  // required for _O_TEXT
#include <stdio.h>  // required for _snwprintf_s() and _fdopen()

static wchar_t *
StaticGetStdHandleNameW(_In_ const DWORD nStdHandle)
{
    switch (nStdHandle)
    {
        case STD_INPUT_HANDLE : return L"STD_INPUT_HANDLE";
        case STD_OUTPUT_HANDLE: return L"STD_OUTPUT_HANDLE";
        case STD_ERROR_HANDLE : return L"STD_ERROR_HANDLE";
        default:
        {
            assert(FALSE);
            return NULL;  // unreachable
        }
    }
}

static HANDLE
StaticGetStdHandle(_In_ const DWORD nStdHandle)
{
    // Ref: https://docs.microsoft.com/en-us/windows/console/getstdhandle
    const HANDLE h = GetStdHandle(nStdHandle);
    if (INVALID_HANDLE_VALUE == h)
    {
        Win32LastErrorFPrintFWAbort(stderr,                                        // _In_ FILE          *lpStream
                                    L"INVALID_HANDLE_VALUE == GetStdHandle(%ls)",  // _In_ const wchar_t *lpMessageFormat
                                    StaticGetStdHandleNameW(nStdHandle));          // _In_ ...
    }
    else if (NULL == h)
    {
        Win32LastErrorFPrintFWAbort(stderr,                                // _In_ FILE          *lpStream
                                    L"NULL == GetStdHandle(%ls)",          // _In_ const wchar_t *lpMessageFormat
                                    StaticGetStdHandleNameW(nStdHandle));  // _In_ ...
    }
    return h;
}

static void
StaticRedirect(_In_ const DWORD nStdHandle)
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
        Win32LastErrorFPrintFWAbort(stderr,                                  // _In_ FILE          *lpStream
                                    L"-1 == _open_osfhandle(%ls, _O_TEXT)",  // _In_ const wchar_t *lpMessageFormat
                                    StaticGetStdHandleNameW(nStdHandle));    // _In_ ...
    }

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=msvc-170
    const FILE *fpStdNew = _fdopen(fd, mode);
    if (NULL == fpStdNew)
    {
        Win32LastErrorFPrintFWAbort(stderr,                                // _In_ FILE          *lpStream
                                    L"NULL == _fdopen(%ls, \"w\")",        // _In_ const wchar_t *lpMessageFormat
                                    StaticGetStdHandleNameW(nStdHandle));  // _In_ ...
    }

    *fpStd = *fpStdNew;

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setvbuf?view=msvc-170
    if (0 != setvbuf(stdout,  // FILE *stream
                     NULL,    // char *buffer
                     _IONBF,  // int mode
                     0))      // size_t size
    {
        Win32LastErrorFPrintFWAbort(stderr,                                // _In_ FILE          *lpStream
                                    L"0 != setvbuf(%ls, ...)",             // _In_ const wchar_t *lpMessageFormat
                                    StaticGetStdHandleNameW(nStdHandle));  // _In_ ...
    }
}

// Ref: http://dslweb.nwnexus.com/~ast/dload/guicon.htm
// Ref: https://www.asawicki.info/news_1326_redirecting_standard_io_to_windows_console
// Ref: https://gist.github.com/rhoot/2893136
void
Win32RedirectIOToConsole(const size_t ulMinConsoleLines,
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
            DEBUG_LOGWF(stderr, L"ERROR: AllocConsole() return non-TRUE result: %d (TRUE=%d, FALSE=%d)\r\n",
                        bAllocConsoleRef, TRUE, FALSE);
        #else
            Win32LastErrorFPutWSAbort(stderr,              // _In_ FILE          *lpStream
                                      L"AllocConsole()");  // _In_ const wchar_t *lpMessage
        #endif  // __WINE__
    }

    const HANDLE hStdout = StaticGetStdHandle(STD_OUTPUT_HANDLE);

    // Ref: https://docs.microsoft.com/en-us/windows/console/getconsolescreenbufferinfo
    CONSOLE_SCREEN_BUFFER_INFO coninfo = {};
    if (!GetConsoleScreenBufferInfo(hStdout, &coninfo))
    {
        Win32LastErrorFPutWSAbort(stderr,                                                                // _In_ FILE          *lpStream
                                  L"GetConsoleScreenBufferInfo(hStdout, &CONSOLE_SCREEN_BUFFER_INFO)");  // _In_ const wchar_t *lpMessage
    }

    const SHORT sConsoleLines = MinShort(sMaxConsoleLines,
                                         MaxShort(sMinConsoleLines, coninfo.dwSize.Y));

    if (sConsoleLines != coninfo.dwSize.Y)
    {
        // Ref: https://docs.microsoft.com/en-us/windows/console/setconsolescreenbuffersize
        const COORD dwSize = {.X = coninfo.dwSize.X, .Y = sConsoleLines};
        if (!SetConsoleScreenBufferSize(hStdout, dwSize))
        {
            Win32LastErrorFPutWSAbort(stderr,                                             // _In_ FILE          *lpStream
                                      L"SetConsoleScreenBufferSize(hStdout, &coninfo)");  // _In_ const wchar_t *lpMessage
        }
    }

    StaticRedirect(STD_INPUT_HANDLE);
    StaticRedirect(STD_OUTPUT_HANDLE);
    StaticRedirect(STD_ERROR_HANDLE);
}

