#include "wstr.h"
#include "xmalloc.h"
#include "log.h"
#include "win32_last_error.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW
#include <windows.h>
#include <errno.h>

void
SafeWCharArrCopy(_Out_ wchar_t       *lpDestWCharArr,                // dest wstr ptr
                 _In_  size_t         ulDestWCharArrLenPlusNulChar,  // dest wstr length (including null char)
                 _In_  const wchar_t *lpSrcWCharArr,                 // source wstr ptr
                 _In_  const size_t   ulWCharCount)                  // wchar count to be copied (excluding null char)
{
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strncpy-s-strncpy-s-l-wcsncpy-s-wcsncpy-s-l-mbsncpy-s-mbsncpy-s-l?view=msvc-170
    const errno_t e = wcsncpy_s(lpDestWCharArr,
                                ulDestWCharArrLenPlusNulChar,
                                lpSrcWCharArr,
                                ulWCharCount);
    if (0 != e)
    {
        // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strerror-strerror-wcserror-wcserror?view=msvc-170
        const wchar_t *lpErrWCharArr = _wcserror(e);
        Win32LastErrorFPrintFWAbort(stderr,  // _In_ FILE          *lpStream,
                                    L"Fatal error: wcsncpy_s(..., ulDestWCharArrLenPlusNulChar[%zd], lpSrcWCharArr[%ls], ulWCharCount[%zd]) -> %d: [%ls]",  // _In_ const wchar_t *lpMessageFormat,
                                    ulDestWCharArrLenPlusNulChar, lpSrcWCharArr, ulWCharCount, e, lpErrWCharArr);  // _In_ ...
    }
}

void
WStrAssertValid(_In_ const struct WStr *lpWStr)
{
    assert(NULL != lpWStr);
    if (lpWStr->ulSize > 0)
    {
        assert(NULL != lpWStr->lpWCharArr);
    }
}

void
WStrFree(_Inout_ struct WStr *lpWStr)
{
    WStrAssertValid(lpWStr);

    xfree((void **) &(lpWStr->lpWCharArr));
    lpWStr->ulSize = 0;  // Explicit
}

BOOL
WStrIsEmpty(_In_ const struct WStr *lpWStr)
{
    WStrAssertValid(lpWStr);

    const BOOL x = (0 == lpWStr->ulSize);
    return x;
}

int
WStrCompare(_In_ const struct WStr *lpWStrLeft,
            _In_ const struct WStr *lpWStrRight)
{
    WStrAssertValid(lpWStrLeft);
    WStrAssertValid(lpWStrRight);

    const wchar_t *lpLeftWCharArr  = (0 == lpWStrLeft->ulSize ) ? L"" : lpWStrLeft->lpWCharArr;
    const wchar_t *lpRightWCharArr = (0 == lpWStrRight->ulSize) ? L"" : lpWStrRight->lpWCharArr;

    const int cmp = wcscmp(lpLeftWCharArr, lpRightWCharArr);
    return cmp;
}

static void
WStrCopyWCharArr0(_Inout_ struct WStr   *lpDestWStr,
                  _In_    const wchar_t *lpSrcWCharArr,
                  _In_    const size_t   ulSrcSize)
{
    WStrFree(lpDestWStr);
    // Intentional: Allow (NULL == lpSrcWCharArr)

    lpDestWStr->ulSize = ulSrcSize;
    if (0 == ulSrcSize) {
        return;
    }

    lpDestWStr->lpWCharArr = xcalloc(lpDestWStr->ulSize + LEN_NUL_CHAR, sizeof(wchar_t));

    SafeWCharArrCopy(lpDestWStr->lpWCharArr,
                     lpDestWStr->ulSize + LEN_NUL_CHAR,
                     lpSrcWCharArr,
                     ulSrcSize);
}

void
WStrCopyWCharArr(_Inout_ struct WStr   *lpDestWStr,
                 _In_    const wchar_t *lpSrcWCharArr,
                 _In_    const size_t   ulSrcSize)
{
//    assert(NULL != lpSrcWCharArr);
    if (NULL == lpSrcWCharArr)
    {
        assert(0 == ulSrcSize);
    }

    WStrCopyWCharArr0(lpDestWStr, lpSrcWCharArr, ulSrcSize);
}

void
WStrCopyWStr(_Inout_ struct WStr       *lpDestWStr,
             _In_    const struct WStr *lpSrcWStr)
{
    WStrAssertValid(lpSrcWStr);

    WStrCopyWCharArr0(lpDestWStr, lpSrcWStr->lpWCharArr, lpSrcWStr->ulSize);
}

void
WStrSPrintF(_Inout_ struct WStr   *lpDestWStr,
            // @EmptyStringAllowed
            _In_    const wchar_t *lpFormatWCharArr,
            _In_    ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-172
    va_list ap;
    va_start(ap, lpFormatWCharArr);
    if (false == WStrSPrintFV2(lpDestWStr,        // _Inout_ struct WStr   *lpDestWStr
                               stderr,            // _In_    FILE          *lpErrorStream
                               lpFormatWCharArr,  // _In_    const wchar_t *lpFormatWCharArr
                               ap))               // _In_    va_list        ap
    {
        abort();
    }
}

bool
WStrSPrintF2(_Inout_ struct WStr   *lpDestWStr,
             _In_    FILE          *lpErrorStream,
             // @EmptyStringAllowed
             _In_    const wchar_t *lpFormatWCharArr,
             _In_    ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-172
    va_list ap;
    va_start(ap, lpFormatWCharArr);
    const bool b = WStrSPrintFV2(lpDestWStr,        // _Inout_ struct WStr   *lpDestWStr
                                 lpErrorStream,     // _In_    FILE          *lpErrorStream
                                 lpFormatWCharArr,  // _In_    const wchar_t *lpFormatWCharArr
                                 ap);               // _In_    va_list        ap
    va_end(ap);
    return b;
}

void
WStrSPrintFV(_Inout_ struct WStr   *lpDestWStr,
             // @EmptyStringAllowed
             _In_    const wchar_t *lpFormatWCharArr,
             _In_    va_list        ap)
{
    if (false == WStrSPrintFV2(lpDestWStr,        // _Inout_ struct WStr   *lpDestWStr
                               stderr,            // _In_    FILE          *lpErrorStream
                               lpFormatWCharArr,  // _In_    const wchar_t *lpFormatWCharArr
                               ap))               // _In_    va_list        ap
    {
        abort();
    }
}

bool
WStrSPrintFV2(_Inout_ struct WStr   *lpDestWStr,
              _In_    FILE          *lpErrorStream,
              // @EmptyStringAllowed
              _In_    const wchar_t *lpFormatWCharArr,
              _In_    va_list        ap)
{
    WStrFree(lpDestWStr);
    assert(NULL != lpErrorStream);
    assert(NULL != lpFormatWCharArr);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-172
    va_list ap_copy;
    va_copy(ap_copy, ap);

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-vsprintf-l-vswprintf-vswprintf-l-vswprintf-l?view=msvc-170
    // Ref: https://en.cppreference.com/w/c/io/vfwprintf
    // "Writes the results to a wide string buffer. At most bufsz-1 wide characters are written followed by null wide character.
    //  The resulting wide character string will be terminated with a null wide character, unless bufsz is zero."
    // "The number of wide characters written if successful or negative value if an error occurred.
    //  If the resulting string gets truncated due to bufsz limit, function returns the total number of characters
    //  (not including the terminating null wide character) which would have been written, if the limit were not imposed."
    const int cch = vswprintf(NULL,              // wchar_t *buffer
                              0,                 // size_t bufsz
                              lpFormatWCharArr,  // const wchar_t *format
                              ap_copy);          // va_list vlist
    va_end(ap_copy);

    if (cch < 0)
    {
        Win32LastErrorFPrintFW2(lpErrorStream,      // _In_  FILE          *lpStream
                                lpErrorStream,      // _Out_ FILE          *lpErrorStream
                                L"Internal error: -1 == vswprintf(NULL, 0, lpFormatWCharArr[%ls], ap_copy)",  // _In_  const wchar_t *lpMessageFormat
                                lpFormatWCharArr);  // _In_  ...
        return false;
    }

    lpDestWStr->ulSize = cch;
    lpDestWStr->lpWCharArr = xcalloc(lpDestWStr->ulSize + LEN_NUL_CHAR, sizeof(wchar_t));

    va_list ap_copy2;
    va_copy(ap_copy2, ap);

    const int cch2 = vswprintf(lpDestWStr->lpWCharArr,             // wchar_t *buffer
                               lpDestWStr->ulSize + LEN_NUL_CHAR,  // size_t bufsz
                               lpFormatWCharArr,                   // const wchar_t *format
                               ap_copy2);                          // va_list vlist
    va_end(ap_copy2);

    if (cch2 < 0)
    {
        WStrFree(lpDestWStr);
        Win32LastErrorFPrintFW2(lpErrorStream,                                         // _In_  FILE          *lpStream
                                lpErrorStream,                                         // _Out_ FILE          *lpErrorStream
                                L"Internal error: -1 == vswprintf(lpDestWStr->lpWCharArr, lpDestWStr->ulSize + LEN_NUL_CHAR[%zd], lpFormatWCharArr[%ls], ap_copy)",  // _In_  const wchar_t *lpMessageFormat
                                lpDestWStr->ulSize + LEN_NUL_CHAR, lpFormatWCharArr);  // _In_  ...
        return false;
    }

    if (cch != cch2)
    {
        WStrFree(lpDestWStr);
        LogWF(lpErrorStream, L"Internal error: cch[%d] != cch2[%d]", cch, cch2);
        return false;
    }

    return true;
}

void
WStrConcat(_Inout_ struct WStr       *lpDestWStr,
           _In_    const struct WStr *lpSrcWStr,
           _In_    const struct WStr *lpSrcWStr2)
{
    WStrAssertValid(lpSrcWStr);
    WStrAssertValid(lpSrcWStr2);

    WStrFree(lpDestWStr);

    lpDestWStr->ulSize = lpSrcWStr->ulSize + lpSrcWStr2->ulSize;
    if (0 == lpDestWStr->ulSize) {
        return;
    }

    lpDestWStr->lpWCharArr = xcalloc(lpDestWStr->ulSize + LEN_NUL_CHAR, sizeof(wchar_t));

    SafeWCharArrCopy(lpDestWStr->lpWCharArr,
                     lpSrcWStr->ulSize + LEN_NUL_CHAR,
                     lpSrcWStr->lpWCharArr,
                     lpSrcWStr->ulSize);

    SafeWCharArrCopy(lpDestWStr->lpWCharArr + lpSrcWStr->ulSize,
                     lpSrcWStr2->ulSize + LEN_NUL_CHAR,
                     lpSrcWStr2->lpWCharArr,
                     lpSrcWStr2->ulSize);
}

void
WStrConcatMany(_Inout_ struct WStr       *lpDestWStr,
               _In_    const struct WStr *lpSrcWStr,
               _In_    const struct WStr *lpSrcWStr2,
               _In_    ...)
{
    WStrAssertValid(lpSrcWStr);
    WStrAssertValid(lpSrcWStr2);

    WStrFree(lpDestWStr);

    lpDestWStr->ulSize = lpSrcWStr->ulSize + lpSrcWStr2->ulSize;

    va_list ap;
    va_start(ap, lpSrcWStr2);
    while (TRUE)
    {
        struct WStr *lpWStr = va_arg(ap, struct WStr *);
        if (NULL == lpWStr)
        {
            break;
        }
        lpDestWStr->ulSize += lpWStr->ulSize;
    }
    va_end(ap);

    if (0 == lpDestWStr->ulSize) {
        return;
    }

    lpDestWStr->lpWCharArr = xcalloc(lpDestWStr->ulSize + LEN_NUL_CHAR, sizeof(wchar_t));

    size_t ulOffset = 0;
    SafeWCharArrCopy(lpDestWStr->lpWCharArr + ulOffset,
                     lpSrcWStr->ulSize + LEN_NUL_CHAR,
                     lpSrcWStr->lpWCharArr,
                     lpSrcWStr->ulSize);

    ulOffset += lpSrcWStr->ulSize;

    SafeWCharArrCopy(lpDestWStr->lpWCharArr + ulOffset,
                     lpSrcWStr2->ulSize + LEN_NUL_CHAR,
                     lpSrcWStr2->lpWCharArr,
                     lpSrcWStr2->ulSize);

    ulOffset += lpSrcWStr2->ulSize;

    va_list ap2;
    va_start(ap2, lpSrcWStr2);
    while (TRUE)
    {
        struct WStr *lpWStr = va_arg(ap2, struct WStr *);
        if (NULL == lpWStr)
        {
            break;
        }

        SafeWCharArrCopy(lpDestWStr->lpWCharArr + ulOffset,
                         lpWStr->ulSize + LEN_NUL_CHAR,
                         lpWStr->lpWCharArr,
                         lpWStr->ulSize);

        ulOffset += lpWStr->ulSize;
    }
    va_end(ap2);
}

void
WStrTrim(_Inout_ struct WStr                 *lpWStr,
         _In_    const enum EWStrTrim         eWStrTrim,
         _In_    const WStrCharPredicateFunc  fpWStrCharPredicateFunc)
{
    WStrAssertValid(lpWStr);
    assert(eWStrTrim >= WSTR_LTRIM && eWStrTrim <= (WSTR_LTRIM | WSTR_RTRIM));
    assert(NULL != fpWStrCharPredicateFunc);

    if (0 == lpWStr->ulSize) {
        return;
    }

    // Iterate forward to count leading whitespace chars
    size_t ulLeadingCount = 0;
    if (0 != (eWStrTrim & WSTR_LTRIM))
    {
        for (size_t i = 0; i < lpWStr->ulSize; ++i)
        {
            const wchar_t ch = lpWStr->lpWCharArr[i];
            if (!fpWStrCharPredicateFunc(ch)) {
                break;
            }
            ++ulLeadingCount;
        }
    }

    // If trim all wchars, then free lpWStr.
    if (lpWStr->ulSize == ulLeadingCount)
    {
        WStrFree(lpWStr);
        return;
    }

    // Iterate backward to count trailing whitespace chars
    size_t ulTrailingCount = 0;
    if (0 != (eWStrTrim & WSTR_RTRIM))
    {
        // Note: Reverse iteration is tricky with unsigned values in C!  Example: 0U - 1U == SIZE_MAX
        for (size_t i = lpWStr->ulSize - 1U; i < SIZE_MAX; --i)
        {
            const wchar_t ch = lpWStr->lpWCharArr[i];
            if (!fpWStrCharPredicateFunc(ch)) {
                break;
            }
            ++ulTrailingCount;
        }
    }

    // If trim all wchars, then free lpWStr.
    if (lpWStr->ulSize == ulTrailingCount)
    {
        WStrFree(lpWStr);
        return;
    }

    const size_t ulTrimmedSize = lpWStr->ulSize - ulLeadingCount - ulTrailingCount;

    wchar_t *lpWCharArr = xcalloc(ulTrimmedSize + LEN_NUL_CHAR, sizeof(wchar_t));

    SafeWCharArrCopy(lpWCharArr,
                     ulTrimmedSize + LEN_NUL_CHAR,
                     lpWStr->lpWCharArr + ulLeadingCount,
                     ulTrimmedSize);

    WStrFree(lpWStr);

    lpWStr->lpWCharArr = lpWCharArr;
    lpWStr->ulSize     = ulTrimmedSize;
}

void
WStrLTrimSpace(_Inout_ struct WStr *lpWStr)
{
    WStrTrim(lpWStr, WSTR_LTRIM, iswspace);
}

void
WStrRTrimSpace(_Inout_ struct WStr *lpWStr)
{
    WStrTrim(lpWStr, WSTR_RTRIM, iswspace);
}

void
WStrTrimSpace(_Inout_ struct WStr *lpWStr)
{
    WStrTrim(lpWStr, WSTR_LTRIM | WSTR_RTRIM, iswspace);
}

const int UNLIMITED_MIN_TOKEN_COUNT = -1;
const int UNLIMITED_MAX_TOKEN_COUNT = -1;

static void
WStrSplit0(_In_    const struct WStr             *lpWStrText,
           _In_    const struct WStr             *lpWStrDelim,
           _In_    const struct WStrSplitOptions *lpOptions,
           _In_    const BOOL                     bDiscardFinalEmptyToken,
           _Inout_ struct WStrArr                *lpTokenWStrArr)
{
    WStrAssertValid(lpWStrText);
    WStrAssertValid(lpWStrDelim);
    // Do not allow empty delim
    assert(0 != lpWStrDelim->ulSize);
    assert(NULL != lpOptions);
    assert(UNLIMITED_MIN_TOKEN_COUNT == lpOptions->iMinTokenCount || lpOptions->iMinTokenCount >= 1);
    assert(UNLIMITED_MAX_TOKEN_COUNT == lpOptions->iMaxTokenCount || lpOptions->iMaxTokenCount >= 2);
    WStrArrFree(lpTokenWStrArr);

    // Important: Text after last delim is final token.
    const int iMaxDelimCount = (UNLIMITED_MAX_TOKEN_COUNT == lpOptions->iMaxTokenCount)
        ? UNLIMITED_MAX_TOKEN_COUNT : (lpOptions->iMaxTokenCount - 1);
    size_t ulDelimCount = 0;

    // Step 1: Count number of delimiters
    const wchar_t *lpIter = lpWStrText->lpWCharArr;
    while (TRUE)
    {
        const wchar_t *lpNextDelim = wcsstr(lpIter, lpWStrDelim->lpWCharArr);
        if (NULL == lpNextDelim) {
            break;
        }

        lpIter = lpNextDelim + lpWStrDelim->ulSize;

        // This condition only applies if at least one delimiter is found.
        // Why?  Both "abc" and "abc\r\n" will split as: ["abc"]
        // And: "\r\n" will split as [""]
        // But: "" will also split as [""]
        if (L'\0' == *lpIter && TRUE == bDiscardFinalEmptyToken) {
            break;
        }

        ++ulDelimCount;

        if (UNLIMITED_MAX_TOKEN_COUNT != iMaxDelimCount && ((size_t) iMaxDelimCount) == ulDelimCount) {
            break;
        }
    }

    const size_t ulTokenCount = 1U + ulDelimCount;

    if (UNLIMITED_MIN_TOKEN_COUNT != lpOptions->iMinTokenCount && ulTokenCount < ((size_t) lpOptions->iMinTokenCount))
    {
        Win32LastErrorFPrintFWAbort(stderr,  // _In_ FILE          *lpStream,
                                    L"ERROR: Failed to split [%ls] with delim [%ls]: ulTokenCount < lpOptions->iMinTokenCount: %lu < %d",  // _In_ const wchar_t *lpMessageFormat,
                                    lpWStrText->lpWCharArr, lpWStrDelim->lpWCharArr, ulTokenCount, lpOptions->iMinTokenCount);             // _In_ ...
    }

    WStrArrAlloc(lpTokenWStrArr, ulTokenCount);

    // Step 2: Extract tokens between delimiters
    lpIter = lpWStrText->lpWCharArr;
    for (size_t ulTokenIndex = 0; ulTokenIndex < ulTokenCount; ++ulTokenIndex)
    {
        // Intention: Include (UNLIMITED_MAX_TOKEN_COUNT == lpOptions->iMaxTokenCount) for readability.
        const BOOL bIsLastToken =
            (UNLIMITED_MAX_TOKEN_COUNT == lpOptions->iMaxTokenCount)
                ? FALSE : (1U + ulTokenIndex == ((size_t) lpOptions->iMaxTokenCount));

        const wchar_t *lpNextDelim = wcsstr(lpIter, lpWStrDelim->lpWCharArr);
        if (NULL == lpNextDelim || bIsLastToken)
        {
            // Point to trailing null char
            lpNextDelim = lpWStrText->lpWCharArr + lpWStrText->ulSize;
        }

        const ptrdiff_t lTokenLen = lpNextDelim - lpIter;
        // Note: Empty token is allowed, e.g., L""
        assert(lTokenLen >= 0);

        WStrCopyWCharArr(lpTokenWStrArr->lpWStrArr + ulTokenIndex, lpIter, lTokenLen);

        // Note: In final iteration, 'lpIter' may point to an invalid memory location.
        lpIter = lpNextDelim + lpWStrDelim->ulSize;
    }

    if (NULL != lpOptions->fpNullableWStrConsumerFunc)
    {
        WStrArrForEach(lpTokenWStrArr, lpOptions->fpNullableWStrConsumerFunc);
    }
}

void
WStrSplit(_In_    const struct WStr             *lpWStrText,
          _In_    const struct WStr             *lpWStrDelim,
          _In_    const struct WStrSplitOptions *lpOptions,
          _Inout_ struct WStrArr                *lpTokenWStrArr)
{
    const BOOL bDiscardFinalEmptyToken = FALSE;
    WStrSplit0(lpWStrText, lpWStrDelim, lpOptions, bDiscardFinalEmptyToken, lpTokenWStrArr);
}

void
WStrSplitNewLine(_In_    const struct WStr             *lpWStrText,
                 _In_    const struct WStrSplitOptions *lpOptions,
                 _Inout_ struct WStrArr                *lpTokenWStrArr)
{
    WStrAssertValid(lpWStrText);
    WStrArrFree(lpTokenWStrArr);

    const BOOL bDiscardFinalEmptyToken = TRUE;

    if (NULL != wcsstr(lpWStrText->lpWCharArr, L"\r\n"))
    {
        struct WStr wstrDelim = {.lpWCharArr = L"\r\n", .ulSize = 2};
        WStrSplit0(lpWStrText, &wstrDelim, lpOptions, bDiscardFinalEmptyToken, lpTokenWStrArr);
    }
    else  // Intentional: Do not check if contains L"\n".  Why?  Always apply rules for lpOptions->iMinTokenCount.
    {
        struct WStr wstrDelim = {.lpWCharArr = L"\n", .ulSize = 1};
        WStrSplit0(lpWStrText, &wstrDelim, lpOptions, bDiscardFinalEmptyToken, lpTokenWStrArr);
    }
}

void
WStrJoin(_In_    const struct WStrArr *lpWStrArr,
         _In_    const struct WStr    *lpDelimWStr,
         _Inout_ struct WStr          *lpDestWStr)
{
    assert(NULL != lpWStrArr);
    assert(NULL != lpDelimWStr);
    assert(NULL != lpDestWStr);

    WStrArrAssertValid(lpWStrArr);
    WStrAssertValid(lpDelimWStr);
    WStrFree(lpDestWStr);

    if (0 == lpWStrArr->ulSize) {
        return;
    }

    lpDestWStr->ulSize = lpDelimWStr->ulSize * (lpWStrArr->ulSize - 1UL);

    for (size_t i = 0; i < lpWStrArr->ulSize; ++i)
    {
        const struct WStr *lpWStr = lpWStrArr->lpWStrArr + i;
        lpDestWStr->ulSize += lpWStr->ulSize;
    }

    // We are trying to join all empty strings with empty delim!
    if (0 == lpDestWStr->ulSize) {
        return;
    }

    lpDestWStr->lpWCharArr = xcalloc(lpDestWStr->ulSize + LEN_NUL_CHAR, sizeof(wchar_t));

    wchar_t *lpWCharArr = lpDestWStr->lpWCharArr;
    for (size_t i = 0; i < lpWStrArr->ulSize; ++i)
    {
        const struct WStr *lpWStr = lpWStrArr->lpWStrArr + i;
        if (i > 0)
        {
            SafeWCharArrCopy(lpWCharArr,                          // dest wstr ptr
                             lpDelimWStr->ulSize + LEN_NUL_CHAR,  // dest wstr length (including null char)
                             lpDelimWStr->lpWCharArr,             // source wstr ptr
                             lpDelimWStr->ulSize);                // wchar count to be copied (excluding null char)

            lpWCharArr += lpDelimWStr->ulSize;
        }

        SafeWCharArrCopy(lpWCharArr,                     // dest wstr ptr
                         lpWStr->ulSize + LEN_NUL_CHAR,  // dest wstr length (including null char)
                         lpWStr->lpWCharArr,             // source wstr ptr
                         lpWStr->ulSize);                // wchar count to be copied (excluding null char)

        lpWCharArr += lpWStr->ulSize;
    }
}

// Maybe: WStrFileAppend
// Ref: https://docs.microsoft.com/en-us/windows/win32/fileio/appending-one-file-to-another-file

void
WStrFileWrite(_In_ const wchar_t     *lpFilePath,
              _In_ const UINT         codePage,  // Ex: CP_UTF8
              _In_ const struct WStr *lpWStr)
{
    assert(NULL != lpFilePath);
    assert(NULL != lpWStr);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
    const HANDLE hWriteFile = CreateFile(lpFilePath,             // [in] LPCWSTR lpFileName
                                         GENERIC_WRITE,          // [in] DWORD dwDesiredAccess
                                         0,                      // [in] DWORD dwShareMode
                                         NULL,                   // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes
                                         CREATE_ALWAYS,          // [in] DWORD dwCreationDisposition
                                         FILE_ATTRIBUTE_NORMAL,  // [in] DWORD dwFlagsAndAttributes
                                         NULL);                  // [in, optional] hTemplateFile
    if (INVALID_HANDLE_VALUE == hWriteFile)
    {
        Win32LastErrorFPrintFWAbort(stderr,       // _In_ FILE          *lpStream,
                                    L"CreateFile(lpFilePath[%ls], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)",  // _In_ const wchar_t *lpMessageFormat,
                                    lpFilePath);  // _In_ ...
    }

    if (lpWStr->ulSize > 0)
    {
        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
        // Note: If 'cchWideChar' == -1, then resulting count includes trailing null char.
        const int iCharArrLen = WideCharToMultiByte(codePage,              // [in] UINT CodePage
                                                    WC_ERR_INVALID_CHARS,  // [in] DWORD dwFlags
                                                    lpWStr->lpWCharArr,    // [in] LPCWCH lpWideCharStr
                                                    -1,                    // [in] int cchWideChar
                                                    NULL,                  // [out/opt] LPSTR lpMultiByteStr
                                                    0,                     // [in] int cbMultiByte
                                                    NULL,                  // [in/opt] LPCCH lpDefaultChar
                                                    NULL);                 // [out/opt] LPBOOL lpUsedDefaultChar
        assert(iCharArrLen >= 0);
        if (0 == iCharArrLen)
        {
            Win32LastErrorFPrintFWAbort(stderr,                         // _In_ FILE          *lpStream,
                                        L"WideCharToMultiByte(codePage[%ud], WC_ERR_INVALID_CHARS, lpWStr->lpWCharArr[%ld], -1, NULL, 0, NULL, NULL))",  // _In_ const wchar_t *lpMessageFormat,
                                        codePage, lpWStr->lpWCharArr);  // _In_ ...
        }

        // Important: WriteFile() only writes chars not wchars.
        const size_t ulCharArrLen = sizeof(char) * iCharArrLen;  // includes trailing null char

        char *lpCharArr = xcalloc(iCharArrLen, sizeof(char));

        // Note: If 'cchWideChar' == -1, then resulting lpCharArr includes trailing null char.
        const int iCharArrLen2 = WideCharToMultiByte(codePage,              // [in] UINT CodePage
                                                     WC_ERR_INVALID_CHARS,  // [in] DWORD dwFlags
                                                     lpWStr->lpWCharArr,    // [in] LPCWCH lpWideCharStr
                                                     -1,                    // [in] int cchWideChar
                                                     lpCharArr,             // [out/opt] LPSTR lpMultiByteStr
                                                     ulCharArrLen,          // [in] int cbMultiByte
                                                     NULL,                  // [in/opt] LPCCH lpDefaultChar
                                                     NULL);                 // [out/opt] LPBOOL lpUsedDefaultChar
        assert(iCharArrLen2 >= 0);
        if (0 == iCharArrLen2)
        {
            Win32LastErrorFPrintFWAbort(stderr,                         // _In_ FILE          *lpStream,
                                        L"WideCharToMultiByte(codePage[%ud], WC_ERR_INVALID_CHARS, lpWStr->lpWCharArr[%ld], -1, lpCharArr, ulCharArrLen, NULL, NULL))",  // _In_ const wchar_t *lpMessageFormat,
                                        codePage, lpWStr->lpWCharArr);  // _In_ ...
        }
        assert(iCharArrLen == iCharArrLen2);

        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
        DWORD numberOfBytesWritten = 0;
        if (!WriteFile(hWriteFile,                   // [in] HANDLE hFile
                       lpCharArr,                    // [in] LPCVOID lpBuffer
                       ulCharArrLen - LEN_NUL_CHAR,  // [in] DWORD nNumberOfBytesToWrite
                       &numberOfBytesWritten,        // [out/opt] LPDWORD lpNumberOfBytesWritten
                       NULL))                        // [in/out/opt] LPOVERLAPPED lpOverlapped
        {
            Win32LastErrorFPutWSAbort(stderr,         // _In_ FILE          *lpStream
                                      L"WriteFile");  // _In_ const wchar_t *lpMessage
        }

        xfree((void **) &lpCharArr);
    }

    if (!CloseHandle(hWriteFile))
    {
        Win32LastErrorFPutWSAbort(stderr,                       // _In_ FILE          *lpStream
                                  L"CloseHandle(hWriteFile)");  // _In_ const wchar_t *lpMessage
    }
}

void
WStrFileRead(_In_    const wchar_t *lpFilePathWCharArr,
             _In_    const UINT     codePage,  // Ex: CP_UTF8
             _Inout_ struct WStr   *lpDestWStr)
{
    assert(NULL != lpFilePathWCharArr);
    WStrFree(lpDestWStr);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
    const HANDLE hReadFile = CreateFile(lpFilePathWCharArr,     // [in] LPCWSTR lpFileName
                                        GENERIC_READ,           // [in] DWORD dwDesiredAccess
                                        FILE_SHARE_READ,        // [in] DWORD dwShareMode
                                        NULL,                   // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes
                                        OPEN_EXISTING,          // [in] DWORD dwCreationDisposition
                                        FILE_ATTRIBUTE_NORMAL,  // [in] DWORD dwFlagsAndAttributes
                                        NULL);                  // [in, optional] hTemplateFile
    if (INVALID_HANDLE_VALUE == hReadFile)
    {
        Win32LastErrorFPrintFWAbort(stderr,               // _In_ FILE          *lpStream,
                                    L"CreateFile(lpFileName[%ls], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)",  // _In_ const wchar_t *lpMessageFormat,
                                    lpFilePathWCharArr);  // _In_ ...
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesize
    const LPDWORD lpdwHighFileSize = NULL;
    const DWORD dwFileSize = GetFileSize(hReadFile, lpdwHighFileSize);
    if (INVALID_FILE_SIZE == dwFileSize)
    {
        Win32LastErrorFPrintFWAbort(stderr,                           // _In_ FILE          *lpStream
                                    L"GetFileSize(lpFileName[%ls])",  // _In_ const wchar_t *lpMessageFormat
                                    lpFilePathWCharArr);              // _In_ ...
    }

    if (0 == dwFileSize)
    {
        if (!CloseHandle(hReadFile))
        {
            Win32LastErrorFPrintFWAbort(stderr,                                      // _In_ FILE          *lpStream
                                        L"CloseHandle(hReadFile, lpFileName[%ls])",  // _In_ const wchar_t *lpMessageFormat
                                        lpFilePathWCharArr);                         // _In_ ...
        }
        // Above, WStrFree(lpDestWStr) is called.  Thus, lpDestWStr has zero length.
        return;
    }

    // Important: ReadFile() only reads chars not wchars.
    const size_t ulCharArrLen = sizeof(char) * (dwFileSize + LEN_NUL_CHAR);

    char *lpCharArr = xcalloc(dwFileSize + LEN_NUL_CHAR, sizeof(char));

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
    DWORD dwNumberOfBytesRead = 0;
    if (!ReadFile(hReadFile, lpCharArr, ulCharArrLen, &dwNumberOfBytesRead, NULL))
    {
        Win32LastErrorFPrintFWAbort(stderr,                       // _In_ FILE          *lpStream
                                    L"ReadFile(lpFileName:%ls)",  // _In_ const wchar_t *lpMessageFormat
                                    lpFilePathWCharArr);          // _In_ ...
    }
    assert(dwFileSize == dwNumberOfBytesRead);
    lpCharArr[dwNumberOfBytesRead] = '\0';

    if (!CloseHandle(hReadFile))
    {
        Win32LastErrorFPrintFWAbort(stderr,                                     // _In_ FILE          *lpStream
                                    L"CloseHandle(hReadFile, lpFileName:%ls)",  // _In_ const wchar_t *lpMessageFormat
                                    lpFilePathWCharArr);                        // _In_ ...
    }

    // Note: "BOM" == byte order mark
    // Ref: https://docs.microsoft.com/en-us/windows/win32/intl/using-byte-order-marks
    char *lpCharArrAfterBOM = lpCharArr;
    if (dwFileSize >= 3 && ((char) 0xEF) == lpCharArr[0] && ((char) 0xBB) == lpCharArr[1] && ((char) 0xBF) == lpCharArr[2])
    {
        lpCharArrAfterBOM += 3;  // Skip UTF-8 BOM
    }
    else if (dwFileSize >= 4 && ((char) 0xFF) == lpCharArr[0] && ((char) 0xFE) == lpCharArr[1] && ((char) 0x00) == lpCharArr[2] && ((char) 0x00) == lpCharArr[3])
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"UTF-32LE (little endian) BOM (byte order mark) is not supported!");  // _In_ const wchar_t *lpMessage
    }
    else if (dwFileSize >= 4 && ((char) 0x00) == lpCharArr[0] && ((char) 0x00) == lpCharArr[1] && ((char) 0xFF) == lpCharArr[2] && ((char) 0xFE) == lpCharArr[3])
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"UTF-32BE (big endian) BOM (byte order mark) is not supported!");  // _In_ const wchar_t *lpMessage
    }
    else if (dwFileSize >= 2 && ((char) 0xFF) == lpCharArr[0] && ((char) 0xFE) == lpCharArr[1])
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"UTF-16LE (little endian) BOM (byte order mark) is not supported!");  // _In_ const wchar_t *lpMessage
    }
    else if (dwFileSize >= 2 && ((char) 0xFE) == lpCharArr[0] && ((char) 0xFF) == lpCharArr[1])
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"UTF-16BE (big endian) BOM (byte order mark) is not supported!");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
    // Ref: https://stackoverflow.com/a/6693107/257299
    // Note: If 'cbMultiByte' == -1, then resulting count includes trailing null char.
    const int iWCharArrLen = MultiByteToWideChar(codePage,              // [in] UINT codePage
                                                 MB_ERR_INVALID_CHARS,  // [in] DWORD dwFlags
                                                 lpCharArrAfterBOM,     // [in] char *lpMultiByteStr
                                                 -1,                    // [in] int cbMultiByte
                                                 NULL,                  // [out/opt] wchar_t *lpWideCharStr
                                                 0);                    // [in] int cchWideChar
    assert(iWCharArrLen >= 0);
    if (0 == iWCharArrLen)
    {
        Win32LastErrorFPrintFWAbort(stderr,     // _In_ FILE          *lpStream,
                                    L"MultiByteToWideChar(codePage[%ud], MB_ERR_INVALID_CHARS, lpCharArrAfterBOM, -1, NULL, 0)",  // _In_ const wchar_t *lpMessageFormat,
                                    codePage);  // _In_ ...
    }

    wchar_t* lpWCharArr = xcalloc(iWCharArrLen, sizeof(wchar_t));

    // Note: If 'cbMultiByte' == -1, then resulting lpWCharArr includes trailing null char.
    const int iWCharArrLen2 = MultiByteToWideChar(codePage,              // [in] UINT codePage
                                                  MB_ERR_INVALID_CHARS,  // [in] DWORD dwFlags
                                                  lpCharArrAfterBOM,     // [in] char *lpMultiByteStr
                                                  -1,                    // [in] int cbMultiByte
                                                  lpWCharArr,            // [out/opt] wchar_t *lpWideCharStr
                                                  iWCharArrLen);         // [in] int cchWideChar
    assert(iWCharArrLen2 >= 0);
    if (0 == iWCharArrLen2)
    {
        Win32LastErrorFPrintFWAbort(stderr,     // _In_ FILE          *lpStream,
                                    L"MultiByteToWideChar(codePage[%ud], MB_ERR_INVALID_CHARS, lpCharArrAfterBOM, lpWCharArr, iWCharArrLen)",  // _In_ const wchar_t *lpMessageFormat,
                                    codePage);  // _In_ ...
    }
    assert(iWCharArrLen == iWCharArrLen2);

    xfree((void **) &lpCharArr);

    lpDestWStr->lpWCharArr = lpWCharArr;
    lpDestWStr->ulSize     = iWCharArrLen - LEN_NUL_CHAR;
}

void
WStrArrAssertValid(_In_ const struct WStrArr *lpWStrArr)
{
    assert(NULL != lpWStrArr);
    if (lpWStrArr->ulSize > 0)
    {
        assert(NULL != lpWStrArr->lpWStrArr);
    }
}

void
WStrArrFree(_Inout_ struct WStrArr *lpWStrArr)
{
    WStrArrAssertValid(lpWStrArr);

    if (NULL != lpWStrArr->lpWStrArr)
    {
        for (size_t i = 0; i < lpWStrArr->ulSize; ++i)
        {
            WStrFree(lpWStrArr->lpWStrArr + i);
        }
        xfree((void **) &(lpWStrArr->lpWStrArr));
    }
    lpWStrArr->ulSize = 0;  // Explicit
}

void
WStrArrAlloc(_Inout_ struct WStrArr *lpWStrArr,
             _In_    const size_t    ulSize)
{
    WStrArrFree(lpWStrArr);
    if (ulSize > 0)
    {
        lpWStrArr->lpWStrArr = xcalloc(ulSize, sizeof(struct WStr));
        lpWStrArr->ulSize = ulSize;
    }
}

void
WStrArrCopyWCharArrArr(_Inout_ struct WStrArr  *lpDestWStrArr,
                       _In_    const wchar_t  **lppNullableSrcWCharArrArr,
                       _In_    const size_t     ulSrcCount)
{
    assert(NULL != lpDestWStrArr);

    WStrArrFree(lpDestWStrArr);
    WStrArrAlloc(lpDestWStrArr, ulSrcCount);

    if (0 == ulSrcCount)
    {
        assert(NULL == lppNullableSrcWCharArrArr);
        return;
    }
    else
    {
        assert(NULL != lppNullableSrcWCharArrArr);
    }

    for (size_t i = 0; i < ulSrcCount; ++i)
    {
        struct WStr *lpWStr = lpDestWStrArr->lpWStrArr + i;
        WStrCopyWCharArr(lpWStr, lppNullableSrcWCharArrArr[i], wcslen(lppNullableSrcWCharArrArr[i]));
    }
}

void
WStrArrForEach(_Inout_ struct WStrArr         *lpWStrArr,
               _In_    const WStrConsumerFunc  fpWStrConsumerFunc)
{
    WStrArrAssertValid(lpWStrArr);
    assert(NULL != fpWStrConsumerFunc);

    for (size_t i = 0; i < lpWStrArr->ulSize; ++i)
    {
        struct WStr *lpWStr = lpWStrArr->lpWStrArr + i;
        fpWStrConsumerFunc(lpWStr);
    }
}

