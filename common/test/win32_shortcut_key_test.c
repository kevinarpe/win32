#include "win32_shortcut_key.h"
//#include "test_assert.inc"
#include <windows.h>  // required for wWinMain()
#include <stdio.h>    // required for printf()
#include <assert.h>   // required for assert()

static void TestWin32ShortcutKeyTryParseModifiers(const wchar_t                *lpTokenWCharArr,
                                                  const wchar_t                *lpShortcutKeyWCharArr,
                                                  const enum EWin32KeyModifier  eExpectedModifiers,
                                                  const wchar_t                *lpNullableExpectedErrorWCharArr)
{
    printf("TestWin32ShortcutKeyTryParseModifiers(tokenWStr[%ls], shortcutKeyWStr[%ls], eModifiers[%d], errorWStr[%ls])\r\n",
           lpTokenWCharArr, lpShortcutKeyWCharArr, eExpectedModifiers, lpNullableExpectedErrorWCharArr);
    const BOOL bExpectedResult = (NULL == lpNullableExpectedErrorWCharArr);
    const struct WStr tokenWStr = WSTR_FROM_VALUE(lpTokenWCharArr);
    const struct WStr shortcutKeyWStr = WSTR_FROM_VALUE(lpShortcutKeyWCharArr);
    const struct WStr expectedErrorWStr = WSTR_FROM_VALUE(lpNullableExpectedErrorWCharArr);
    enum EWin32KeyModifier eKeyModifiers = 0;
    struct WStr errorWStr = {0};
    const BOOL bResult = Win32ShortcutKeyTryParseModifiers(&tokenWStr,
                                                      &shortcutKeyWStr,
                                                      &eKeyModifiers,
                                                      &errorWStr);
    assert(bExpectedResult == bResult);
    assert(eExpectedModifiers == eKeyModifiers);
    assert(0 == WStrCompare(&expectedErrorWStr, &errorWStr));
}

static void TestWin32ShortcutKeyTryParseWStr(const wchar_t            *lpShortcutKeyWCharArr,
                                             const struct Win32ShortcutKey *lpExpectedShortcutKey,
                                             const wchar_t            *lpNullableExpectedErrorWCharArr)
{
    printf("TestWin32ShortcutKeyTryParseWStr(lpShortcutKeyWStr[%ls], lpExpectedShortcutKey{.eKeyModifiers = %d, .dwVkCode = %lu}, errorWStr[%ls])\r\n",
           lpShortcutKeyWCharArr, lpExpectedShortcutKey->eKeyModifiers, lpExpectedShortcutKey->dwVkCode, lpNullableExpectedErrorWCharArr);
    const BOOL bExpectedResult = (NULL == lpNullableExpectedErrorWCharArr);
    const struct WStr shortcutKeyWStr = WSTR_FROM_VALUE(lpShortcutKeyWCharArr);
    struct Win32ShortcutKey shortcutKey = {0};
    const struct WStr expectedErrorWStr = WSTR_FROM_VALUE(lpNullableExpectedErrorWCharArr);
    struct WStr errorWStr = {0};
    const BOOL bResult = Win32ShortcutKeyTryParseWStr(&shortcutKeyWStr,
                                                      &shortcutKey,
                                                      &errorWStr);
    printf("bResult[%d], shortcutKey{ .eKeyModifiers[%d], .dwVkCode[%lu] }, errorWStr[%ls]\r\n",
           bResult, shortcutKey.eKeyModifiers, shortcutKey.dwVkCode, errorWStr.lpWCharArr);
    assert(bExpectedResult == bResult);
    assert(lpExpectedShortcutKey->eKeyModifiers == shortcutKey.eKeyModifiers);
    assert(lpExpectedShortcutKey->dwVkCode == shortcutKey.dwVkCode);
    assert(0 == WStrCompare(&expectedErrorWStr, &errorWStr));
}

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI wWinMain(__attribute__((unused)) HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
                    __attribute__((unused)) HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
                    __attribute__((unused)) PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
                    __attribute__((unused)) int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    TestWin32ShortcutKeyTryParseModifiers(L"LShift", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_SHIFT_LEFT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"lshift", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_SHIFT_LEFT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"LSHIFT", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_SHIFT_LEFT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"Shift", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [Shift] is not supported: Please use [LShift] or [RShift]");
    TestWin32ShortcutKeyTryParseModifiers(L"shift", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [shift] is not supported: Please use [LShift] or [RShift]");
    TestWin32ShortcutKeyTryParseModifiers(L"SHIFT", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [SHIFT] is not supported: Please use [LShift] or [RShift]");

    TestWin32ShortcutKeyTryParseModifiers(L"RShift", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_SHIFT_RIGHT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"rshift", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_SHIFT_RIGHT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"RSHIFT", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_SHIFT_RIGHT, NULL);

    TestWin32ShortcutKeyTryParseModifiers(L"LCtrl", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_CTRL_LEFT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"lctrl", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_CTRL_LEFT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"LCTRL", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_CTRL_LEFT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"Ctrl", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [Ctrl] is not supported: Please use [LCtrl] or [RCtrl]");
    TestWin32ShortcutKeyTryParseModifiers(L"ctrl", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [ctrl] is not supported: Please use [LCtrl] or [RCtrl]");
    TestWin32ShortcutKeyTryParseModifiers(L"CTRL", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [CTRL] is not supported: Please use [LCtrl] or [RCtrl]");

    TestWin32ShortcutKeyTryParseModifiers(L"RCtrl", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_CTRL_RIGHT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"rctrl", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_CTRL_RIGHT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"RCTRL", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_CTRL_RIGHT, NULL);

    TestWin32ShortcutKeyTryParseModifiers(L"LAlt", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_ALT_LEFT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"lalt", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_ALT_LEFT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"LALT", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_ALT_LEFT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"Alt", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [Alt] is not supported: Please use [LAlt] or [RAlt]");
    TestWin32ShortcutKeyTryParseModifiers(L"alt", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [alt] is not supported: Please use [LAlt] or [RAlt]");
    TestWin32ShortcutKeyTryParseModifiers(L"ALT", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [ALT] is not supported: Please use [LAlt] or [RAlt]");

    TestWin32ShortcutKeyTryParseModifiers(L"RAlt", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_ALT_RIGHT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"ralt", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_ALT_RIGHT, NULL);
    TestWin32ShortcutKeyTryParseModifiers(L"RALT", L"LCtrl+LShift+LAlt+0x50", WIN32_KM_ALT_RIGHT, NULL);

    TestWin32ShortcutKeyTryParseWStr(L"LCtrl+LShift+LAlt+0x50",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = WIN32_KM_SHIFT_LEFT | WIN32_KM_CTRL_LEFT | WIN32_KM_ALT_LEFT, .dwVkCode = 0x50 }),
                                     NULL);
    TestWin32ShortcutKeyTryParseWStr(L"lctrl+lshift+lalt+0x50",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = WIN32_KM_SHIFT_LEFT | WIN32_KM_CTRL_LEFT | WIN32_KM_ALT_LEFT, .dwVkCode = 0x50 }),
                                     NULL);
    TestWin32ShortcutKeyTryParseWStr(L"LCTRL+LSHIFT+LALT+0x50",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = WIN32_KM_SHIFT_LEFT | WIN32_KM_CTRL_LEFT | WIN32_KM_ALT_LEFT, .dwVkCode = 0x50 }),
                                     NULL);
    TestWin32ShortcutKeyTryParseWStr(L"0x50",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = 0, .dwVkCode = 0 }),
                                     L"Failed to parse shortcut key [0x50]: Zero key modifiers found, e.g., [LCtrl+]");
    TestWin32ShortcutKeyTryParseWStr(L"Ctrl+LShift+LAlt+0x50",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = 0, .dwVkCode = 0 }),
                                     L"Failed to parse shortcut key [Ctrl+LShift+LAlt+0x50]: Shortcut key modifier [Ctrl] is not supported: Please use [LCtrl] or [RCtrl]");
    TestWin32ShortcutKeyTryParseWStr(L"LCtrl+Shift+LAlt+0x50",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = 0, .dwVkCode = 0 }),
                                     L"Failed to parse shortcut key [LCtrl+Shift+LAlt+0x50]: Shortcut key modifier [Shift] is not supported: Please use [LShift] or [RShift]");
    TestWin32ShortcutKeyTryParseWStr(L"LCtrl+LShift+Alt+0x50",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = 0, .dwVkCode = 0 }),
                                     L"Failed to parse shortcut key [LCtrl+LShift+Alt+0x50]: Shortcut key modifier [Alt] is not supported: Please use [LAlt] or [RAlt]");
    TestWin32ShortcutKeyTryParseWStr(L"LCtrl+LShift+LAlt",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = 0, .dwVkCode = 0 }),
                                     L"Failed to parse shortcut key [LCtrl+LShift+LAlt]: Failed to parse virtual key code [LAlt]: Expected hexidecimal integer, e.g., 0x50");
    TestWin32ShortcutKeyTryParseWStr(L"LCtrl+LShift+LAlt+P",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = 0, .dwVkCode = 0 }),
                                     L"Failed to parse shortcut key [LCtrl+LShift+LAlt+P]: Failed to parse virtual key code [P]: Expected hexidecimal integer, e.g., 0x50");
    TestWin32ShortcutKeyTryParseWStr(L"LCtrl+LShift+LAlt+0x432A",
                                     &((const struct Win32ShortcutKey) { .eKeyModifiers = 0, .dwVkCode = 0 }),
                                     L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x432A]: Invalid virtual key code [0x432A]->0x432A: Min: 0x01, Max: 0xFE");
    return 0;
}

