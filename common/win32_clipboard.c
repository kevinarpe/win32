#include "win32_clipboard.h"
#include "win32_last_error.h"

void
Win32ClipboardWriteWStr(_In_ HWND               hWnd,
                        _In_ const struct WStr *lpWStr)
{
    WStrAssertValid(lpWStr);
    // Ref: https://learn.microsoft.com/en-us/windows/win32/dataxchg/using-the-clipboard
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-openclipboard
    if (!OpenClipboard(hWnd))
    {
        Win32LastErrorFPutWSAbort(stderr,                                 // _In_ FILE          *lpStream
                                  L"ClipboardWriteWStr: OpenClipboard");  // _In_ const wchar_t *lpMessage
    }
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-emptyclipboard
    if (!EmptyClipboard())
    {
        Win32LastErrorFPutWSAbort(stderr,                                  // _In_ FILE          *lpStream
                                  L"ClipboardWriteWStr: EmptyClipboard");  // _In_ const wchar_t *lpMessage
    }
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalalloc
    const size_t ulGlobalSize = sizeof(wchar_t) * (1 + lpWStr->ulSize);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE,  // [in] UINT   uFlags
                                  ulGlobalSize);  // [in] SIZE_T dwBytes
    if (NULL == hGlobal)
    {
        Win32LastErrorFPutWSAbort(stderr,                               // _In_ FILE          *lpStream
                                  L"ClipboardWriteWStr: GlobalAlloc");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globallock
    LPVOID lpGlobalData = GlobalLock(hGlobal);  // [in] HGLOBAL hMem
    if (NULL == lpGlobalData)
    {
        Win32LastErrorFPutWSAbort(stderr,                              // _In_ FILE          *lpStream
                                  L"ClipboardWriteWStr: GlobalLock");  // _In_ const wchar_t *lpMessage
    }
    wchar_t *lpGlobalWCharArr = lpGlobalData;

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-s-wcscpy-s-mbscpy-s?view=msvc-170
    if (wcscpy_s(lpGlobalWCharArr,     // [inout] wchar_t *dest
                 // "Ensure that this size accounts for the terminating NULL following the string."
                 ulGlobalSize,         // [in]    rsize_t dest_size
                 lpWStr->lpWCharArr))  // [in]    const wchar_t *src))
    {
        Win32LastErrorFPrintFWAbort(stderr,                             // _In_ FILE          *lpStream,
                                    L"ClipboardWriteWStr: wcscpy_s(lpGlobalWCharArr, ulGlobalSize[%zd], lpWStr->lpWCharArr[%ls])",  // _In_ const wchar_t *lpMessageFormat,
                                    ulGlobalSize, lpWStr->lpWCharArr);  // ...
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalunlock
    if (0 == GlobalUnlock(hGlobal)  // [in] HGLOBAL hMem
        && NO_ERROR != GetLastError())
    {
        Win32LastErrorFPutWSAbort(stderr,                                // _In_ FILE          *lpStream
                                  L"ClipboardWriteWStr: GlobalUnlock");  // _In_ const wchar_t *lpMessage
    }

    if (NULL == SetClipboardData(CF_UNICODETEXT,  // [in]           UINT   uFormat,
                                 hGlobal))        // [in, optional] HANDLE hMem
    {
        Win32LastErrorFPutWSAbort(stderr,                                                         // _In_ FILE          *lpStream
                                  L"ClipboardWriteWStr: SetClipboardData(CF_UNICODETEXT, ...)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-closeclipboard
    if (!CloseClipboard())
    {
        Win32LastErrorFPutWSAbort(stderr,                                  // _In_ FILE          *lpStream
                                  L"ClipboardWriteWStr: CloseClipboard");  // _In_ const wchar_t *lpMessage
    }
}

