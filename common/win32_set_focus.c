#include "win32_set_focus.h"
#include "win32_last_error.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW

// @Nullable
HWND
Win32SetFocus(_In_opt_ HWND           hNullableWnd,
              _In_     const wchar_t *lpszErrorMsgFmt,
              _In_     ...)
{
    assert(NULL != lpszErrorMsgFmt);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setlasterror
    SetLastError(0);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setfocus
    // Before this function returns, the following window messages are received by wndproc:
    // WM_QUERYNEWPALETTE, WM_ACTIVATEAPP, WM_NCACTIVATE, WM_ACTIVATE(, WM_SETFOCUS, WM_KILLFOCUS), WM_COMMAND
    // @Nullable
    const HWND nullableHWndLostFocus = SetFocus(hNullableWnd);
    if (NULL == nullableHWndLostFocus  // [in, optional] HWND hWnd
        && 0 != GetLastError())
    {
        // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
        va_list ap;
        va_start(ap, lpszErrorMsgFmt);
        Win32LastErrorFPrintFWVAbort(stderr,           // _In_  FILE          *lpStream
                                     lpszErrorMsgFmt,  // _In_  const wchar_t *lpMessageFormat
                                     ap);              // _In_  va_list        ap
    }
    return nullableHWndLostFocus;
}

