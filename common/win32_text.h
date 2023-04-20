#ifndef H_COMMON_WIN32_TEXT
#define H_COMMON_WIN32_TEXT

#include "wstr.h"

/**
 * Calculate the height and width of some text.  Multi-line text is allowed, but be careful with 'format' flags.
 *
 * @param format
 *        flags to control how text size is measured
 *        single line text without 'prefix chars' ('&' in "&OK") should use:
 *        DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE
 *        See 'format' param here: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawtextw
 */
void
Win32TextCalcSize(_In_  HDC                hDC,
                  _In_  HFONT              hFont,
                  _In_  const struct WStr *lpWStr,
                  _In_  UINT               format,
                  _Out_ SIZE              *lpSize);  // struct { LONG cx; LONG cy; }

// Digits plus English alphabet twice is 10 + (2 x 26) = 62 chars
extern const struct WStr WIN32_ENGLISH_FONT_SAMPLE_WSTR;  // = WSTR_FROM_LITERAL(L"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

// Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw
extern const struct WStrArr WIN32_MESSAGE_BOX_BUTTON_TEXT_WSTR_ARR;  // = { WSTR_FROM_LITERAL(L"Abort"), ... };

#endif  // H_COMMON_WIN32_TEXT

