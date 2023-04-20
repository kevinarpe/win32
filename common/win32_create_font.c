#include "win32_create_font.h"
#include "win32_last_error.h"
#include <assert.h>

void
Win32CreateFont(_Inout_ struct Win32Font *lpWin32Font)
{
    assert(NULL != lpWin32Font);
    assert(lpWin32Font->dpi.dpi > 0);
    // '-1' for trailing '\0' (NUL) char
    assert(lpWin32Font->fontFaceNameWStr.ulSize <= sizeof(lpWin32Font->logFont.lfFaceName) - 1);

    memset(&lpWin32Font->logFont, 0, sizeof(lpWin32Font->logFont));

    // This is an (in)famous font size formula from Microsoft.
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-muldiv
    // Ref: https://jeffpar.github.io/kbarchive/kb/074/Q74299/
    //      "When an application calls the CreateFont() or CreateFontIndirect() functions and
    //       specifies a negative value for the height parameter, the font mapper provides
    //       the closest match for the character height rather than the cell height."
    // Ref: https://learn.microsoft.com/en-us/windows/win32/learnwin32/dpi-and-device-independent-pixels
    // Ref: http://www.winprog.org/tutorial/fonts.html
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logfontw
    //      "lfHeight: < 0  The font mapper transforms this value into device units and matches its absolute value against the character height of the available fonts."
    //      "For the MM_TEXT mapping mode, you can use the following formula to specify a height for a font with a specified point size:
    //       lfHeight = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);"
    // Ex: -28
    lpWin32Font->logFont.lfHeight =
        -1L * MulDiv(lpWin32Font->fontPointSize,  // [in] int nNumber
                     lpWin32Font->dpi.dpi,        // [in] int nNumerator
                     POINTS_PER_INCH);            // [in] int nDenominator

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-s-wcscpy-s-mbscpy-s?view=msvc-170
    if (wcscpy_s(lpWin32Font->logFont.lfFaceName,                                                       // [inout] wchar_t *dest
                 sizeof(lpWin32Font->logFont.lfFaceName) / sizeof(lpWin32Font->logFont.lfFaceName[0]),  // [in]    rsize_t dest_size
                 lpWin32Font->fontFaceNameWStr.lpWCharArr))                                             // [int]   const wchar_t *src
    {
        Win32LastErrorFPrintFWAbort(stderr,                                     // _In_ FILE          *lpStream,
                                    L"wcscpy_s(lpWin32Font->logFont.lfFaceName, ..., lpWin32Font->fontFaceNameWStr.lpWCharArr[%ls])",  // _In_ const wchar_t *lpMessageFormat,
                                    lpWin32Font->fontFaceNameWStr.lpWCharArr);  // ...
    }

    lpWin32Font->logFont.lfWeight = lpWin32Font->weight;
    lpWin32Font->logFont.lfItalic = lpWin32Font->isItalic;
    lpWin32Font->logFont.lfUnderline = lpWin32Font->isUnderline;
    lpWin32Font->logFont.lfStrikeOut = lpWin32Font->isStrikeOut;

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfontindirectw
    lpWin32Font->hFont = CreateFontIndirectW(&lpWin32Font->logFont);
    if (NULL == lpWin32Font->hFont)
    {
        Win32LastErrorFPrintFWAbort(stderr,                                                           // _In_ FILE          *lpStream,
                                    L"CreateFontIndirectW(%ld, \"%ls\")",                             // _In_ const wchar_t *lpMessageFormat,
                                    lpWin32Font->logFont.lfHeight, lpWin32Font->logFont.lfFaceName);  // ...
    }
}

