#ifndef H_COMMON_WIN32_SET_FOCUS
#define H_COMMON_WIN32_SET_FOCUS

#include "win32.h"

/**
 * @param hWnd
 *        window (widget) handle to set focus
 *
 * @param lpszErrorMsgFmt
 *        printf-style format string when set focus fails
 *        may be empty; usually does NOT end with newline: L"\r\n"
 *        Ex: L"my error message: %d %d %c"
 *
 * @param ...
 *        zero or more args used by {@code lpszMsgFmt}
 *        Ex: 123, -456, 'b'
 *
 * @return nullable window (widget) handle that *lost* focus
 */
// @Nullable
HWND
Win32SetFocus(_In_opt_ HWND           hWnd,
              _In_     const wchar_t *lpszErrorMsgFmt, ...);

#endif  // H_COMMON_WIN32_SET_FOCUS

