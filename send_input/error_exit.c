#include "error_exit.h"
#include <windows.h>
#include <strsafe.h>  // required for StringCchPrintf()

#ifndef UNICODE
#define UNICODE
#endif

wchar_t g_lpErrorMsgBuffer[g_ulErrorMsgBufferSize] = {};

// Ref: https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
void ErrorExit(const wchar_t *lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localalloc
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lstrlenw
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcchprintfw
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        L"%s failed with error %d: %s",
        lpszFunction, dw, lpMsgBuf);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, L"Error", MB_OK);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localfree
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}

