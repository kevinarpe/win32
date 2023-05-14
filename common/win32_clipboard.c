#include "win32_clipboard.h"
#include "win32_last_error.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW
#include <stdio.h>

void
Win32ClipboardClearAbort()
{
    if (false == Win32ClipboardClear())
    {
        abort();
    }
}

bool
Win32ClipboardClear()
{
    const bool b = Win32ClipboardClear2(stderr);  // _Out_ FILE *lpErrorStream
    return b;
}

bool
Win32ClipboardClear2(_Out_ FILE *lpErrorStream)
{
    assert(lpErrorStream);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/dataxchg/using-the-clipboard
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-openclipboard
    if (!OpenClipboard(NULL))  // [in, optional] HWND hWndNewOwner
    {
        Win32LastErrorFPutWS(lpErrorStream,                              // _In_ FILE          *lpStream
                             L"Win32ClipboardReadWStr: OpenClipboard");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-emptyclipboard
    if (!EmptyClipboard())
    {
        Win32LastErrorFPutWS(lpErrorStream,                               // _In_ FILE          *lpStream
                             L"Win32ClipboardReadWStr: EmptyClipboard");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-closeclipboard
    if (!CloseClipboard())
    {
        Win32LastErrorFPutWS(lpErrorStream,                               // _In_ FILE          *lpStream
                             L"Win32ClipboardReadWStr: CloseClipboard");  // _In_ const wchar_t *lpMessage
        return false;
    }
    return true;
}

void
Win32ClipboardReadWStrAbort(// @EmptyStringAllowed
                            _Inout_ struct WStr *lpWStr)
{
    if (false == Win32ClipboardReadWStr(lpWStr))  // _Inout_ struct WStr *lpWStr
    {
        abort();
    }
}

bool
Win32ClipboardReadWStr(// @EmptyStringAllowed
                       _Inout_ struct WStr *lpWStr)
{
    const bool b = Win32ClipboardReadWStr2(lpWStr,   // _Inout_ struct WStr *lpWStr
                                           stderr);  // _Out_   FILE        *lpErrorStream
    return b;
}

bool
Win32ClipboardReadWStr2(// @EmptyStringAllowed
                        _Inout_ struct WStr *lpWStr,
                        _Out_   FILE        *lpErrorStream)
{
    WStrAssertValid(lpWStr);
    assert(NULL != lpErrorStream);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/dataxchg/using-the-clipboard
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-openclipboard
    if (!OpenClipboard(NULL))  // [in, optional] HWND hWndNewOwner
    {
        Win32LastErrorFPutWS(lpErrorStream,                              // _In_ FILE          *lpStream
                             L"Win32ClipboardReadWStr: OpenClipboard");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclipboarddata
    // @Nullable
    const HANDLE hGlobal = GetClipboardData(CF_UNICODETEXT);  // [in] UINT uFormat
    if (NULL == hGlobal)
    {
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globallock
    wchar_t *lpGlobalWCharArr = GlobalLock(hGlobal);  // [in] HGLOBAL hMem
    if (NULL == lpGlobalWCharArr)
    {
        Win32LastErrorFPutWS(lpErrorStream,                           // _In_ FILE          *lpStream
                             L"Win32ClipboardReadWStr: GlobalLock");  // _In_ const wchar_t *lpMessage
        return false;
    }

    WStrCopyWCharArr(lpWStr,                     // _Inout_ struct WStr   *lpDestWStr
                     lpGlobalWCharArr,           // _In_    const wchar_t *lpSrcWCharArr
                     wcslen(lpGlobalWCharArr));  // _In_    const size_t   ulSrcSize

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalunlock
    if (0 == GlobalUnlock(hGlobal)  // [in] HGLOBAL hMem
        && NO_ERROR != GetLastError())
    {
        Win32LastErrorFPutWS(lpErrorStream,                             // _In_ FILE          *lpStream
                             L"Win32ClipboardReadWStr: GlobalUnlock");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-closeclipboard
    if (!CloseClipboard())
    {
        Win32LastErrorFPutWS(lpErrorStream,                               // _In_ FILE          *lpStream
                             L"Win32ClipboardReadWStr: CloseClipboard");  // _In_ const wchar_t *lpMessage
        return false;
    }
    return true;
}

void
Win32ClipboardWriteWStrAbort(// @Nullable
                             _In_ HWND               hNullableWnd,
                             _In_ const struct WStr *lpWStr)
{
    if (false == Win32ClipboardWriteWStr(hNullableWnd,  // _In_  HWND               hNullableWnd
                                         lpWStr))       // _In_  const struct WStr *lpWStr
    {
        abort();
    }
}

bool
Win32ClipboardWriteWStr(// @Nullable
                        _In_  HWND               hNullableWnd,
                        _In_  const struct WStr *lpWStr)
{
    const bool b = Win32ClipboardWriteWStr2(hNullableWnd,  // _In_  HWND               hNullableWnd
                                            lpWStr,        // _In_  const struct WStr *lpWStr
                                            stderr);       // _Out_ FILE              *lpErrorStream)
    return b;
}

bool
Win32ClipboardWriteWStr2(// @Nullable
                         _In_  HWND               hNullableWnd,
                         _In_  const struct WStr *lpWStr,
                         _Out_ FILE              *lpErrorStream)
{
    WStrAssertValid(lpWStr);
    assert(NULL != lpErrorStream);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/dataxchg/using-the-clipboard
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-openclipboard
    if (!OpenClipboard(hNullableWnd))  // [in, optional] HWND hWndNewOwner
    {
        Win32LastErrorFPutWS(lpErrorStream,                                 // _In_ FILE          *lpStream
                             L"ClipboardWriteWStr: OpenClipboard");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-emptyclipboard
    if (!EmptyClipboard())
    {
        Win32LastErrorFPutWS(lpErrorStream,                                  // _In_ FILE          *lpStream
                             L"ClipboardWriteWStr: EmptyClipboard");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalalloc
    const size_t ulGlobalSize = sizeof(wchar_t) * (1 + lpWStr->ulSize);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE,  // [in] UINT   uFlags
                                  ulGlobalSize);  // [in] SIZE_T dwBytes
    if (NULL == hGlobal)
    {
        Win32LastErrorFPutWS(lpErrorStream,                               // _In_ FILE          *lpStream
                             L"ClipboardWriteWStr: GlobalAlloc");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globallock
    wchar_t *lpGlobalWCharArr = GlobalLock(hGlobal);  // [in] HGLOBAL hMem
    if (NULL == lpGlobalWCharArr)
    {
        Win32LastErrorFPutWS(lpErrorStream,                              // _In_ FILE          *lpStream
                             L"ClipboardWriteWStr: GlobalLock");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-s-wcscpy-s-mbscpy-s?view=msvc-170
    if (wcscpy_s(lpGlobalWCharArr,     // [inout] wchar_t *dest
                 // "Ensure that this size accounts for the terminating NULL following the string."
                 ulGlobalSize,         // [in]    rsize_t dest_size
                 lpWStr->lpWCharArr))  // [in]    const wchar_t *src))
    {
        Win32LastErrorFPrintFW(lpErrorStream,                             // _In_ FILE          *lpStream,
                               L"ClipboardWriteWStr: wcscpy_s(lpGlobalWCharArr, ulGlobalSize[%zd], lpWStr->lpWCharArr[%ls])",  // _In_ const wchar_t *lpMessageFormat,
                               ulGlobalSize, lpWStr->lpWCharArr);  // ...
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalunlock
    if (0 == GlobalUnlock(hGlobal)  // [in] HGLOBAL hMem
        && NO_ERROR != GetLastError())
    {
        Win32LastErrorFPutWS(lpErrorStream,                                // _In_ FILE          *lpStream
                             L"ClipboardWriteWStr: GlobalUnlock");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setclipboarddata
    if (NULL == SetClipboardData(CF_UNICODETEXT,  // [in]           UINT   uFormat
                                 hGlobal))        // [in, optional] HANDLE hMem
    {
        Win32LastErrorFPutWS(lpErrorStream,                                                         // _In_ FILE          *lpStream
                             L"ClipboardWriteWStr: SetClipboardData(CF_UNICODETEXT, ...)");  // _In_ const wchar_t *lpMessage
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-closeclipboard
    if (!CloseClipboard())
    {
        Win32LastErrorFPutWS(lpErrorStream,                           // _In_ FILE          *lpStream
                             L"ClipboardWriteWStr: CloseClipboard");  // _In_ const wchar_t *lpMessage
        return false;
    }
    return true;
}

