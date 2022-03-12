#include "error_exit.h"
#include "win32.h"
#include "log.h"
#include <assert.h>
//#include <strsafe.h>  // required for StringCchPrintf()
#include <stdio.h>

/*
// Ref: https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
void ErrorExit(_In_ const wchar_t *lpszFunction)
{
    // Retrieve the system error message for the last-error code

    wchar_t *lpMsgBuf;
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
*/

// Ref: https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
static void static_fprintf_error_then_exit(_In_ const DWORD dwLastError)
{
    const DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    wchar_t *lpWCharArr = NULL;
    const DWORD dwStrLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER
                                          | FORMAT_MESSAGE_FROM_SYSTEM
                                          | FORMAT_MESSAGE_IGNORE_INSERTS,  // [in] DWORD dwFlags
                                          NULL,                             // [in, optional] LPCVOID lpSource
                                          dwLastError,                      // [in] DWORD dwMessageId
                                          dwLanguageId,                     // [in] DWORD dwLanguageId
                                          (LPWSTR) &lpWCharArr,             // [out] LPWSTR lpBuffer
                                          0,                                // [in] DWORD nSize
                                          NULL);                            // [in, optional] va_list *Arguments
    if (0 == dwStrLen)
    {
        Log(stderr, "ERROR: 0 == FormatMessageW(...)");
        assert(0 != dwStrLen);  // Crash
    }

    LogF(stderr, "ERROR: Failed with error %lu: %ls", dwLastError, lpWCharArr);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localfree
    LocalFree(lpWCharArr);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
    ExitProcess(dwLastError);
}

void ErrorExit(_In_ const char *lpszMsg)
{
    assert(NULL != lpszMsg);

    const DWORD dwLastError = GetLastError();

    Log(stderr, lpszMsg);
    static_fprintf_error_then_exit(dwLastError);
}

void ErrorExitF(_In_ const char *lpszMsgFmt, ...)
{
    assert(NULL != lpszMsgFmt);

    const DWORD dwLastError = GetLastError();

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpszMsgFmt);

    LogFV(stderr, lpszMsgFmt, ap);

    va_end(ap);

    static_fprintf_error_then_exit(dwLastError);
}

