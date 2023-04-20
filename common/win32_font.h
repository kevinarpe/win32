#ifndef H_COMMON_WIN32_FONT
#define H_COMMON_WIN32_FONT

#include "win32.h"

struct Win32FontGetMessageBoxFont
{
    NONCLIENTMETRICSW nonClientMetricsW;
    HFONT hFont;
};

void
Win32FontGetMessageBoxFont(_Out_ struct Win32FontGetMessageBoxFont *lpFont);

void
Win32FontGetMessageBoxFontCached(_Out_ struct Win32FontGetMessageBoxFont *lpFont);

BOOL
Win32FontLogicalWEquals(_In_ LOGFONTW *lpLogFontWA,
                        _In_ LOGFONTW *lpLogFontWB);

#endif  // H_COMMON_WIN32_FONT
