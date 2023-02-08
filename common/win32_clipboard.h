#ifndef H_COMMON_WIN32_CLIPBOARD
#define H_COMMON_WIN32_CLIPBOARD

#include "wstr.h"

void
Win32ClipboardWriteWStr(_In_ HWND               hWnd,
                        _In_ const struct WStr *lpWStr);

#endif  // H_COMMON_WIN32_CLIPBOARD

