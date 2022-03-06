#include "config.h"
#include "error_exit.h"
#include "xmalloc.h"
#include <assert.h>   // required for assert()
#include <windows.h>
#include <stdio.h>

static void ConfigEntryDynArr_AssertValid(_In_ const struct ConfigEntryDynArr *lpDynArr)
{
    assert(NULL != lpDynArr);
    assert(lpDynArr->ulCapacity >= 0);
    assert(lpDynArr->ulSize <= lpDynArr->ulCapacity);
}

static void ConfigEntryDynArr_IncreaseCapacity(_Inout_ struct ConfigEntryDynArr *lpDynArr)
{
    ConfigEntryDynArr_AssertValid(lpDynArr);

    if (0 == lpDynArr->ulCapacity)
    {
        lpDynArr->lpConfigEntryArr = xcalloc(1U, sizeof(struct ConfigEntry));
        lpDynArr->ulCapacity = 1;
    }
    else
    {
        ++(lpDynArr->ulCapacity);
        xrealloc((void **) &(lpDynArr->lpConfigEntryArr),
                 sizeof(struct ConfigEntry) * lpDynArr->ulCapacity);
    }
}

static void ConfigEntryDynArr_Append(_Inout_ struct ConfigEntryDynArr *lpDynArr,
                                     _In_    const struct ConfigEntry *lpConfigEntry)
{
    ConfigEntryDynArr_AssertValid(lpDynArr);

    if (lpDynArr->ulSize == lpDynArr->ulCapacity)
    {
        ConfigEntryDynArr_IncreaseCapacity(lpDynArr);
    }

    lpDynArr->lpConfigEntryArr[lpDynArr->ulSize] = *lpConfigEntry;
    ++(lpDynArr->ulSize);
}

static void ConfigAssertValid(_In_ const struct ConfigEntryDynArr *lpDynArr)
{
    if (0 == lpDynArr->ulSize)
    {
        ErrorExit(L"Zero config entries found!");
    }

    for (size_t i = 0; i < lpDynArr->ulSize; ++i)
    {
        for (size_t j = 1 + i; j < lpDynArr->ulSize; ++j)
        {
            const struct ConfigEntry *lpEntry  = lpDynArr->lpConfigEntryArr + i;
            const struct ConfigEntry *lpEntry2 = lpDynArr->lpConfigEntryArr + j;
            if (lpEntry->shortcutKey.eModifiers == lpEntry2->shortcutKey.eModifiers
                && lpEntry->shortcutKey.dwVkCode == lpEntry2->shortcutKey.dwVkCode)
            {
                _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                             L"Config entries #%zd and #%zd have the same shortcut key\r\n", (1 + i), (1 + j));

                ErrorExit(g_lpErrorMsgBuffer);
            }
        }
    }
}

void ConfigParseFile(_In_    const wchar_t            *lpConfigFilePath,
                     _In_    const UINT                codePage,  // Ex: CP_UTF8
                     _Inout_ struct ConfigEntryDynArr *lpDynArr)
{
    struct WStr textWStr = {};
    WStrFileRead(lpConfigFilePath, codePage, &textWStr);

    const int iMaxLineCount = -1;
    struct WStrArr lineWStrArr = {};
    WStrSplitNewLine(&textWStr, iMaxLineCount, &lineWStrArr);
    // Intentional: LTrim here.  Why?  Do NOT RTrim to allow SendKeys text to contain leading & trailing whitespace.
    WStrArrForEach(&lineWStrArr, WStrLTrimSpace);

    for (size_t ulLineIndex = 0; ulLineIndex < lineWStrArr.ulSize; ++ulLineIndex)
    {
        const struct WStr *lpLineWStr = lineWStrArr.lpWStrArr + ulLineIndex;

        if (0 == lpLineWStr->ulSize) {
            continue;  // skip blank line
        }

        if (L'#' == lpLineWStr->lpWCharArr[0]) {
            continue;  // skip comment -- begins with '#'
        }

        struct ConfigEntry configEntry = {};
        ConfigParseLine(ulLineIndex, lpLineWStr, &configEntry);
        ConfigEntryDynArr_Append(lpDynArr, &configEntry);
    }

    WStrFree(&textWStr);
    WStrArrFree(&lineWStrArr);

    ConfigAssertValid(lpDynArr);
}

void ConfigParseLine(_In_  const size_t        ulLineIndex,
                     _In_  const struct WStr  *lpLineWStr,  // Ex: L"Ctrl+Shift+Alt+0x70|username"
                     _Out_ struct ConfigEntry *lpConfigEntry)
{
    WStrAssertValid(lpLineWStr);
    assert(NULL != lpConfigEntry);

    wchar_t *lpDelimWCharArr = L"|";
    struct WStr delimWStr = {.lpWCharArr = lpDelimWCharArr, .ulSize = wcslen(lpDelimWCharArr)};

    // Ex: "Ctrl+Shift+Alt+0x70|username" -> ["Ctrl+Shift+Alt+0x70", "username"]
    const int iMaxTokenCount = 2;
    struct WStrArr tokenWStrArr = {};
    WStrSplit(lpLineWStr, &delimWStr, iMaxTokenCount, &tokenWStrArr);
    // Intentional: Do NOT trim.  Why?  SendKeys text may contain leading and trailing whitespace.

    if (1 == tokenWStrArr.ulSize)
    {
        // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/snprintf-s-snprintf-s-l-snwprintf-s-snwprintf-s-l
        _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                     L"Config file: Line #%zd: Failed to find delim [%ls]\r\n"
                     L"Line: %ls\r\n",
                     (1 + ulLineIndex), lpDelimWCharArr, lpLineWStr->lpWCharArr);

        ErrorExit(g_lpErrorMsgBuffer);
    }

    // Ex: L"Ctrl+Shift+Alt+0x70|username" -> L"Ctrl+Shift+Alt+0x70"
    struct WStr *lpShortcutKeyWStr = tokenWStrArr.lpWStrArr + 0;  // '+0" -> Explicit
    // Ex: L"  Ctrl+Shift+Alt+0x70  " -> L"Ctrl+Shift+Alt+0x70"
    WStrTrimSpace(lpShortcutKeyWStr);

    // Ex: L"Ctrl+Shift+Alt+0x70|username"     -> L"username"
    // Ex: L"Ctrl+Shift+Alt+0x70|  username  " -> L"  username  "
    const struct WStr *lpSendKeysWStr = tokenWStrArr.lpWStrArr + 1;
    // Intentional: Do not trim 'lpSendKeysWStr'

    if (0 == lpShortcutKeyWStr->ulSize)
    {
        _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                     L"Config file: Line #%zd: Left side shortcut key is empty\r\n"
                     L"Line: %ls\r\n",
                     (1 + ulLineIndex), lpLineWStr->lpWCharArr);

        ErrorExit(g_lpErrorMsgBuffer);
    }
    else if (0 == lpSendKeysWStr->ulSize)
    {
        _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                     L"Config file: Line #%zd: Right side send keys text is empty\r\n"
                     L"Line: %ls\r\n",
                     (1 + ulLineIndex), lpLineWStr->lpWCharArr);

        ErrorExit(g_lpErrorMsgBuffer);
    }

    ConfigParseShortcutKey(lpShortcutKeyWStr, ulLineIndex, lpLineWStr, &(lpConfigEntry->shortcutKey));

    ConfigParseSendKeys(lpSendKeysWStr, &(lpConfigEntry->inputKeyArr));

    // Intentional: MUST copy.  Do not assign.  Why?  WStrArrFree() is called next.
    WStrCopyWStr(&(lpConfigEntry->sendKeysWStr), lpSendKeysWStr);

    WStrArrFree(&tokenWStrArr);
}

void ConfigParseShortcutKey(_In_  const struct WStr  *lpShortcutKeyWStr,  // Ex: L"Ctrl+Shift+Alt+0x70"
                            _In_  const size_t        ulLineIndex,
                            _In_  const struct WStr  *lpLineWStr,         // Ex: L"Ctrl+Shift+Alt+0x70|username"
                            _Out_ struct ShortcutKey *lpShortcutKey)
{
    WStrAssertValid(lpShortcutKeyWStr);
    WStrAssertValid(lpLineWStr);
    assert(NULL != lpShortcutKey);

    wchar_t *lpDelimWCharArr = L"+";
    struct WStr delimWStr = {.lpWCharArr = lpDelimWCharArr, .ulSize = wcslen(lpDelimWCharArr)};

    // Ex: "0x70" -> [], "Ctrl+Shift+Alt+0x70" -> ["Ctrl", "Shift", "Alt", "0x70"]
    const int iMaxTokenCount = -1;
    struct WStrArr tokenWStrArr = {};
    WStrSplit(lpShortcutKeyWStr, &delimWStr, iMaxTokenCount, &tokenWStrArr);
    WStrArrForEach(&tokenWStrArr, WStrTrimSpace);

    enum EKeyModifier eModifiers = 0;

    // Modifiers always appear before virtual-key code.  If more than two tokens, there is at least one modifier.
    if (tokenWStrArr.ulSize >= 2)
    {
        for (size_t i = 0; i < tokenWStrArr.ulSize - 1; ++i)
        {
            // Ex: "Ctrl"
            const struct WStr *lpTokenWStr = tokenWStrArr.lpWStrArr + i;
            ConfigParseModifier(lpTokenWStr, lpShortcutKeyWStr, ulLineIndex, lpLineWStr, &eModifiers);
        }
    }

    // Ex: "0x70"
    const struct WStr *lpVkCodeWStr = tokenWStrArr.lpWStrArr + (tokenWStrArr.ulSize - 1);
    // Ex: "0x70" -> (unsigned int) 0x70
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/sscanf-sscanf-l-swscanf-swscanf-l?view=msvc-170
    const int iFieldCount = 1;
    DWORD dwVkCode = 0;
    // Note: Prefix '0x' and '0X' are automatically ignored.  Also, hex chars may be upper or lowercase.
    if (iFieldCount != swscanf(lpVkCodeWStr->lpWCharArr, L"%x", &dwVkCode))
    {
        // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/snprintf-s-snprintf-s-l-snwprintf-s-snwprintf-s-l
        _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                     L"Config file: Line #%zd: Failed to parse virtual key code [%ls]%d\r\n"
                     L"Line: %ls\r\n",
                     (1 + ulLineIndex), lpVkCodeWStr->lpWCharArr, dwVkCode, lpLineWStr->lpWCharArr);

        ErrorExit(g_lpErrorMsgBuffer);
    }

    WStrArrFree(&tokenWStrArr);

    lpShortcutKey->eModifiers = eModifiers;
    lpShortcutKey->dwVkCode   = dwVkCode;
}

static BOOL ConfigParseModifier0(_In_    const wchar_t           *lpTokenWCharArr,    // Ex: L"LShift"
                                 _In_    const enum EKeyModifier  eModifier,          // Ex: SHIFT_LEFT
                                 _In_    const struct WStr       *lpTokenWStr,        // Ex: L"LShift"
                                 _In_    const struct WStr       *lpShortcutKeyWStr,  // Ex: L"LCtrl+LShift+LAlt+0x70"
                                 _In_    const size_t             ulLineIndex,
                                 _In_    const struct WStr       *lpLineWStr,         // Ex: L"LCtrl+LShift+LAlt+0x70|username"
                                 _Inout_ enum EKeyModifier       *peModifiers)
{
    if (0 == _wcsicmp(lpTokenWCharArr, lpTokenWStr->lpWCharArr))
    {
        if (0 != (eModifier & *peModifiers))
        {
            _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                         L"Config file: Line #%zd: Multiple %ls modifiers are not allowed: [%ls]\r\n"
                         L"Line: %ls\r\n",
                         (1 + ulLineIndex), lpTokenWCharArr, lpShortcutKeyWStr->lpWCharArr, lpLineWStr->lpWCharArr);

            ErrorExit(g_lpErrorMsgBuffer);
        }
        *peModifiers |= eModifier;
        return TRUE;
    }
    return FALSE;
}

void ConfigParseModifier(_In_    const struct WStr *lpTokenWStr,        // Ex: L"Shift"
                         _In_    const struct WStr *lpShortcutKeyWStr,  // Ex: L"Ctrl+Shift+Alt+0x70"
                         _In_    const size_t       ulLineIndex,
                         _In_    const struct WStr *lpLineWStr,         // Ex: L"Ctrl+Shift+Alt+0x70|username"
                         _Inout_ enum EKeyModifier *peModifiers)
{
    WStrAssertValid(lpTokenWStr);
    WStrAssertValid(lpShortcutKeyWStr);
    WStrAssertValid(lpLineWStr);
    assert(NULL != peModifiers);

    if (0 == _wcsicmp(L"Shift", lpTokenWStr->lpWCharArr))
    {
        ErrorExit(L"Shortcut key modifier 'Shift' is not supported.  Please use 'LShift' or 'RShift'.");
    }
    if (0 == _wcsicmp(L"Ctrl", lpTokenWStr->lpWCharArr))
    {
        ErrorExit(L"Shortcut key modifier 'Ctrl' is not supported.  Please use 'LCtrl' or 'RCtrl'.");
    }
    if (0 == _wcsicmp(L"Alt", lpTokenWStr->lpWCharArr))
    {
        ErrorExit(L"Shortcut key modifier 'Alt' is not supported.  Please use 'LAlt' or 'RAlt'.");
    }

    if (!ConfigParseModifier0(L"LShift", SHIFT_LEFT , lpTokenWStr, lpShortcutKeyWStr, ulLineIndex, lpLineWStr, peModifiers)
    &&  !ConfigParseModifier0(L"RShift", SHIFT_RIGHT, lpTokenWStr, lpShortcutKeyWStr, ulLineIndex, lpLineWStr, peModifiers)
    &&  !ConfigParseModifier0(L"LCtrl" , CTRL_LEFT  , lpTokenWStr, lpShortcutKeyWStr, ulLineIndex, lpLineWStr, peModifiers)
    &&  !ConfigParseModifier0(L"RCtrl" , CTRL_RIGHT , lpTokenWStr, lpShortcutKeyWStr, ulLineIndex, lpLineWStr, peModifiers)
    &&  !ConfigParseModifier0(L"LAlt"  , ALT_LEFT   , lpTokenWStr, lpShortcutKeyWStr, ulLineIndex, lpLineWStr, peModifiers)
    &&  !ConfigParseModifier0(L"RAlt"  , ALT_RIGHT  , lpTokenWStr, lpShortcutKeyWStr, ulLineIndex, lpLineWStr, peModifiers))
    {
        // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/snprintf-s-snprintf-s-l-snwprintf-s-snwprintf-s-l
        _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                     L"Config file: Line #%zd: Unknown modifier: [%ls]\r\n"
                     L"Line: %ls\r\n",
                     (1 + ulLineIndex), lpTokenWStr->lpWCharArr, lpLineWStr->lpWCharArr);

        ErrorExit(g_lpErrorMsgBuffer);
    }
}

void ConfigParseSendKeys(_In_  const struct WStr  *lpSendKeysWStr,  // Ex: L"username"
                         _Out_ struct InputKeyArr *lpInputKeyArr)
{
    WStrAssertValid(lpSendKeysWStr);
    assert(lpSendKeysWStr->ulSize > 0);
    assert(NULL != lpInputKeyArr);

    // Each down keystroke must have a matching up keystroke
    lpInputKeyArr->ulSize = 2U * lpSendKeysWStr->ulSize;

    lpInputKeyArr->lpInputKeyArr = xcalloc(lpInputKeyArr->ulSize, sizeof(INPUT));

    size_t ulInputKeyArrIndex = 0;
    for (size_t i = 0; i < lpSendKeysWStr->ulSize; ++i, ulInputKeyArrIndex += 2)
    {
        const wchar_t wchar = lpSendKeysWStr->lpWCharArr[i];

        INPUT *lpInputKeyDown = lpInputKeyArr->lpInputKeyArr + ulInputKeyArrIndex;
        INPUT *lpInputKeyUp   = lpInputKeyArr->lpInputKeyArr + ulInputKeyArrIndex + 1;

        lpInputKeyDown->type = INPUT_KEYBOARD;
        lpInputKeyDown->ki.wVk = 0;
        lpInputKeyDown->ki.wScan = wchar;
        lpInputKeyDown->ki.dwFlags = KEYEVENTF_UNICODE;

        lpInputKeyUp->type = INPUT_KEYBOARD;
        lpInputKeyUp->ki.wVk = 0;
        lpInputKeyUp->ki.wScan = wchar;
        lpInputKeyUp->ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    }
}

