#include "win32_layout.h"
#include "win32_last_error.h"
#include "win32_text.h"
#include "assertive.h"
#include <assert.h>

void
Win32LayoutCalcDefaultButtonSize(_In_  HDC                                  hDC,
                                 // @Nullable
                                 _In_  HFONT                                hNullableFont,
                                 _Out_ struct Win32LayoutDefaultButtonSize *lpDefaultButtonSize)
{
    assert(NULL != lpDefaultButtonSize);

    HFONT hFont = hNullableFont;
    if (NULL == hFont)
    {
        // Ref: https://stackoverflow.com/a/35338788/257299
        NONCLIENTMETRICSW nonClientMetricsW = { .cbSize = sizeof(NONCLIENTMETRICSW) };
        // "If the function fails, the return value is zero."
        if (TRUE != SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,    // [in]      UINT  uiAction
                                          sizeof(NONCLIENTMETRICSW),  // [in]      UINT  uiParam
                                          &nonClientMetricsW,         // [in, out] PVOID pvParam
                                          0))                         // [in]      UINT  fWinIni
        {
            Win32LastErrorFPutWSAbort(stderr,                                  // _In_ FILE          *lpStream
                                      L"TRUE != SystemParametersInfoW(...)");  // _In_ const wchar_t *lpMessage
        }

        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfontindirectw
        hFont = CreateFontIndirectW(&nonClientMetricsW.lfMessageFont);
        if (NULL == hFont)
        {
            Win32LastErrorFPrintFWAbort(stderr,                                                                                 // _In_ FILE          *lpStream
                                        L"CreateFontIndirectW(lfHeight:%ld, lfFaceName[%ls])",                                  // _In_ const wchar_t *lpMessageFormat
                                        nonClientMetricsW.lfMessageFont.lfHeight, nonClientMetricsW.lfMessageFont.lfFaceName);  // _In_ ...
        }
    }

    lpDefaultButtonSize->hFont = hFont;

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectobject
    const HGDIOBJ prevHFont = SelectObject(hDC, (HGDIOBJ) hFont);
    if (NULL == prevHFont)
    {
        Win32LastErrorFPutWSAbort(stderr,                                  // _In_ FILE          *lpStream
                                  L"SelectObject(hDC, (HGDIOBJ) hFont)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/memset-wmemset?view=msvc-170
    memset(&lpDefaultButtonSize->fontMetrics,          // void *dest
           0,                                          // int c
           sizeof(lpDefaultButtonSize->fontMetrics));  // size_t count

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-gettextmetricsw
    if (!GetTextMetricsW(hDC, &lpDefaultButtonSize->fontMetrics))
    {
        Win32LastErrorFPutWSAbort(stderr,                                                      // _In_ FILE          *lpStream
                                  L"GetTextMetricsW(dc, &lpDefaultButtonSize->fontMetrics)");  // _In_ const wchar_t *lpMessage
    }

    SIZE fontSampleSize = {0};
    Win32TextCalcSize(hDC,                                        // _In_  HDC                hDC
                      hFont,                                      // _In_  HFONT              hNullableFont
                      &WIN32_ENGLISH_FONT_SAMPLE_WSTR,            // _In_  const struct WStr *lpWStr
                      DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE,  // _In_  UINT               format
                      &fontSampleSize);                           // _Out_ SIZE              *lpSize

    // Ref: https://learn.microsoft.com/en-us/previous-versions/ms997619(v=msdn.10)?redirectedfrom=MSDN
    // "One horizontal dialog unit is equal to one-fourth of the average character width for the current system font."
    const int cHDialogUnit      =  4;
    // "One vertical dialog unit is equal to one-eighth of an average character height for the current system font."
    const int cVDialogUnit      =  8;
    // "The default height for most single-line controls is 14 DLUs."
    const int cVButtonHeightDLU = 14;
    // "Size of Common Dialog Box Controls: Command buttons: Width (DLUs): 50"
    const int cHButtonWidthDLU  = 50;

    // Ex: 930 * 50 / (4 * 62) = 188
    lpDefaultButtonSize->buttonSize.cx =
        MulDiv(fontSampleSize.cx,                                      // [in] int nNumber
               cHButtonWidthDLU,                                       // [in] int nNumerator
               cHDialogUnit * WIN32_ENGLISH_FONT_SAMPLE_WSTR.ulSize);  // [in] int nDenominator

    // Ex: 33 * 14 / 8 = 58
    lpDefaultButtonSize->buttonSize.cy =
        MulDiv(lpDefaultButtonSize->fontMetrics.tmHeight,  // [in] int nNumber
               cVButtonHeightDLU,                          // [in] int nNumerator
               cVDialogUnit);                              // [in] int nDenominator

    LONG largestTextWidth = 0;
    for (size_t i = 0; i < WIN32_MESSAGE_BOX_BUTTON_TEXT_WSTR_ARR.ulSize; ++i)
    {
        const struct WStr *lpWStr = WIN32_MESSAGE_BOX_BUTTON_TEXT_WSTR_ARR.lpWStrArr + i;
        SIZE size = {0};
        Win32TextCalcSize(hDC,                                        // _In_  HDC                hDC
                          hFont,                                      // _In_  HFONT              hNullableFont
                          lpWStr,                                     // _In_  const struct WStr *lpWStr
                          DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE,  // _In_  UINT               format
                          &size);                                     // _Out_ SIZE              *lpSize
        if (size.cx > largestTextWidth)
        {
            largestTextWidth = size.cx;
        }
    }

    // Big assumption!: Default button width includes text width plus margins.
    // 'largestTextWidth' is only text width, and exlucded margins.
    AssertWF(largestTextWidth < lpDefaultButtonSize->buttonSize.cx,             // _In_ const bool     bAssertResult
             L"largestTextWidth:%ld < lpDefaultButtonSize->buttonSize.cx:%ld",  // _In_ const wchar_t *lpMessageFormatWCharArr
             largestTextWidth, lpDefaultButtonSize->buttonSize.cx);             // _In_ ...

    lpDefaultButtonSize->minLeftOrRightMargin = (lpDefaultButtonSize->buttonSize.cx - largestTextWidth) / 2;

    if (NULL == SelectObject(hDC, (HGDIOBJ) prevHFont))
    {
        Win32LastErrorFPutWSAbort(stderr,                                      // _In_ FILE          *lpStream
                                  L"SelectObject(hDC, (HGDIOBJ) prevHFont)");  // _In_ const wchar_t *lpMessage
    }
}

void
Win32LayoutCalcButtonSize(_In_  HDC                                  hDC,
                          _In_  struct WStr                         *lpButtonText,
                          _In_  struct Win32LayoutDefaultButtonSize *lpDefaultButtonSize,
                          _Out_ SIZE                                *lpButtonSize)
{
    assert(NULL != lpDefaultButtonSize);
    assert(NULL != lpButtonSize);

    SIZE buttonTextSize = {0};
    Win32TextCalcSize(hDC,                                        // _In_  HDC                hDC
                      lpDefaultButtonSize->hFont,                 // _In_  HFONT              hFont
                      lpButtonText,                               // _In_  const struct WStr *lpWStr
                      DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE,  // _In_  UINT               format
                      &buttonTextSize);                           // _Out_ SIZE              *lpSize

    const LONG buttonWidth = buttonTextSize.cx + (2 * lpDefaultButtonSize->minLeftOrRightMargin);

    if (buttonWidth <= lpDefaultButtonSize->buttonSize.cx)
    {
        *lpButtonSize = lpDefaultButtonSize->buttonSize;
    }
    else
    {
        lpButtonSize->cx = buttonWidth;
        lpButtonSize->cy = lpDefaultButtonSize->buttonSize.cy;
    }
}

