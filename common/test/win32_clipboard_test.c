#include "win32_clipboard.h"
#include "win32_last_error.h"
#include "assertive.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

static void
TestWin32ClipboardReadWStr()
{
    puts("TestWin32ClipboardReadWStr\r\n");

    struct WStr inWStr = WSTR_FROM_LITERAL(L"abc");
    Win32ClipboardWriteWStr(NULL,      // _In_ HWND               hNullableWnd,
                            &inWStr);  // _In_ const struct WStr *lpWStr
    struct WStr outWStr = {0};
    const bool b = Win32ClipboardReadWStr(&outWStr);  // _Inout_ struct WStr *lpWStr
    assert(true == b);

    AssertWF(0 == wcscmp(inWStr.lpWCharArr, outWStr.lpWCharArr),               // _In_ const bool     bAssertResult
             L"0 == wcscmp(inWStr.lpWCharArr[%ls], outWStr.lpWCharArr[%ls])",  // _In_ const wchar_t *lpMessageFormatWCharArr
             inWStr.lpWCharArr, outWStr.lpWCharArr);                           // _In_ ...

    Win32ClipboardClearAbort();

    struct WStr outWStr2 = {0};
    const bool b2 = Win32ClipboardReadWStr(&outWStr2);  // _Inout_ struct WStr *lpWStr
    assert(false == b2);
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

    TestWin32ClipboardReadWStr();
    return 0;
}

