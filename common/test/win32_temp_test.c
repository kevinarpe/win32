#include "win32_temp.h"
#include <assert.h>

static void
TestWin32TempGetTempDirPath()
{
    puts("TestWin32TempGetTempDirPath\r\n");

    struct WStr wstr = {0};
    Win32TempGetTempDirPath(&wstr);

    printf("Win32TempGetTempDirPath: [%ls]\r\n", wstr.lpWCharArr);
    assert(wstr.ulSize > 0);
    assert(wstr.lpWCharArr[wstr.ulSize - 1] == L'\\');
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

    TestWin32TempGetTempDirPath();

    return 0;
}

