#include "wstrtoint.h"
#include <windows.h>  // required for wWinMain()
#include <stdio.h>    // required for printf()
#include <assert.h>   // required for assert()

static void TestWStrTryParseToUInt8(_In_ wchar_t       *lpWCharArr,
                                    _In_ const int      base,
                                    _In_ const BOOL     bExpectedResult,
                                    _In_ const uint8_t  ucExpectedResult)
{
    printf("TestWStrTryParseToUInt8: [%ls][%d] -> %d/%hhu\n", lpWCharArr, base, bExpectedResult, ucExpectedResult);

    struct WStr wstrText = {.lpWCharArr = lpWCharArr, .ulSize = wcslen(lpWCharArr)};
    uint8_t ucResult = 0;
    const BOOL bResult = WStrTryParseToUInt8(&wstrText, base, &ucResult, stderr, "ERROR: Failed to parse text [%ls] as uint8_t", lpWCharArr);
    assert(bExpectedResult == bResult);
    if (!bResult) {
        return;
    }

    if (ucExpectedResult != ucResult)
    {
        printf("ERROR: ucExpectedResult != ucResult: %hhu != %hhu\n", ucExpectedResult, ucResult);
        assert(ucExpectedResult == ucResult);
    }
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

    TestWStrTryParseToUInt8(L"0", 10, TRUE, (uint8_t) 0);
    TestWStrTryParseToUInt8(L"17", 10, TRUE, (uint8_t) 17);
    TestWStrTryParseToUInt8(L"177", 10, TRUE, (uint8_t) 177);
    TestWStrTryParseToUInt8(L"255", 10, TRUE, (uint8_t) 255);
    // ERROR: Failed to parse text [256] as uint8_t: Value > UINT8_MAX: 256 > 255
    TestWStrTryParseToUInt8(L"256", 10, FALSE, 0);
    // ERROR: Failed to parse text [-1] as uint8_t: Value is negative: -1
    TestWStrTryParseToUInt8(L"-1", 10, FALSE, 0);
    // ERROR: Failed to parse text [-1777] as uint8_t: Value is negative: -1777
    TestWStrTryParseToUInt8(L"-1777", 10, FALSE, 0);
    // ERROR: Failed to parse text [abc] as uint8_t: Failed to parse all chars -- one or more trailing non-digit chars: [abc]
    TestWStrTryParseToUInt8(L"abc", 10, FALSE, 0);
    // ERROR: Failed to parse text [123abc] as uint8_t: Failed to parse all chars -- one or more trailing non-digit chars: [abc]
    TestWStrTryParseToUInt8(L"123abc", 10, FALSE, 0);
    // ERROR: Failed to parse text [-123abc] as uint8_t: Failed to parse all chars -- one or more trailing non-digit chars: [abc]
    TestWStrTryParseToUInt8(L"-123abc", 10, FALSE, 0);
    // ERROR: Failed to parse text [-177777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777] as uint8_t: Value is out of range (too large or too small)
    TestWStrTryParseToUInt8(L"-177777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777", 10, FALSE, 0);

    TestWStrTryParseToUInt8(L"17", 16, TRUE, (uint8_t) 0x17);
    TestWStrTryParseToUInt8(L"0x17", 16, TRUE, (uint8_t) 0x17);
    TestWStrTryParseToUInt8(L"0X17", 16, TRUE, (uint8_t) 0x17);

    return 0;
}

