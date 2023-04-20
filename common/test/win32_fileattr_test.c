#include "win32_fileattr.h"
#include "test_assert.inc"
#include <windows.h>  // required for wWinMain()
#include <stdio.h>    // required for printf()
#include <assert.h>   // required for assert()

static void TestWin32FileAttrMaskToWStrArr(_In_ const DWORD     dwFileAttrMask,
                                      _In_ const wchar_t **lppExpectedArr,
                                      _In_ const size_t    ulExpectedCount)
{
    printf("TestWin32FileAttrMaskToWStrArr: dwFileAttrMask[%ld]\n", dwFileAttrMask);

    struct WStrArr wstrArr = {};
    Win32FileAttrMaskToWStrArr(dwFileAttrMask, &wstrArr);

    AssertWStrArrEqual(&wstrArr, lppExpectedArr, ulExpectedCount);
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
{
    printf("sizeof(\"ab\"): %zd\n", sizeof("ab"));
    const char *z = "ab";
    printf("sizeof(z): %zd\n", sizeof(z));
    printf("sizeof(*z): %zd\n", sizeof(*z));
    const char zz[] = "ab";
    printf("sizeof(zz): %zd\n", sizeof(zz));
    printf("sizeof(*zz): %zd\n", sizeof(*zz));
}
{
    const wchar_t *lppExpectedOutputArr[] = {L"INVALID_FILE_ATTRIBUTES"};
    TestWin32FileAttrMaskToWStrArr(INVALID_FILE_ATTRIBUTES, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));
}
{
    const wchar_t *lppExpectedOutputArr[] = {L"FILE_ATTRIBUTE_READONLY"};
    TestWin32FileAttrMaskToWStrArr(FILE_ATTRIBUTE_READONLY, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));
}
{
    const wchar_t *lppExpectedOutputArr[] = {L"FILE_ATTRIBUTE_READONLY", L"FILE_ATTRIBUTE_HIDDEN"};
    TestWin32FileAttrMaskToWStrArr(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));
    TestWin32FileAttrMaskToWStrArr(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));
}
{
    const wchar_t *lppExpectedOutputArr[] = {L"FILE_ATTRIBUTE_READONLY", L"FILE_ATTRIBUTE_HIDDEN", L"FILE_ATTRIBUTE_SYSTEM"};
    TestWin32FileAttrMaskToWStrArr(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));
}
{
    const wchar_t *lppExpectedOutputArr[] = {L"FILE_ATTRIBUTE_READONLY", L"FILE_ATTRIBUTE_HIDDEN", L"FILE_ATTRIBUTE_SYSTEM", L"FILE_ATTRIBUTE_INTEGRITY_STREAM"};
    const DWORD dwFILE_ATTRIBUTE_INTEGRITY_STREAM = 0x8000;
    TestWin32FileAttrMaskToWStrArr(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | dwFILE_ATTRIBUTE_INTEGRITY_STREAM, lppExpectedOutputArr, sizeof(lppExpectedOutputArr) / sizeof(lppExpectedOutputArr[0]));
}
    return 0;
}

