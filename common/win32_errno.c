#include "win32_errno.h"
#include "log.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW

void
Win32ErrnoGetWStr(_In_    const int    lastErrno,
                  _Inout_ struct WStr *lpWStr)
{
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-strerror-wcserror-wcserror?view=msvc-170
    const wchar_t *lpErrorWCharArr = _wcserror(lastErrno);

    WStrCopyWCharArr(lpWStr,                    // _Inout_ struct WStr   *lpDestWStr
                     lpErrorWCharArr,           // _In_    const wchar_t *lpSrcWCharArr
                     wcslen(lpErrorWCharArr));  // _In_    const size_t   ulSrcSize
}

void
Win32ErrnoFPutW(_In_ FILE *lpStream)
{
    assert(NULL != lpStream);

    // Ref: https://en.cppreference.com/w/c/error/errno
    // "errno is a preprocessor macro (but see note below) that expands to a thread-local (since C11) modifiable lvalue of type int."
    const int lastErrno = errno;
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-strerror-wcserror-wcserror?view=msvc-170
    const wchar_t *lpErrorWCharArr = _wcserror(lastErrno);

    LogWF(lpStream, L"ERROR: errno(%d): %ls\r\n", lastErrno, lpErrorWCharArr);
}

void
Win32ErrnoFPutWAbort(_In_ FILE *lpStream)
{
    Win32ErrnoFPutW(lpStream);
    abort();
}

void
Win32ErrnoFPutWS(_In_ FILE          *lpStream,
                 // @EmptyStringAllowed
                 _In_ const wchar_t *lpMessage)
{
    assert(NULL != lpStream);
    assert(NULL != lpMessage);

    // Ref: https://en.cppreference.com/w/c/error/errno
    // "errno is a preprocessor macro (but see note below) that expands to a thread-local (since C11) modifiable lvalue of type int."
    const int lastErrno = errno;
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-strerror-wcserror-wcserror?view=msvc-170
    const wchar_t *lpErrorWCharArr = _wcserror(lastErrno);

    if ('\0' == lpMessage[0])
    {
        LogWF(lpStream, L"ERROR: errno(%d): %ls\r\n", lastErrno, lpErrorWCharArr);
    }
    else
    {
        LogWF(lpStream, L"ERROR: %ls: errno(%d): %ls\r\n", lpMessage, lastErrno, lpErrorWCharArr);
    }
}

void
Win32ErrnoFPutWSAbort(_In_ FILE          *lpStream,
                      // @EmptyStringAllowed
                      _In_ const wchar_t *lpMessage)
{
    Win32ErrnoFPutWS(lpStream,    // _In_  FILE          *lpStream
                     lpMessage);  // _Out_ FILE          *lpErrorStream
    abort();
}

void
Win32ErrnoFPrintFW(_In_ FILE          *lpStream,
                   // @EmptyStringAllowed
                   _In_ const wchar_t *lpMessageFormat,
                   _In_ ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpMessageFormat);
    const bool b = Win32ErrnoFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
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
Win32ErrnoFPrintFWAbort(_In_ FILE          *lpStream,
                        // @EmptyStringAllowed
                        _In_ const wchar_t *lpMessageFormat,
                        _In_ ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpMessageFormat);
    Win32ErrnoFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
                         lpMessageFormat,  // _In_  const wchar_t *lpMessageFormat
                         ap,               // _In_  va_list        ap
                         stderr);          // _Out_ FILE          *lpErrorStream
    va_end(ap);
    abort();
}

bool
Win32ErrnoFPrintFW2(_In_  FILE          *lpStream,
                    _Out_ FILE          *lpErrorStream,
                    // @EmptyStringAllowed
                    _In_  const wchar_t *lpMessageFormat,
                    _In_  ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpMessageFormat);
    const bool b = Win32ErrnoFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
                                        lpMessageFormat,  // _In_  const wchar_t *lpMessageFormat
                                        ap,               // _In_  va_list        ap
                                        lpErrorStream);   // _Out_ FILE          *lpErrorStream
    va_end(ap);
    return b;
}

void
Win32ErrnoFPrintFWV(_In_  FILE          *lpStream,
                    // @EmptyStringAllowed
                    _In_  const wchar_t *lpMessageFormat,
                    _In_  va_list        ap)
{
    if (false == Win32ErrnoFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
                                      lpMessageFormat,  // _In_  const wchar_t *lpMessageFormat
                                      ap,               // _In_  va_list        ap
                                      stderr))          // _Out_ FILE          *lpErrorStream
    {
        abort();
    }
}

void
Win32ErrnoFPrintFWVAbort(_In_  FILE          *lpStream,
                         // @EmptyStringAllowed
                         _In_  const wchar_t *lpMessageFormat,
                         _In_  va_list        ap)
{
    Win32ErrnoFPrintFWV2(lpStream,         // _In_  FILE          *lpStream
                         lpMessageFormat,  // _In_  const wchar_t *lpMessageFormat
                         ap,               // _In_  va_list        ap
                         stderr);          // _Out_ FILE          *lpErrorStream
    abort();
}

bool
Win32ErrnoFPrintFWV2(_In_  FILE          *lpStream,
                     // @EmptyStringAllowed
                     _In_  const wchar_t *lpMessageFormat,
                     _In_  va_list        ap,
                     _Out_ FILE          *lpErrorStream)
{
    assert(NULL != lpStream);
    assert(NULL != lpMessageFormat);
    assert(NULL != lpErrorStream);

    // Ref: https://en.cppreference.com/w/c/error/errno
    // "errno is a preprocessor macro (but see note below) that expands to a thread-local (since C11) modifiable lvalue of type int."
    const int lastErrno = errno;
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-strerror-wcserror-wcserror?view=msvc-170
    const wchar_t *lpErrorWCharArr = _wcserror(lastErrno);

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
        const int lastErrno2 = errno;
        // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-strerror-wcserror-wcserror?view=msvc-170
        const wchar_t *lpErrorWCharArr2 = _wcserror(lastErrno2);
        LogWF(lpErrorStream, L"errno(%d): %ls\r\n", lastErrno2, lpErrorWCharArr2);
        return false;
    }

    if (strlen > 0)
    {
        fputws(L": ", lpStream);
    }

    fwprintf(lpStream, L"errno(%d): %ls\r\n", lastErrno, lpErrorWCharArr);
    return true;
}

