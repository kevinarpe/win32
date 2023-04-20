#include "assertive.h"
#include "log.h"
#include <assert.h>  // required for assert

void
AssertW(_In_ const bool     bAssertResult,
        // @EmptyStringAllowed
        _In_ const wchar_t *lpMessageWCharArr)
{
    assert(NULL != lpMessageWCharArr);

    if (false == bAssertResult)
    {
        LogWF(stderr,                     // _In_ FILE          *fp
              L"Assert failed: %ls\r\n",  // _In_ const wchar_t *lpszMsgFmt
              lpMessageWCharArr);         // _In_ ...

        abort();
    }
}

void
AssertWF(_In_ const bool     bAssertResult,
         // @EmptyStringAllowed
         _In_ const wchar_t *lpMessageFormatWCharArr,
         ...)
{

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpMessageFormatWCharArr);

    AssertWFV(bAssertResult,            // _In_ const bool     bAssertResult
              lpMessageFormatWCharArr,  // _In_ const wchar_t *lpMessageFormatWCharArr
              ap);                      // _In_ va_list        ap

    va_end(ap);
}

void
AssertWFV(_In_ const bool     bAssertResult,
          // @EmptyStringAllowed
          _In_ const wchar_t *lpMessageFormatWCharArr,
          _In_ va_list        ap)
{
    assert(NULL != lpMessageFormatWCharArr);

//printf(">>>>>>>>>>>>>>>>>>>>> false == bAssertResult:%d\r\n", bAssertResult);
    if (false == bAssertResult)
    {
        LogWF(stderr,               // _In_ FILE          *fp
              L"Assert failed: ");  // _In_ const wchar_t *lpszMsg

        // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
        va_list ap_copy;
        va_copy(ap_copy, ap);
        // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vfprintf-vfprintf-l-vfwprintf-vfwprintf-l?view=msvc-170
        vfwprintf(stderr, lpMessageFormatWCharArr, ap_copy);
        va_end(ap_copy);

        // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fputs-fputws?view=msvc-170
        fputws(L"\r\n", stderr);

        // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/abort?view=msvc-170
        abort();
    }
}

