#include "win32_last_error.h"
#include "log.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW

/**
 * @param lpWStr
 *        Ex: {@code "Path not found"}
 *        Ownership of lpWStr->lpWCharArr is not passed to caller.
 */
static bool
StaticGetErrorMessageW(_In_  const DWORD  dwLastError,
                       _Out_ struct WStr *lpWStr,
                       _Out_ FILE        *lpErrorStream)
{
    assert(NULL != lpWStr);
    assert(NULL != lpErrorStream);

    // Ref: https://github.com/dotnet/runtime/blob/3b63eb1346f1ddbc921374a5108d025662fb5ffd/src/coreclr/utilcode/posterror.cpp#L113
    #define WIN32_NEW_LINE_LEN 2
    #define LP_BUFFER_WCHAR_ARR_LEN (512 + WIN32_NEW_LINE_LEN)
    // TODO: Consider thread-local storage: https://learn.microsoft.com/en-us/windows/win32/procthread/thread-local-storage
    static wchar_t lpBufferWCharArr[LP_BUFFER_WCHAR_ARR_LEN] = {0};
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-makelangid
    // "User default language"
    const DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagew
    DWORD dwStrLen =
        FormatMessageW(// "The function should search the system message-table resource(s) for the requested message."
                       FORMAT_MESSAGE_FROM_SYSTEM,       // [in] DWORD dwFlags
                       NULL,                             // [in, optional] LPCVOID lpSource
                       dwLastError,                      // [in] DWORD dwMessageId
                       dwLanguageId,                     // [in] DWORD dwLanguageId
                       lpBufferWCharArr,                 // [out] LPWSTR lpBuffer
                       LP_BUFFER_WCHAR_ARR_LEN,          // [in] DWORD nSize
                       NULL);                            // [in, optional] va_list *Arguments

    // "If the function fails, the return value is zero. To get extended error information, call GetLastError."
    if (0 == dwStrLen)
    {
        LogWF(lpErrorStream, L"ERROR: 0 == FormatMessageW(dwFlags:FORMAT_MESSAGE_ALLOCATE_BUFFER, dwMessageId:%u...)\r\n", dwLastError);
        return false;
    }

    // Ref: https://stackoverflow.com/a/30726494/257299
    // Remove remaining trailing L"\r\n"
    // Ref(iswspace): https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/isspace-iswspace-isspace-l-iswspace-l?view=msvc-170
    while (dwStrLen > 0 && iswspace(lpBufferWCharArr[dwStrLen - 1]))
    {
        --dwStrLen;
        lpBufferWCharArr[dwStrLen] = 0;
    }

    // Remove remaining trailing L"."
    if (dwStrLen > 0 && L'.' == lpBufferWCharArr[dwStrLen - 1])
    {
        --dwStrLen;
        lpBufferWCharArr[dwStrLen] = 0;
    }

    lpWStr->lpWCharArr = lpBufferWCharArr;
    lpWStr->ulSize = dwStrLen;
    return true;
}

void
Win32LastErrorGetWStr(_In_    const DWORD  dwLastError,
                      _Inout_ struct WStr *lpWStr)
{
    if (false == Win32LastErrorGetWStr2(dwLastError,  // _In_  const DWORD  dwLastError
                                        lpWStr,       // _Out_ struct WStr *lpWStr
                                        stderr))      // _Out_ FILE        *lpErrorStream
    {
        abort();
    }
}

bool
Win32LastErrorGetWStr2(_In_  const DWORD  dwLastError,
                       _Out_ struct WStr *lpWStr,
                       _Out_ FILE        *lpErrorStream)
{
    struct WStr wstr = {0};
    if (false == StaticGetErrorMessageW(dwLastError,     // _In_  const DWORD  dwLastError
                                        &wstr,           // _Out_ struct WStr *lpWStr
                                        lpErrorStream))  // _Out_ FILE        *lpErrorStream
    {
        return false;
    }

    WStrCopyWStr(lpWStr,  // _Inout_ struct WStr       *lpDestWStr
                 &wstr);  // _In_    const struct WStr *lpSrcWStr
    return true;
}

void
Win32LastErrorFPutW(_In_ FILE *lpStream)
{
    if (false == Win32LastErrorFPutW2(lpStream,  // _In_  FILE *lpStream,
                                      stderr))   // _Out_ FILE *lpErrorStream
    {
        abort();
    }
}

void
Win32LastErrorFPutWAbort(_In_ FILE *lpStream)
{
    Win32LastErrorFPutW(lpStream);
    abort();
}

bool
Win32LastErrorFPutW2(_In_  FILE *lpStream,
                     _Out_ FILE *lpErrorStream)
{
    assert(NULL != lpStream);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
    const DWORD dwLastError = GetLastError();
    struct WStr wstr = {0};
    if (false == StaticGetErrorMessageW(dwLastError,     // _In_  const DWORD  dwLastError
                                        &wstr,           // _Out_ struct WStr *lpWStr
                                        lpErrorStream))  // _Out_ FILE        *lpErrorStream
    {
        return false;
    }
    LogWF(lpStream, L"ERROR: Win32 Last Error: %u/0x%X: %ls\r\n", dwLastError, dwLastError, wstr.lpWCharArr);
    return true;
}

void
Win32LastErrorFPutWS(_In_ FILE          *lpStream,
                     // @EmptyStringAllowed
                     _In_ const wchar_t *lpMessage)
{
    if (false == Win32LastErrorFPutWS2(lpStream,   // _In_  FILE          *lpStream
                                       lpMessage,  // _In_  const wchar_t *lpMessage
                                       stderr))    // _Out_ FILE          *lpErrorStream
    {
        abort();
    }
}

void
Win32LastErrorFPutWSAbort(_In_ FILE          *lpStream,
                          // @EmptyStringAllowed
                          _In_ const wchar_t *lpMessage)
{
    Win32LastErrorFPutWS(lpStream,    // _In_  FILE *lpStream
                         lpMessage);  // _Out_ FILE *lpErrorStream
    abort();
}

bool
Win32LastErrorFPutWS2(_In_  FILE          *lpStream,
                      // @EmptyStringAllowed
                      _In_  const wchar_t *lpMessage,
                      _Out_ FILE          *lpErrorStream)
{
    assert(NULL != lpStream);
    assert(NULL != lpMessage);
    assert(NULL != lpErrorStream);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
    const DWORD dwLastError = GetLastError();
    struct WStr wstr = {0};
    if (false == StaticGetErrorMessageW(dwLastError,     // _In_  const DWORD  dwLastError
                                        &wstr,           // _Out_ struct WStr *lpWStr
                                        lpErrorStream))  // _Out_ FILE        *lpErrorStream
    {
        return false;
    }
    LogWF(lpStream, L"ERROR: %ls%lsWin32 Last Error: %u/0x%X: %ls\r\n",
          lpMessage, (0 == lpMessage[0] ? L"" : L": "), dwLastError, dwLastError, wstr.lpWCharArr);
    return true;
}

void
Win32LastErrorFPrintFW(_In_ FILE          *lpStream,
                       // @EmptyStringAllowed
                       _In_ const wchar_t *lpMessageFormat, ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpMessageFormat);
    const bool b = Win32LastErrorFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
                                            lpMessageFormat,  // _In_  const wchar_t *lpMessageFormat
                                            ap,               // _In_  va_list        ap
                                            stderr);          // _Out_ FILE          *lpErrorStream
    va_end(ap);
    if (false == b)
    {
        abort();
    }
}

void
Win32LastErrorFPrintFWAbort(_In_ FILE          *lpStream,
                            // @EmptyStringAllowed
                            _In_ const wchar_t *lpMessageFormat, ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpMessageFormat);
    Win32LastErrorFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
                             lpMessageFormat,  // _In_  const wchar_t *lpMessageFormat
                             ap,               // _In_  va_list        ap
                             stderr);          // _Out_ FILE          *lpErrorStream
    va_end(ap);
    abort();
}

bool
Win32LastErrorFPrintFW2(_In_  FILE          *lpStream,
                        _Out_ FILE          *lpErrorStream,
                        // @EmptyStringAllowed
                        _In_  const wchar_t *lpMessageFormat, ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpMessageFormat);
    const bool b = Win32LastErrorFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
                                            lpMessageFormat,  // _In_  const wchar_t *lpMessageFormat
                                            ap,               // _In_  va_list        ap
                                            lpErrorStream);   // _Out_ FILE          *lpErrorStream
    va_end(ap);
    return b;
}

void
Win32LastErrorFPrintFWV(_In_  FILE          *lpStream,
                        // @EmptyStringAllowed
                        _In_  const wchar_t *lpMessageFormat,
                        _In_  va_list        ap)
{
    if (false == Win32LastErrorFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
                                          lpMessageFormat,  // _In_  const wchar_t *lpMessageFormat
                                          ap,               // _In_  va_list        ap
                                          stderr))          // _Out_ FILE          *lpErrorStream
    {
        abort();
    }
}

void
Win32LastErrorFPrintFWVAbort(_In_  FILE          *lpStream,
                             // @EmptyStringAllowed
                             _In_  const wchar_t *lpMessageFormat,
                             _In_  va_list        ap)
{
    Win32LastErrorFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
                             lpMessageFormat,  // _In_  const wchar_t *lpMessageFormat
                             ap,               // _In_  va_list        ap
                             stderr);          // _Out_ FILE          *lpErrorStream
    abort();
}

bool
Win32LastErrorFPrintFWV2(_In_  FILE          *lpStream,
                         // @EmptyStringAllowed
                         _In_  const wchar_t *lpMessageFormat,
                         _In_  va_list        ap,
                         _Out_ FILE          *lpErrorStream)
{
    assert(NULL != lpStream);
    assert(NULL != lpMessageFormat);
    assert(NULL != lpErrorStream);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
    const DWORD dwLastError = GetLastError();

    struct WStr wstr = {0};
    if (false == StaticGetErrorMessageW(dwLastError,     // _In_  const DWORD  dwLastError
                                        &wstr,           // _Out_ struct WStr *lpWStr
                                        lpErrorStream))  // _Out_ FILE        *lpErrorStream
    {
        return false;
    }

    // Print prefix, e.g., "2023-01-31 00:51:45.064 +09:00 ERROR: "
    LogW(lpStream, L"ERROR: ");

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap_copy;
    va_copy(ap_copy, ap);
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vfprintf-vfprintf-l-vfwprintf-vfwprintf-l?view=msvc-170
    // "If execution is allowed to continue, the functions return -1 and set errno to EINVAL."
    const int strlen = vfwprintf(lpStream, lpMessageFormat, ap_copy);
    va_end(ap_copy);

    if (-1 == strlen)
    {
        // Ref: https://en.cppreference.com/w/c/error/errno
        // "errno is a preprocessor macro (but see note below) that expands to a thread-local (since C11) modifiable lvalue of type int."
        const int last_errno = errno;
        // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-strerror-wcserror-wcserror?view=msvc-170
        const wchar_t *lpErrorWCharArr = _wcserror(last_errno);
        LogWF(lpErrorStream, L"Errno:%d: %ls\r\n", last_errno, lpErrorWCharArr);
        return false;
    }

    if (strlen > 0)
    {
        fputws(L": ", lpStream);
    }

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fprintf-fprintf-l-fwprintf-fwprintf-l?view=msvc-170
    fwprintf(lpStream, L"LastError:%u/0x%X: %ls\r\n", dwLastError, dwLastError, wstr.lpWCharArr);
    return true;
}

