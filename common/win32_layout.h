#ifndef H_COMMON_WIN32_LAYOUT
#define H_COMMON_WIN32_LAYOUT

#include "win32.h"
#include "wstr.h"

struct Win32LayoutDefaultButtonSize
{
    /**
     * if (NULL == hNullableFont), then:
     * hFont = SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ...) -> CreateFontIndirectW(NONCLIENTMETRICSW.lfMessageFont)
     */
    HFONT       hFont;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-textmetricw
    TEXTMETRICW fontMetrics;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/windef/ns-windef-size
    /** Standard push button size according to Win32 design guidelines. */
    SIZE        buttonSize;
    /**
     * Minimum left or right margin for a button.  This is calculated as one half of default button width minus the
     * largest text width for all standard message box buttons, e.g., "Yes", "No", "Cancel", etc.
     * To be clear, this margin should be applied twice (left and right) for full button width.
     */
    UINT        minLeftOrRightMargin;
};

/**
 * Computes the default button size according to Win32 design guidelines.
 *
 * @param hDC
 *        result of {@link GetDC()}
 *
 * @param hNullableFont
 *        if {@code null}, load font via {@code SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ...)}
 *        <br>and: {@code NONCLIENTMETRICSW.lfMessageFont}
 *
 * @param lpDefaultButtonSize
 *        function result
 */
void
Win32LayoutCalcDefaultButtonSize(_In_  HDC                                  hDC,
                                 // @Nullable
                                 _In_  HFONT                                hNullableFont,
                                 _Out_ struct Win32LayoutDefaultButtonSize *lpDefaultButtonSize);

/**
 * Computes the recommended size of a button, given its text and default button size.
 *
 * @param hDC
 *        result of {@link GetDC()}
 *
 * @param lpButtonText
 *        text for push button, e.g., {@code L"Help"}
 *        This should not include ampersand (&) for mnemonic character indicator, e.g., {@code L"&Help"}.
 *
 * @param lpDefaultButtonSize
 *        result from {@code Win32LayoutCalcDefaultButtonSize()}
 *
 * @param lpButtonSize
 *        function result
 */
void
Win32LayoutCalcButtonSize(_In_  HDC                                  hDC,
                          _In_  struct WStr                         *lpButtonText,
                          _In_  struct Win32LayoutDefaultButtonSize *lpDefaultButtonSize,
                          _Out_ SIZE                                *lpButtonSize);

#endif  // H_COMMON_WIN32_LAYOUT

