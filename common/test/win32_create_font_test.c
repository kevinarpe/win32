#include "win32_create_font.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

static void
TestWin32CreateFont()
{
    puts("TestWin32CreateFont\r\n");

    struct Win32Font f = {0};
    f.fontPointSize    = (UINT) 14;
    f.dpi              = (struct Win32DPI) { .dpiAwareness = (DPI_AWARENESS) DPI_AWARENESS_PER_MONITOR_AWARE, .dpi = (UINT) 144 };
    f.weight           = (LONG) FW_NORMAL;
    f.isItalic         = (BOOL) FALSE;
    f.isUnderline      = (BOOL) FALSE;
    f.isStrikeOut      = (BOOL) FALSE;
    f.fontFaceNameWStr = (struct WStr) WSTR_FROM_LITERAL(L"Consolas");

    Win32CreateFont(&f);  // _Inout_ struct Win32Font *lpWin32Font

    assert(NULL != f.hFont);
}

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI wWinMain(__attribute__((unused)) HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
                    __attribute__((unused)) HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
                    __attribute__((unused)) PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
                    __attribute__((unused)) int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-error-mode?view=msvc-170
    _set_error_mode(_OUT_TO_STDERR);  // assert to STDERR

    TestWin32CreateFont();

    return 0;
}

