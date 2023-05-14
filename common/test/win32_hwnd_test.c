#include "win32_hwnd.h"
#include "win32_last_error.h"
#include "assertive.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <commctrl.h>

// Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
LRESULT CALLBACK
WindowProc(_In_ const HWND   hWnd,
           _In_ const UINT   uMsg,
           _In_ const WPARAM wParam,
           _In_ const LPARAM lParam)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-defwindowprocw
    const LRESULT x = DefWindowProc(hWnd, uMsg, wParam, lParam);
    return x;
}

static void
TestWin32WindowLongPtrW(_In_ HINSTANCE hInstance)
{
    puts("TestWin32WindowLongPtrW\r\n");

    const wchar_t *lpszClassName = L"TestWin32WindowLongPtrW";

    WNDCLASSEXW wndClassExW = {
        // "The size, in bytes, of this structure. Set this member to sizeof(WNDCLASSEX). Be sure to set this member before calling the GetClassInfoEx function."
        .cbSize = (UINT) sizeof(WNDCLASSEXW),
        // "The class style(s). This member can be any combination of the Class Styles."
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-class-styles
        .style = (UINT) CS_DROPSHADOW,
        // "A pointer to the window procedure. You must use the CallWindowProc function to call the window procedure."
        .lpfnWndProc = (WNDPROC) WindowProc,
        // "The number of extra bytes to allocate following the window-class structure. The system initializes the bytes to zero."
        .cbClsExtra = (int) 0,
        // "The number of extra bytes to allocate following the window instance. The system initializes the bytes to zero."
        // Ref: https://stackoverflow.com/a/65876605/257299
        .cbWndExtra = (int) 5 * sizeof(void *),
        // "A handle to the instance that contains the window procedure for the class."
        // Ref: https://stackoverflow.com/questions/20140117/why-does-createwindow-take-a-hinstance-as-an-argument-if-was-already-provided
        // Ref: https://devblogs.microsoft.com/oldnewthing/20050418-59/?p=35873
        .hInstance = hInstance,
        // "A handle to the class icon. This member must be a handle to an icon resource. If this member is NULL, the system provides a default icon."
        .hIcon = (HICON) 0,
        // "A handle to the class cursor. This member must be a handle to a cursor resource.
        //  If this member is NULL, an application must explicitly set the cursor shape whenever the mouse moves into the application's window.
        .hCursor = NULL,
        // "A handle to the class background brush. This member can be a handle to the physical brush to be used for painting the background, or it can be a color value.
        //  A color value must be one of the following standard system colors (the value 1 must be added to the chosen color)."
        .hbrBackground = (HBRUSH) (1 + COLOR_BTNFACE),
        // "Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
        //  If you use an integer to identify the menu, use the MAKEINTRESOURCE macro. If this member is NULL, windows belonging to this class have no default menu."
        .lpszMenuName = (LPCWSTR) 0,
        // "A pointer to a null-terminated string or is an atom. If this parameter is an atom,
        //  it must be a class atom created by a previous call to the RegisterClass or RegisterClassEx function.
        //  The atom must be in the low-order word of lpszClassName; the high-order word must be zero.
        //  If lpszClassName is a string, it specifies the window class name. The class name can be any name registered with RegisterClass or RegisterClassEx,
        //  or any of the predefined control-class names.
        //  The maximum length for lpszClassName is 256. If lpszClassName is greater than the maximum length, the RegisterClassEx function will fail."
        .lpszClassName = lpszClassName,
        // "A handle to a small icon that is associated with the window class.
        //  If this member is NULL, the system searches the icon resource specified by the hIcon member for an icon of the appropriate size to use as the small icon."
        .hIconSm = (HICON) NULL,
    };

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
    ATOM registerClassExAtom = RegisterClassExW(&wndClassExW);
    if (0 == registerClassExAtom)
    {
        Win32LastErrorFPutWSAbort(stderr,              // _In_ FILE          *lpStream
                                  L"RegisterClassW");  // _In_ const wchar_t *lpMessage
    }

    HWND hWnd =
        CreateWindowExW(
            0,              // [in]           DWORD     dwExStyle
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/static-controls
            lpszClassName,  // [in, optional] LPCWSTR   lpClassName
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
    if (NULL == hWnd)
    {
        Win32LastErrorFPutWSAbort(stderr,               // _In_ FILE          *lpStream
                                  L"CreateWindowExW");  // _In_ const wchar_t *lpMessage
    }

    // @Nullable
    const void *nullablePtr =
        Win32TryGetWindowLongPtrW(hWnd,       // _In_ const HWND     hWnd
                                  0,          // _In_ const int      nIndex
                                  L"desc1");  // _In_ const wchar_t *lpDescription

    AssertWF(NULL == nullablePtr,        // _In_ const bool     bAssertResult
             L"NULL == nullablePtr:%p",  // _In_ const wchar_t *lpMessageFormatWCharArr
             nullablePtr);               // _In_ ...

    // @Nullable
    const void *nullablePtr2 =
        Win32TryGetWindowLongPtrW(hWnd,       // _In_ const HWND     hWnd
                                  1,          // _In_ const int      nIndex
                                  L"desc2");  // _In_ const wchar_t *lpDescription

    AssertWF(NULL == nullablePtr2,        // _In_ const bool     bAssertResult
             L"NULL == nullablePtr2:%p",  // _In_ const wchar_t *lpMessageFormatWCharArr
             nullablePtr2);               // _In_ ...

    Win32SetWindowLongPtrW(hWnd,          // _In_ const HWND     hWnd
                           3,             // _In_ const int      nIndex
                           NULL,          // _In_ const void    *dwNullableNewLong
                           L"desc3");     // _In_ const wchar_t *lpDescription

    Win32SetWindowLongPtrW(hWnd,          // _In_ const HWND     hWnd
                           3,             // _In_ const int      nIndex
                           &nullablePtr,  // _In_ const void    *dwNullableNewLong
                           L"desc4");     // _In_ const wchar_t *lpDescription

    // @Nullable
    const void *nullablePtr3 =
        Win32TryGetWindowLongPtrW(hWnd,       // _In_ const HWND     hWnd
                                  3,          // _In_ const int      nIndex
                                  L"desc5");  // _In_ const wchar_t *lpDescription

    AssertWF(nullablePtr3 == &nullablePtr,           // _In_ const bool     bAssertResult
             L"nullablePtr3:%p == %p:&nullablePtr",  // _In_ const wchar_t *lpMessageFormatWCharArr
             nullablePtr3, &nullablePtr);             // _In_ ...

    const void *ptr3 =
        Win32GetWindowLongPtrW(hWnd,       // _In_ const HWND     hWnd
                               3,          // _In_ const int      nIndex
                               L"desc5");  // _In_ const wchar_t *lpDescription

    AssertWF(ptr3 == &nullablePtr,                   // _In_ const bool     bAssertResult
             L"nullablePtr3:%p == %p:&nullablePtr",  // _In_ const wchar_t *lpMessageFormatWCharArr
             ptr3, &nullablePtr);                    // _In_ ...
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

    TestWin32WindowLongPtrW(hInstance);  // _In_ HINSTANCE hInstance
    return 0;
}

