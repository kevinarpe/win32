#include "win32_shortcut_key.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW

const struct WStr WIN32_KM_SHIFT_LEFT_WSTR  = WSTR_FROM_LITERAL(L"LShift");
const struct WStr WIN32_KM_SHIFT_RIGHT_WSTR = WSTR_FROM_LITERAL(L"RShift");
const struct WStr WIN32_KM_CTRL_LEFT_WSTR   = WSTR_FROM_LITERAL(L"LCtrl");
const struct WStr WIN32_KM_CTRL_RIGHT_WSTR  = WSTR_FROM_LITERAL(L"RCtrl");
const struct WStr WIN32_KM_ALT_LEFT_WSTR    = WSTR_FROM_LITERAL(L"LAlt");
const struct WStr WIN32_KM_ALT_RIGHT_WSTR   = WSTR_FROM_LITERAL(L"RAlt");

// Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
// virtual key code -> enum EWin32KeyModifier
const enum EWin32KeyModifier
WIN32_VIRTUAL_KEY_CODE_TO_KEY_MODIFIER_ARR[256] =
{
    // indices 0-159
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    WIN32_KM_SHIFT_LEFT,   // VK_LSHIFT: 160
    WIN32_KM_SHIFT_RIGHT,  // VK_RSHIFT: 161

    WIN32_KM_CTRL_LEFT,    // VK_LCONTROL: 162
    WIN32_KM_CTRL_RIGHT,   // VK_RCONTROL: 163

    WIN32_KM_ALT_LEFT,     // VK_LMENU: 164
    WIN32_KM_ALT_RIGHT,    // VK_RMENU: 165

    // indices 166-255
//    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

const struct WStr *
WIN32_VIRTUAL_KEY_CODE_TO_LPWSTR[256] =
{
    // indices 0-159
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    &WIN32_KM_SHIFT_LEFT_WSTR,   // VK_LSHIFT: 160
    &WIN32_KM_SHIFT_RIGHT_WSTR,  // VK_RSHIFT: 161

    &WIN32_KM_CTRL_LEFT_WSTR,    // VK_LCONTROL: 162
    &WIN32_KM_CTRL_RIGHT_WSTR,   // VK_RCONTROL: 163

    &WIN32_KM_ALT_LEFT_WSTR,     // VK_LMENU: 164
    &WIN32_KM_ALT_RIGHT_WSTR,    // VK_RMENU: 165

    // indices 166-255
//    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

BOOL
Win32ShortcutKeyTryParseWStr(_In_  const struct WStr  *lpShortcutKeyWStr,
                             _Out_ struct Win32ShortcutKey *lpShortcutKey,
                             _Out_ struct WStr        *lpErrorWStr)
{
    WStrAssertValid(lpShortcutKeyWStr);
    assert(NULL != lpShortcutKey);
    WStrAssertValid(lpErrorWStr);

    const struct WStr delimWStr = WSTR_FROM_LITERAL(L"+");

    const struct WStrSplitOptions splitOptions = {
        .iMinTokenCount             = UNLIMITED_MIN_TOKEN_COUNT,
        .iMaxTokenCount             = UNLIMITED_MAX_TOKEN_COUNT,
        .fpNullableWStrConsumerFunc = WStrTrimSpace,
    };

    // Ex: "0x50" -> ["0x50"], "Ctrl+Shift+Alt+0x50" -> ["Ctrl", "Shift", "Alt", "0x50"]
    struct WStrArr tokenWStrArr = {};
    WStrSplit(lpShortcutKeyWStr, &delimWStr, &splitOptions, &tokenWStrArr);

    if (1 == tokenWStrArr.ulSize)
    {
        WStrSPrintF(lpErrorWStr, L"Failed to parse shortcut key [%ls]: Zero key modifiers found, e.g., [%ls+]",
                    lpShortcutKeyWStr->lpWCharArr, WIN32_KM_CTRL_LEFT_WSTR.lpWCharArr);
        return FALSE;
    }

    enum EWin32KeyModifier eKeyModifiers = 0;

    // Intentional: Skip last token which will be a virtual key code.
    for (size_t i = 0; i < tokenWStrArr.ulSize - 1; ++i)
    {
        // Ex: "LCtrl"
        const struct WStr *lpTokenWStr = tokenWStrArr.lpWStrArr + i;
        if (FALSE == Win32ShortcutKeyTryParseModifiers(lpTokenWStr, lpShortcutKeyWStr, &eKeyModifiers, lpErrorWStr))
        {
            // lpErrorWStr is set
            return FALSE;
        }
    }

    // Ex: "0x50"
    const struct WStr *lpVkCodeWStr = tokenWStrArr.lpWStrArr + (tokenWStrArr.ulSize - 1);
    // Ex: "0x50" -> (signed int) 0x50
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/sscanf-sscanf-l-swscanf-swscanf-l?view=msvc-170
    const int fieldCount = 1;
    // Intentional: Virtual key code has type DWORD which is an unsigned integer value.  It is not safe to scanf() unsigned int.
    // Ref: https://stackoverflow.com/a/38872682/257299
    signed int signedVkCode = 0;
    // Note: Prefixes '0x' and '0X' are automatically ignored.  Also, hex chars may be upper or lowercase.
    if (fieldCount != swscanf(lpVkCodeWStr->lpWCharArr, L"%x", &signedVkCode))
    {
        WStrSPrintF(lpErrorWStr, L"Failed to parse shortcut key [%ls]: Failed to parse virtual key code [%ls]: Expected hexidecimal integer, e.g., 0x50",
                    lpShortcutKeyWStr->lpWCharArr, lpVkCodeWStr->lpWCharArr);
        return FALSE;
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    if (signedVkCode < 0x01 || signedVkCode > 0xFE)
    {
        WStrSPrintF(lpErrorWStr, L"Failed to parse shortcut key [%ls]: Invalid virtual key code [%ls]->0x%X: Min: 0x01, Max: 0xFE",
                    lpShortcutKeyWStr->lpWCharArr, lpVkCodeWStr->lpWCharArr, signedVkCode);
        return FALSE;
    }

    WStrArrFree(&tokenWStrArr);

    lpShortcutKey->eKeyModifiers = eKeyModifiers;
    lpShortcutKey->dwVkCode      = signedVkCode;
    return TRUE;
}

static BOOL
Win32ShortcutKeyTryParseModifiers0(_In_  const struct WStr            *lpExpectedTokenWStr,  // Ex: L"LShift"
                                   _In_  const enum EWin32KeyModifier  eKeyModifier,         // Ex: WIN32_KM_SHIFT_LEFT
                                   _In_  const struct WStr            *lpTokenWStr,          // Ex: L"LShift"
                                   _In_  const struct WStr            *lpShortcutKeyWStr,    // Ex: L"LCtrl+LShift+LAlt+0x70"
                                   _Out_ enum EWin32KeyModifier       *peKeyModifiers,
                                   _Out_ struct WStr                  *lpErrorWStr)
{
    // Intentional: Case-insensitive comparison
    if (0 == _wcsicmp(lpExpectedTokenWStr->lpWCharArr, lpTokenWStr->lpWCharArr))
    {
        if (0 != (eKeyModifier & *peKeyModifiers))
        {
            WStrSPrintF(lpErrorWStr, L"Failed to parse shortcut key [%ls]: Multiple %ls modifiers are not allowed",
                        lpShortcutKeyWStr->lpWCharArr, lpExpectedTokenWStr->lpWCharArr);
            return FALSE;
        }
        *peKeyModifiers |= eKeyModifier;
        return TRUE;
    }
    return FALSE;
}

BOOL
Win32ShortcutKeyTryParseModifiers(_In_  const struct WStr      *lpTokenWStr,        // Ex: L"LShift"
                                  _In_  const struct WStr      *lpShortcutKeyWStr,  // Ex: L"Ctrl+Shift+Alt+0x70"
                                  _Out_ enum EWin32KeyModifier *peKeyModifiers,
                                  _Out_ struct WStr            *lpErrorWStr)
{
    WStrAssertValid(lpTokenWStr);
    WStrAssertValid(lpShortcutKeyWStr);
    assert(NULL != peKeyModifiers);
    WStrAssertValid(lpErrorWStr);

    // Intentional: Generic "shift" is not supported, only "lshift" or "rshift" is supported.
    if (0 == _wcsicmp(L"Shift", lpTokenWStr->lpWCharArr))
    {
        WStrSPrintF(lpErrorWStr, L"Failed to parse shortcut key [%ls]: Shortcut key modifier [%ls] is not supported: Please use [%ls] or [%ls]",
                    lpShortcutKeyWStr->lpWCharArr, lpTokenWStr->lpWCharArr, WIN32_KM_SHIFT_LEFT_WSTR.lpWCharArr, WIN32_KM_SHIFT_RIGHT_WSTR.lpWCharArr);
        return FALSE;
    }

    // Intentional: Generic "ctrl" is not supported, only "lctrl" or "rctrl" is supported.
    if (0 == _wcsicmp(L"Ctrl", lpTokenWStr->lpWCharArr))
    {
        WStrSPrintF(lpErrorWStr, L"Failed to parse shortcut key [%ls]: Shortcut key modifier [%ls] is not supported: Please use [%ls] or [%ls]",
                    lpShortcutKeyWStr->lpWCharArr, lpTokenWStr->lpWCharArr, WIN32_KM_CTRL_LEFT_WSTR.lpWCharArr, WIN32_KM_CTRL_RIGHT_WSTR.lpWCharArr);
        return FALSE;
    }

    // Intentional: Generic "alt" is not supported, only "lalt" or "ralt" is supported.
    if (0 == _wcsicmp(L"Alt", lpTokenWStr->lpWCharArr))
    {
        WStrSPrintF(lpErrorWStr, L"Failed to parse shortcut key [%ls]: Shortcut key modifier [%ls] is not supported: Please use [%ls] or [%ls]",
                    lpShortcutKeyWStr->lpWCharArr, lpTokenWStr->lpWCharArr, WIN32_KM_ALT_LEFT_WSTR.lpWCharArr, WIN32_KM_ALT_RIGHT_WSTR.lpWCharArr);
        return FALSE;
    }

    // Technical note: Win32ShortcutKeyTryParseModifiers0() can fail multiple times and set lpErrorWStr.
    // Subsequent failures will free lpErrorWStr, then set again with new error message.
    // Only the last error will be set in lpErrorWStr.  This is good enough for our needs.
    if (!Win32ShortcutKeyTryParseModifiers0(&WIN32_KM_SHIFT_LEFT_WSTR , WIN32_KM_SHIFT_LEFT , lpTokenWStr, lpShortcutKeyWStr, peKeyModifiers, lpErrorWStr)
    &&  !Win32ShortcutKeyTryParseModifiers0(&WIN32_KM_SHIFT_RIGHT_WSTR, WIN32_KM_SHIFT_RIGHT, lpTokenWStr, lpShortcutKeyWStr, peKeyModifiers, lpErrorWStr)
    &&  !Win32ShortcutKeyTryParseModifiers0(&WIN32_KM_CTRL_LEFT_WSTR  , WIN32_KM_CTRL_LEFT  , lpTokenWStr, lpShortcutKeyWStr, peKeyModifiers, lpErrorWStr)
    &&  !Win32ShortcutKeyTryParseModifiers0(&WIN32_KM_CTRL_RIGHT_WSTR , WIN32_KM_CTRL_RIGHT , lpTokenWStr, lpShortcutKeyWStr, peKeyModifiers, lpErrorWStr)
    &&  !Win32ShortcutKeyTryParseModifiers0(&WIN32_KM_ALT_LEFT_WSTR   , WIN32_KM_ALT_LEFT   , lpTokenWStr, lpShortcutKeyWStr, peKeyModifiers, lpErrorWStr)
    &&  !Win32ShortcutKeyTryParseModifiers0(&WIN32_KM_ALT_RIGHT_WSTR  , WIN32_KM_ALT_RIGHT  , lpTokenWStr, lpShortcutKeyWStr, peKeyModifiers, lpErrorWStr))
    {
        if (0 == lpErrorWStr->ulSize)
        {
            WStrSPrintF(lpErrorWStr, L"Failed to parse shortcut key [%ls]: Unknown modifier: [%ls]",
                        lpShortcutKeyWStr->lpWCharArr, lpTokenWStr->lpWCharArr);
        }
        return FALSE;
    }
    return TRUE;
}

