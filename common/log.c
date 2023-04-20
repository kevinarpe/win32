#include "log.h"
#include "win32.h"
#include <assert.h>
#include <stdlib.h>  // required for abs()

static void
LogPrefix(FILE *fp)
{
    assert(NULL != fp);

    static BOOL bIsInitDone = FALSE;

    static TIME_ZONE_INFORMATION tz = {};
    if (!bIsInitDone)
    {
        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-gettimezoneinformation
        __attribute__((unused))
        const DWORD dw = GetTimeZoneInformation(&tz);
        bIsInitDone = TRUE;
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getlocaltime
    SYSTEMTIME dt = {};
    GetLocalTime(&dt);

    // Japan Standard Time is UTC+09:00
    // ... that is 9 * 60 = 720 minutes
    // UTC = local time + bias
    // ... thus, JST bias will be *negative*.

    const char plusMinus = (tz.Bias <= 0) ? '+' : '-';
    const int  hours     = abs(tz.Bias) / 60;
    const int  minutes   = abs(tz.Bias) % 60;

    // Ex: "2022-03-10 22:17:47.123 +09:00 "
    fprintf(fp,
            "%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu %c%02d:%02d ",
            dt.wYear, dt.wMonth, dt.wDay, dt.wHour, dt.wMinute, dt.wSecond, dt.wMilliseconds,
            plusMinus, hours, minutes);
}

void
LogW(_In_ FILE          *fp,
     // @EmptyStringAllowed
     _In_ const wchar_t *lpszMsg)
{
    assert(NULL != fp);
    assert(NULL != lpszMsg);

    LogPrefix(fp);
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fputs-fputws?view=msvc-170
    fputws(lpszMsg, fp);
}

void
LogWF(_In_ FILE          *fp,
      // @EmptyStringAllowed
      _In_ const wchar_t *lpszMsgFmt,
      _In_ ...)
{
    assert(NULL != fp);
    assert(NULL != lpszMsgFmt);

    LogPrefix(fp);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpszMsgFmt);
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vfprintf-vfprintf-l-vfwprintf-vfwprintf-l?view=msvc-170
    vfwprintf(fp, lpszMsgFmt, ap);
    va_end(ap);
}

void
LogWFV(_In_ FILE          *fp,
       // @EmptyStringAllowed
       _In_ const wchar_t *lpszMsgFmt,
       _In_ va_list        ap)
{
    assert(NULL != fp);
    assert(NULL != lpszMsgFmt);

    LogPrefix(fp);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap_copy;
    va_copy(ap_copy, ap);
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vfprintf-vfprintf-l-vfwprintf-vfwprintf-l?view=msvc-170
    vfwprintf(fp, lpszMsgFmt, ap_copy);
    va_end(ap_copy);
}

