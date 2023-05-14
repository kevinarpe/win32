#include "win32_dpi.h"
#include "assertive.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <commctrl.h>

static void
TestWin32DPIGet(_In_ HINSTANCE hInstance)
{
    puts("TestWin32DPIGet\r\n");

    HWND hWnd =
        CreateWindowExW(
            0,              // [in]           DWORD     dwExStyle
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/static-controls
            WC_STATICW,     // [in, optional] LPCWSTR   lpClassName
            L"dummy text",  // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            0,              // [in]           DWORD     dwStyle
            CW_USEDEFAULT,  // [in]           int       X
            CW_USEDEFAULT,  // [in]           int       Y
            CW_USEDEFAULT,  // [in]           int       nWidth
            CW_USEDEFAULT,  // [in]           int       nHeight
            NULL,           // [in, optional] HWND      hWndParent
            NULL,           // [in, optional] HMENU     hMenu
            hInstance,      // [in, optional] HINSTANCE hInstance
            NULL);          // [in, optional] LPVOID    lpParam

    struct Win32DPI win32Dpi = WIN32_DPI_INIT;

    Win32DPIGet(&win32Dpi,  // _Inout_ struct Win32DPI *lpDpi
                hWnd);      // _In_    const HWND       hWnd

    AssertWF(DPI_AWARENESS_INVALID != win32Dpi.dpiAwareness,           // _In_ const bool     bAssertResult
             L"DPI_AWARENESS_INVALID:%d != win32Dpi.dpiAwareness:%d",  // _In_ const wchar_t *lpMessageFormatWCharArr
             DPI_AWARENESS_INVALID, win32Dpi.dpiAwareness);            // _In_ ...

    AssertWF(WIN32_DPI_UNSET != win32Dpi.dpi,           // _In_ const bool     bAssertResult
             L"WIN32_DPI_UNSET:%u != win32Dpi.dpi:%u",  // _In_ const wchar_t *lpMessageFormatWCharArr
             WIN32_DPI_UNSET, win32Dpi.dpi);            // _In_ ...
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

    TestWin32DPIGet(hInstance);  // _In_ HINSTANCE hInstance
    return 0;
}

