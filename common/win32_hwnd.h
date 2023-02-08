#ifndef H_COMMON_WIN32_HWND
#define H_COMMON_WIN32_HWND

#include "win32.h"

void
Win32SetWindowLongPtrW(_In_ const HWND     hWnd,
                       _In_ const int      nIndex,
                       _In_ const void    *dwNewLong,
                       _In_ const wchar_t *lpDescription);
// @Nullable
void *
Win32TryGetWindowLongPtrW(_In_ const HWND     hWnd,
                          _In_ const int      nIndex,
                          _In_ const wchar_t *lpDescription);
// @Nullable
void *
Win32GetWindowLongPtrW(_In_ const HWND     hWnd,
                       _In_ const int      nIndex,
                       _In_ const wchar_t *lpDescription);

#endif  // H_COMMON_WIN32_HWND

