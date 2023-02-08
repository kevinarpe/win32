#include "win32_hwnd.h"
#include "error_exit.h"

void
Win32SetWindowLongPtrW(_In_ const HWND     hWnd,
                       _In_ const int      nIndex,
                       _In_ const void    *dwNewLong,
                       _In_ const wchar_t *lpDescription)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setlasterror
    SetLastError(0);  // [in] DWORD dwErrCode
    
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowlongptrw
    // "To determine success or failure, clear the last error information by calling SetLastError with 0, then call SetWindowLongPtr.
    //  Function failure will be indicated by a return value of zero and a GetLastError result that is nonzero."
    if (0 == SetWindowLongPtrW(hWnd,                   // [in] HWND     hWnd
                               nIndex,                 // [in] int      nIndex
                               (LONG_PTR) dwNewLong))  // [in] LONG_PTR dwNewLong
    {   
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        const DWORD dwLastError = GetLastError();
        if (0 != dwLastError)
        {   
            ErrorExitW(lpDescription);
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
            ErrorExitW(lpDescription);
        }
    }
    return (void *) x;
}

// @Nullable
void *
Win32GetWindowLongPtrW(_In_ const HWND     hWnd,
                       _In_ const int      nIndex,
                       _In_ const wchar_t *lpDescription)
{
    void *x = Win32TryGetWindowLongPtrW(hWnd, nIndex, lpDescription);
    if (NULL == x)
    {
        ErrorExitWF(L"NULL == %ls", lpDescription);
    }
    return x;
}

