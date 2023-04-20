#include "win32_text.h"
#include "win32_last_error.h"
#include <assert.h>

// Digits plus English alphabet twice is 10 + (2 x 26) = 62 chars
// extern
const struct WStr WIN32_ENGLISH_FONT_SAMPLE_WSTR =
    WSTR_FROM_LITERAL(L"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

// Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw
// extern
const struct WStr WIN32_MESSAGE_BOX_BUTTON_TEXT_WSTR_ARR_0[] = {
    WSTR_FROM_LITERAL(L"Abort"),
    WSTR_FROM_LITERAL(L"Retry"),
    WSTR_FROM_LITERAL(L"Ignore"),
    WSTR_FROM_LITERAL(L"Cancel"),
    WSTR_FROM_LITERAL(L"Try Again"),
    WSTR_FROM_LITERAL(L"Continue"),
    WSTR_FROM_LITERAL(L"Help"),
    WSTR_FROM_LITERAL(L"OK"),
    WSTR_FROM_LITERAL(L"Yes"),
    WSTR_FROM_LITERAL(L"No"),
};

const struct WStrArr WIN32_MESSAGE_BOX_BUTTON_TEXT_WSTR_ARR = {
    .lpWStrArr = (struct WStr *) WIN32_MESSAGE_BOX_BUTTON_TEXT_WSTR_ARR_0,
    .ulSize    = sizeof(WIN32_MESSAGE_BOX_BUTTON_TEXT_WSTR_ARR_0),
};

void
Win32TextCalcSize(_In_  HDC                hDC,
                  _In_  HFONT              hFont,
                  _In_  const struct WStr *lpWStr,
                  _In_  UINT               format,
                  _Out_ SIZE              *lpSize)
{
    assert(NULL != hDC);
    assert(NULL != hFont);
    WStrAssertValid(lpWStr);
    assert(NULL != lpSize);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectobject
    const HGDIOBJ prevHFont = SelectObject(hDC, (HGDIOBJ) hFont);
    if (NULL == prevHFont)
    {
        Win32LastErrorFPutWSAbort(stderr,                                  // _In_ FILE          *lpStream
                                  L"SelectObject(hDC, (HGDIOBJ) hFont)");  // _In_ const wchar_t *lpMessage
    }

    // About DrawText() vs GetTextExtentPoint32W():
    // Ref: https://stackoverflow.com/questions/1126730/how-to-find-the-width-of-a-string-in-pixels-in-win32

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawtextw
    RECT rect = {0};
    if (!DrawTextW(hDC,                 // [in]      HDC     hdc,
                   lpWStr->lpWCharArr,  // [in, out] LPCWSTR lpchText,
                   lpWStr->ulSize,      // [in]      int     cchText,
                   &rect,               // [in, out] LPRECT  lprc,
                   format))             // [in]      UINT    format
    {
        Win32LastErrorFPrintFWAbort(stderr,                                                                            // _In_ FILE          *lpStream
                                    L"DrawText(hDC, lpWStr->lpWCharArr[%ls], lpWStr->ulSize[%zd], &rect, format:%u)",  // _In_ const wchar_t *lpMessageFormat
                                    lpWStr->lpWCharArr, lpWStr->ulSize, format);                                       // _In_ ...
    }

    if (NULL == SelectObject(hDC, (HGDIOBJ) prevHFont))
    {
        Win32LastErrorFPutWSAbort(stderr,                                      // _In_ FILE          *lpStream
                                  L"SelectObject(hDC, (HGDIOBJ) prevHFont)");  // _In_ const wchar_t *lpMessage
    }

    lpSize->cx = RECT_WIDTH(rect);
    lpSize->cy = RECT_HEIGHT(rect);
}

