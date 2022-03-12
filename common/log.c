#include "log.h"
#include "win32.h"
#include <assert.h>
#include <stdlib.h>  // required for abs()

static void static_fprintf_prefix(FILE *fp)
{
    assert(NULL != fp);

    static BOOL bIsInitDone = FALSE;

    static TIME_ZONE_INFORMATION tz = {};
    if (!bIsInitDone)
    {
        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-gettimezoneinformation
        __attribute__((unused)) const DWORD dw = GetTimeZoneInformation(&tz);
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
    const int hours   = abs(tz.Bias) / 60;
    const int minutes = abs(tz.Bias) % 60;

    // Ex: "2022-03-10 22:17:47.123 +09:00 "
    fprintf(fp,
            "%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu %c%02d:%02d ",
            dt.wYear, dt.wMonth, dt.wDay, dt.wHour, dt.wMinute, dt.wSecond, dt.wMilliseconds,
            plusMinus, hours, minutes);
}

void Log(FILE *fp, const char *lpszMsg)
{
    assert(NULL != fp);
    assert(NULL != lpszMsg);

    static_fprintf_prefix(fp);
    fputs(lpszMsg, fp);
    fputs("\n", fp);
}

void LogF(FILE *fp, const char *lpszMsgFmt, ...)
{
    assert(NULL != fp);
    assert(NULL != lpszMsgFmt);

    static_fprintf_prefix(fp);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpszMsgFmt);

    vfprintf(fp, lpszMsgFmt, ap);
    fputs("\n", fp);

    va_end(ap);
}

void LogFV(FILE *fp, const char *lpszMsgFmt, va_list ap)
{
    assert(NULL != fp);
    assert(NULL != lpszMsgFmt);

    static_fprintf_prefix(fp);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap_copy;
    va_copy(ap_copy, ap);

    vfprintf(fp, lpszMsgFmt, ap_copy);
    fputs("\n", fp);

    va_end(ap_copy);
}

