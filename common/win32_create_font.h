#ifndef H_COMMON_WIN32_CREATE_FONT
#define H_COMMON_WIN32_CREATE_FONT

#include "win32.h"
#include "wstr.h"
#include "win32_dpi.h"

struct Win32Font
{
    // Inputs
    // Classic font "point" size, e.g., 14
    UINT            fontPointSize;
    struct Win32DPI dpi;
    // "The weight of the font in the range 0 through 1000. For example, 400 (FW_NORMAL/FW_REGULAR) is normal
    //  and 700 (FW_BOLD) is bold. If this value is zero, a default weight is used."
    LONG            weight;
    BOOL            isItalic;
    BOOL            isUnderline;
    BOOL            isStrikeOut;
    // Ref: https://learn.microsoft.com/en-us/typography/font-list/consolas
    // Ex: L"Consolas"
    struct WStr     fontFaceNameWStr;

    // Outputs
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logfontw
    LOGFONTW        logFont;
    HFONT           hFont;
};

void
Win32CreateFont(_Inout_ struct Win32Font *lpWin32Font);

#endif  // H_COMMON_WIN32_CREATE_FONT
