#include "error_exit.h"
#include "win32.h"
#include "log.h"
#include "win32_memory.h"
#include <assert.h>
#include <stdio.h>

// Ref: https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
static void
FwprintfErrorThenAbort(_In_ const DWORD dwLastError)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-makelangid
    const DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    wchar_t *lpWCharArr = NULL;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagew
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
        LogW(stderr, L"ERROR: 0 == FormatMessageW(...)\r\n");
        assert(0 != dwStrLen);  // Crash
    }

    fwprintf(stderr, L"Failed with error %lu: %ls\r\n", dwLastError, lpWCharArr);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localfree
//    LocalFree(lpWCharArr);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
//    ExitProcess(dwLastError);

    abort();
}

static void
PrintLastErrorThenAbort(_In_ const DWORD dwLastError)
{
    if (0 == dwLastError)
    {
        fputws(L"\r\n", stderr);
        abort();
    }
    else
    {
        fputws(L": ", stderr);
        FwprintfErrorThenAbort(dwLastError);
    }
}

void
ErrorExitW(// @EmptyStringAllowed
           _In_ const wchar_t *lpszMsg)
{
    assert(NULL != lpszMsg);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
    const DWORD dwLastError = GetLastError();

    // Print prefix, e.g., "2023-01-31 00:51:45.064 +09:00 ERROR: "
    LogW(stderr, L"ERROR: ");
    fputws(lpszMsg, stderr);
    PrintLastErrorThenAbort(dwLastError);
}

void
ErrorExitWF(// @EmptyStringAllowed
            _In_ const wchar_t *lpszMsgFmt, ...)
{
    assert(NULL != lpszMsgFmt);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
    const DWORD dwLastError = GetLastError();

    // Print prefix, e.g., "2023-01-31 00:51:45.064 +09:00 ERROR: "
    LogW(stderr, L"ERROR: ");

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpszMsgFmt);
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vfprintf-vfprintf-l-vfwprintf-vfwprintf-l?view=msvc-170
    vfwprintf(stderr, lpszMsgFmt, ap);
    va_end(ap);

    PrintLastErrorThenAbort(dwLastError);
}


void
ErrorExitWFV(// @EmptyStringAllowed
             _In_ const wchar_t *lpszMsgFmt,
             _In_ va_list        ap)
{
    assert(NULL != lpszMsgFmt);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
    const DWORD dwLastError = GetLastError();

    // Print prefix, e.g., "2023-01-31 00:51:45.064 +09:00 ERROR: "
    LogW(stderr, L"ERROR: ");

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap_copy;
    va_copy(ap_copy, ap);
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vfprintf-vfprintf-l-vfwprintf-vfwprintf-l?view=msvc-170
    vfwprintf(stderr, lpszMsgFmt, ap_copy);
    va_end(ap_copy);

    PrintLastErrorThenAbort(dwLastError);
}

