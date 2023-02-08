#ifndef H_COMMON_WIN32_DPI
#define H_COMMON_WIN32_DPI

#include "win32.h"
#include "wstr.h"

// Ref: https://learn.microsoft.com/en-us/windows/win32/learnwin32/dpi-and-device-independent-pixels
// "In typography, the size of type is measured in units called points. One point equals 1/72 of an inch."
extern const int POINTS_PER_INCH;  // = 72;

extern const wchar_t *WIN32_DPI_AWARENESS_TO_WCHAR_ARR_MAP[];

#define WIN32_DPI_AWARENESS_TO_WCHAR_ARR(/* DPI_AWARENESS */ dpiAwareness) \
    WIN32_DPI_AWARENESS_TO_WCHAR_ARR_MAP[1 + (dpiAwareness)]

struct Win32DPI
{
    DPI_AWARENESS dpiAwareness;
    // Note: Default is 96 (100% scaling): #define USER_DEFAULT_SCREEN_DPI ...
    UINT          dpi;
};

#define WIN32_DPI_UNSET -1

// Ex: struct DPI dpi = WIN32_DPI_INIT;
#define WIN32_DPI_INIT \
    ((struct Win32DPI) {                       \
        .dpiAwareness = DPI_AWARENESS_INVALID, \
        .dpi = -1,                             \
    });

/**
 * @param lpDpi
 *        Callers must init the struct with: DPI_INIT(struct DPI dpi)
 */
void
Win32DPIGet(_Inout_ struct Win32DPI *lpDpi,
            _In_    const HWND       hWnd);

#endif  // H_COMMON_WIN32_DPI

