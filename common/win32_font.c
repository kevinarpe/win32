#include "win32_font.h"
#include "win32_last_error.h"
#include <assert.h>

static struct Win32FontGetMessageBoxFont cachedFont = {
    .nonClientMetricsW = { .cbSize = sizeof(NONCLIENTMETRICSW) },
};

void
Win32FontGetMessageBoxFontCached(_Out_ struct Win32FontGetMessageBoxFont *lpFont)
{
    assert(NULL != lpFont);

    if (0 == cachedFont.hFont)
    {
        Win32FontGetMessageBoxFont(&cachedFont);
    }

    memcpy(lpFont, &cachedFont, sizeof(cachedFont));
}

void
Win32FontGetMessageBoxFont(_Out_ struct Win32FontGetMessageBoxFont *lpFont)
{
    assert(NULL != lpFont);

    // Ref: https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366920(v=vs.85)
    ZeroMemory(lpFont, sizeof(*lpFont));
    lpFont->nonClientMetricsW.cbSize = sizeof(NONCLIENTMETRICSW);

    // Ref: https://stackoverflow.com/a/35338788/257299
    // "If the function fails, the return value is zero."
    if (0 != SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,     // [in]      UINT  uiAction
                                   sizeof(NONCLIENTMETRICSW),   // [in]      UINT  uiParam
                                   &lpFont->nonClientMetricsW,  // [in, out] PVOID pvParam
                                   0))                          // [in]      UINT  fWinIni
    {
        Win32LastErrorFPutWSAbort(stderr,                                                   // _In_ FILE          *lpStream
                                  L"SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ...)");  // _In_ const wchar_t *lpMessage
    }
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfontindirectw
    lpFont->hFont = CreateFontIndirectW(&lpFont->nonClientMetricsW.lfMessageFont);
    // "If the function fails, the return value is NULL."
    if (NULL == lpFont->hFont)
    {
        Win32LastErrorFPrintFWAbort(stderr,                                                                                                 // _In_ FILE          *lpStream
                                    L"CreateFontIndirectW(lfHeight:%ld, lfFaceName[%ls])",                                                  // _In_ const wchar_t *lpMessageFormat
                                    lpFont->nonClientMetricsW.lfMessageFont.lfHeight, lpFont->nonClientMetricsW.lfMessageFont.lfFaceName);  // _In_ ...
    }
}

BOOL
Win32FontLogicalWEquals(_In_ LOGFONTW *lpLogFontWA,
                        _In_ LOGFONTW *lpLogFontWB)
{
    assert(NULL != lpLogFontWA);
    assert(NULL != lpLogFontWB);

    // Ref: https://stackoverflow.com/questions/141720/how-do-you-compare-structs-for-equality-in-c
    const BOOL x = lpLogFontWA->lfHeight == lpLogFontWB->lfHeight
        && lpLogFontWA->lfWidth          == lpLogFontWB->lfWidth
        && lpLogFontWA->lfEscapement     == lpLogFontWB->lfEscapement
        && lpLogFontWA->lfOrientation    == lpLogFontWB->lfOrientation
        && lpLogFontWA->lfWeight         == lpLogFontWB->lfWeight
        && lpLogFontWA->lfItalic         == lpLogFontWB->lfItalic
        && lpLogFontWA->lfUnderline      == lpLogFontWB->lfUnderline
        && lpLogFontWA->lfStrikeOut      == lpLogFontWB->lfStrikeOut
        && lpLogFontWA->lfCharSet        == lpLogFontWB->lfCharSet
        && lpLogFontWA->lfOutPrecision   == lpLogFontWB->lfOutPrecision
        && lpLogFontWA->lfClipPrecision  == lpLogFontWB->lfClipPrecision
        && lpLogFontWA->lfQuality        == lpLogFontWB->lfQuality
        && lpLogFontWA->lfPitchAndFamily == lpLogFontWB->lfPitchAndFamily;
    return x;
}

