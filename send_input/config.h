#ifndef _H_CONFIG
#define _H_CONFIG

#include "wstr.h"
#include <winuser.h>  // required for INPUT

// Captain Obvious says: These are all bitwise flags (power of two).
enum EKeyModifier
{
    SHIFT_LEFT  = 1,
    SHIFT_RIGHT = 2,

    CTRL_LEFT   = 4,
    CTRL_RIGHT  = 8,

    ALT_LEFT    = 16,
    ALT_RIGHT   = 32,
};

struct ShortcutKey
{
    enum EKeyModifier eModifiers;
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-kbdllhookstruct
    // See: KBDLLHOOKSTRUCT->vkCode
    DWORD dwVkCode;
};

struct InputKeyArr
{
    INPUT *lpInputKeyArr;
    size_t ulSize;
};

struct ConfigEntry
{
    struct ShortcutKey shortcutKey;
    struct WStr        sendKeysWStr;
    struct InputKeyArr inputKeyArr;
    size_t             ulSendKeysCount;
};

struct ConfigEntryDynArr
{
    struct ConfigEntry *lpConfigEntryArr;
    size_t ulSize;
    size_t ulCapacity;
};

void ConfigParseFile(_In_    const wchar_t            *lpConfigFilePath,
                     _In_    const UINT                codePage,  // Ex: CP_UTF8
                     _Inout_ struct ConfigEntryDynArr *lpDynArr);

void ConfigParseLine(_In_  const size_t        ulLineIndex,
                     _In_  const struct WStr  *lpLineWStr,  // Ex: L"Ctrl+Shift+Alt+0x70|username"
                     _Out_ struct ConfigEntry *lpConfigEntry);

void ConfigParseShortcutKey(_In_  const struct WStr  *lpShortcutKeyWStr,  // Ex: L"Ctrl+Shift+Alt+0x70"
                            _In_  const size_t        ulLineIndex,
                            _In_  const struct WStr  *lpLineWStr,
                            _Out_ struct ShortcutKey *lpShortcutKey);

void ConfigParseModifier(_In_    const struct WStr *lpTokenWStr,        // Ex: L"Shift"
                         _In_    const struct WStr *lpShortcutKeyWStr,  // Ex: L"Ctrl+Shift+Alt+0x70"
                         _In_    const size_t       ulLineIndex,
                         _In_    const struct WStr *lpLineWStr,
                         _Inout_ enum EKeyModifier *peModifiers);

void ConfigParseSendKeys(_In_  const struct WStr  *lpSendKeysWStr,  // Ex: L"username"
                         _Out_ struct InputKeyArr *lpInputKeyArr);

#endif  // _H_CONFIG

