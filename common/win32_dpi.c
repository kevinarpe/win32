#include "win32_dpi.h"
#include "error_exit.h"
#include "log.h"
#include <assert.h>

// Ref: https://learn.microsoft.com/en-us/windows/win32/learnwin32/dpi-and-device-independent-pixels
// "In typography, the size of type is measured in units called points. One point equals 1/72 of an inch."
const int POINTS_PER_INCH = 72;

// Ref: https://learn.microsoft.com/en-us/windows/win32/api/windef/ne-windef-dpi_awareness
const wchar_t *WIN32_DPI_AWARENESS_TO_WCHAR_ARR_MAP[] = {
    L"DPI_AWARENESS_INVALID",            // -1
    L"DPI_AWARENESS_UNAWARE",            // 0
    L"DPI_AWARENESS_SYSTEM_AWARE",       // 1
    L"DPI_AWARENESS_PER_MONITOR_AWARE",  // 2
};

void
Win32DPIGet(_Inout_ struct Win32DPI *lpDpi,
            _In_    const HWND       hWnd)
{
    assert(NULL != lpDpi);
    // Ref: https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/DPIAwarenessPerWindow/client/DpiAwarenessContext.cpp#L241
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getthreaddpiawarenesscontext
    const DPI_AWARENESS_CONTEXT dpiCx = GetThreadDpiAwarenessContext();
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getawarenessfromdpiawarenesscontext
    const DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(dpiCx);
    if (DPI_AWARENESS_INVALID == dpiAwareness)
    {
        ErrorExitWF(L"Win32DPIGet: DPI_AWARENESS_INVALID[%d] == GetAwarenessFromDpiAwarenessContext(...)", DPI_AWARENESS_INVALID);
    }

    // Default: 96, but frequently higher for 4K montiors, e.g., 144
    UINT dpi = -1;

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/windef/ne-windef-dpi_awareness
    switch (dpiAwareness)
    {
        case DPI_AWARENESS_INVALID:
        {
            ErrorExitWF(L"Win32DPIGet: DPI_AWARENESS_INVALID[%d] == GetAwarenessFromDpiAwarenessContext(...)", DPI_AWARENESS_INVALID);
            break;
        }
        case DPI_AWARENESS_UNAWARE:
        case DPI_AWARENESS_SYSTEM_AWARE:
        {
            // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforsystem
            dpi = GetDpiForSystem();
            break;
        }
        case DPI_AWARENESS_PER_MONITOR_AWARE:
        {
            // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforwindow
            dpi = GetDpiForWindow(hWnd);  // [in] HWND hwnd
            break;
        }
        default:
        {
            ErrorExitWF(L"Win32DPIGet: Unknown DPI_AWARENESS[%d] == GetAwarenessFromDpiAwarenessContext(...)", dpiAwareness);
        }
    }

    if (dpiAwareness != lpDpi->dpiAwareness || dpi != lpDpi->dpi)
    {
        DEBUG_LOGWF(stdout, L"INFO: Win32DPIGet: DPI_AWARENESS dpiAwareness: %d/%ls->%d/%ls, UINT dpi: %u->%u\r\n",
                    lpDpi->dpiAwareness, WIN32_DPI_AWARENESS_TO_WCHAR_ARR(lpDpi->dpiAwareness),
                    dpiAwareness, WIN32_DPI_AWARENESS_TO_WCHAR_ARR(dpiAwareness),
                    *lpDpi, dpi);
    }
    lpDpi->dpiAwareness = dpiAwareness;
    lpDpi->dpi          = dpi;
}

