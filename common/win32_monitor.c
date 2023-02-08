#include "win32_monitor.h"
#include "error_exit.h"
#include <assert.h>

// Ref: "How do I get the handle of the primary monitor?" -> https://devblogs.microsoft.com/oldnewthing/20070809-00/?p=25643
void
Win32MonitorGetInfoForPrimary(_Out_ struct Win32MonitorInfo *lpMonitorInfo)
{
    assert(NULL != lpMonitorInfo);

    const POINT ptZeroZero = {};

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-monitorfrompoint
    const HMONITOR hPrimaryMonitor = MonitorFromPoint(ptZeroZero, MONITOR_DEFAULTTOPRIMARY);
    if (NULL == hPrimaryMonitor)
    {
        ErrorExitW(L"Win32MonitorGetInfoForPrimary: NULL == MonitorFromPoint(ptZeroZero, MONITOR_DEFAULTTOPRIMARY)");
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-monitorinfo
    MONITORINFO mi = {.cbSize = sizeof(mi)};

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmonitorinfow
    if (!GetMonitorInfo(hPrimaryMonitor, &mi))
    {
        ErrorExitW(L"Win32MonitorGetInfoForPrimary: GetMonitorInfo(hPrimaryMonitor, &mi)");
    }

    lpMonitorInfo->ptZeroZero  = ptZeroZero;
    lpMonitorInfo->hMonitor    = hPrimaryMonitor;
    lpMonitorInfo->monitorInfo = mi;
}

