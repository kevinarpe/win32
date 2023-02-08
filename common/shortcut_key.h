#ifndef H_COMMON_SHORTCUT_KEY
#define H_COMMON_SHORTCUT_KEY

#include "win32.h"
#include "wstr.h"

// Captain Obvious says: These are all bitwise flags (power of two).
enum EKeyModifier
{
    KM_SHIFT_LEFT  = 1,
    KM_SHIFT_RIGHT = 2,

    KM_CTRL_LEFT   = 4,
    KM_CTRL_RIGHT  = 8,

    KM_ALT_LEFT    = 16,
    KM_ALT_RIGHT   = 32,
};

extern const struct WStr KM_SHIFT_LEFT_WSTR ;  // = WSTR_FROM_LITERAL(L"LShift");
extern const struct WStr KM_SHIFT_RIGHT_WSTR;  // = WSTR_FROM_LITERAL(L"RShift");
extern const struct WStr KM_CTRL_LEFT_WSTR  ;  // = WSTR_FROM_LITERAL(L"LCtrl");
extern const struct WStr KM_CTRL_RIGHT_WSTR ;  // = WSTR_FROM_LITERAL(L"RCtrl");
extern const struct WStr KM_ALT_LEFT_WSTR   ;  // = WSTR_FROM_LITERAL(L"LAlt");
extern const struct WStr KM_ALT_RIGHT_WSTR  ;  // = WSTR_FROM_LITERAL(L"RAlt");

// Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
// virtual key code -> enum EKeyModifier
extern const enum EKeyModifier VIRTUAL_KEY_CODE_TO_KEY_MODIFIER_ARR[256];
extern const struct WStr *     VIRTUAL_KEY_CODE_TO_LPWSTR[256];

struct ShortcutKey
{
    enum EKeyModifier eKeyModifiers;
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-kbdllhookstruct
    // See: KBDLLHOOKSTRUCT->vkCode
    DWORD             dwVkCode;
};

/**
 * @param lpShortcutKeyWStr
 *        Ex: L"LCtrl+LShift+LAlt+0x50"
 *
 * @param lpShortcutKey
 *        output value -- only set if return result is TRUE
 *        Ex: (struct ShortcutKey) { .eKeyModifiers = KM_CTRL_LEFT | KM_SHIFT_LEFT | KM_ALT_LEFT, .dwVkCode = 0x50 }
 *
 * @param lpErrorWStr
 *        output value -- only set if return result is FALSE
 *        Ex: 
 *
 * @return TRUE on success
 */
BOOL
ShortcutKeyTryParseWStr(_In_  const struct WStr  *lpShortcutKeyWStr,
                        _Out_ struct ShortcutKey *lpShortcutKey,
                        _Out_ struct WStr        *lpErrorWStr);

// ...

BOOL
ShortcutKeyTryParseModifiers(_In_  const struct WStr *lpTokenWStr,        // Ex: L"Shift"
                             _In_  const struct WStr *lpShortcutKeyWStr,  // Ex: L"Ctrl+Shift+Alt+0x70"
                             _Out_ enum EKeyModifier *peKeyModifiers,
                             _Out_ struct WStr       *lpErrorWStr);

#endif  // H_COMMON_SHORTCUT_KEY

