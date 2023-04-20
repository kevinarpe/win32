#include "wstr.h"
#include "win32_last_error.h"
#include "test_assert.inc"
#include <windows.h>  // required for wWinMain()
#include <stdio.h>    // required for printf()
#include <assert.h>   // required for assert()

static void TestWStrFree(_Inout_ struct WStr *lpWStr)
{
    WStrFree(lpWStr);
    assert(NULL == lpWStr->lpWCharArr);
    assert(0 == lpWStr->ulSize);
}

static void TestWStrArrFree(_Inout_ struct WStrArr *lpWStrArr)
{
    WStrArrFree(lpWStrArr);
    assert(NULL == lpWStrArr->lpWStrArr);
    assert(0 == lpWStrArr->ulSize);
}

static void TestWStrCopy(_In_ const wchar_t *lpWCharArr)
{
    printf("TestWStrCopy: (%zd)[%ls]\r\n", wcslen(lpWCharArr), lpWCharArr);

    const size_t ulSize = wcslen(lpWCharArr);
    struct WStr wstr = {};
    WStrCopyWCharArr(&wstr, lpWCharArr, ulSize);

    if (0 == ulSize)
    {
        assert(NULL == wstr.lpWCharArr);
    }
    else
    {
        assert(0 == wcscmp(lpWCharArr, wstr.lpWCharArr));
    }
    assert(ulSize == wstr.ulSize);

    struct WStr wstr2 = {};
    WStrCopyWStr(&wstr2, &wstr);

    if (0 == ulSize)
    {
        assert(NULL == wstr2.lpWCharArr);
    }
    else
    {
        assert(0 == wcscmp(wstr2.lpWCharArr, wstr.lpWCharArr));
    }
    assert(wstr2.ulSize == wstr.ulSize);

    TestWStrFree(&wstr);
    TestWStrFree(&wstr2);
}

typedef void (*WStrTrimSpaceFunc)(_Inout_ struct WStr *lpWStr);

static void TestWStrTrimSpace(_In_ const wchar_t           *lpWStrTrimSpaceFuncName,
                              _In_ const WStrTrimSpaceFunc  fpWStrTrimSpaceFunc,
                              _In_ const wchar_t           *lpWCharArr,
                              _In_ const wchar_t           *lpExpectedWCharArr)
{
    printf("TestWStrTrimSpace[%ls]: (%zd)[%ls] -> (%zd)[%ls]\r\n", lpWStrTrimSpaceFuncName, wcslen(lpWCharArr), lpWCharArr, wcslen(lpExpectedWCharArr), lpExpectedWCharArr);

    const size_t ulSize = wcslen(lpWCharArr);
    struct WStr wstr = {};
    WStrCopyWCharArr(&wstr, lpWCharArr, ulSize);
    fpWStrTrimSpaceFunc(&wstr);

    if (0 == wstr.ulSize)
    {
        assert(NULL == wstr.lpWCharArr);
    }
    else
    {
        assert(0 == wcscmp(lpExpectedWCharArr, wstr.lpWCharArr));
    }
    assert(wstr.ulSize == wcslen(lpExpectedWCharArr));

    TestWStrFree(&wstr);
}

static void TestWStrSplit(_In_ wchar_t                 *lpWCharArr,
                          _In_ wchar_t                 *lpDelim,
                          _In_ const int                iMinTokenCount,
                          _In_ const int                iMaxTokenCount,
                          _In_ const WStrConsumerFunc   fpNullableWStrConsumerFunc,
                          _In_ const wchar_t          **lppExpectedTokenArr,
                          _In_ const size_t             ulExpectedTokenCount)
{
    printf("TestWStrSplit: [%ls][%ls][min:%d,max:%d] -> (%zd)", lpWCharArr, lpDelim, iMinTokenCount, iMaxTokenCount, ulExpectedTokenCount);
    for (size_t i = 0; i < ulExpectedTokenCount; ++i)
    {
        printf("[%ls]", lppExpectedTokenArr[i]);
    }
    printf("\r\n");

    struct WStrSplitOptions wStrSplitOptions = {.iMinTokenCount             = iMinTokenCount,
                                                .iMaxTokenCount             = iMaxTokenCount,
                                                .fpNullableWStrConsumerFunc = fpNullableWStrConsumerFunc};

    struct WStr wstrText = {.lpWCharArr = lpWCharArr, .ulSize = wcslen(lpWCharArr)};
    struct WStr wstrDelim = {.lpWCharArr = lpDelim, .ulSize = wcslen(lpDelim)};
    struct WStrArr wstrArr = {};
    WStrSplit(&wstrText, &wstrDelim, &wStrSplitOptions, &wstrArr);

    AssertWStrArrEqual(&wstrArr, lppExpectedTokenArr, ulExpectedTokenCount);
    TestWStrArrFree(&wstrArr);
}

static void TestWStrSplitNewLine(_In_ wchar_t                 *lpWCharArr,
                                 _In_ const int                iMinTokenCount,
                                 _In_ const int                iMaxTokenCount,
                                 _In_ const WStrConsumerFunc   fpNullableWStrConsumerFunc,
                                 _In_ const wchar_t          **lppExpectedLineArr,
                                 _In_ const size_t             ulExpectedLineCount)
{
    assert(ulExpectedLineCount > 0);

    printf("TestWStrSplitNewLine: [%ls][min:%d,max:%d] -> (%zd)", lpWCharArr, iMinTokenCount, iMaxTokenCount, ulExpectedLineCount);
    for (size_t i = 0; i < ulExpectedLineCount; ++i)
    {
        printf("[%ls]", lppExpectedLineArr[i]);
    }
    printf("\r\n");

    struct WStrSplitOptions wStrSplitOptions = {.iMinTokenCount             = iMinTokenCount,
                                                .iMaxTokenCount             = iMaxTokenCount,
                                                .fpNullableWStrConsumerFunc = fpNullableWStrConsumerFunc};

    struct WStr wstrText = {.lpWCharArr = lpWCharArr, .ulSize = wcslen(lpWCharArr)};
    struct WStrArr wstrArr = {};
    WStrSplitNewLine(&wstrText, &wStrSplitOptions, &wstrArr);

    printf("Actual: (%zd)", wstrArr.ulSize);
    for (size_t i = 0; i < wstrArr.ulSize; ++i)
    {
        printf("[%ls]", wstrArr.lpWStrArr[i].lpWCharArr);
    }
    printf("\r\n");

    AssertWStrArrEqual(&wstrArr, lppExpectedLineArr, ulExpectedLineCount);
    TestWStrArrFree(&wstrArr);
}

static void TestWStrJoin(_In_ const wchar_t **lppTokenArr,
                         _In_ const size_t    ulTokenCount,
                         _In_ wchar_t        *lpDelim,
                         _In_ const wchar_t  *lpExpectedOutput)
{
    printf("TestWStrJoin(%zd, [%ls]) -> [%ls]\n", ulTokenCount, lpDelim, lpExpectedOutput);

    struct WStrArr wstrArr = {};
    WStrArrCopyWCharArrArr(&wstrArr, lppTokenArr, ulTokenCount);

    struct WStr wstrDelim = {.lpWCharArr = lpDelim, .ulSize = wcslen(lpDelim)};
    struct WStr wstrText = {};
    WStrJoin(&wstrArr, &wstrDelim, &wstrText);

    if (0 == wstrText.ulSize)
    {
        assert(0 == wcscmp(L"", lpExpectedOutput));
    }
    else {
        assert(0 == wcscmp(wstrText.lpWCharArr, lpExpectedOutput));
    }

    WStrArrFree(&wstrArr);
    WStrFree(&wstrText);
}

static void TestWStrForEachTrimSpace(_In_ wchar_t        *lpWCharArr,
                                     _In_ wchar_t        *lpDelim,
                                     _In_ const int       iMinTokenCount,
                                     _In_ const int       iMaxTokenCount,
                                     _In_ const wchar_t **lppExpectedTokenArr,
                                     _In_ const size_t    ulExpectedTokenCount)
{
    printf("TestWStrForEachTrimSpace: [%ls][%ls][min:%d,max:%d] -> (%zd)", lpWCharArr, lpDelim, iMinTokenCount, iMaxTokenCount, ulExpectedTokenCount);
    for (size_t i = 0; i < ulExpectedTokenCount; ++i)
    {
        printf("[%ls]", lppExpectedTokenArr[i]);
    }
    printf("\r\n");

    struct WStrSplitOptions wStrSplitOptions = {.iMinTokenCount             = iMinTokenCount,
                                                .iMaxTokenCount             = iMaxTokenCount,
                                                .fpNullableWStrConsumerFunc = WStrTrimSpace};

    struct WStr wstrText = {.lpWCharArr = lpWCharArr, .ulSize = wcslen(lpWCharArr)};
    struct WStr wstrDelim = {.lpWCharArr = lpDelim, .ulSize = wcslen(lpDelim)};
    struct WStrArr wstrArr = {};
    WStrSplit(&wstrText, &wstrDelim, &wStrSplitOptions, &wstrArr);
    WStrArrForEach(&wstrArr, WStrTrimSpace);

    AssertWStrArrEqual(&wstrArr, lppExpectedTokenArr, ulExpectedTokenCount);
    TestWStrArrFree(&wstrArr);
}

static size_t StaticGetFileSize(_In_ const wchar_t *lpFilePath)
{
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
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"CreateFile(lpFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesize
    const LPDWORD lpdwHighFileSize = NULL;
    const DWORD dwFileSize = GetFileSize(hReadFile, lpdwHighFileSize);
    if (INVALID_FILE_SIZE == dwFileSize)
    {
        Win32LastErrorFPutWSAbort(stderr,           // _In_ FILE          *lpStream
                                  L"GetFileSize");  // _In_ const wchar_t *lpMessage
    }

    if (!CloseHandle(hReadFile))
    {
        Win32LastErrorFPutWSAbort(stderr,                      // _In_ FILE          *lpStream
                                  L"CloseHandle(hReadFile)");  // _In_ const wchar_t *lpMessage
    }

    return dwFileSize;
}

static void TestWStrFileWriteAndRead(_In_ wchar_t      *lpWCharArr,
                                     _In_ const size_t  ulExpectFileSize)
{
    printf("TestWStrFileWrite: [%zd][%ls]\r\n", wcslen(lpWCharArr), lpWCharArr);

    const UINT codePage = CP_UTF8;

    const wchar_t *lpFilePath = L"TestWStr.txt";

    // Intentional: Ignore return value (BOOL)
    DeleteFile(lpFilePath);

    struct WStr outputWStrText = {.lpWCharArr = lpWCharArr, .ulSize = wcslen(lpWCharArr)};
    WStrFileWrite(lpFilePath, codePage, &outputWStrText);

    const size_t ulFileSize = StaticGetFileSize(lpFilePath);

    if (ulFileSize != ulExpectFileSize)
    {
        printf("ulFileSize != ulExpectFileSize: %zd != %zd\r\n", ulFileSize, ulExpectFileSize);
    }
    assert(ulFileSize == ulExpectFileSize);

    struct WStr inputWStrText = {};
    WStrFileRead(lpFilePath, codePage, &inputWStrText);

    if (outputWStrText.ulSize != inputWStrText.ulSize)
    {
        printf("outputWStrText.ulSize != inputWStrText.ulSize: %zd != %zd\r\n", outputWStrText.ulSize, inputWStrText.ulSize);
    }
    assert(outputWStrText.ulSize == inputWStrText.ulSize);

    if (0 == inputWStrText.ulSize)
    {
        assert(NULL == inputWStrText.lpWCharArr);
    }
    else
    {
        assert(0 == wcscmp(outputWStrText.lpWCharArr, inputWStrText.lpWCharArr));
    }

    WStrFree(&inputWStrText);

    // Intentional: Ignore return value (BOOL)
    DeleteFile(lpFilePath);
}

static void TestWStrArrCopyWCharArrArr(_In_ const wchar_t **lppTokenArr,
                                       _In_ const size_t    ulTokenCount)
{
    printf("TestWStrArrCopyWCharArrArr(%zd)\n", ulTokenCount);

    struct WStrArr wstrArr = {};
    WStrArrCopyWCharArrArr(&wstrArr, lppTokenArr, ulTokenCount);

    AssertWStrArrEqual(&wstrArr, lppTokenArr, ulTokenCount);

    WStrArrFree(&wstrArr);
}

static void TestWStrIsEmpty(const wchar_t *lpNullableWCharArr,
                            const BOOL bExpectedResult)
{
    printf("TestWStrIsEmpty: [%ls] -> %s\r\n",
           (NULL == lpNullableWCharArr ? L"" : lpNullableWCharArr),
           (bExpectedResult ? "TRUE" : "FALSE"));

//    const struct WStr wstr = WSTR_FROM_LITERAL(lpNullableWCharArr);
    const struct WStr wstr = WSTR_FROM_VALUE(lpNullableWCharArr);

    const BOOL bResult = WStrIsEmpty(&wstr);
    assert(bExpectedResult == bResult);
}

static void TestWStrConcat(const wchar_t *lpNullableDestWCharArr,
                           const wchar_t *lpNullableWCharArr,
                           const wchar_t *lpNullableWCharArr2,
                           const wchar_t *lpExpectedDestWCharArr)
{
    printf("TestWStrConcat: [%ls] -> [%ls] + [%ls] -> [%ls]\r\n",
           (NULL == lpNullableDestWCharArr ? L"" : lpNullableDestWCharArr),
           (NULL == lpNullableWCharArr  ? L"" : lpNullableWCharArr),
           (NULL == lpNullableWCharArr2 ? L"" : lpNullableWCharArr2),
           lpExpectedDestWCharArr);

    struct WStr destWStr = {0};
    if (NULL != lpNullableDestWCharArr)
    {
        WStrCopyWCharArr(&destWStr, lpNullableDestWCharArr, wcslen(lpNullableDestWCharArr));
    }
    struct WStr wstr = WSTR_FROM_VALUE(lpNullableWCharArr);
    struct WStr wstr2 = WSTR_FROM_VALUE(lpNullableWCharArr2);

    WStrConcat(&destWStr, &wstr, &wstr2);

    const size_t ulExpectedSize = wcslen(lpExpectedDestWCharArr);
    if (0 == ulExpectedSize)
    {
        assert(WStrIsEmpty(&destWStr));
    }
    else
    {
        assert(0 == wcscmp(lpExpectedDestWCharArr, destWStr.lpWCharArr));
    }
}

static void TestWStrConcatMany(const wchar_t     *lpTestDesc,
                               const wchar_t     *lpNullableExpectedWCharArr,
                               const struct WStr *lpActualWStr)
{
    printf("%ls\n", lpTestDesc);

    const struct WStr expectedWStr = WSTR_FROM_VALUE(lpNullableExpectedWCharArr);

    if (0 == expectedWStr.ulSize)
    {
        assert(0 == lpActualWStr->ulSize);
    }
    else
    {
        assert(0 == wcscmp(lpNullableExpectedWCharArr, lpActualWStr->lpWCharArr));
    }
}

static void TestWStrSPrintF(const wchar_t *lpExpectedDestWCharArr,
                            struct WStr   *lpDestWStr,
                            const wchar_t *lpFormatWCharArr,
                            const wchar_t *lpArgsDescWCharArr)
{
    printf("TestWStrSPrintF: [%ls] == WStrSPrintF([%ls], %ls)\r\n", lpExpectedDestWCharArr, lpFormatWCharArr, lpArgsDescWCharArr);

    assert(0 == wcscmp(lpExpectedDestWCharArr, lpDestWStr->lpWCharArr));
    
    WStrFree(lpDestWStr);
}

static void TestWStrSPrintFV(const wchar_t *lpExpectedDestWCharArr,
                             const wchar_t *lpFormatWCharArr,
                             const wchar_t *lpArgsDescWCharArr,
                             ...)
{
    printf("TestWStrSPrintFV: [%ls] == WStrSPrintFV([%ls], %ls)\r\n", lpExpectedDestWCharArr, lpFormatWCharArr, lpArgsDescWCharArr);

    va_list ap;
    va_start(ap, lpArgsDescWCharArr);

    struct WStr destWStr = {0};
    WStrSPrintFV(&destWStr, lpFormatWCharArr, ap);

    va_end(ap);

    assert(0 == wcscmp(lpExpectedDestWCharArr, destWStr.lpWCharArr));

    WStrFree(&destWStr);
}

static void TestWStrCompareEqual(const wchar_t *lpNullableLeftWCharArr,
                                 const wchar_t *lpNullableRightWCharArr)
{
    const int expectedCmp = 0;
    printf("TestWStrCompareEqual: WStrCompare(\"%ls\", \"%ls\") == %d\r\n", lpNullableLeftWCharArr, lpNullableRightWCharArr, expectedCmp);
    const struct WStr wstrLeft  = WSTR_FROM_VALUE(lpNullableLeftWCharArr);
    const struct WStr wstrRight = WSTR_FROM_VALUE(lpNullableRightWCharArr);
    const int cmp = WStrCompare(&wstrLeft, &wstrRight);
    AssertEqual("%d", "cmp", cmp, "expectedCmp", expectedCmp);
}

static void TestWStrCompareLess(const wchar_t *lpNullableLeftWCharArr,
                                const wchar_t *lpNullableRightWCharArr)
{
    const int expectedCmp = 0;
    printf("TestWStrCompareLess: WStrCompare(\"%ls\", \"%ls\") < %d\r\n", lpNullableLeftWCharArr, lpNullableRightWCharArr, expectedCmp);
    const struct WStr wstrLeft  = WSTR_FROM_VALUE(lpNullableLeftWCharArr);
    const struct WStr wstrRight = WSTR_FROM_VALUE(lpNullableRightWCharArr);
    const int cmp = WStrCompare(&wstrLeft, &wstrRight);
    AssertLess("%d", "cmp", cmp, "expectedCmp", expectedCmp);
}

static void TestWStrCompareGreater(const wchar_t *lpNullableLeftWCharArr,
                                const wchar_t *lpNullableRightWCharArr)
{
    const int expectedCmp = 0;
    printf("TestWStrCompareGreater: WStrCompare(\"%ls\", \"%ls\") < %d\r\n", lpNullableLeftWCharArr, lpNullableRightWCharArr, expectedCmp);
    const struct WStr wstrLeft  = WSTR_FROM_VALUE(lpNullableLeftWCharArr);
    const struct WStr wstrRight = WSTR_FROM_VALUE(lpNullableRightWCharArr);
    const int cmp = WStrCompare(&wstrLeft, &wstrRight);
    AssertGreater("%d", "cmp", cmp, "expectedCmp", expectedCmp);
}

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI wWinMain(__attribute__((unused)) HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
                    __attribute__((unused)) HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
                    __attribute__((unused)) PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
                    __attribute__((unused)) int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-error-mode?view=msvc-170
    _set_error_mode(_OUT_TO_STDERR);  // assert to STDERR

    TestWStrCopy(L"abc");
    TestWStrCopy(L"");

    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L"  abc def  123  \r\n", L"abc def  123");
    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L"  abc def  123  ", L"abc def  123");
    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L"abc def  123  ", L"abc def  123");
    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L"  abc def  123", L"abc def  123");
    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L"abc def  123", L"abc def  123");
    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L"  ", L"");
    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L"", L"");
    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L" \v\t\r\n ", L"");
    // These do not print to STDOUT correctly, but asserts are OK.
    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L" \u6771 ", L"\u6771"); // 東 using 'universal char name'
    TestWStrTrimSpace(L"WStrTrimSpace", WStrTrimSpace, L" 東京 ", L"東京");

    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L"  abc def  123  \r\n", L"abc def  123  \r\n");
    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L"  abc def  123  ", L"abc def  123  ");
    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L"abc def  123  ", L"abc def  123  ");
    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L"  abc def  123", L"abc def  123");
    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L"abc def  123", L"abc def  123");
    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L"  ", L"");
    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L"", L"");
    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L" \v\t\r\n ", L"");
    // These do not print to STDOUT correctly, but asserts are OK.
    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L" \u6771 ", L"\u6771 "); // 東 using 'universal char name'
    TestWStrTrimSpace(L"WStrLTrimSpace", WStrLTrimSpace, L" 東京 ", L"東京 ");

    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L"  abc def  123  \r\n", L"  abc def  123");
    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L"  abc def  123  ", L"  abc def  123");
    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L"abc def  123  ", L"abc def  123");
    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L"  abc def  123", L"  abc def  123");
    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L"abc def  123", L"abc def  123");
    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L"  ", L"");
    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L"", L"");
    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L" \v\t\r\n ", L"");
    // These do not print to STDOUT correctly, but asserts are OK.
    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L" \u6771 ", L" \u6771"); // 東 using 'universal char name'
    TestWStrTrimSpace(L"WStrRTrimSpace", WStrRTrimSpace, L" 東京 ", L" 東京");

    // TestWStrSplit()
    {
    const wchar_t *lppExpectedOutputArr[] = {L"a", L"bc", L"def"};
    TestWStrSplit(L"a|bc|def", L"|", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, NULL, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    TestWStrSplit(L"a::bc::def", L"::", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, NULL, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L"", L""};
    TestWStrSplit(L"||a|bc|||def||", L"|", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, NULL, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc|def"};
    TestWStrSplit(L"a|bc|def", L"|", UNLIMITED_MIN_TOKEN_COUNT, 2, NULL, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));

    const wchar_t *lppExpectedOutputArr4[] = {L"a|bc|def"};
    TestWStrSplit(L"a|bc|def", L"~", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, NULL, lppExpectedOutputArr4, sizeof(lppExpectedOutputArr4) / sizeof(lppExpectedOutputArr4[0]));
    }

    // TestWStrSplitNewLine(): Windows newline (\r\n)
    {
    const wchar_t *lppExpectedOutputArr[] = {L"a", L"bc", L"def"};
    TestWStrSplitNewLine(L"a\r\nbc\r\ndef", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, NULL, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L""};
    TestWStrSplitNewLine(L"\r\n\r\na\r\nbc\r\n\r\n\r\ndef\r\n\r\n", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, NULL, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc\r\ndef"};
    TestWStrSplitNewLine(L"a\r\nbc\r\ndef", UNLIMITED_MIN_TOKEN_COUNT, 2, NULL, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));

    const wchar_t *lppExpectedOutputArr4[] = {L"abc"};
    TestWStrSplitNewLine(L"abc", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, NULL, lppExpectedOutputArr4, sizeof(lppExpectedOutputArr4) / sizeof(lppExpectedOutputArr4[0]));
    }

    // TestWStrSplitNewLine(): UNIX newline (\n)
    {
    const wchar_t *lppExpectedOutputArr[] = {L"a", L"bc", L"def"};
    TestWStrSplitNewLine(L"a\nbc\ndef", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, NULL, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L""};
    TestWStrSplitNewLine(L"\n\na\nbc\n\n\ndef\n\n", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, NULL, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc\ndef"};
    TestWStrSplitNewLine(L"a\nbc\ndef", UNLIMITED_MIN_TOKEN_COUNT, 2, NULL, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));
    }

    {
    const wchar_t *lppTokenArr[] = {L"a", L"bc", L"def"};
    TestWStrJoin(lppTokenArr, sizeof(lppTokenArr) / sizeof(lppTokenArr[0]), L", ", L"a, bc, def");

    const wchar_t *lppTokenArr2[] = {L"def"};
    TestWStrJoin(lppTokenArr2, sizeof(lppTokenArr2) / sizeof(lppTokenArr2[0]), L", ", L"def");
    TestWStrJoin(lppTokenArr2, sizeof(lppTokenArr2) / sizeof(lppTokenArr2[0]), L"", L"def");

    const wchar_t *lppTokenArr3[] = {L"", L"", L""};
    TestWStrJoin(lppTokenArr3, sizeof(lppTokenArr3) / sizeof(lppTokenArr3[0]), L", ", L", , ");
    TestWStrJoin(lppTokenArr3, sizeof(lppTokenArr3) / sizeof(lppTokenArr3[0]), L"", L"");
    }

    // TestWStrForEachTrimSpace(): No extra whitespace
    {
    const wchar_t *lppExpectedOutputArr[] = {L"a", L"bc", L"def"};
    TestWStrForEachTrimSpace(L"a|bc|def", L"|", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    TestWStrForEachTrimSpace(L"a::bc::def", L"::", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L"", L""};
    TestWStrForEachTrimSpace(L"||a|bc|||def||", L"|", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc|def"};
    TestWStrForEachTrimSpace(L"a|bc|def", L"|", UNLIMITED_MIN_TOKEN_COUNT, 2, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));

    const wchar_t *lppExpectedOutputArr4[] = {L"a|bc|def"};
    TestWStrForEachTrimSpace(L"a|bc|def", L"~", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, lppExpectedOutputArr4, sizeof(lppExpectedOutputArr4) / sizeof(lppExpectedOutputArr4[0]));
    }

    // TestWStrForEachTrimSpace(): Contains extra whitespace
    {
    const wchar_t *lppExpectedOutputArr[] = {L"a", L"bc", L"def"};
    TestWStrForEachTrimSpace(L" a|bc |\tdef", L"|", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    TestWStrForEachTrimSpace(L" a::bc ::\tdef", L"::", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L"", L""};
    TestWStrForEachTrimSpace(L"|\t|a|\tbc|    ||def|\t|", L"|", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc|def"};
    TestWStrForEachTrimSpace(L"a  |  bc|def\t", L"|", UNLIMITED_MIN_TOKEN_COUNT, 2, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));

    const wchar_t *lppExpectedOutputArr4[] = {L"a|bc|  def"};
    TestWStrForEachTrimSpace(L"\ta|bc|  def  ", L"~", UNLIMITED_MIN_TOKEN_COUNT, UNLIMITED_MAX_TOKEN_COUNT, lppExpectedOutputArr4, sizeof(lppExpectedOutputArr4) / sizeof(lppExpectedOutputArr4[0]));
    }

    TestWStrFileWriteAndRead(L"", 0);
    TestWStrFileWriteAndRead(L"abc", 3);
    TestWStrFileWriteAndRead(L"abc\r\ndef", 8);
    TestWStrFileWriteAndRead(L"abc東京def", 12);  // each kanji is 3 bytes in UTF-8

    {
    TestWStrArrCopyWCharArrArr((const wchar_t **) NULL, 0UL);

    const wchar_t *lppTokenArr1[] = {L"def"};
    TestWStrArrCopyWCharArrArr(lppTokenArr1, sizeof(lppTokenArr1) / sizeof(lppTokenArr1[0]));

    const wchar_t *lppTokenArr3[] = {L"a", L"bc", L"def"};
    TestWStrArrCopyWCharArrArr(lppTokenArr3, sizeof(lppTokenArr3) / sizeof(lppTokenArr3[0]));
    }

    TestWStrIsEmpty(NULL, TRUE);
    TestWStrIsEmpty(L"", TRUE);
    TestWStrIsEmpty(L"abc", FALSE);

    TestWStrConcat(NULL, L"abc", L"def", L"abcdef");
    TestWStrConcat(L"", L"abc", L"def", L"abcdef");
    TestWStrConcat(L"xyz", L"abc", L"def", L"abcdef");
    TestWStrConcat(L"xyz", L"", L"", L"");
    TestWStrConcat(L"xyz", L"abc", L"", L"abc");
    TestWStrConcat(L"xyz", L"", L"def", L"def");

    {
        struct WStr destWStr = {0};
        const struct WStr wstr = WSTR_FROM_LITERAL(NULL);
        const struct WStr wstr2 = WSTR_FROM_LITERAL(NULL);
        WStrConcatMany(&destWStr, &wstr, &wstr2, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: NULL == WStr(NULL) + WStr(NULL)",
                           NULL,
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrCopyWCharArr(&destWStr, L"", wcslen(L""));
        const struct WStr wstr = WSTR_FROM_LITERAL(NULL);
        const struct WStr wstr2 = WSTR_FROM_LITERAL(NULL);
        WStrConcatMany(&destWStr, &wstr, &wstr2, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: L\"\" == WStr(NULL) + WStr(NULL)",
                           NULL,
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrCopyWCharArr(&destWStr, L"", wcslen(L""));
        const struct WStr wstr = WSTR_FROM_LITERAL(L"");
        const struct WStr wstr2 = WSTR_FROM_LITERAL(NULL);
        WStrConcatMany(&destWStr, &wstr, &wstr2, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: L\"\" == WStr(L\"\") + WStr(NULL)",
                           NULL,
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrCopyWCharArr(&destWStr, L"", wcslen(L""));
        const struct WStr wstr = WSTR_FROM_LITERAL(L"");
        const struct WStr wstr2 = WSTR_FROM_LITERAL(L"");
        WStrConcatMany(&destWStr, &wstr, &wstr2, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: L\"\" == WStr(L\"\") + WStr(L\"\")",
                           NULL,
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrCopyWCharArr(&destWStr, L"", wcslen(L""));
        const struct WStr wstr = WSTR_FROM_LITERAL(L"abc");
        const struct WStr wstr2 = WSTR_FROM_LITERAL(L"");
        WStrConcatMany(&destWStr, &wstr, &wstr2, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: L\"abc\" == WStr(L\"abc\") + WStr(L\"\")",
                           L"abc",
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrCopyWCharArr(&destWStr, L"", wcslen(L""));
        const struct WStr wstr = WSTR_FROM_LITERAL(L"");
        const struct WStr wstr2 = WSTR_FROM_LITERAL(L"abc");
        WStrConcatMany(&destWStr, &wstr, &wstr2, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: L\"abc\" == WStr(L\"abc\") + WStr(L\"\")",
                           L"abc",
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrCopyWCharArr(&destWStr, L"", wcslen(L""));
        const struct WStr wstr = WSTR_FROM_LITERAL(L"abc");
        const struct WStr wstr2 = WSTR_FROM_LITERAL(L"def");
        WStrConcatMany(&destWStr, &wstr, &wstr2, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: L\"abcdef\" == WStr(L\"abc\") + WStr(L\"def\")",
                           L"abcdef",
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrCopyWCharArr(&destWStr, L"", wcslen(L""));
        const struct WStr wstr = WSTR_FROM_LITERAL(L"abc");
        const struct WStr wstr2 = WSTR_FROM_LITERAL(L"def");
        const struct WStr wstr3 = WSTR_FROM_LITERAL(L"ghi123");
        WStrConcatMany(&destWStr, &wstr, &wstr2, &wstr3, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: L\"abcdefghi123\" == WStr(L\"abc\") + WStr(L\"def\") + WStr(L\"ghi123\")",
                           L"abcdefghi123",
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrCopyWCharArr(&destWStr, L"xyz", wcslen(L""));
        const struct WStr wstr = WSTR_FROM_LITERAL(L"abc");
        const struct WStr wstr2 = WSTR_FROM_LITERAL(L"def");
        const struct WStr wstr3 = WSTR_FROM_LITERAL(L"ghi123");
        WStrConcatMany(&destWStr, &wstr, &wstr2, &wstr3, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: L\"abcdefghi123\" == WStr(L\"abc\") + WStr(L\"def\") + WStr(L\"ghi123\")",
                           L"abcdefghi123",
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrCopyWCharArr(&destWStr, L"xyz", wcslen(L""));
        const struct WStr wstr = WSTR_FROM_LITERAL(L"abc");
        const struct WStr wstr2 = WSTR_FROM_LITERAL(L"def");
        const struct WStr wstr3 = WSTR_FROM_LITERAL(L"ghi123");
        const struct WStr wstr4 = WSTR_FROM_LITERAL(L"456");
        WStrConcatMany(&destWStr, &wstr, &wstr2, &wstr3, &wstr4, NULL);
        TestWStrConcatMany(L"TestWStrConcatMany: L\"abcdefghi123456\" == WStr(L\"abc\") + WStr(L\"def\") + WStr(L\"ghi123\") + WStr(L\"456\")",
                           L"abcdefghi123456",
                           &destWStr);
    }
    {
        struct WStr destWStr = {0};
        WStrSPrintF(&destWStr, L"abc%d%ls", 123, L"def");
        TestWStrSPrintF(L"abc123def", &destWStr, L"abc%d%ls", L"123, L\"def\"");
    }
    {
        struct WStr destWStr = {0};
        WStrSPrintF(&destWStr, L"");
        TestWStrSPrintF(L"", &destWStr, L"", L"");
    }
    {
        struct WStr destWStr = {0};
        WStrSPrintF(&destWStr, L"abc123def");
        TestWStrSPrintF(L"abc123def", &destWStr, L"abc123def", L"");
    }
    {
        struct WStr destWStr = {0};
        WStrSPrintF(&destWStr, L"%ls", L"abc123def");
        TestWStrSPrintF(L"abc123def", &destWStr, L"%ls", L"L\"abc123def\"");
    }
    {
        struct WStr destWStr = {0};
        WStrSPrintF(&destWStr, L"%ls%ls%ls", L"abc", L"123", L"def");
        TestWStrSPrintF(L"abc123def", &destWStr, L"%ls%ls%ls", L"L\"abc\", L\"123\", L\"def\"");
    }

    TestWStrSPrintFV(L"abc123def", L"abc%d%ls", L"123, L\"def\"", 123, L"def");
    TestWStrSPrintFV(L"", L"", L"");
    TestWStrSPrintFV(L"abc123def", L"abc123def", L"");
    TestWStrSPrintFV(L"abc123def", L"%ls", L"L\"abc123def\"", L"abc123def");
    TestWStrSPrintFV(L"abc123def", L"%ls%ls%ls", L"L\"abc\", L\"123\", L\"def\"", L"abc", L"123", L"def");

    TestWStrCompareEqual(NULL, NULL);
    TestWStrCompareEqual(L"", NULL);
    TestWStrCompareEqual(L"", L"");
    TestWStrCompareEqual(NULL, L"");
    TestWStrCompareEqual(NULL, L"");

    TestWStrCompareLess(NULL, L"abc");
    TestWStrCompareLess(L"", L"abc");
    TestWStrCompareLess(L"", L" ");
    TestWStrCompareLess(L"abc", L"abcd");

    TestWStrCompareGreater(L"abc", NULL);
    TestWStrCompareGreater(L"abc", L"");
    TestWStrCompareGreater(L" ", L"");
    TestWStrCompareGreater(L"abcd", L"abc");

    return 0;
}

