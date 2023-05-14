#include "win32_hwnd.h"
#include "win32_last_error.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW

void
Win32SetWindowLongPtrW(_In_ const HWND     hWnd,
                       _In_ const int      nIndex,
                       // @Nullable
                       _In_ const void    *dwNullableNewLong,
                       _In_ const wchar_t *lpDescription)
{
    assert(NULL != hWnd);
    assert(NULL != lpDescription);
    assert(L'\0' != lpDescription[0]);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setlasterror
    SetLastError(0);  // [in] DWORD dwErrCode
    
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowlongptrw
    // "To determine success or failure, clear the last error information by calling SetLastError with 0, then call SetWindowLongPtr.
    //  Function failure will be indicated by a return value of zero and a GetLastError result that is nonzero."
    if (0 == SetWindowLongPtrW(hWnd,                           // [in] HWND     hWnd
                               nIndex,                         // [in] int      nIndex
                               (LONG_PTR) dwNullableNewLong))  // [in] LONG_PTR dwNewLong
    {   
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        const DWORD dwLastError = GetLastError();
        if (0 != dwLastError)
        {   
            Win32LastErrorFPutWSAbort(stderr,          // _In_ FILE          *lpStream
                                      lpDescription);  // _In_ const wchar_t *lpMessage
        }
    }
}

// @Nullable
void *
Win32TryGetWindowLongPtrW(_In_ const HWND     hWnd,
                          _In_ const int      nIndex,
                          _In_ const wchar_t *lpDescription)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setlasterror
    SetLastError(0);  // [in] DWORD dwErrCode

    const LONG_PTR x = GetWindowLongPtrW(hWnd,     // [in] HWND hWnd
                                         nIndex);  // [in] int  nIndex
    if (0 == x)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        const DWORD dwLastError = GetLastError();
        if (0 != dwLastError)
        {
            Win32LastErrorFPutWSAbort(stderr,          // _In_ FILE          *lpStream
                                      lpDescription);  // _In_ const wchar_t *lpMessage
        }
    }
    return (void *) x;
}

void *
Win32GetWindowLongPtrW(_In_ const HWND     hWnd,
                       _In_ const int      nIndex,
                       _In_ const wchar_t *lpDescription)
{
    void *x = Win32TryGetWindowLongPtrW(hWnd, nIndex, lpDescription);
    if (NULL == x)
    {
        Win32LastErrorFPrintFWAbort(stderr,          // _In_ FILE          *lpStream
                                    L"NULL == %ls",  // _In_ const wchar_t *lpMessageFormat
                                    lpDescription);  // ...
    }
    return x;
}

