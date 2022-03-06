#include "wstr.h"
#include "error_exit.h"
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

static void AssertWStrArrEqual(_In_ const struct WStrArr  *wstrArr,
                               _In_ const wchar_t        **lppExpectedTokenArr,
                               _In_ const size_t           ulExpectedTokenCount)
{
    assert(ulExpectedTokenCount == wstrArr->ulSize);
    if (0 == ulExpectedTokenCount)
    {
        assert(NULL == lppExpectedTokenArr);
        assert(NULL == wstrArr->lpWStrArr);
    }
    else
    {
        for (size_t i = 0; i < ulExpectedTokenCount; ++i)
        {
            if (L'\0' == lppExpectedTokenArr[i][0])
            {
                assert(NULL == wstrArr->lpWStrArr[i].lpWCharArr);
            }
            else
            {
                assert(0 == wcscmp(lppExpectedTokenArr[i], wstrArr->lpWStrArr[i].lpWCharArr));
            }
            assert(wcslen(lppExpectedTokenArr[i]) == wstrArr->lpWStrArr[i].ulSize);
        }
    }
}

static void TestWStrSplit(_In_ wchar_t        *lpWCharArr,
                          _In_ wchar_t        *lpDelim,
                          _In_ const int       iMaxTokenCount,
                          _In_ const wchar_t **lppExpectedTokenArr,
                          _In_ const size_t    ulExpectedTokenCount)
{
    printf("TestWStrSplit: [%ls][%ls][%d] -> (%zd)", lpWCharArr, lpDelim, iMaxTokenCount, ulExpectedTokenCount);
    for (size_t i = 0; i < ulExpectedTokenCount; ++i)
    {
        printf("[%ls]", lppExpectedTokenArr[i]);
    }
    printf("\r\n");

    struct WStr wstrText = {.lpWCharArr = lpWCharArr, .ulSize = wcslen(lpWCharArr)};
    struct WStr wstrDelim = {.lpWCharArr = lpDelim, .ulSize = wcslen(lpDelim)};
    struct WStrArr wstrArr = {};
    WStrSplit(&wstrText, &wstrDelim, iMaxTokenCount, &wstrArr);

    AssertWStrArrEqual(&wstrArr, lppExpectedTokenArr, ulExpectedTokenCount);
    TestWStrArrFree(&wstrArr);
}

static void TestWStrSplitNewLine(_In_ wchar_t        *lpWCharArr,
                                 _In_ const int       iMaxLineCount,
                                 _In_ const wchar_t **lppExpectedLineArr,
                                 _In_ const size_t    ulExpectedLineCount)
{
    assert(ulExpectedLineCount > 0);

    printf("TestWStrSplitNewLine: [%ls][%d] -> (%zd)", lpWCharArr, iMaxLineCount, ulExpectedLineCount);
    for (size_t i = 0; i < ulExpectedLineCount; ++i)
    {
        printf("[%ls]", lppExpectedLineArr[i]);
    }
    printf("\r\n");

    struct WStr wstrText = {.lpWCharArr = lpWCharArr, .ulSize = wcslen(lpWCharArr)};
    struct WStrArr wstrArr = {};
    WStrSplitNewLine(&wstrText, iMaxLineCount, &wstrArr);

    printf("Actual: (%zd)", wstrArr.ulSize);
    for (size_t i = 0; i < wstrArr.ulSize; ++i)
    {
        printf("[%ls]", wstrArr.lpWStrArr + i);
    }
    printf("\r\n");

    AssertWStrArrEqual(&wstrArr, lppExpectedLineArr, ulExpectedLineCount);
    TestWStrArrFree(&wstrArr);
}

static void TestWStrForEachTrimSpace(_In_ wchar_t        *lpWCharArr,
                                     _In_ wchar_t        *lpDelim,
                                     _In_ const int       iMaxTokenCount,
                                     _In_ const wchar_t **lppExpectedTokenArr,
                                     _In_ const size_t    ulExpectedTokenCount)
{
    printf("TestWStrForEachTrimSpace: [%ls][%ls][%d] -> (%zd)", lpWCharArr, lpDelim, iMaxTokenCount, ulExpectedTokenCount);
    for (size_t i = 0; i < ulExpectedTokenCount; ++i)
    {
        printf("[%ls]", lppExpectedTokenArr[i]);
    }
    printf("\r\n");

    struct WStr wstrText = {.lpWCharArr = lpWCharArr, .ulSize = wcslen(lpWCharArr)};
    struct WStr wstrDelim = {.lpWCharArr = lpDelim, .ulSize = wcslen(lpDelim)};
    struct WStrArr wstrArr = {};
    WStrSplit(&wstrText, &wstrDelim, iMaxTokenCount, &wstrArr);
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
        ErrorExit(L"CreateFile(lpFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)");
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesize
    const LPDWORD lpdwHighFileSize = NULL;
    const DWORD dwFileSize = GetFileSize(hReadFile, lpdwHighFileSize);
    if (INVALID_FILE_SIZE == dwFileSize)
    {
        ErrorExit(L"GetFileSize");
    }

    if (!CloseHandle(hReadFile))
    {
        ErrorExit(L"CloseHandle(hReadFile)");
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

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI wWinMain(HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
                    HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
                    PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
                    int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
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
    TestWStrSplit(L"a|bc|def", L"|", -1, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    TestWStrSplit(L"a::bc::def", L"::", -1, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L"", L""};
    TestWStrSplit(L"||a|bc|||def||", L"|", -1, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc|def"};
    TestWStrSplit(L"a|bc|def", L"|", 2, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));

    const wchar_t *lppExpectedOutputArr4[] = {L"a|bc|def"};
    TestWStrSplit(L"a|bc|def", L"~", -1, lppExpectedOutputArr4, sizeof(lppExpectedOutputArr4) / sizeof(lppExpectedOutputArr4[0]));
    }

    // TestWStrSplitNewLine(): Windows newline (\r\n)
    {
    const wchar_t *lppExpectedOutputArr[] = {L"a", L"bc", L"def"};
    TestWStrSplitNewLine(L"a\r\nbc\r\ndef", -1, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L""};
    TestWStrSplitNewLine(L"\r\n\r\na\r\nbc\r\n\r\n\r\ndef\r\n\r\n", -1, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc\r\ndef"};
    TestWStrSplitNewLine(L"a\r\nbc\r\ndef", 2, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));

    const wchar_t *lppExpectedOutputArr4[] = {L"abc"};
    TestWStrSplitNewLine(L"abc", -1, lppExpectedOutputArr4, sizeof(lppExpectedOutputArr4) / sizeof(lppExpectedOutputArr4[0]));
    }

    // TestWStrSplitNewLine(): UNIX newline (\n)
    {
    const wchar_t *lppExpectedOutputArr[] = {L"a", L"bc", L"def"};
    TestWStrSplitNewLine(L"a\nbc\ndef", -1, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L""};
    TestWStrSplitNewLine(L"\n\na\nbc\n\n\ndef\n\n", -1, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc\ndef"};
    TestWStrSplitNewLine(L"a\nbc\ndef", 2, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));
    }

    // TestWStrForEachTrimSpace(): No extra whitespace
    {
    const wchar_t *lppExpectedOutputArr[] = {L"a", L"bc", L"def"};
    TestWStrForEachTrimSpace(L"a|bc|def", L"|", -1, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    TestWStrForEachTrimSpace(L"a::bc::def", L"::", -1, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L"", L""};
    TestWStrForEachTrimSpace(L"||a|bc|||def||", L"|", -1, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc|def"};
    TestWStrForEachTrimSpace(L"a|bc|def", L"|", 2, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));

    const wchar_t *lppExpectedOutputArr4[] = {L"a|bc|def"};
    TestWStrForEachTrimSpace(L"a|bc|def", L"~", -1, lppExpectedOutputArr4, sizeof(lppExpectedOutputArr4) / sizeof(lppExpectedOutputArr4[0]));
    }

    // TestWStrForEachTrimSpace(): Contains extra whitespace
    {
    const wchar_t *lppExpectedOutputArr[] = {L"a", L"bc", L"def"};
    TestWStrForEachTrimSpace(L" a|bc |\tdef", L"|", -1, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    TestWStrForEachTrimSpace(L" a::bc ::\tdef", L"::", -1, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));

    const wchar_t *lppExpectedOutputArr2[] = {L"", L"", L"a", L"bc", L"", L"", L"def", L"", L""};
    TestWStrForEachTrimSpace(L"|\t|a|\tbc|    ||def|\t|", L"|", -1, lppExpectedOutputArr2, sizeof(lppExpectedOutputArr2) / sizeof(lppExpectedOutputArr2[0]));

    const wchar_t *lppExpectedOutputArr3[] = {L"a", L"bc|def"};
    TestWStrForEachTrimSpace(L"a  |  bc|def\t", L"|", 2, lppExpectedOutputArr3, sizeof(lppExpectedOutputArr3) / sizeof(lppExpectedOutputArr3[0]));

    const wchar_t *lppExpectedOutputArr4[] = {L"a|bc|  def"};
    TestWStrForEachTrimSpace(L"\ta|bc|  def  ", L"~", -1, lppExpectedOutputArr4, sizeof(lppExpectedOutputArr4) / sizeof(lppExpectedOutputArr4[0]));
    }

    TestWStrFileWriteAndRead(L"", 0);
    TestWStrFileWriteAndRead(L"abc", 3);
    TestWStrFileWriteAndRead(L"abc\r\ndef", 8);
    TestWStrFileWriteAndRead(L"abc東京def", 12);  // each kanji is 3 bytes in UTF-8

    return 0;
}

