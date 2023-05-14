#include "wstrtoint.h"
#include "log.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW
#include <inttypes.h>  // required for wcstoimax()
#include <errno.h>
#include <stdarg.h>    // required for va_list, etc.

BOOL
WStrTryParseToUInt8(_In_    const struct WStr *lpWStr,
                    _In_    const int          base,
                    _Out_   uint8_t           *lpUInt8,
                    _Inout_ FILE              *fp,
                    _In_    const wchar_t     *lpszMsgFmt, ...)
{
    WStrAssertValid(lpWStr);
    assert(NULL != lpUInt8);
    assert(NULL != fp);
    assert(NULL != lpszMsgFmt);

    // Intentional: Clear errno before wcstoimax()
    errno = 0;
    wchar_t *endptr = NULL;
    const intmax_t lResult = wcstoimax(lpWStr->lpWCharArr, &endptr, base);
    if (ERANGE == errno)
    {
        // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
        va_list ap;
        va_start(ap, lpszMsgFmt);

        DEBUG_LOGWFV(fp, lpszMsgFmt, ap);

        va_end(ap);

        fprintf(fp, ": Value is out of range (too large or too small)\r\n");
        return FALSE;
    }

    if (endptr != lpWStr->lpWCharArr + lpWStr->ulSize)
    {
        va_list ap;
        va_start(ap, lpszMsgFmt);

        DEBUG_LOGWFV(fp, lpszMsgFmt, ap);

        va_end(ap);

        fprintf(fp, ": Failed to parse all chars -- one or more trailing non-digit chars: [%ls]\r\n", endptr);
        return FALSE;
    }

    if (lResult < 0)
    {
        va_list ap;
        va_start(ap, lpszMsgFmt);

        DEBUG_LOGWFV(fp, lpszMsgFmt, ap);

        va_end(ap);

        fprintf(fp, ": Value is negative: %jd\r\n", lResult);
        return FALSE;
    }

    if (lResult > UINT8_MAX)
    {
        va_list ap;
        va_start(ap, lpszMsgFmt);

        DEBUG_LOGWFV(fp, lpszMsgFmt, ap);

        va_end(ap);

        fprintf(fp, ": Value > UINT8_MAX: %jd > %hhu\r\n", lResult, UINT8_MAX);
        return FALSE;
    }

    *lpUInt8 = lResult;
    return TRUE;
}

