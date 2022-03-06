#include "wstr.h"
#include "error_exit.h"  // required for ErrorExit()
#include "xmalloc.h"
#include <assert.h>
#include <windows.h>

// Null character is either: (char) '\0' or (wchar_t) L'\0'
const size_t LEN_NULL_CHAR = 1;

void WStrAssertValid(_In_ const struct WStr *lpWStr)
{
    assert(NULL != lpWStr);
    if (lpWStr->ulSize > 0)
    {
        assert(NULL != lpWStr->lpWCharArr);
    }
}

void WStrFree(_Inout_ struct WStr *lpWStr)
{
    WStrAssertValid(lpWStr);

    xfree((void **) &(lpWStr->lpWCharArr));
    lpWStr->ulSize = 0;  // Explicit
}

static void InternalWStrCopyWCharArr(_Inout_ struct WStr   *lpDestWStr,
                                     _In_    const wchar_t *lpSrcWCharArr,
                                     _In_    const size_t   ulSrcSize)
{
    WStrFree(lpDestWStr);
    // Intentional: Allow NULL here
//    assert(NULL != lpSrcWCharArr);

    lpDestWStr->ulSize = ulSrcSize;
    if (0 == ulSrcSize) {
        return;
    }

    lpDestWStr->lpWCharArr = xcalloc(lpDestWStr->ulSize + LEN_NULL_CHAR, sizeof(wchar_t));

    wcsncpy_s(lpDestWStr->lpWCharArr,              // dest wstr ptr
              lpDestWStr->ulSize + LEN_NULL_CHAR,  // dest wstr length (including null char)
              lpSrcWCharArr,                       // source wstr ptr
              ulSrcSize);                          // wchar count to be copied (excluding null char)
}

void WStrCopyWCharArr(_Inout_ struct WStr   *lpDestWStr,
                      _In_    const wchar_t *lpSrcWCharArr,
                      _In_    const size_t   ulSrcSize)
{
    assert(NULL != lpSrcWCharArr);

    InternalWStrCopyWCharArr(lpDestWStr, lpSrcWCharArr, ulSrcSize);
}

void WStrCopyWStr(_Inout_ struct WStr       *lpDestWStr,
                  _In_    const struct WStr *lpSrcWStr)
{
    WStrAssertValid(lpSrcWStr);

    InternalWStrCopyWCharArr(lpDestWStr, lpSrcWStr->lpWCharArr, lpSrcWStr->ulSize);
}

void WStrTrim(_Inout_ struct WStr                 *lpWStr,
              _In_    const enum EWStrTrim         eWStrTrim,
              _In_    const WStrCharPredicateFunc  fpWStrCharPredicateFunc)
{
    WStrAssertValid(lpWStr);
    assert(eWStrTrim >= WSTR_LTRIM && eWStrTrim <= (WSTR_LTRIM | WSTR_RTRIM));

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
        for (size_t i = lpWStr->ulSize - 1; i >= 0; --i)
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

    wchar_t *lpWCharArr = xcalloc(ulTrimmedSize + LEN_NULL_CHAR, sizeof(wchar_t));

    wcsncpy_s(lpWCharArr,                           // dest wstr ptr
              ulTrimmedSize + LEN_NULL_CHAR,        // dest wstr length (including null char)
              lpWStr->lpWCharArr + ulLeadingCount,  // source wstr ptr
              ulTrimmedSize);                       // wchar count to be copied (excluding null char)

    WStrFree(lpWStr);

    lpWStr->lpWCharArr = lpWCharArr;
    lpWStr->ulSize     = ulTrimmedSize;
}

void WStrLTrimSpace(_Inout_ struct WStr *lpWStr)
{
    WStrTrim(lpWStr, WSTR_LTRIM, iswspace);
}

void WStrRTrimSpace(_Inout_ struct WStr *lpWStr)
{
    WStrTrim(lpWStr, WSTR_RTRIM, iswspace);
}

void WStrTrimSpace(_Inout_ struct WStr *lpWStr)
{
    WStrTrim(lpWStr, WSTR_LTRIM | WSTR_RTRIM, iswspace);
}

static void InternalWStrSplit(_In_    const struct WStr *lpWStrText,
                              _In_    const struct WStr *lpWStrDelim,
                              _In_    const int          iMaxTokenCount,  // -1 for unlimited
                              _In_    const BOOL         bDiscardFinalEmptyToken,
                              _Inout_ struct WStrArr    *lpTokenWStrArr)
{
    WStrAssertValid(lpWStrText);
    WStrAssertValid(lpWStrDelim);
    // Do not allow empty delim
    assert(0 != lpWStrDelim->ulSize);
    assert(-1 == iMaxTokenCount || iMaxTokenCount >= 2);
    WStrArrFree(lpTokenWStrArr);

    // Important: Text after last delim is final token.
    const int iMaxDelimCount = (-1 == iMaxTokenCount) ? -1 : (iMaxTokenCount - 1);
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

        if (-1 != iMaxDelimCount && iMaxDelimCount == ulDelimCount) {
            break;
        }
    }

    const size_t ulTokenCount = 1U + ulDelimCount;
    WStrArrAlloc(lpTokenWStrArr, ulTokenCount);

    // Step 2: Extract tokens between delimiters
    lpIter = lpWStrText->lpWCharArr;
    for (size_t ulTokenIndex = 0; ulTokenIndex < ulTokenCount; ++ulTokenIndex)
    {
        // Intention: Include (-1 == iMaxTokenCount) for readability.
        const BOOL bIsLastToken = (-1 == iMaxTokenCount) ? FALSE : (1U + ulTokenIndex == iMaxTokenCount);

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
}

void WStrSplit(_In_    const struct WStr *lpWStrText,
               _In_    const struct WStr *lpWStrDelim,
               _In_    const int          iMaxTokenCount,  // -1 for unlimited
               _Inout_ struct WStrArr    *lpTokenWStrArr)
{
    const BOOL bDiscardFinalEmptyToken = FALSE;
    InternalWStrSplit(lpWStrText, lpWStrDelim, iMaxTokenCount, bDiscardFinalEmptyToken, lpTokenWStrArr);
}

void WStrSplitNewLine(_In_    const struct WStr *lpWStrText,
                      _In_    const int          iMaxLineCount,
                      _Inout_ struct WStrArr    *lpTokenWStrArr)
{
    WStrAssertValid(lpWStrText);
    WStrArrFree(lpTokenWStrArr);

    const BOOL bDiscardFinalEmptyToken = TRUE;

    if (NULL != wcsstr(lpWStrText->lpWCharArr, L"\r\n"))
    {
        struct WStr wstrDelim = {.lpWCharArr = L"\r\n", .ulSize = 2};
        InternalWStrSplit(lpWStrText, &wstrDelim, iMaxLineCount, bDiscardFinalEmptyToken, lpTokenWStrArr);
    }
    else if (NULL != wcsstr(lpWStrText->lpWCharArr, L"\n"))
    {
        struct WStr wstrDelim = {.lpWCharArr = L"\n", .ulSize = 1};
        InternalWStrSplit(lpWStrText, &wstrDelim, iMaxLineCount, bDiscardFinalEmptyToken, lpTokenWStrArr);
    }
    else
    {
        WStrArrAlloc(lpTokenWStrArr, 1);
        WStrCopyWStr(lpTokenWStrArr->lpWStrArr, lpWStrText);
    }
}

// Maybe: WStrFileAppend
// Ref: https://docs.microsoft.com/en-us/windows/win32/fileio/appending-one-file-to-another-file

void WStrFileWrite(_In_ const wchar_t     *lpFilePath,
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
        ErrorExit(L"CreateFile(lpFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)");
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
            ErrorExit(L"WideCharToMultiByte(codePage, WC_ERR_INVALID_CHARS, lpWStr->lpWCharArr, -1, NULL, 0, NULL, NULL))");
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
            ErrorExit(L"WideCharToMultiByte(codePage, WC_ERR_INVALID_CHARS, lpWStr->lpWCharArr, -1, lpCharArr, ulCharArrLen, NULL, NULL))");
        }
        assert(iCharArrLen == iCharArrLen2);

        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
        DWORD numberOfBytesWritten = 0;
        if (!WriteFile(hWriteFile,                    // [in] HANDLE hFile
                       lpCharArr,                     // [in] LPCVOID lpBuffer
                       ulCharArrLen - LEN_NULL_CHAR,  // [in] DWORD nNumberOfBytesToWrite
                       &numberOfBytesWritten,         // [out/opt] LPDWORD lpNumberOfBytesWritten
                       NULL))                         // [in/out/opt] LPOVERLAPPED lpOverlapped
        {
            ErrorExit(L"WriteFile");
        }

        xfree((void **) &lpCharArr);
    }

    if (!CloseHandle(hWriteFile))
    {
        ErrorExit(L"CloseHandle(hWriteFile)");
    }
}

void WStrFileRead(_In_    const wchar_t *lpFilePath,
                  _In_    const UINT     codePage,  // Ex: CP_UTF8
                  _Inout_ struct WStr   *lpDestWStr)
{
    assert(NULL != lpFilePath);
    WStrFree(lpDestWStr);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
    const HANDLE hReadFile = CreateFile(lpFilePath,             // [in] LPCWSTR lpFileName
                                        GENERIC_READ,           // [in] DWORD dwDesiredAccess
                                        FILE_SHARE_READ,        // [in] DWORD dwShareMode
                                        NULL,                   // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes
                                        OPEN_EXISTING,          // [in] DWORD dwCreationDisposition
                                        FILE_ATTRIBUTE_NORMAL,  // [in] DWORD dwFlagsAndAttributes
                                        NULL);                  // [in, optional] hTemplateFile
    if (INVALID_HANDLE_VALUE == hReadFile)
    {
        ErrorExit(L"CreateFile(lpFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)");
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesize
    const LPDWORD lpdwHighFileSize = NULL;
    const DWORD dwFileSize = GetFileSize(hReadFile, lpdwHighFileSize);
    if (INVALID_FILE_SIZE == dwFileSize)
    {
        ErrorExit(L"GetFileSize");
    }

    if (0 == dwFileSize)
    {
        if (!CloseHandle(hReadFile))
        {
            ErrorExit(L"CloseHandle(hReadFile)");
        }
        return;
    }

    // Important: ReadFile() only reads chars not wchars.
    const size_t ulCharArrLen = sizeof(char) * (dwFileSize + LEN_NULL_CHAR);

    char *lpCharArr = xcalloc(dwFileSize + LEN_NULL_CHAR, sizeof(char));

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
    DWORD dwNumberOfBytesRead = 0;
    if (!ReadFile(hReadFile, lpCharArr, ulCharArrLen, &dwNumberOfBytesRead, NULL))
    {
        ErrorExit(L"ReadFile");
    }
    assert(dwFileSize == dwNumberOfBytesRead);
    lpCharArr[dwNumberOfBytesRead] = '\0';

    if (!CloseHandle(hReadFile))
    {
        ErrorExit(L"CloseHandle(hReadFile)");
    }

    // Note: "BOM" == byte order mark
    // Ref: https://docs.microsoft.com/en-us/windows/win32/intl/using-byte-order-marks
    char *lpCharArrAfterBOM = lpCharArr;
    if (dwFileSize >= 3 && 0xEF == lpCharArr[0] && 0xBB == lpCharArr[1] && 0xBF == lpCharArr[2])
    {
        lpCharArrAfterBOM += 3;  // Skip UTF-8 BOM
    }
    else if (dwFileSize >= 4 && 0xFF == lpCharArr[0] && 0xFE == lpCharArr[1] && 0x00 == lpCharArr[2] && 0x00 == lpCharArr[3])
    {
        ErrorExit(L"UTF-32LE (little endian) BOM (byte order mark) is not supported!");
    }
    else if (dwFileSize >= 4 && 0x00 == lpCharArr[0] && 0x00 == lpCharArr[1] && 0xFF == lpCharArr[2] && 0xFE == lpCharArr[3])
    {
        ErrorExit(L"UTF-32BE (big endian) BOM (byte order mark) is not supported!");
    }
    else if (dwFileSize >= 2 && 0xFF == lpCharArr[0] && 0xFE == lpCharArr[1])
    {
        ErrorExit(L"UTF-16LE (little endian) BOM (byte order mark) is not supported!");
    }
    else if (dwFileSize >= 2 && 0xFE == lpCharArr[0] && 0xFF == lpCharArr[1])
    {
        ErrorExit(L"UTF-16BE (big endian) BOM (byte order mark) is not supported!");
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
        ErrorExit(L"MultiByteToWideChar(codePage, MB_ERR_INVALID_CHARS, lpCharArrAfterBOM, -1, NULL, 0)");
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
        ErrorExit(L"MultiByteToWideChar(codePage, MB_ERR_INVALID_CHARS, lpCharArrAfterBOM, lpWCharArr, iWCharArrLen)");
    }
    assert(iWCharArrLen == iWCharArrLen2);

    xfree((void **) &lpCharArr);

    lpDestWStr->lpWCharArr = lpWCharArr;
    lpDestWStr->ulSize     = iWCharArrLen - LEN_NULL_CHAR;
}

void WStrArrAssertValid(_In_ const struct WStrArr *lpWStrArr)
{
    assert(NULL != lpWStrArr);
    if (lpWStrArr->ulSize > 0)
    {
        assert(NULL != lpWStrArr->lpWStrArr);
    }
}

void WStrArrFree(_Inout_ struct WStrArr *lpWStrArr)
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

void WStrArrAlloc(_Inout_ struct WStrArr *lpWStrArr,
                  _In_    const size_t    ulSize)
{
    WStrArrFree(lpWStrArr);
    assert(ulSize > 0);  // alloc an empty array seems silly!

    lpWStrArr->lpWStrArr = xcalloc(ulSize, sizeof(struct WStr));
    lpWStrArr->ulSize = ulSize;
}

void WStrArrForEach(_Inout_ struct WStrArr         *lpWStrArr,
                    _In_    const WStrConsumerFunc  fpWStrConsumerFunc)
{
    WStrArrAssertValid(lpWStrArr);

    for (size_t i = 0; i < lpWStrArr->ulSize; ++i)
    {
        struct WStr *lpWStr = lpWStrArr->lpWStrArr + i;
        fpWStrConsumerFunc(lpWStr);
    }
}

