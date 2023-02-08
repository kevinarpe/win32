#include "shortcut_key.h"
#include "error_exit.h"
//#include "test_assert.inc"
#include <windows.h>  // required for wWinMain()
#include <stdio.h>    // required for printf()
#include <assert.h>   // required for assert()

static void TestShortcutKeyTryParseModifiers(const wchar_t           *lpTokenWCharArr,
                                             const wchar_t           *lpShortcutKeyWCharArr,
                                             const enum EKeyModifier  eExpectedModifiers,
                                             const wchar_t           *lpNullableExpectedErrorWCharArr)
{
    printf("TestShortcutKeyTryParseModifiers(tokenWStr[%ls], shortcutKeyWStr[%ls], eModifiers[%d], errorWStr[%ls])\r\n",
           lpTokenWCharArr, lpShortcutKeyWCharArr, eExpectedModifiers, lpNullableExpectedErrorWCharArr);
    const BOOL bExpectedResult = (NULL == lpNullableExpectedErrorWCharArr);
    const struct WStr tokenWStr = WSTR_FROM_VALUE(lpTokenWCharArr);
    const struct WStr shortcutKeyWStr = WSTR_FROM_VALUE(lpShortcutKeyWCharArr);
    const struct WStr expectedErrorWStr = WSTR_FROM_VALUE(lpNullableExpectedErrorWCharArr);
    enum EKeyModifier eModifiers = 0;
    struct WStr errorWStr = {0};
    const BOOL bResult = ShortcutKeyTryParseModifiers(&tokenWStr,
                                                      &shortcutKeyWStr,
                                                      &eModifiers,
                                                      &errorWStr);
    assert(bExpectedResult == bResult);
    assert(eExpectedModifiers == eModifiers);
    assert(0 == WStrCompare(&expectedErrorWStr, &errorWStr));
}

/**
 * @param lpShortcutKeyWStr
 *        Ex: L"LCtrl+LShift+LAlt+0x50"
 *
 * @param lpShortcutKey
 *        output value -- only set if return result is TRUE
 *        Ex: (struct ShortcutKey) { .eModifiers = KM_CTRL_LEFT | KM_SHIFT_LEFT | KM_ALT_LEFT, .dwVkCode = 0x50 }
 *
 * @param lpErrorWStr
 *        output value -- only set if return result is FALSE
 *        Ex:
 *
 * @return TRUE on success

BOOL ShortcutKeyTryParseWStr(_In_  const struct WStr  *lpShortcutKeyWStr,
                             _Out_ struct ShortcutKey *lpShortcutKey,
                             _Out_ struct WStr        *lpErrorWStr);

struct ShortcutKey
{
    enum EKeyModifier eModifiers;
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-kbdllhookstruct
    // See: KBDLLHOOKSTRUCT->vkCode
    DWORD             dwVkCode;
};
*/
static void TestShortcutKeyTryParseWStr(const wchar_t            *lpShortcutKeyWCharArr,
                                        const struct ShortcutKey *lpExpectedShortcutKey,
                                        const wchar_t            *lpNullableExpectedErrorWCharArr)
{
    printf("TestShortcutKeyTryParseWStr(lpShortcutKeyWStr[%ls], lpShortcutKey{.eModifiers = %d, .dwVkCode = %lu}, errorWStr[%ls])\r\n",
           lpShortcutKeyWCharArr, lpExpectedShortcutKey->eModifiers, lpExpectedShortcutKey->dwVkCode, lpNullableExpectedErrorWCharArr);
    const BOOL bExpectedResult = (NULL == lpNullableExpectedErrorWCharArr);
    const struct WStr shortcutKeyWStr = WSTR_FROM_VALUE(lpShortcutKeyWCharArr);
    struct ShortcutKey shortcutKey = {0};
    const struct WStr expectedErrorWStr = WSTR_FROM_VALUE(lpNullableExpectedErrorWCharArr);
    struct WStr errorWStr = {0};
    const BOOL bResult = ShortcutKeyTryParseWStr(&shortcutKeyWStr,
                                                 &shortcutKey,
                                                 &errorWStr);
    printf("bResult[%d], shortcutKey{ .eModifiers[%d], .dwVkCode[%lu] }, errorWStr[%ls]\r\n",
           bResult, shortcutKey.eModifiers, shortcutKey.dwVkCode, errorWStr.lpWCharArr);
    assert(bExpectedResult == bResult);
    assert(lpExpectedShortcutKey->eModifiers == shortcutKey.eModifiers);
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
    TestShortcutKeyTryParseModifiers(L"LShift", L"LCtrl+LShift+LAlt+0x50", KM_SHIFT_LEFT, NULL);
    TestShortcutKeyTryParseModifiers(L"lshift", L"LCtrl+LShift+LAlt+0x50", KM_SHIFT_LEFT, NULL);
    TestShortcutKeyTryParseModifiers(L"LSHIFT", L"LCtrl+LShift+LAlt+0x50", KM_SHIFT_LEFT, NULL);
    TestShortcutKeyTryParseModifiers(L"Shift", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [Shift] is not supported: Please use [LShift] or [RShift]");
    TestShortcutKeyTryParseModifiers(L"shift", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [shift] is not supported: Please use [LShift] or [RShift]");
    TestShortcutKeyTryParseModifiers(L"SHIFT", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [SHIFT] is not supported: Please use [LShift] or [RShift]");

    TestShortcutKeyTryParseModifiers(L"RShift", L"LCtrl+LShift+LAlt+0x50", KM_SHIFT_RIGHT, NULL);
    TestShortcutKeyTryParseModifiers(L"rshift", L"LCtrl+LShift+LAlt+0x50", KM_SHIFT_RIGHT, NULL);
    TestShortcutKeyTryParseModifiers(L"RSHIFT", L"LCtrl+LShift+LAlt+0x50", KM_SHIFT_RIGHT, NULL);

    TestShortcutKeyTryParseModifiers(L"LCtrl", L"LCtrl+LShift+LAlt+0x50", KM_CTRL_LEFT, NULL);
    TestShortcutKeyTryParseModifiers(L"lctrl", L"LCtrl+LShift+LAlt+0x50", KM_CTRL_LEFT, NULL);
    TestShortcutKeyTryParseModifiers(L"LCTRL", L"LCtrl+LShift+LAlt+0x50", KM_CTRL_LEFT, NULL);
    TestShortcutKeyTryParseModifiers(L"Ctrl", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [Ctrl] is not supported: Please use [LCtrl] or [RCtrl]");
    TestShortcutKeyTryParseModifiers(L"ctrl", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [ctrl] is not supported: Please use [LCtrl] or [RCtrl]");
    TestShortcutKeyTryParseModifiers(L"CTRL", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [CTRL] is not supported: Please use [LCtrl] or [RCtrl]");

    TestShortcutKeyTryParseModifiers(L"RCtrl", L"LCtrl+LShift+LAlt+0x50", KM_CTRL_RIGHT, NULL);
    TestShortcutKeyTryParseModifiers(L"rctrl", L"LCtrl+LShift+LAlt+0x50", KM_CTRL_RIGHT, NULL);
    TestShortcutKeyTryParseModifiers(L"RCTRL", L"LCtrl+LShift+LAlt+0x50", KM_CTRL_RIGHT, NULL);

    TestShortcutKeyTryParseModifiers(L"LAlt", L"LCtrl+LShift+LAlt+0x50", KM_ALT_LEFT, NULL);
    TestShortcutKeyTryParseModifiers(L"lalt", L"LCtrl+LShift+LAlt+0x50", KM_ALT_LEFT, NULL);
    TestShortcutKeyTryParseModifiers(L"LALT", L"LCtrl+LShift+LAlt+0x50", KM_ALT_LEFT, NULL);
    TestShortcutKeyTryParseModifiers(L"Alt", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [Alt] is not supported: Please use [LAlt] or [RAlt]");
    TestShortcutKeyTryParseModifiers(L"alt", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [alt] is not supported: Please use [LAlt] or [RAlt]");
    TestShortcutKeyTryParseModifiers(L"ALT", L"LCtrl+LShift+LAlt+0x50", 0, L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x50]: Shortcut key modifier [ALT] is not supported: Please use [LAlt] or [RAlt]");

    TestShortcutKeyTryParseModifiers(L"RAlt", L"LCtrl+LShift+LAlt+0x50", KM_ALT_RIGHT, NULL);
    TestShortcutKeyTryParseModifiers(L"ralt", L"LCtrl+LShift+LAlt+0x50", KM_ALT_RIGHT, NULL);
    TestShortcutKeyTryParseModifiers(L"RALT", L"LCtrl+LShift+LAlt+0x50", KM_ALT_RIGHT, NULL);

    TestShortcutKeyTryParseWStr(L"LCtrl+LShift+LAlt+0x50",
                                &((const struct ShortcutKey) { .eModifiers = KM_SHIFT_LEFT | KM_CTRL_LEFT | KM_ALT_LEFT, .dwVkCode = 0x50 }),
                                NULL);
    TestShortcutKeyTryParseWStr(L"lctrl+lshift+lalt+0x50",
                                &((const struct ShortcutKey) { .eModifiers = KM_SHIFT_LEFT | KM_CTRL_LEFT | KM_ALT_LEFT, .dwVkCode = 0x50 }),
                                NULL);
    TestShortcutKeyTryParseWStr(L"LCTRL+LSHIFT+LALT+0x50",
                                &((const struct ShortcutKey) { .eModifiers = KM_SHIFT_LEFT | KM_CTRL_LEFT | KM_ALT_LEFT, .dwVkCode = 0x50 }),
                                NULL);
    TestShortcutKeyTryParseWStr(L"0x50",
                                &((const struct ShortcutKey) { .eModifiers = 0, .dwVkCode = 0 }),
                                L"Failed to parse shortcut key [0x50]: Zero key modifiers found, e.g., [LCtrl+]");
    TestShortcutKeyTryParseWStr(L"Ctrl+LShift+LAlt+0x50",
                                &((const struct ShortcutKey) { .eModifiers = 0, .dwVkCode = 0 }),
                                L"Failed to parse shortcut key [Ctrl+LShift+LAlt+0x50]: Shortcut key modifier [Ctrl] is not supported: Please use [LCtrl] or [RCtrl]");
    TestShortcutKeyTryParseWStr(L"LCtrl+Shift+LAlt+0x50",
                                &((const struct ShortcutKey) { .eModifiers = 0, .dwVkCode = 0 }),
                                L"Failed to parse shortcut key [LCtrl+Shift+LAlt+0x50]: Shortcut key modifier [Shift] is not supported: Please use [LShift] or [RShift]");
    TestShortcutKeyTryParseWStr(L"LCtrl+LShift+Alt+0x50",
                                &((const struct ShortcutKey) { .eModifiers = 0, .dwVkCode = 0 }),
                                L"Failed to parse shortcut key [LCtrl+LShift+Alt+0x50]: Shortcut key modifier [Alt] is not supported: Please use [LAlt] or [RAlt]");
    TestShortcutKeyTryParseWStr(L"LCtrl+LShift+LAlt",
                                &((const struct ShortcutKey) { .eModifiers = 0, .dwVkCode = 0 }),
                                L"Failed to parse shortcut key [LCtrl+LShift+LAlt]: Failed to parse virtual key code [LAlt]: Expected hexidecimal integer, e.g., 0x50");
    TestShortcutKeyTryParseWStr(L"LCtrl+LShift+LAlt+P",
                                &((const struct ShortcutKey) { .eModifiers = 0, .dwVkCode = 0 }),
                                L"Failed to parse shortcut key [LCtrl+LShift+LAlt+P]: Failed to parse virtual key code [P]: Expected hexidecimal integer, e.g., 0x50");
    TestShortcutKeyTryParseWStr(L"LCtrl+LShift+LAlt+0x432A",
                                &((const struct ShortcutKey) { .eModifiers = 0, .dwVkCode = 0 }),
                                L"Failed to parse shortcut key [LCtrl+LShift+LAlt+0x432A]: Invalid virtual key code [0x432A]->0x432A: Min: 0x01, Max: 0xFE");

    return 0;
}

