#include "win32_clipboard.h"
#include "error_exit.h"

void
Win32ClipboardWriteWStr(_In_ HWND               hWnd,
                        _In_ const struct WStr *lpWStr)
{
    WStrAssertValid(lpWStr);
    // Ref: https://learn.microsoft.com/en-us/windows/win32/dataxchg/using-the-clipboard
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-openclipboard
    if (!OpenClipboard(hWnd))
    {
        ErrorExitW(L"ClipboardWriteWStr: OpenClipboard");
    }
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-emptyclipboard
    if (!EmptyClipboard())
    {
        ErrorExitW(L"ClipboardWriteWStr: EmptyClipboard");
    }
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalalloc
    const size_t ulGlobalSize = sizeof(wchar_t) * (1 + lpWStr->ulSize);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE,  // [in] UINT   uFlags
                                  ulGlobalSize);  // [in] SIZE_T dwBytes
    if (NULL == hGlobal)
    {
        ErrorExitW(L"ClipboardWriteWStr: GlobalAlloc");
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globallock
    LPVOID lpGlobalData = GlobalLock(hGlobal);  // [in] HGLOBAL hMem
    if (NULL == lpGlobalData)
    {
        ErrorExitW(L"ClipboardWriteWStr: GlobalLock");
    }
    wchar_t *lpGlobalWCharArr = lpGlobalData;

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-s-wcscpy-s-mbscpy-s?view=msvc-170
    if (wcscpy_s(lpGlobalWCharArr,     // [inout] wchar_t *dest
                 // "Ensure that this size accounts for the terminating NULL following the string."
                 ulGlobalSize,         // [in]    rsize_t dest_size
                 lpWStr->lpWCharArr))  // [in]    const wchar_t *src))
    {
        ErrorExitWF(L"ClipboardWriteWStr: wcscpy_s(lpGlobalWCharArr, ulGlobalSize[%zd], lpWStr->lpWCharArr[%ls])",
                    ulGlobalSize, lpWStr->lpWCharArr);
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalunlock
    if (0 == GlobalUnlock(hGlobal)  // [in] HGLOBAL hMem
        && NO_ERROR != GetLastError())
    {
        ErrorExitW(L"ClipboardWriteWStr: GlobalUnlock");
    }

    if (NULL == SetClipboardData(CF_UNICODETEXT,  // [in]           UINT   uFormat,
                                 hGlobal))        // [in, optional] HANDLE hMem
    {
        ErrorExitW(L"ClipboardWriteWStr: SetClipboardData(CF_UNICODETEXT, ...)");
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-closeclipboard
    if (!CloseClipboard())
    {
        ErrorExitW(L"ClipboardWriteWStr: CloseClipboard");
    }
}

