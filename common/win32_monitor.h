#ifndef H_COMMON_WIN32_MONITOR
#define H_COMMON_WIN32_MONITOR

#include "win32.h"

struct Win32MonitorInfo
{
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/windef/ns-windef-point
    POINT       ptZeroZero;  // Always: {.x = 0, .y = 0}
    HMONITOR    hMonitor;
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-monitorinfo
    // rcMonitor is size of full screen window, and includes system taskbar, etc.
    // rcWork is size of maximised window, and excludes system taskbar, etc.
    // Ref: https://stackoverflow.com/a/4041450/257299
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-systemparametersinfow -> SPI_GETWORKAREA
    // Ex(wine/Linux): {cbSize = 40,
    //                  rcMonitor = {left = 0, top = 0, right = 1024, bottom = 768},
    //                  rcWork = {left = 0, top = 0, right = 1024, bottom = 768},
    //                  dwFlags = 1}
    MONITORINFO monitorInfo;
};

void
Win32MonitorGetInfoForPrimary(_Out_ struct Win32MonitorInfo *lpMonitorInfo);

#endif  // H_COMMON_WIN32_MONITOR

