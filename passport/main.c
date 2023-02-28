#include "win32.h"
#include "win32_clipboard.h"
#include "log.h"
#include "wstr.h"
#include "error_exit.h"
#include "win32_monitor.h"
#include "win32_hwnd.h"
#include "win32_dpi.h"
#include "win32_size_grip_control.h"
#include "win32_set_focus.h"
#include "config.h"
#include <windows.h>
#include <assert.h>
#include <wchar.h>
#include <stdio.h>
#include <commctrl.h>
#include <stdbool.h>

// Common types:
// See: https://en.cppreference.com/w/c/types/integer -> Format macro constants
// Ref: https://learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types
// WORD   : 16-bit unsigned int
// DWORD  : 32-bit unsigned int
// UINT   : 32-bit unsigned int
// LONG   : 32-bit   signed int
// WPARAM : 64-bit unsigned long long
// LPARAM : 64-bit   signed long long
// LRESULT: 64-bit   signed long long

// iDEA: Write super clipboard like IntelliJ, but system-wide.
// Ctrl+Shift+Alt+V will pop-up window to select text to paste.
// Each clipboard copy will added to super clipboard.

// TODO: Convert to dialog instead of window.  Then it can be used else where.
//       If extra awesome, also allow memory leak-free destory for dialog.
//       Ref: https://learn.microsoft.com/en-us/windows/win32/dlgbox/dialog-boxes
//       Ref: https://stackoverflow.com/questions/61634/windows-api-dialogs-without-using-resource-files

// TODO: Add new resize window shortcuts: Ctrl+Alt+Shift+Left/Right/Up/Down

// Ref: https://stackoverflow.com/a/37272350/257299
// Ref: https://devblogs.microsoft.com/oldnewthing/20041214-00/?p=37013
// Ref: https://learn.microsoft.com/en-us/cpp/mfc/tn020-id-naming-and-numbering-conventions?view=msvc-170
#define IDC_BUTTON_OK     100
#define IDC_BUTTON_CANCEL 101
//#define ID_SUBCLASS_LIST_BOX 1
const WORD IDM_ACCEL_ESCAPE  = 1000;
const WORD IDM_ACCEL_RETURN  = 1001;
const WORD IDM_ACCEL_CTRL_C  = 1002;

// Ref(ACCEL): https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-accel
// "cmd: WORD: The accelerator identifier. This value is placed in the low-order word of the wParam parameter
//  of the WM_COMMAND or WM_SYSCOMMAND message when the accelerator is pressed."
// Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
const ACCEL ACCEL_ARR[] = {
    {.fVirt = FVIRTKEY, .key = VK_ESCAPE, .cmd = IDM_ACCEL_ESCAPE},
    {.fVirt = FVIRTKEY, .key = VK_RETURN, .cmd = IDM_ACCEL_RETURN},
    {.fVirt = FCONTROL | FVIRTKEY, .key = 0x43, .cmd = IDM_ACCEL_CTRL_C},
};
struct RECTEx
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/windef/ns-windef-rect
    // struct { LONG left; LONG top; LONG right; LONG bottom; };
    RECT r;
    // Pre-compute width and height
    LONG lWidth;
    LONG lHeight;
};
struct LayoutConfig
{
    // Classic font "point" size, e.g., 14
    UINT        fontPointSize;
    // Ref: https://learn.microsoft.com/en-us/typography/font-list/consolas
    // Ex: L"Consolas"
    struct WStr fontFaceNameWStr;
    // Number of pixels (in standard 96 DPI) of spacing between (1) each widget and (2) border of dialog/window, e.g., 5
    UINT        spacing;
    // Ex: 128x128 (in standard 96 DPI)
    SIZE        listBoxMinSize;
    // Ex: L"Select password to copy to clipboard:"
    struct WStr labelDescWStr;
    // Ex: L"Ctrl+C to copy username to clipboard"
    struct WStr labelTipWStr;
};
struct Layout
{
    struct LayoutConfig     config;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-createstructw
    CREATESTRUCTW           createStruct;
    struct Win32DPI         dpi;
    // Ex: 5 * 144 / 96 = 7.5 -> 8
    UINT                    scaledSpacing;
    // Ex: 128 * (144 / 96) = 192
    SIZE                    listBoxScaledMinSize;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logfontw
    LOGFONTW                logFont;
    HFONT                   hFont;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-textmetricw
    TEXTMETRICW             fontMetrics;
    // Ex: L"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    struct WStr             fontSampleWStr;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/windef/ns-windef-size
    // Ex: 930x33
    SIZE                    fontSampleSize;
    struct Win32MonitorInfo primaryMonitorInfo;
    // Coordinates are relative to 'primaryMonitorInfo'.  .left & .top are x & y coordinates.
    struct RECTEx           windowNonClientRectEx;
    struct RECTEx           labelDescRectEx;
    struct RECTEx           listBoxRectEx;
    struct RECTEx           labelTipRectEx;
    struct RECTEx           buttonOkRectEx;
    struct RECTEx           buttonCancelRectEx;
// TODO: Is resize by size grip still broken?
    struct RECTEx           leftSizeGripRectEx;
    struct RECTEx           rightSizeGripRectEx;
};
struct Window
{
    struct Config       config;
    BOOL                bIsInitDone;
    HACCEL              hAccel;
    struct Layout       layout;
    HWND                hWnd;
    HWND                hStaticDesc;
    HWND                hListBox;
    HWND                hStaticTip;
    HWND                hButtonOk;
    HWND                hButtonCancel;
    HWND                hLeftSizeGrip;
    HWND                hRightSizeGrip;
    HHOOK               hHookLowLevelKeyboard;
};
// Used by Set/GetWindowLongPtrW(...)
static const int WINDOW_LONG_PTR_INDEX = 0;
struct Global 
{
    WNDCLASSEXW       wndClassExW;
    ATOM              registerClassExAtom;
    enum EKeyModifier eKeyModifierDownFlags;
    struct Window     win;
};
struct Global global = {0};
// Ref: https://docs.microsoft.com/en-us/windows/win32/winmsg/lowlevelkeyboardproc
static LRESULT CALLBACK
LowLevelKeyboardProc(_In_ const int    nCode,
                     _In_ const WPARAM wParam,  // Any of: WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, or WM_SYSKEYUP
                     _In_ const LPARAM lParam)
{
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-kbdllhookstruct
    const KBDLLHOOKSTRUCT* info = (KBDLLHOOKSTRUCT *) lParam;
//    const BOOL bIsInjected = (0 != (info->flags & LLKHF_INJECTED));
    const BOOL bIsInjected = (LLKHF_INJECTED == (info->flags & LLKHF_INJECTED));
    if (HC_ACTION == nCode && !bIsInjected)
    {
        // Is this key stroke a function key (F1, etc.) or a numpad key?
//        const BOOL bIsExtended = (0 != (info->flags & LLKHF_EXTENDED));

        // If TRUE, then 'wParam' will be WM_SYSKEYDOWN or WM_SYSKEYUP
//        const BOOL bIsAltDown = (0 != (info->flags & LLKHF_ALTDOWN));

        // If TRUE, then key is released (up).  If FALSE, then key is pressed (down).
        // If TRUE, then key is pressed (down).  If FALSE, then key is released (up).
        const BOOL bIsKeyDown = (0 == (info->flags & LLKHF_UP));
        DEBUG_LOGWF(stdout, L"INFO: bIsKeyDown:%s, global.eKeyModifierDownFlags:%d, info->vkCode:%u\r\n",
                    bIsKeyDown ? "TRUE" : "FALSE", global.eKeyModifierDownFlags, info->vkCode);

        const enum EKeyModifier eKeyMod = VIRTUAL_KEY_CODE_TO_KEY_MODIFIER_ARR[info->vkCode];
        if (0 != eKeyMod)
        {
            if (bIsKeyDown)
            {   
                // When key down, add flag 'eKeyModifier'
                global.eKeyModifierDownFlags |= eKeyMod;
            }
            else
            {   
                // When key up, remove flag 'eKeyModifier'
                global.eKeyModifierDownFlags &= ~eKeyMod;
            }
        }
        else if (bIsKeyDown
                 && global.eKeyModifierDownFlags == global.win.config.shortcutKey.eKeyModifiers
                 && info->vkCode                 == global.win.config.shortcutKey.dwVkCode)
        {
            DEBUG_LOGW(stdout, L"INFO: Shortcut key pressed\r\n");
            // Is window minimised?  Show.
            // Ref: https://learn.microsoft.com/en-us/windows/win31/api/winuser/nf-winuser-isiconic
            if (IsIconic(global.win.hWnd))  // [in] HWND hWnd
            {
                DEBUG_LOGW(stdout, L"INFO: IsIconic: ShowWindow(hWnd, SW_NORMAL)\r\n");
                ShowWindow(global.win.hWnd,  // [in] HWND hWnd
                           // "Activates and displays a window. If the window is minimized or maximized, the system restores it to its original size and position.
                           //  An application should specify this flag when displaying the window for the first time."
                           SW_NORMAL);       // [in] int  nCmdShow
            }
            // Is window hidden?  Show.
            // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-iswindowvisible
            else if (!IsWindowVisible(global.win.hWnd))  // [in] HWND hWnd
            {
                DEBUG_LOGW(stdout, L"INFO: !IsWindowVisible: ShowWindow(hWnd, SW_NORMAL)\r\n");
                // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
                // "If the window was previously visible, the return value is nonzero.
                //  If the window was previously hidden, the return value is zero."
                ShowWindow(global.win.hWnd,  // [in] HWND hWnd
                           // "Activates the window and displays it in its current size and position."
//                           SW_SHOW);         // [in] int  nCmdShow
                           // "Activates and displays a window. If the window is minimized or maximized, the system restores it to its original size and position.
                           //  An application should specify this flag when displaying the window for the first time."
                           SW_NORMAL);         // [in] int  nCmdShow
            }
            // Is window visible?  Activate.
            else {
                // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getactivewindow
                // @Nullable
                const HWND hNullableWndActive = GetActiveWindow();
                // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setactivewindow
                const HWND hNullablePrevWndActive = SetActiveWindow(global.win.hWnd);  // [in] HWND hWnd
                if (hNullableWndActive != hNullablePrevWndActive)
                {
                    ErrorExitW(L"SetActiveWindow(global.win.hWnd)");
                }
            }
        }
    }
    const LRESULT x = CallNextHookEx((HHOOK) 0, nCode, wParam, lParam);
    return x;
}
static void
_printfLParamWM_SIZE(_In_ const LPARAM lParam)
{
    __attribute__((unused))
    const WORD wWidth  = LOWORD(lParam);
    __attribute__((unused))
    const WORD wHeight = HIWORD(lParam);
    DEBUG_LOGWF(stdout, L"width:LOWORD(lParam):%d, height:HIWORD(lParam):%d", wWidth, wHeight);
}
/*
static void
_printfLParamRECT(const LPARAM lParam)
{
    const RECT *lpRect = (RECT *) lParam;
    wprintf(L"{.left=%ld, .top=%ld, .right=%ld, .bottom=%ld}",
            lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
}
*/
static void
_logMessage(__attribute__((unused))
            _In_ const LPCWSTR lpMsg,
            _In_ const WPARAM  wParam,
            _In_ const LPARAM  lParam,
            _In_ void (*fpPrintfLParam) (const LPARAM lParam), ...)  // one or more pairs: (LPCWSTR lpValue, WPARAM wParam), followed by NULL
{
    DEBUG_LOGWF(stdout, L"INFO: %ls: ", lpMsg);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, fpPrintfLParam);
    BOOL match = FALSE;
    while (TRUE)
    {
        const LPCWSTR lpValue = va_arg(ap, LPCWSTR);
        if (NULL == lpValue)
        {
            break;
        }
        const WPARAM wParam2 = va_arg(ap, WPARAM);
        if (wParam == wParam2)
        {
            #ifndef NDEBUG
                wprintf(L"WPARAM wParam: %lu/%ls, ", wParam, lpValue);
            #endif
            match = TRUE;
            break;
        }
    }
    if (!match)
    {
        #ifndef NDEBUG
            printf("WPARAM wParam: %llu/???, ", wParam);
        #endif
    }
    #ifndef NDEBUG
        printf("LPARAM lParam: ");
    #endif
    fpPrintfLParam(lParam);
    #ifndef NDEBUG
        puts("\r\n");  // newline only
    #endif
}
/*
// Ref: https://stackoverflow.com/a/15977613/257299
static WPARAM
MapLeftRightKeys(_In_ const WPARAM wParam, _In_ const LPARAM lParam)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#keystroke-message-flags
    const UINT scanCode   = (lParam & 0x00ff0000) >> 16;
    const BOOL isExtended = (lParam & 0x01000000) != 0;

    switch (wParam)
    {
        case VK_SHIFT:
        {
            // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mapvirtualkeyw
            const UINT virtualKeyCode =
                MapVirtualKeyW(scanCode,  // [in] UINT uCode,
                               // "The uCode parameter is a scan code and is translated into a virtual-key code that distinguishes between left- and right-hand keys.
                               //  If there is no translation, the function returns 0."
                               MAPVK_VSC_TO_VK_EX);  // [in] UINT uMapType

            if (0 == virtualKeyCode)
            {
                // Note: There won't actually get a GetLastError(), but we want to log and abort here.
                ErrorExitWF(L"Failed to convert VK_SHIFT to VK_LSHIFT or VK_RSHIFT: WPARAM wParam[%llu], LPARAM lParam[%lld], UINT scanCode[%u], BOOL isExtended[%s]",
                            wParam, lParam, scanCode, isExtended ? "TRUE" : "FALSE");
            }
            return virtualKeyCode;
        }
        case VK_CONTROL:
        {
            const WPARAM x = isExtended ? VK_RCONTROL : VK_LCONTROL;
            return x;
        }
        case VK_MENU:
        {
            const WPARAM x = isExtended ? VK_RMENU : VK_LMENU;
            return x;
        }
        default:
        {
            return wParam;
        }
    }
}
*/
/*
LRESULT CALLBACK
GlobalWindowProc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam, BOOL *lpIsProcessed)
{
    assert(NULL != lpIsProcessed);

    DEBUG_LOGWF(stdout, L"GlobalWindowProc(HWND hWnd[%p], UINT uMsg[%u/%ls], WPARAM wParam[%llu], LPARAM lParam[%lld], ...)\n",
                hWnd, uMsg, Win32MsgToText(uMsg), wParam, lParam);

    *lpIsProcessed = TRUE;
    switch (uMsg)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
        case WM_KEYDOWN:
        // Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-syskeydown
        case WM_SYSKEYDOWN:  // Alt+Key
        {
            const WPARAM wParam2 = MapLeftRightKeys(wParam, lParam);
            if (wParam2 != wParam)
            {
                const enum ModifierKeyFlag flag = VIRTUAL_KEY_CODE_TO_MODIFIER_KEY_FLAG_ARR[wParam2];
                const enum ModifierKeyFlag before = g_modifierKeyDownFlags;
                g_modifierKeyDownFlags |= flag;
                DEBUG_LOGWF(stdout, L"%s: Map wParam[%llu->%llu]: g_modifierKeyDownFlags[%d->%d]\n",
                            (WM_KEYUP == uMsg ? "WM_KEYUP" : "WM_SYSKEYUP"), wParam, wParam2, before, g_modifierKeyDownFlags);
                // Intentional: Do not return here: Allow DefWindowProc(...) to be called.
                break;
            }
            else if ('C' == wParam && MK_IS_ALT_KEY_DOWN_ONLY(g_modifierKeyDownFlags))
            {
                const WPARAM wParam3 = MAKELPARAM(IDC_BUTTON_CANCEL,  // [in] WORD loword
                                                  BN_CLICKED);        // [in] WORD hiword
                const LRESULT x = WindowProc(g_window.hWnd, WM_COMMAND, wParam3, (LPARAM) g_window.hButtonCancel);  // (tail) recursion
                return x;
            }
            else if ('O' == wParam && MK_IS_ALT_KEY_DOWN_ONLY(g_modifierKeyDownFlags))
            {
                const WPARAM wParam3 = MAKELPARAM(IDC_BUTTON_OK,  // [in] WORD loword
                                                  BN_CLICKED);    // [in] WORD hiword
                const LRESULT x = WindowProc(g_window.hWnd, WM_COMMAND, wParam3, (LPARAM) g_window.hButtonOk);  // (tail) recursion
                return x;
            }
            else if ('C' == wParam && MK_IS_CONTROL_KEY_DOWN_ONLY(g_modifierKeyDownFlags))
            {
// TODO: Replace me with current selected item in listview
                const struct WStr textWStr = WSTR_FROM_LITERAL(L"username");
                ClipboardWriteWStr(g_window.hWnd, &textWStr);
                DEBUG_LOGWF(stdout, L"Copied [%ls] to clipboard\n", textWStr.lpWCharArr);
                // "If an application processes this message, it should return zero."
                return 0;
            }
            else if (VK_ESCAPE == wParam && 0 == g_modifierKeyDownFlags)
            {
                const WPARAM wParam3 = MAKELPARAM(IDC_BUTTON_CANCEL,  // [in] WORD loword
                                                  BN_CLICKED);        // [in] WORD hiword
                const LRESULT x = WindowProc(g_window.hWnd, WM_COMMAND, wParam3, (LPARAM) g_window.hButtonCancel);  // (tail) recursion
                return x;
            }
            break;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keyup
        case WM_KEYUP:
        // Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-syskeyup
        case WM_SYSKEYUP:  // Alt+Key
        {
            const WPARAM wParam2 = MapLeftRightKeys(wParam, lParam);
            if (wParam2 != wParam)
            {
                const enum ModifierKeyFlag flag = VIRTUAL_KEY_CODE_TO_MODIFIER_KEY_FLAG_ARR[wParam2];
                const enum ModifierKeyFlag before = g_modifierKeyDownFlags;
                g_modifierKeyDownFlags &= ~flag;
                DEBUG_LOGWF(stdout, L"%s: Map wParam[%llu->%llu]: g_modifierKeyDownFlags[%d->%d]\n",
                            (WM_KEYUP == uMsg ? "WM_KEYUP" : "WM_SYSKEYUP"), wParam, wParam2, before, g_modifierKeyDownFlags);
                // Intentional: Do not return here: Allow DefWindowProc(...) to be called.
                break;
            }
            break;
        }
    }
    *lpIsProcessed = FALSE;
    return 0;
}
*/
/*
// Ref: https://stackoverflow.com/a/31659946/257299
// Ref: https://devblogs.microsoft.com/oldnewthing/?p=41883
// Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-setwindowsubclass
// Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-removewindowsubclass
// Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nc-commctrl-subclassproc
LRESULT CALLBACK
ListBoxSubclassProc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam, const UINT_PTR uIdSubclass, const DWORD_PTR dwRefData)
{
    DEBUG_LOGWF(stdout, L"ListBoxSubclassProc(HWND hWnd[%p], UINT uMsg[%u/%ls], WPARAM wParam[%llu], LPARAM lParam[%lld], UINT_PTR uIdSubclass[%llu], DWORD_PTR dwRefData[%llu])\n",
                hWnd, uMsg, Win32MsgToText(uMsg), wParam, lParam, uIdSubclass, dwRefData);

    BOOL isProcessed = FALSE;
    const LRESULT lResult = GlobalWindowProc(hWnd, uMsg, wParam, lParam, &isProcessed);
    if (TRUE == isProcessed)
    {
        return lResult;
    }

    switch (uMsg)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-ncdestroy
        // Ref: https://devblogs.microsoft.com/oldnewthing/?p=41883
        case WM_NCDESTROY:
        {
            // Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-removewindowsubclass
            if (FALSE == RemoveWindowSubclass(g_window.hListBox, &ListBoxSubclassProc, ID_SUBCLASS_LIST_BOX))
            {
                ErrorExitW(L"RemoveWindowSubclass(g_window.hListBox, &ListBoxSubclassProc, ID_SUBCLASS_LIST_BOX)");
            }
            break;
        }
    }
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-defsubclassproc
    const LRESULT x = DefSubclassProc(hWnd, uMsg, wParam, lParam);
    return x;
}
*/
static LONG
rectWidth(_In_ RECT *lpRect)
{
    assert(NULL != lpRect);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/windef/ns-windef-rect
    // "right: Specifies the x-coordinate of the lower-right corner of the rectangle."
    // "left: Specifies the x-coordinate of the upper-left corner of the rectangle."
    const LONG x = lpRect->right - lpRect->left;
    return x;
}
static LONG
rectHeight(_In_ RECT *lpRect)
{
    assert(NULL != lpRect);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/windef/ns-windef-rect
    // "bottom: Specifies the y-coordinate of the lower-right corner of the rectangle."
    // "top: Specifies the y-coordinate of the upper-left corner of the rectangle."
    const LONG x = lpRect->bottom - lpRect->top;
    return x;
}
static void
setRectEx(_Inout_ struct RECTEx *lpRectEx,
          _In_    const LONG     left,
          _In_    const LONG     top,
          _In_    const LONG     right,
          _In_    const LONG     bottom)
{
    assert(NULL != lpRectEx);

    lpRectEx->r.left   = left;
    lpRectEx->r.top    = top;
    lpRectEx->r.right  = right;
    lpRectEx->r.bottom = bottom;
    lpRectEx->lWidth   = right - left;
    lpRectEx->lHeight  = bottom - top;
}
static void
GetTextSize(_In_  HDC                hDC,
            _In_  const struct WStr *lpWStr,
            _Out_ SIZE              *lpSize)
{
    WStrAssertValid(lpWStr);
    assert(NULL != lpSize);

    // About DrawText() vs GetTextExtentPoint32W():
    // Ref: https://stackoverflow.com/questions/1126730/how-to-find-the-width-of-a-string-in-pixels-in-win32

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawtextw
    RECT rect = {0};
    if (!DrawTextW(hDC,                                         // [in]      HDC     hdc,
                   lpWStr->lpWCharArr,                          // [in, out] LPCWSTR lpchText,
                   lpWStr->ulSize,                              // [in]      int     cchText,
                   &rect,                                       // [in, out] LPRECT  lprc,
                   (UINT) (                                     // [in]      UINT    format
                       DT_CALCRECT  // "Determines the width and height of the rectangle.
                                    //  If there is only one line of text, DrawText modifies the right side of the rectangle so that it bounds the last character in the line.
                                    //  DrawText returns the height of the formatted text but does not draw the text."
                       | DT_NOPREFIX  // "Turns off processing of prefix characters.
                                      //  Normally, DrawText interprets the mnemonic-prefix character & as a directive to underscore the character that follows,
                                      //  and the mnemonic-prefix characters && as a directive to print a single &. By specifying DT_NOPREFIX, this processing is turned off."
                       | DT_SINGLELINE)))  // "Displays text on a single line only. Carriage returns and line feeds do not break the line."
    {
        ErrorExitWF(L"DrawText(hDC, lpWStr->lpWCharArr[%ls], lpWStr->ulSize[%zd], &rect, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE)",
                    lpWStr->lpWCharArr, lpWStr->ulSize);
    }

    lpSize->cx = rectWidth(&rect);
    lpSize->cy = rectHeight(&rect);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-gettextextentpoint32w
    SIZE size = {0};
    if (!GetTextExtentPoint32W(hDC,                 // [in]  HDC     hdc,
                               lpWStr->lpWCharArr,  // [in]  LPCWSTR lpString,
                               lpWStr->ulSize,      // [in]  int     c,
                               &size))              // [out] LPSIZE  lpsz
    {
        ErrorExitWF(L"GetTextExtentPoint32W(hDC, lpWStr->lpWCharArr[%ls], lpWStr->ulSize[%zd], &size)",
                    lpWStr->lpWCharArr, lpWStr->ulSize);
    }

    assert(lpSize->cx == size.cx);
    assert(lpSize->cy == size.cy);
}
// TODO: Fix window resize so there is *zero* extra work.
static void
WindowLayoutInit(_In_ const HWND     hWnd,
                 // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-createstructw
                 _In_ CREATESTRUCTW *lpCreateStruct)
{
    assert(NULL != lpCreateStruct);

    struct Window *lpWin    = lpCreateStruct->lpCreateParams;
    struct Layout *lpLayout = &lpWin->layout;

    Win32DPIGet(&lpLayout->dpi, hWnd);

    // Ex: 5 * 144 / 96 = 7.5 -> 8
    lpLayout->scaledSpacing = MulDiv(lpLayout->config.spacing,  // [in] int nNumber
                                     lpLayout->dpi.dpi,         // [in] int nNumerator
                                     USER_DEFAULT_SCREEN_DPI);  // [in] int nDenominator

    // If our monitor is HiDPI, e.g., 4K, then we need to scale our pixels.
    // Ex: 128 x (144 / 96) = 192
    lpLayout->listBoxScaledMinSize =
        (SIZE)
        {
            .cx = MulDiv(lpLayout->config.listBoxMinSize.cx,  // [in] int nNumber
                         lpLayout->dpi.dpi,                   // [in] int nNumerator
                         USER_DEFAULT_SCREEN_DPI),            // [in] int nDenominator
            .cy = MulDiv(lpLayout->config.listBoxMinSize.cy,  // [in] int nNumber
                         lpLayout->dpi.dpi,                   // [in] int nNumerator
                         USER_DEFAULT_SCREEN_DPI)             // [in] int nDenominator
        };

    // This is an (in)famous font size formula from Microsoft.
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-muldiv
    // Ref: https://jeffpar.github.io/kbarchive/kb/074/Q74299/
    //      "When an application calls the CreateFont() or CreateFontIndirect() functions and
    //       specifies a negative value for the height parameter, the font mapper provides
    //       the closest match for the character height rather than the cell height."
    // Ref: https://learn.microsoft.com/en-us/windows/win32/learnwin32/dpi-and-device-independent-pixels
    // Ref: http://www.winprog.org/tutorial/fonts.html
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logfontw
    //      "lfHeight: < 0	The font mapper transforms this value into device units and matches its absolute value against the character height of the available fonts."
    //      "For the MM_TEXT mapping mode, you can use the following formula to specify a height for a font with a specified point size:
    //       lfHeight = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);"
    // Ex: -28
    lpLayout->logFont.lfHeight =
        -1L * MulDiv(lpLayout->config.fontPointSize,  // [in] int nNumber
                     lpLayout->dpi.dpi,               // [in] int nNumerator
                     POINTS_PER_INCH);                // [in] int nDenominator

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-s-wcscpy-s-mbscpy-s?view=msvc-170
    if (wcscpy_s(lpLayout->logFont.lfFaceName,                                                    // [inout] wchar_t *dest
                 sizeof(lpLayout->logFont.lfFaceName) / sizeof(lpLayout->logFont.lfFaceName[0]),  // [in]    rsize_t dest_size
                 lpLayout->config.fontFaceNameWStr.lpWCharArr))                                   // [int]   const wchar_t *src
    {
        ErrorExitWF(L"wcscpy_s(lpLayout->logFont.lfFaceName, ..., lpLayout->config.fontFaceNameWStr.lpWCharArr[%ls])",
                    lpLayout->config.fontFaceNameWStr.lpWCharArr);
    }
                 
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfontindirectw
    lpLayout->hFont = CreateFontIndirectW(&lpLayout->logFont);
    if (NULL == lpLayout->hFont)
    {
        ErrorExitWF(L"CreateFontIndirectW(lfHeight:%ld, lfFaceName[%ls])", lpLayout->logFont.lfHeight, lpLayout->logFont.lfFaceName);
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc
    HDC hDC = GetDC(hWnd);  // [in] HWND hWnd
    if (NULL == hDC)
    {
        ErrorExitW(L"GetDC(hWnd)");
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectobject
    if (NULL == SelectObject(hDC, (HGDIOBJ) (lpLayout->hFont)))
    {
        ErrorExitW(L"SelectObject(hDC, (HGDIOBJ) (lpLayout->hFont))");
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-gettextmetricsw
    if (!GetTextMetricsW(hDC, &lpLayout->fontMetrics))
    {
        ErrorExitW(L"GetTextMetricsW(dc, &lpLayout->fontMetrics)");
    }

    // Digits plus English alphabet twice is 10 + (2 x 26) = 62 chars
    lpLayout->fontSampleWStr = WSTR_FROM_LITERAL(L"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    // Ex: 930x33
    GetTextSize(hDC,                         // [in]    HDC      hdc
                &lpLayout->fontSampleWStr,   // [in]    WStr    *lpWStr
                &lpLayout->fontSampleSize);  // [inout] SIZE    *lpSize

    // Ref: https://learn.microsoft.com/en-us/previous-versions/ms997619(v=msdn.10)?redirectedfrom=MSDN
    // "One horizontal dialog unit is equal to one-fourth of the average character width for the current system font."
    const int iHDiaLogUnit      =  4;
    // "One vertical dialog unit is equal to one-eighth of an average character height for the current system font."
    const int iVDiaLogUnit      =  8;
    // "The default height for most single-line controls is 14 DLUs."
    const int iVButtonHeightDLU = 14;
    // "Size of Common Dialog Box Controls: Command buttons: Width (DLUs): 50"
    const int iHButtonWidthDLU  = 50;

    // Ex: 930 * 50 / (4 * 62) = 188
    const int iButtonWidth = MulDiv(lpLayout->fontSampleSize.cx,                      // [in] int nNumber
                                    iHButtonWidthDLU,                                 // [in] int nNumerator
                                    iHDiaLogUnit * lpLayout->fontSampleWStr.ulSize);  // [in] int nDenominator
    // Ex: 33 * 14 / 8 = 58
    const int iButtonHeight = MulDiv(lpLayout->fontMetrics.tmHeight,  // [in] int nNumber
                                     iVButtonHeightDLU,               // [in] int nNumerator
                                     iVDiaLogUnit);                   // [in] int nDenominator

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawtext
    // Ex: 555x33
    SIZE labelDescTextSize = {0};
    GetTextSize(hDC,                              // [in]    HDC      hdc
                &lpLayout->config.labelDescWStr,  // [in]    WStr    *lpWStr
                &labelDescTextSize);              // [inout] SIZE    *lpSize

    // Ex: 540x33
    SIZE labelTipTextSize = {0};
    GetTextSize(hDC,                             // [in]    HDC      hdc
                &lpLayout->config.labelTipWStr,  // [in]    WStr    *lpWStr
                &labelTipTextSize);              // [inout] SIZE    *lpSize

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-releasedc
    if (!ReleaseDC(NULL, hDC))
    {
        ErrorExitW(L"ReleaseDC(NULL, hDC)");
    }

    // Ex: 5 * 144 / 96 -> 7.5 -> 8
    const size_t sp = lpLayout->scaledSpacing;

    const size_t ullSizeGripControlWidthAndHeight =
        MulDiv(WIN32_SIZE_GRIP_CONTROL_WIDTH_AND_HEIGHT,  // [in] int nNumber
               lpLayout->dpi.dpi,                         // [in] int nNumerator
               USER_DEFAULT_SCREEN_DPI);                  // [in] int nDenominator

    // Ex: 555
    const LONG lMinWidthWithoutEdgeSpacing =
        max(
            max(
                max(lpLayout->listBoxScaledMinSize.cx,
                    labelDescTextSize.cx),
                labelTipTextSize.cx),
            ullSizeGripControlWidthAndHeight + iButtonWidth + sp + sp + iButtonWidth + ullSizeGripControlWidthAndHeight);

    // Ex: 565
    const LONG lWidth = sp + lMinWidthWithoutEdgeSpacing + sp;

    // Ex: 405
    const LONG lHeight = sp + labelDescTextSize.cy + sp + lpLayout->listBoxScaledMinSize.cy + sp + labelTipTextSize.cy + sp + iButtonHeight + sp;

    Win32MonitorGetInfoForPrimary(&lpLayout->primaryMonitorInfo);

    // Ex: 1024
    const LONG lPrimaryMonitorWidth  = rectWidth(&lpLayout->primaryMonitorInfo.monitorInfo.rcWork);
    // Ex: 768
    const LONG lPrimaryMonitorHeight = rectHeight(&lpLayout->primaryMonitorInfo.monitorInfo.rcWork);

    // Ex: 229
    const LONG left = (lPrimaryMonitorWidth - lWidth) / 2;
    // Ex: 181
    const LONG top  = (lPrimaryMonitorHeight - lHeight) / 2;

    // Ref: https://stackoverflow.com/a/61681245/257299
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-adjustwindowrectex
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-adjustwindowrectexfordpi
    RECT adjRect = {.left   = left,
                    .top    = top,
                    .right  = left + lWidth,
                    .bottom = top + lHeight};

    if (!AdjustWindowRectExForDpi(
            &adjRect,                                        // [in, out] LPRECT lpRect
            lpCreateStruct->style,                           // [in]      DWORD  dwStyle
            (NULL == lpCreateStruct->hMenu) ? FALSE : TRUE,  // [in]      BOOL   bMenu
            lpCreateStruct->dwExStyle,                       // [in]      DWORD  dwExStyle
            lpLayout->dpi.dpi))                              // [in]      UINT   dpi
    {
        ErrorExitW(L"AdjustWindowRectExForDpi");
    }

    DEBUG_LOGWF(stdout, L"INFO: AdjustWindowRectExForDpi(): RECT{.left = %d->%d, .top = %d->%d, .right = %d->%d, .bottom = %d->%d}\n",
                left, adjRect.left, top, adjRect.top, left + lWidth, adjRect.right, top + lHeight, adjRect.bottom);

    setRectEx(&lpLayout->windowNonClientRectEx,
              adjRect.left,
              adjRect.top,
              adjRect.right,
              adjRect.bottom);

    struct RECTEx windowClientRectEx = {0};
    setRectEx(&windowClientRectEx,
              0,         // left
              0,         // top
              lWidth,    // right
              lHeight);  // bottom

    setRectEx(&lpLayout->labelDescRectEx,
              sp,  // left
              sp,  // top
              sp + labelDescTextSize.cx,
              sp + labelDescTextSize.cy);

    setRectEx(&lpLayout->listBoxRectEx,
              lpLayout->labelDescRectEx.r.left,
              lpLayout->labelDescRectEx.r.bottom + sp, 
              lpLayout->labelDescRectEx.r.left + lMinWidthWithoutEdgeSpacing,
              lpLayout->labelDescRectEx.r.bottom + sp + lpLayout->listBoxScaledMinSize.cy);

    setRectEx(&lpLayout->labelTipRectEx,
              lpLayout->labelDescRectEx.r.left,
              lpLayout->listBoxRectEx.r.bottom + sp,
              lpLayout->labelDescRectEx.r.left + labelTipTextSize.cx,
              lpLayout->listBoxRectEx.r.bottom + sp + labelTipTextSize.cy);

    setRectEx(&lpLayout->leftSizeGripRectEx,
              windowClientRectEx.r.left,                                       // left
              windowClientRectEx.r.bottom - ullSizeGripControlWidthAndHeight,  // top
              windowClientRectEx.r.left + ullSizeGripControlWidthAndHeight,    // right
              windowClientRectEx.r.bottom);                                    // bottom

    setRectEx(&lpLayout->buttonOkRectEx,
              lpLayout->labelDescRectEx.r.left + ullSizeGripControlWidthAndHeight,  // left
              lpLayout->labelTipRectEx.r.bottom + sp,                               // top
              lpLayout->labelDescRectEx.r.left + iButtonWidth,                      // right
              lpLayout->labelTipRectEx.r.bottom + sp + iButtonHeight);              // bottom

    setRectEx(&lpLayout->buttonCancelRectEx,
              lpLayout->listBoxRectEx.r.right - iButtonWidth,                      // left
              lpLayout->labelTipRectEx.r.bottom + sp,                              // top
              lpLayout->listBoxRectEx.r.right - ullSizeGripControlWidthAndHeight,  // right
              lpLayout->labelTipRectEx.r.bottom + sp + iButtonHeight);             // bottom

    setRectEx(&lpLayout->rightSizeGripRectEx,
              windowClientRectEx.r.right - ullSizeGripControlWidthAndHeight,   // left
              windowClientRectEx.r.bottom - ullSizeGripControlWidthAndHeight,  // top
              windowClientRectEx.r.right,                                      // right
              windowClientRectEx.r.bottom);                                    // bottom

    __attribute__((unused))
    const int dummy = 1;
}
// Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-create
static void
WindowProc_WM_CREATE(_In_ const HWND   hWnd,
                     __attribute__((unused))
                     _In_ const WPARAM wParam,
                     _In_ const LPARAM lParam)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-createstructw
    CREATESTRUCTW *lpCreateStruct = (CREATESTRUCTW *) lParam;
    struct Window *lpWin = lpCreateStruct->lpCreateParams;
    Win32SetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, lpWin, L"SetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, lpWin)");
    lpWin->layout.createStruct = *lpCreateStruct;
    lpWin->hWnd = hWnd;

    WindowLayoutInit(hWnd, lpCreateStruct);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos
    if (!SetWindowPos(hWnd,                                         // [in]           HWND hWnd
                      NULL,                                         // [in, optional] HWND hWndInsertAfter
                      // "The new position of the left side of the window, in client coordinates."
                      // Note: For top-level windows (NULL == hWndParent), "client coordinates" are actually "screen coordinates".
                      lpWin->layout.windowNonClientRectEx.r.left,   // [in]           int  X,
                      lpWin->layout.windowNonClientRectEx.r.top,    // [in]           int  Y,
                      // "The new width of the window, in pixels."
                      lpWin->layout.windowNonClientRectEx.lWidth,   // [in]           int  cx,
                      // "The new height of the window, in pixels."
                      lpWin->layout.windowNonClientRectEx.lHeight,  // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOZORDER))               // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hWnd, ...)");
    }

    // Before this function returns, the following window messages are received by wndproc: WM_PARENTNOTIFY
    lpWin->hStaticDesc =
        CreateWindowExW(
            0,  // [in]           DWORD     dwExStyle
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/static-controls
            WC_STATICW,  // [in, optional] LPCWSTR   lpClassName
            lpWin->layout.config.labelDescWStr.lpWCharArr,  // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            (DWORD) (  // [in]           DWORD     dwStyle
                WS_CHILD  // The window is a child window. A window with this style cannot have a menu bar. This style cannot be used with the WS_POPUP style.
                | WS_VISIBLE  // The window is initially visible.
                // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/static-control-styles
                | SS_LEFT  // A simple rectangle and left-aligns the text in the rectangle. The text is formatted before it is displayed.
                           // Words that extend past the end of a line are automatically wrapped to the beginning of the next left-aligned line.
                           // Words that are longer than the width of the control are truncated.
                | SS_NOPREFIX),  // Prevents interpretation of any ampersand (&) characters in the control's text as accelerator prefix characters.
                                 // These are displayed with the ampersand removed and the next character in the string underlined.
                                 // This static control style may be included with any of the defined static controls.
                                 // You can combine SS_NOPREFIX with other styles.
                                 // This can be useful when filenames or other strings that may contain an ampersand (&) must be displayed in a static control in a dialog box.
            lpWin->layout.labelDescRectEx.r.left,   // [in]           int       X
            lpWin->layout.labelDescRectEx.r.top,    // [in]           int       Y
            lpWin->layout.labelDescRectEx.lWidth,   // [in]           int       nWidth
            lpWin->layout.labelDescRectEx.lHeight,  // [in]           int       nHeight
            hWnd,  // [in, optional] HWND      hWndParent
            NULL,  // [in, optional] HMENU     hMenu
            lpCreateStruct->hInstance,  // [in, optional] HINSTANCE hInstance
            NULL);  //  [in, optional] LPVOID    lpParam

    if (NULL == lpWin->hStaticDesc)
    {
        ErrorExitWF(L"hStaticDesc: CreateWindowExW(lpClassName[%ls])", WC_STATICW);
    }

    DEBUG_LOGWF(stdout, L"INFO: hStaticDesc: %p\r\n", lpWin->hStaticDesc);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendmessage
    // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-setfont?redirectedfrom=MSDN
    // "This message does not return a value."
    SendMessage(
        lpWin->hStaticDesc,                  //  [in] HWND   hWnd,
        WM_SETFONT,                             //  [in] UINT   Msg,
        (WPARAM) lpWin->layout.hFont,  // [in] WPARAM wParam,
        // "The low-order word of lParam specifies whether the control should be redrawn immediately upon setting the font.
        //  If this parameter is TRUE, the control redraws itself."
        MAKELPARAM(                             //   [in] LPARAM lParam
            FALSE,  // [in] WORD loword
            0));    // [in] WORD hiword

    // Before this function returns, the following window messages are received by wndproc: WM_PARENTNOTIFY
    lpWin->hListBox =
        CreateWindowExW(
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
            (DWORD) (                   // [in]           DWORD     dwExStyle,
                WS_EX_CLIENTEDGE),      // "The window has a border with a sunken edge."
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/list-boxes
            WC_LISTBOXW,  // [in, optional] LPCWSTR   lpClassName,
            NULL,  // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            (DWORD) (  // [in]           DWORD     dwStyle,
                WS_CHILD  // The window is a child window. A window with this style cannot have a menu bar. This style cannot be used with the WS_POPUP style.
                | WS_VISIBLE  // The window is initially visible.
                | WS_TABSTOP    // The window is a control that can receive the keyboard focus when the user presses the TAB key.
                                // Pressing the TAB key changes the keyboard focus to the next control with the WS_TABSTOP style.
                | WS_VSCROLL  // The window has a vertical scroll bar.
                // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/list-box-styles
                | LBS_HASSTRINGS  // "Specifies that a list box contains items consisting of strings.
                                  //  The list box maintains the memory and addresses for the strings
                                  //  so that the application can use the LB_GETTEXT message to retrieve the text for a particular item.
                                  //  By default, all list boxes except owner-drawn list boxes have this style.
                                  //  You can create an owner-drawn list box either with or without this style."
                | LBS_NOINTEGRALHEIGHT),  // "Specifies that the size of the list box is exactly the size specified by the application when it created the list box.
                                          //  Normally, the system sizes a list box so that the list box does not display partial items."
            lpWin->layout.listBoxRectEx.r.left,  // [in]           int       X,
            lpWin->layout.listBoxRectEx.r.top,  // [in]           int       Y,
            lpWin->layout.listBoxRectEx.lWidth,  // [in]           int       nWidth,
            lpWin->layout.listBoxRectEx.lHeight,  // [in]           int       nHeight,
            hWnd,  // [in, optional] HWND      hWndParent,
            NULL,  // [in, optional] HMENU     hMenu,
            lpCreateStruct->hInstance,  // [in, optional] HINSTANCE hInstance
            NULL);  //  [in, optional] LPVOID    lpParam

    if (NULL == lpWin->hListBox)
    {
        ErrorExitWF(L"hListBox: CreateWindowExW(lpClassName[%ls])", WC_LISTBOXW);
    }

    DEBUG_LOGWF(stdout, L"INFO: hListBox: %p\r\n", lpWin->hListBox);

    for (size_t i = 0; i < lpWin->config.dynArr.ulSize; ++i)
    {
        const struct ConfigEntry *lpConfigEntry = lpWin->config.dynArr.lpConfigEntryArr + i;
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendmessage
        // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/lb-addstring
        // "The return value is the zero-based index of the string in the list box.
        //  If an error occurs, the return value is LB_ERR.
        //  If there is insufficient space to store the new string, the return value is LB_ERRSPACE."
        const LRESULT lResult =
            SendMessage(
                lpWin->hListBox,                                   // [in] HWND   hWnd,
                LB_ADDSTRING,                                      // [in] UINT   Msg,
                // "This parameter is not used."
                (WPARAM) NULL,                                     // [in] WPARAM wParam,
                // "A pointer to the null-terminated string that is to be added."
                (LPARAM) lpConfigEntry->usernameWStr.lpWCharArr);  // [in] LPARAM lParam

        if (LB_ERR == lResult)
        {
            ErrorExitWF(L"Failed to add listbox #%zu: LB_ERR == SendMessage(lpWin->hListBox, LB_ADDSTRING, NULL, lpConfigEntry->usernameWStr.lpWCharArr[%ls])",
                        (1 + i), lpConfigEntry->usernameWStr.lpWCharArr);
        }
        else if (LB_ERRSPACE == lResult)
        {
            ErrorExitWF(L"Failed to add listbox #%zu: LB_ERRSPACE == SendMessage(lpWin->hListBox, LB_ADDSTRING, NULL, lpConfigEntry->usernameWStr.lpWCharArr[%ls])",
                        (1 + i), lpConfigEntry->usernameWStr.lpWCharArr);
        }
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendmessage
    // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/lb-addstring
    // "The return value is the zero-based index of the string in the list box.
    //  If an error occurs, the return value is LB_ERR.
    //  If there is insufficient space to store the new string, the return value is LB_ERRSPACE."
    const LRESULT lResult =
        SendMessage(
            lpWin->hListBox,  // [in] HWND   hWnd,
            LB_SETCURSEL,     // [in] UINT   Msg,
            // "Specifies the zero-based index of the string that is selected. If this parameter is -1, the list box is set to have no selection."
            (WPARAM) 0,       // [in] WPARAM wParam,
            // "This parameter is not used."
            (LPARAM) NULL);   // [in] LPARAM lParam

    if (LB_ERR == lResult)
    {
        ErrorExitW(L"LB_ERR == SendMessage(lpWin->hListBox, LB_SETCURSEL, index:0, NULL)");
    }
/*
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-setwindowsubclass
    if (FALSE == SetWindowSubclass(lpWin->hListBox,  // [in] HWND         hWnd,
                                   ListBoxSubclassProc,  // [in] SUBCLASSPROC pfnSubclass,
                                   ID_SUBCLASS_LIST_BOX,  // [in] UINT_PTR     uIdSubclass,
                                   (DWORD_PTR) NULL))  // [in] DWORD_PTR    dwRefData
    {
        ErrorExitW(L"SetWindowSubclass(hListBox, ...)");
    }
*/
    // "This message does not return a value."
    SendMessage(
        lpWin->hListBox,                     //  [in] HWND   hWnd,
        WM_SETFONT,                             //  [in] UINT   Msg,
        (WPARAM) lpWin->layout.hFont,  // [in] WPARAM wParam,
        // "The low-order word of lParam specifies whether the control should be redrawn immediately upon setting the font.
        //  If this parameter is TRUE, the control redraws itself."
        MAKELPARAM(                             //   [in] LPARAM lParam
            FALSE,  // [in] WORD loword
            0));    // [in] WORD hiword

    // Before this function returns, the following window messages are received by wndproc: WM_PARENTNOTIFY
    lpWin->hStaticTip =
        CreateWindowExW(
            0,  // [in]           DWORD     dwExStyle,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/static-controls
                WC_STATICW,  // [in, optional] LPCWSTR   lpClassName,
            lpWin->layout.config.labelTipWStr.lpWCharArr,  // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            (DWORD) (  // [in]           DWORD     dwStyle,
                WS_CHILD  // The window is a child window. A window with this style cannot have a menu bar. This style cannot be used with the WS_POPUP style.
                | WS_VISIBLE  // The window is initially visible.
                // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/static-control-styles
                | SS_LEFT  // A simple rectangle and left-aligns the text in the rectangle. The text is formatted before it is displayed.
                           // Words that extend past the end of a line are automatically wrapped to the beginning of the next left-aligned line.
                           // Words that are longer than the width of the control are truncated.
                | SS_NOPREFIX),  // Prevents interpretation of any ampersand (&) characters in the control's text as accelerator prefix characters.
                                 // These are displayed with the ampersand removed and the next character in the string underlined.
                                 // This static control style may be included with any of the defined static controls.
                                 // You can combine SS_NOPREFIX with other styles.
                                 // This can be useful when filenames or other strings that may contain an ampersand (&) must be displayed in a static control in a dialog box.
            lpWin->layout.labelTipRectEx.r.left,  // [in]           int       X,
            lpWin->layout.labelTipRectEx.r.top,  // [in]           int       Y,
            lpWin->layout.labelTipRectEx.lWidth,  // [in]           int       nWidth,
            lpWin->layout.labelTipRectEx.lHeight,  // [in]           int       nHeight,
            hWnd,  // [in, optional] HWND      hWndParent,
            NULL,  // [in, optional] HMENU     hMenu,
            lpCreateStruct->hInstance,  // [in, optional] HINSTANCE hInstance
            NULL);  //  [in, optional] LPVOID    lpParam

    if (NULL == lpWin->hStaticTip)
    {
        ErrorExitWF(L"hStaticTip: CreateWindowExW(lpClassName[%ls])", WC_STATICW);
    }

    DEBUG_LOGWF(stdout, L"INFO: hStaticTip: %p\r\n", lpWin->hStaticTip);

    // "This message does not return a value."
    SendMessage(
        lpWin->hStaticTip,                   //  [in] HWND   hWnd,
        WM_SETFONT,                             //  [in] UINT   Msg,
        (WPARAM) lpWin->layout.hFont,  // [in] WPARAM wParam,
        // "The low-order word of lParam specifies whether the control should be redrawn immediately upon setting the font.
        //  If this parameter is TRUE, the control redraws itself."
        MAKELPARAM(                             //   [in] LPARAM lParam
            FALSE,  // [in] WORD loword
            0));    // [in] WORD hiword

    struct Win32SizeGripControlCreateParams leftSizeGripControlCreateParams = { .orientation = WIN32_SGC_BOTTOM_LEFT };
    // Before this function returns, the following window messages are received by wndproc: WM_PARENTNOTIFY
    lpWin->hLeftSizeGrip =
        CreateWindowExW(
            0,                                         // [in]           DWORD     dwExStyle,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/buttons
            WIN32_SIZE_GRIP_CONTROL_CLASS_NAMEW,       // [in, optional] LPCWSTR   lpClassName,
            NULL,                                      // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            (DWORD) (
                WS_CHILD  // The window is a child window. A window with this style cannot have a menu bar. This style cannot be used with the WS_POPUP style.
                | WS_VISIBLE),  // The window is initially visible.
            lpWin->layout.leftSizeGripRectEx.r.left,   // [in]           int       X,
            lpWin->layout.leftSizeGripRectEx.r.top,    // [in]           int       Y,
            lpWin->layout.leftSizeGripRectEx.lWidth,   // [in]           int       nWidth,
            lpWin->layout.leftSizeGripRectEx.lHeight,  // [in]           int       nHeight,
            hWnd,                                      // [in, optional] HWND      hWndParent,
            NULL,                                      // [in, optional] HMENU     hMenu,
            lpCreateStruct->hInstance,                 // [in, optional] HINSTANCE hInstance
            &leftSizeGripControlCreateParams);         // [in, optional] LPVOID    lpParam

    if (NULL == lpWin->hLeftSizeGrip)
    {
        ErrorExitWF(L"hLeftSizeGrip: CreateWindowExW(lpClassName[%ls])", WIN32_SIZE_GRIP_CONTROL_CLASS_NAMEW);
    }

    DEBUG_LOGWF(stdout, L"INFO: hLeftSizeGrip: %p\r\n", lpWin->hLeftSizeGrip);

    // Before this function returns, the following window messages are received by wndproc: WM_PARENTNOTIFY
    lpWin->hButtonOk =
        CreateWindowExW(
            0,  // [in]           DWORD     dwExStyle,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/buttons
            WC_BUTTONW,  // [in, optional] LPCWSTR   lpClassName,
            L"&OK",  // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            (DWORD) (  // [in]           DWORD     dwStyle,
                WS_CHILD  // The window is a child window. A window with this style cannot have a menu bar. This style cannot be used with the WS_POPUP style.
                | WS_VISIBLE  // The window is initially visible.
                | WS_TABSTOP    // The window is a control that can receive the keyboard focus when the user presses the TAB key.
                                // Pressing the TAB key changes the keyboard focus to the next control with the WS_TABSTOP style.
                // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/button-styles
                | BS_CENTER  // Centers text horizontally in the button rectangle.
                | BS_VCENTER  // Places text in the middle (vertically) of the button rectangle.
                | BS_DEFPUSHBUTTON  // (default push button) Creates a push button that behaves like a BS_PUSHBUTTON style button, but has a distinct appearance.
                                    // If the button is in a dialog box, the user can select the button by pressing the ENTER key,
                                    // even when the button does not have the input focus.
                                    // This style is useful for enabling the user to quickly select the most likely (default) option.
                | BS_PUSHBUTTON  // Creates a push button that posts a WM_COMMAND message to the owner window when the user selects the button.
                | BS_TEXT),  // Specifies that the button displays text.
            lpWin->layout.buttonOkRectEx.r.left,  // [in]           int       X,
            lpWin->layout.buttonOkRectEx.r.top,  // [in]           int       Y,
            lpWin->layout.buttonOkRectEx.lWidth,  // [in]           int       nWidth,
            lpWin->layout.buttonOkRectEx.lHeight,  // [in]           int       nHeight,
            hWnd,  // [in, optional] HWND      hWndParent,
            (HMENU) IDC_BUTTON_OK,  // [in, optional] HMENU     hMenu,
            lpCreateStruct->hInstance,  // [in, optional] HINSTANCE hInstance
            NULL);  //  [in, optional] LPVOID    lpParam

    if (NULL == lpWin->hButtonOk)
    {
        ErrorExitWF(L"hButtonOk: CreateWindowExW(lpClassName[%ls])", WC_BUTTONW);
    }

    DEBUG_LOGWF(stdout, L"INFO: hButtonOk: %p\r\n", lpWin->hButtonOk);

    // "This message does not return a value."
    SendMessage(
        lpWin->hButtonOk,                    //  [in] HWND   hWnd,
        WM_SETFONT,                             //  [in] UINT   Msg,
        (WPARAM) lpWin->layout.hFont,  // [in] WPARAM wParam,
        // "The low-order word of lParam specifies whether the control should be redrawn immediately upon setting the font.
        //  If this parameter is TRUE, the control redraws itself."
        MAKELPARAM(                             //   [in] LPARAM lParam
            FALSE,  // [in] WORD loword
            0));    // [in] WORD hiword

    // Before this function returns, the following window messages are received by wndproc: WM_PARENTNOTIFY
    lpWin->hButtonCancel =
        CreateWindowExW(
            0,  // [in]           DWORD     dwExStyle,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/buttons
            WC_BUTTONW,  // [in, optional] LPCWSTR   lpClassName,
            L"&Cancel",  // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            (DWORD) (  // [in]           DWORD     dwStyle,
                WS_CHILD  // The window is a child window. A window with this style cannot have a menu bar. This style cannot be used with the WS_POPUP style.
                | WS_VISIBLE  // The window is initially visible.
                | WS_TABSTOP    // The window is a control that can receive the keyboard focus when the user presses the TAB key.
                                // Pressing the TAB key changes the keyboard focus to the next control with the WS_TABSTOP style.
                // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/button-styles
                | BS_CENTER  // Centers text horizontally in the button rectangle.
                | BS_VCENTER  // Places text in the middle (vertically) of the button rectangle.
                | BS_PUSHBUTTON  // Creates a push button that posts a WM_COMMAND message to the owner window when the user selects the button.
                | BS_TEXT),  // Specifies that the button displays text.
            lpWin->layout.buttonCancelRectEx.r.left,  // [in]           int       X,
            lpWin->layout.buttonCancelRectEx.r.top,  // [in]           int       Y,
            lpWin->layout.buttonCancelRectEx.lWidth,  // [in]           int       nWidth,
            lpWin->layout.buttonCancelRectEx.lHeight,  // [in]           int       nHeight,
            hWnd,  // [in, optional] HWND      hWndParent,
            (HMENU) IDC_BUTTON_CANCEL,  // [in, optional] HMENU     hMenu,
            lpCreateStruct->hInstance,  // [in, optional] HINSTANCE hInstance
            NULL);  //  [in, optional] LPVOID    lpParam

    if (NULL == lpWin->hButtonCancel)
    {
        ErrorExitWF(L"hButtonCancel: CreateWindowExW(lpClassName[%ls])", WC_BUTTONW);
    }

    DEBUG_LOGWF(stdout, L"INFO: hButtonCancel: %p\r\n", lpWin->hButtonCancel);

    // "This message does not return a value."
    SendMessage(
        lpWin->hButtonCancel,                //  [in] HWND   hWnd,
        WM_SETFONT,                             //  [in] UINT   Msg,
        (WPARAM) lpWin->layout.hFont,  // [in] WPARAM wParam,
        // "The low-order word of lParam specifies whether the control should be redrawn immediately upon setting the font.
        //  If this parameter is TRUE, the control redraws itself."
        MAKELPARAM(                             //   [in] LPARAM lParam
            FALSE,  // [in] WORD loword
            0));    // [in] WORD hiword
/*
// TODO: Still needed?
    // Before this function returns, the following window messages are received by wndproc: WM_PARENTNOTIFY
    // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/tooltip-controls
    lpWin->hButtonCancelTooltip =
        CreateWindowExW(
            0,  // [in]           DWORD     dwExStyle,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/buttons
            TOOLTIPS_CLASS,  // [in, optional] LPCWSTR   lpClassName,
            L"Escape to cancel",  // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            (DWORD) (  // [in]           DWORD     dwStyle,
                // "The window is a pop-up window."
                WS_POPUP
                // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/tooltip-styles
                // "Prevents the system from stripping ampersand characters from a string or terminating a string at a tab character.
                //  Without this style, the system automatically strips ampersand characters and terminates a string at the first tab character.
                //  This allows an application to use the same string as both a menu item and as text in a tooltip control."
                | TTS_NOPREFIX
                // "Indicates that the tooltip control appears when the cursor is on a tool, even if the tooltip control's owner window is inactive.
                //  Without this style, the tooltip appears only when the tool's owner window is active."
                | TTS_ALWAYSTIP),  // Specifies that the button displays text.
            CW_USEDEFAULT,  // [in]           int       X,
            CW_USEDEFAULT,  // [in]           int       Y,
            CW_USEDEFAULT,  // [in]           int       nWidth,
            CW_USEDEFAULT,  // [in]           int       nHeight,
            lpWin->hButtonCancel,  // [in, optional] HWND      hWndParent,
            NULL,  // [in, optional] HMENU     hMenu,
            lpCreateStruct->hInstance,  // [in, optional] HINSTANCE hInstance
            NULL);  //  [in, optional] LPVOID    lpParam

    if (NULL == lpWin->hButtonCancelTooltip)
    {
        ErrorExitWF(L"hButtonCancelTooltip: CreateWindowExW(lpClassName[%ls])", TOOLTIPS_CLASS);
    }

    // "This message does not return a value."
    SendMessage(
        lpWin->hButtonCancelTooltip,                //  [in] HWND   hWnd,
        WM_SETFONT,                             //  [in] UINT   Msg,
        (WPARAM) lpWin->layout.hFont,  // [in] WPARAM wParam,
        // "The low-order word of lParam specifies whether the control should be redrawn immediately upon setting the font.
        //  If this parameter is TRUE, the control redraws itself."
        MAKELPARAM(                             //   [in] LPARAM lParam
            FALSE,  // [in] WORD loword
            0));    // [in] WORD hiword

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos
    if (!SetWindowPos(lpWin->hButtonCancelTooltip,                // [in]           HWND hWnd
                      HWND_TOPMOST,                               // [in, optional] HWND hWndInsertAfter
                      // "The new position of the left side of the window, in client coordinates."
                      // Note: For top-level windows (NULL == hWndParent), "client coordinates" are actually "screen coordinates".
                      0,                                          // [in]           int  X,
                      0,                                          // [in]           int  Y,
                      // "The new width of the window, in pixels."
                      0,                                          // [in]           int  cx,
                      // "The new height of the window, in pixels."
                      0,                                          // [in]           int  cy,
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE))  // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hWnd, ...)");
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/ns-commctrl-tttoolinfow
    const TOOLINFOW buttonCancelTooltipToolInfo = {
        .cbSize = (UINT) sizeof(TOOLINFOW),  // UINT      cbSize
        .uFlags = (UINT) (
            // "Indicates that the uId member is the window handle to the tool. If this flag is not set, uId is the tool's identifier."
            TTF_IDISHWND
            // "Indicates that the tooltip control should subclass the tool's window to intercept messages, such as WM_MOUSEMOVE."
            | TTF_SUBCLASS),  // UINT      uFlags
        // "Handle to the window that contains the tool."
        .hwnd = (HWND) hWnd,  // HWND      hwnd
        // "If uFlags includes the TTF_IDISHWND flag, uId must specify the window handle to the tool."
        .uId = (UINT_PTR) lpWin->hButtonCancel,  // UINT_PTR  uId
        // "If uFlags includes the TTF_IDISHWND flag, this member is ignored."
        // RECT      rect
        // "Handle to the instance that contains the string resource for the tool."
        // HINSTANCE hinst
        // "Pointer to the buffer that contains the text for the tool"
        .lpszText = (LPWSTR) L"Escape to cancel",  // LPWSTR    lpszText
        // "A 32-bit application-defined value that is associated with the tool."
        // LPARAM    lParam
        // "Reserved. Must be set to NULL."
        // void      *lpReserved
    };

    // "Returns TRUE if successful, or FALSE otherwise."
    if (FALSE == SendMessage(
            lpWin->hButtonCancelTooltip,             // [in] HWND   hWnd
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/ttm-addtool
            TTM_ADDTOOL,                             // [in] UINT   Msg
            (WPARAM) 0,                              // [in] WPARAM wParam
            (LPARAM) &buttonCancelTooltipToolInfo))  // [in] WPARAM wParam
    {
        ErrorExitW(L"SendMessage(lpWin->hButtonCancelTooltip, TTM_ADDTOOL, ...)");
    }
*/
    struct Win32SizeGripControlCreateParams rightSizeGripControlCreateParams = { .orientation = WIN32_SGC_BOTTOM_RIGHT };
    // Before this function returns, the following window messages are received by wndproc: WM_PARENTNOTIFY
    lpWin->hRightSizeGrip =
        CreateWindowExW(
            0,                                          // [in]           DWORD     dwExStyle,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/common-control-window-classes
            // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/buttons
            WIN32_SIZE_GRIP_CONTROL_CLASS_NAMEW,        // [in, optional] LPCWSTR   lpClassName,
            NULL,                                       // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            (DWORD) (
                WS_CHILD  // The window is a child window. A window with this style cannot have a menu bar. This style cannot be used with the WS_POPUP style.
                | WS_VISIBLE),  // The window is initially visible.
            lpWin->layout.rightSizeGripRectEx.r.left,   // [in]           int       X,
            lpWin->layout.rightSizeGripRectEx.r.top,    // [in]           int       Y,
            lpWin->layout.rightSizeGripRectEx.lWidth,   // [in]           int       nWidth,
            lpWin->layout.rightSizeGripRectEx.lHeight,  // [in]           int       nHeight,
            hWnd,                                       // [in, optional] HWND      hWndParent,
            NULL,                                       // [in, optional] HMENU     hMenu,
            lpCreateStruct->hInstance,                  // [in, optional] HINSTANCE hInstance
            &rightSizeGripControlCreateParams);         // [in, optional] LPVOID    lpParam

    if (NULL == lpWin->hRightSizeGrip)
    {
        ErrorExitWF(L"hRightSizeGrip: CreateWindowExW(lpClassName[%ls])", WIN32_SIZE_GRIP_CONTROL_CLASS_NAMEW);
    }

    DEBUG_LOGWF(stdout, L"INFO: hRightSizeGrip: %p\r\n", lpWin->hRightSizeGrip);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowshookexw
    lpWin->hHookLowLevelKeyboard =
        SetWindowsHookEx(WH_KEYBOARD_LL,             // [in] int       idHook
                         LowLevelKeyboardProc,       // [in] HOOKPROC  lpfn
                         lpCreateStruct->hInstance,  // [in] HINSTANCE hmod
                         (DWORD) 0);                 // [in] DWORD     dwThreadId

    if (NULL == lpWin->hHookLowLevelKeyboard)
    {
        ErrorExitW(L"SetWindowsHookEx(WH_KEYBOARD_LL, ...)");
    }
}
// Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-getminmaxinfo?redirectedfrom=MSDN
// Ref: https://stackoverflow.com/questions/5794630/setting-a-windows-minimum-and-maximum-size-using-winapi
// Note: During resize, this is sent *before* WM_SIZE.
static void
WindowProc_WM_GETMINMAXINFO(_In_ const HWND   hWnd,
                            __attribute__((unused))
                            _In_ const WPARAM wParam,
                            _In_ const LPARAM lParam)
{
    // @Nullable
    const struct Window *lpWin = Win32TryGetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, L"GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    if (NULL != lpWin)
    {
        MINMAXINFO *lpMINMAXINFO = (MINMAXINFO *) lParam;
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-minmaxinfo
        // "The minimum tracking width (x member) and the minimum tracking height (y member) of the window."
        // Ref: https://stackoverflow.com/a/22261818/257299
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsystemmetrics
        // SM_CXMINTRACK: The minimum tracking width of a window, in pixels.
        //                The user cannot drag the window frame to a size smaller than these dimensions.
        //                A window can override this value by processing the WM_GETMINMAXINFO message.
        lpMINMAXINFO->ptMinTrackSize = (POINT) {.x = lpWin->layout.windowNonClientRectEx.lWidth,
                                                .y = lpWin->layout.windowNonClientRectEx.lHeight};
    }
}
// Ref: https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command
// "Use of the wParam and lParam parameters are summarized here.
//  Message Source | wParam (high word)                | wParam (low word)              | lParam
//  Menu           | 0                                 | Menu identifier (IDM_*)        | 0
//  Accelerator    | 1                                 | Accelerator identifier (IDM_*) | 0
//  Control        | Control-defined notification code | Control identifier             | Handle to the control window"
static void
WindowProc_WM_COMMAND(_In_ const HWND   hWnd,
                      _In_ const WPARAM wParam,
                      _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, L"GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    BOOL mustClose = FALSE;
    BOOL mustCopyUsername = FALSE;
    BOOL mustCopyPassword = FALSE;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/bn-clicked
    // "wParam: The LOWORD contains the button's control identifier. The HIWORD specifies the notification code.
    //  lParam: A handle to the button."
    if (IDC_BUTTON_OK == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
    {
        __attribute__((unused))
        const HWND hButtonOk = (HWND) lParam;
        mustCopyPassword = TRUE;
    }
    else if (IDC_BUTTON_CANCEL == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
    {
        __attribute__((unused))
        const HWND hButtonCancel = (HWND) lParam;
        mustClose = TRUE;
    }
    else if (1 == HIWORD(wParam) && IDM_ACCEL_ESCAPE == LOWORD(wParam))
    {
        mustClose = TRUE;
    }
    else if (1 == HIWORD(wParam) && IDM_ACCEL_RETURN == LOWORD(wParam))
    {
        mustCopyPassword = TRUE;
    }
    else if (1 == HIWORD(wParam) && IDM_ACCEL_CTRL_C == LOWORD(wParam))
    {
        mustCopyUsername = TRUE;
    }
    //
    if (mustCopyUsername)
    {
        mustClose = FALSE;  // explicit
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendmessage
        // "In a single-selection list box, the return value is the zero-based index of the currently selected item.
        //  If there is no selection, the return value is LB_ERR."
        const LRESULT selectedIndex =
            SendMessage(lpWin->hListBox,  // [in] HWND   hWnd
                        // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/lb-getcursel
                        LB_GETCURSEL,     // [in] UINT   Msg
                        // "Not used; must be zero."
                        (WPARAM) 0,       // [in] WPARAM wParam
                        // "Not used; must be zero."
                        (LPARAM) 0);      // [in] WPARAM wParam

        // Intentional: If no selection, do nothing.
        if (LB_ERR != selectedIndex)
        {
            const struct ConfigEntry *lpConfigEntry = lpWin->config.dynArr.lpConfigEntryArr + selectedIndex;
//DEBUG_LOGWF(stdout, L"INFO: Copy username: [%ls]\r\n", lpConfigEntry->usernameWStr.lpWCharArr);
            Win32ClipboardWriteWStr(lpWin->hWnd, &lpConfigEntry->usernameWStr);
        }
    }
    //
    if (mustCopyPassword)
    {
        mustClose = TRUE;
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendmessage
        // "In a single-selection list box, the return value is the zero-based index of the currently selected item.
        //  If there is no selection, the return value is LB_ERR."
        const LRESULT selectedIndex =
            SendMessage(lpWin->hListBox,  // [in] HWND   hWnd
                        // Ref: https://learn.microsoft.com/en-us/windows/win32/controls/lb-getcursel
                        LB_GETCURSEL,     // [in] UINT   Msg
                        // "Not used; must be zero."
                        (WPARAM) 0,       // [in] WPARAM wParam
                        // "Not used; must be zero."
                        (LPARAM) 0);      // [in] WPARAM wParam

        // Intentional: If no selection, do nothing.
        if (LB_ERR != selectedIndex)
        {
            const struct ConfigEntry *lpConfigEntry = lpWin->config.dynArr.lpConfigEntryArr + selectedIndex;
//DEBUG_LOGWF(stdout, L"INFO: Copy password: [%ls]\r\n", lpConfigEntry->passwordWStr.lpWCharArr);
            Win32ClipboardWriteWStr(lpWin->hWnd, &lpConfigEntry->passwordWStr);
        }
    }
    //
    if (mustClose)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
        // "If the window was previously visible, the return value is nonzero.
        //  If the window was previously hidden, the return value is zero."
        ShowWindow(global.win.hWnd,  // [in] HWND hWnd,
                   SW_HIDE);         // [in] int  nCmdShow
    }
}
// Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-size
static void
WindowProc_WM_SIZE(__attribute__((unused))
                   _In_ const HWND   hWnd,
                   _In_ const WPARAM wParam,
                   _In_ const LPARAM lParam)
{
    _logMessage(L"WM_SIZE", wParam, lParam, _printfLParamWM_SIZE,
                L"SIZE_MAXHIDE", (WPARAM) 4, L"SIZE_MAXIMIZED", (WPARAM) 2, L"SIZE_MAXSHOW", (WPARAM) 3, L"SIZE_MINIMIZED", (WPARAM) 1, L"SIZE_RESTORED", (WPARAM) 0,
                NULL);
    const struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, L"GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    if (!lpWin->bIsInitDone)
    {
        return;
    }

    if (SIZE_MINIMIZED == wParam)
    {
        return;
    }

    // The low-order word of lParam specifies the new width of the client area.
    const DWORD dwWidth  = LOWORD(lParam);
    // The high-order word of lParam specifies the new height of the client area.
    const DWORD dwHeight = HIWORD(lParam);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos
    if (!SetWindowPos(lpWin->hListBox,                      // [in]           HWND hWnd
                      NULL,                                 // [in, optional] HWND hWndInsertAfter
                      lpWin->layout.scaledSpacing,          // [in]           int  X,
                                                            // [in]           int  Y,
                      lpWin->layout.scaledSpacing + lpWin->layout.labelDescRectEx.lHeight + lpWin->layout.scaledSpacing,
                                                            // [in]           int  cx,
                      dwWidth - lpWin->layout.scaledSpacing - lpWin->layout.scaledSpacing,
                                                            // [in]           int  cy,
                      dwHeight - lpWin->layout.scaledSpacing - lpWin->layout.buttonOkRectEx.lHeight
                          - lpWin->layout.scaledSpacing - lpWin->layout.labelTipRectEx.lHeight
                          - lpWin->layout.scaledSpacing - lpWin->layout.scaledSpacing
                          - lpWin->layout.labelDescRectEx.lHeight - lpWin->layout.scaledSpacing,
                      SWP_NOACTIVATE | SWP_NOZORDER))       // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hListBox, ...)");
    }

    if (!SetWindowPos(lpWin->hStaticTip,                            // [in]           HWND hWnd
                      NULL,                                         // [in, optional] HWND hWndInsertAfter
                      lpWin->layout.scaledSpacing,                  // [in]           int  X,
                                                                    // [in]           int  Y,
                      dwHeight - lpWin->layout.scaledSpacing - lpWin->layout.buttonOkRectEx.lHeight
                          - lpWin->layout.scaledSpacing - lpWin->layout.labelTipRectEx.lHeight,
                      0,                                            // [in]           int  cx,
                      0,                                            // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER))  // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hStaticTip, ...)");
    }

    if (!SetWindowPos(lpWin->hLeftSizeGrip,                                 // [in]           HWND hWnd
                      NULL,                                                 // [in, optional] HWND hWndInsertAfter
                      0,                                                    // [in]           int  X
                      dwHeight - lpWin->layout.leftSizeGripRectEx.lHeight,  // [in]           int  Y
                      0,                                                    // [in]           int  cx
                      0,                                                    // [in]           int  cy
                      SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER))          // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hLeftSizeGrip, ...)");
    }

    if (!SetWindowPos(lpWin->hButtonOk,                             // [in]           HWND hWnd
                      NULL,                                         // [in, optional] HWND hWndInsertAfter
                                                                    // [in]           int  X
                      lpWin->layout.scaledSpacing + lpWin->layout.rightSizeGripRectEx.lWidth,
                                                                    // [in]           int  Y
                      dwHeight - lpWin->layout.scaledSpacing - lpWin->layout.buttonOkRectEx.lHeight,
                      0,                                            // [in]           int  cx
                      0,                                            // [in]           int  cy
                      SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER))  // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hButtonOk, ...)");
    }

    if (!SetWindowPos(lpWin->hButtonCancel,                         // [in]           HWND hWnd
                      NULL,                                         // [in, optional] HWND hWndInsertAfter
                                                                    // [in]           int  X
                      dwWidth - lpWin->layout.scaledSpacing - lpWin->layout.rightSizeGripRectEx.lWidth - lpWin->layout.buttonCancelRectEx.lWidth,
                                                                    // [in]           int  Y
                      dwHeight - lpWin->layout.scaledSpacing - lpWin->layout.buttonCancelRectEx.lHeight,
                      0,                                            // [in]           int  cx
                      0,                                            // [in]           int  cy
                      SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER))  // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hButtonCancel, ...)");
    }

    if (!SetWindowPos(lpWin->hRightSizeGrip,                                // [in]           HWND hWnd
                      NULL,                                                 // [in, optional] HWND hWndInsertAfter
                      dwWidth - lpWin->layout.rightSizeGripRectEx.lWidth,   // [in]           int  X
                      dwHeight - lpWin->layout.rightSizeGripRectEx.lWidth,  // [in]           int  Y
                      0,                                                    // [in]           int  cx
                      0,                                                    // [in]           int  cy
                      SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER))          // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hRightSizeGrip, ...)");
    }
}
static void
WindowProc_WM_SETFOCUS(_In_ const HWND   hWnd,
                       __attribute__((unused))
                       _In_ const WPARAM wParam,
                       __attribute__((unused))
                       _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, L"GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    Win32SetFocus(lpWin->hListBox, L"SetFocus(hListBox)");
}
// Ref: https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged
static void
WindowProc_WM_DPICHANGED(_In_ const HWND   hWnd,
                         _In_ const WPARAM wParam,
                         _In_ const LPARAM lParam)
{
    __attribute__((unused))
    const WORD xDpi = LOWORD(wParam);
    __attribute__((unused))
    const WORD yDpi = HIWORD(wParam);
    // "The values of the X-axis and the Y-axis are identical for Windows apps."
    assert(xDpi == yDpi);

    // "A pointer to a RECT structure that provides a suggested size and position of the current window scaled for the new DPI.
    //  The expectation is that apps will reposition and resize windows based on the suggestions provided by lParam when handling this message."
    __attribute__((unused))
    const RECT *lpRect = (RECT *) lParam;
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, L"GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");

    // TODO: If window is dragged between monitors with diff DPI, this will not work!  This func always centers on main monitor.
    // Ref: https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
    // "Common Pitfalls (Win32): Not using the suggested rectangle that is provided in WM_DPICHANGED"
    WindowLayoutInit(lpWin->hWnd, &lpWin->layout.createStruct);
// TODO: Compare lpRect and lpWin->layout.windowNonClientRectEx

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos
    if (!SetWindowPos(lpWin->hWnd,                                  // [in]           HWND hWnd
                      NULL,                                         // [in, optional] HWND hWndInsertAfter
                      // "The new position of the left side of the window, in client coordinates."
                      // Note: For top-level windows (NULL == hWndParent), "client coordinates" are actually "screen coordinates".
                      lpWin->layout.windowNonClientRectEx.r.left,   // [in]           int  X,
                      lpWin->layout.windowNonClientRectEx.r.top,    // [in]           int  Y,
                      // "The new width of the window, in pixels."
                      lpWin->layout.windowNonClientRectEx.lWidth,   // [in]           int  cx,
                      // "The new height of the window, in pixels."
                      lpWin->layout.windowNonClientRectEx.lHeight,  // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOZORDER))               // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hWnd, ...)");
    }

    if (!SetWindowPos(lpWin->hStaticDesc,                     // [in]           HWND hWnd
                      NULL,                                   // [in, optional] HWND hWndInsertAfter
                      lpWin->layout.labelDescRectEx.r.left,   // [in]           int  X,
                      lpWin->layout.labelDescRectEx.r.top,    // [in]           int  Y,
                      lpWin->layout.labelDescRectEx.lWidth,   // [in]           int  cx,
                      lpWin->layout.labelDescRectEx.lHeight,  // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOZORDER))         // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hStaticDesc, ...)");
    }

    if (!SetWindowPos(lpWin->hListBox,                      // [in]           HWND hWnd
                      NULL,                                 // [in, optional] HWND hWndInsertAfter
                      lpWin->layout.listBoxRectEx.r.left,   // [in]           int  X,
                      lpWin->layout.listBoxRectEx.r.top,    // [in]           int  Y,
                      lpWin->layout.listBoxRectEx.lWidth,   // [in]           int  cx,
                      lpWin->layout.listBoxRectEx.lHeight,  // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOZORDER))       // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hListBox, ...)");
    }

    if (!SetWindowPos(lpWin->hStaticTip,                     // [in]           HWND hWnd
                      NULL,                                  // [in, optional] HWND hWndInsertAfter
                      lpWin->layout.labelTipRectEx.r.left,   // [in]           int  X,
                      lpWin->layout.labelTipRectEx.r.top,    // [in]           int  Y,
                      lpWin->layout.labelTipRectEx.lWidth,   // [in]           int  cx,
                      lpWin->layout.labelTipRectEx.lHeight,  // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOZORDER))        // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hStaticTip, ...)");
    }

    if (!SetWindowPos(lpWin->hLeftSizeGrip,                      // [in]           HWND hWnd
                      NULL,                                      // [in, optional] HWND hWndInsertAfter
                      lpWin->layout.leftSizeGripRectEx.r.left,   // [in]           int  X,
                      lpWin->layout.leftSizeGripRectEx.r.top,    // [in]           int  Y,
                      lpWin->layout.leftSizeGripRectEx.lWidth,   // [in]           int  cx,
                      lpWin->layout.leftSizeGripRectEx.lHeight,  // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOZORDER))            // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hLeftSizeGrip, ...)");
    }

    if (!SetWindowPos(lpWin->hButtonOk,                      // [in]           HWND hWnd
                      NULL,                                  // [in, optional] HWND hWndInsertAfter
                      lpWin->layout.buttonOkRectEx.r.left,   // [in]           int  X,
                      lpWin->layout.buttonOkRectEx.r.top,    // [in]           int  Y,
                      lpWin->layout.buttonOkRectEx.lWidth,   // [in]           int  cx,
                      lpWin->layout.buttonOkRectEx.lHeight,  // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOZORDER))        // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hButtonOk, ...)");
    }

    if (!SetWindowPos(lpWin->hButtonCancel,                      // [in]           HWND hWnd
                      NULL,                                      // [in, optional] HWND hWndInsertAfter
                      lpWin->layout.buttonCancelRectEx.r.left,   // [in]           int  X,
                      lpWin->layout.buttonCancelRectEx.r.top,    // [in]           int  Y,
                      lpWin->layout.buttonCancelRectEx.lWidth,   // [in]           int  cx,
                      lpWin->layout.buttonCancelRectEx.lHeight,  // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOZORDER))            // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hButtonCancel, ...)");
    }

    if (!SetWindowPos(lpWin->hRightSizeGrip,                      // [in]           HWND hWnd
                      NULL,                                       // [in, optional] HWND hWndInsertAfter
                      lpWin->layout.rightSizeGripRectEx.r.left,   // [in]           int  X,
                      lpWin->layout.rightSizeGripRectEx.r.top,    // [in]           int  Y,
                      lpWin->layout.rightSizeGripRectEx.lWidth,   // [in]           int  cx,
                      lpWin->layout.rightSizeGripRectEx.lHeight,  // [in]           int  cy,
                      SWP_NOACTIVATE | SWP_NOZORDER))             // [in]           UINT uFlags
    {
        ErrorExitW(L"SetWindowPos(hRightSizeGrip, ...)");
    }
}
// Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-close
static void
WindowProc_WM_CLOSE(_In_ const HWND   hWnd,
                    // "This parameter is not used."
                    _In_ const WPARAM wParam,
                    // "This parameter is not used."
                    _In_ const LPARAM lParam)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox
    const int result = MessageBoxW(hWnd,  // [in, optional] HWND    hWnd,
                                   L"Exit?",  // [in, optional] LPCTSTR lpText,
                                   global.wndClassExW.lpszClassName,  // [in, optional] LPCTSTR lpCaption,
                                   MB_OKCANCEL | MB_ICONWARNING | MB_DEFBUTTON2);  // [in]           UINT    uType
    if (IDOK == result)
    {
        // "By default, the DefWindowProc function calls the DestroyWindow function to destroy the window."
        DefWindowProc(hWnd, WM_CLOSE, wParam, lParam);
        return;
    }
}
// Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
LRESULT CALLBACK
WindowProc(_In_ const HWND   hWnd,
           _In_ const UINT   uMsg,
           _In_ const WPARAM wParam,
           _In_ const LPARAM lParam)
{
//    DEBUG_LOGWF(stdout, L"INFO: WindowProc(HWND hWnd[%p], UINT uMsg[%u/%ls], WPARAM wParam[%llu]hi:%lu,lo:%lu, LPARAM lParam[%lld])\n",
//                hWnd, uMsg, Win32MsgToText(uMsg), wParam, HIWORD(wParam), LOWORD(wParam), lParam);
    switch (uMsg)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-create
        case WM_CREATE:
        {
            WindowProc_WM_CREATE(hWnd, wParam, lParam);
            // "If an application processes this message, it should return zero to continue creation of the window."
            return 0;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-getminmaxinfo?redirectedfrom=MSDN
        // Ref: https://stackoverflow.com/questions/5794630/setting-a-windows-minimum-and-maximum-size-using-winapi
        // Note: During resize, this is sent *before* WM_SIZE.
        case WM_GETMINMAXINFO:
        {
            WindowProc_WM_GETMINMAXINFO(hWnd, wParam, lParam);
            // "If an application processes this message, it should return zero."
            return 0;
        }
        case WM_COMMAND:
        {
            WindowProc_WM_COMMAND(hWnd, wParam, lParam);
            // "If an application processes this message, it should return zero."
            return 0;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-size
        case WM_SIZE:
        {
            WindowProc_WM_SIZE(hWnd, wParam, lParam);
            // "If an application processes this message, it should return zero."
            return 0;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-setfocus
        // "Sent to a window after it has gained the keyboard focus."
        case WM_SETFOCUS:
        {
            WindowProc_WM_SETFOCUS(hWnd, wParam, lParam);
            // "An application should return zero if it processes this message."
            return 0;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged
        case WM_DPICHANGED:
        {
            WindowProc_WM_DPICHANGED(hWnd, wParam, lParam);
            // "If an application processes this message, it should return zero."
            return 0;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-close
        // "By default, the DefWindowProc function calls the DestroyWindow function to destroy the window."
        case WM_CLOSE:
        {
            WindowProc_WM_CLOSE(hWnd, wParam, lParam);
            // "If an application processes this message, it should return zero."
            return 0;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-destroy
        case WM_DESTROY:
        {
            PostQuitMessage(0);  // [in] int nExitCode
            // "If an application processes this message, it should return zero."
            return 0;
        }
    }
//    DEBUG_LOGW(stdout, L"DefWindowProc(...)\n");
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-defwindowprocw
    const LRESULT x = DefWindowProc(hWnd, uMsg, wParam, lParam);
    return x;
}
static void
ShowHelpThenExit(_In_opt_ const wchar_t *lpszNullableErrorMsgFmt, ...)
{
    if (NULL != lpszNullableErrorMsgFmt)
    {
        puts("\nError: ");

        // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
        va_list ap;
        va_start(ap, lpszNullableErrorMsgFmt);
        // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vprintf-vprintf-l-vwprintf-vwprintf-l?view=msvc-170
        vwprintf(lpszNullableErrorMsgFmt, ap);
        va_end(ap);

        puts("\n");
    }

    printf("\n");
    printf("Usage: %ls CONFIG_FILE_PATH [/?] [-h] [-help] [--help]\n", __wargv[0]);
    printf("Passport: The Password Helper\n");
    printf("\n");
    printf("Required Arguments:\n");
    printf("    CONFIG_FILE_PATH: path to config file\n");
    printf("        Example: \"C:\\src\\path-to-my-config-file.txt\"\n");
    printf("\n");
    printf("        Config file format:\n");
    printf("\n");
    printf("            Line format: <shortcut-key>|<send-keys-text>\n");
    printf("            <shortcut-key> format: {L/RCtrl+}{L/RShift+}{L/RAlt+}<virtual-key-code>\n");
    printf("\n");
    printf("            ... where {LCtrl+} and {RCtrl+} are optional left/right Control key indicators,\n");
    printf("                e.g., LCtrl+0x70 for LCtrl+F1 or RCtrl+0x71 for RCtrl+F2\n");
    printf("\n");
    printf("            ... where {LShift+} and {RShift+} are optional left/right Shift key indicators,\n");
    printf("                e.g., LShift+0x70 for LShift+F1 or RShift+0x71 for RShift+F2\n");
    printf("\n");
    printf("            ... where {LAlt+} and {RAlt+} are optional left/right Alt key indicators,\n");
    printf("                e.g., LAlt+0x70 for LAlt+F1 or RAlt+0x71 for RAlt+F2\n");
    printf("\n");
    printf("            ... where L/RCtrl, L/RShift, L/RAlt modifiers may be combined,\n");
    printf("                e.g., LCtrl+LShift+LAlt+0x70 for LCtrl+LShift+LAlt+F1\n");
    printf("\n");
    printf("            ... where <virtual-key-code> is hexidecimal code (with 0x prefix) for Win32 virtual-key code,\n");
    printf("                e.g., 0x70 for F1\n");
    printf("                Read more here: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes\n");
    printf("\n");
    printf("            Whitespace is ignored, except in <send-keys-text>.\n");
    printf("            Empty lines are ignored.\n");
    printf("            If first character is '#', then entire line is treated as a comment and ignored.\n");
    printf("\n");
    printf("            Example(1): # This is a comment.\n");
    printf("            Example(2): LCtrl+LShift+LAlt+0x70|username\n");
    printf("                        ... will send input 'username'  for keyboard shortcut: LCtrl+LShift+LAlt+F1\n");
    printf("            Example(3): LCtrl+LShift+LAlt+0x71|P*assw0rd\n");
    printf("                        ... will send input 'P*assw0rd' for keyboard shortcut: LCtrl+LShift+LAlt+F2\n");
    printf("\n");
    printf("Optional Arguments:\n");
    printf("    /? or -h or -help or --help\n");
    printf("        Show this help page\n");
    printf("\n");

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
    ExitProcess(1);
}
static void
ParseCommandLineArgs(_Out_ wchar_t **lppConfigFilePathWCharArr)
{
    assert(NULL != lppConfigFilePathWCharArr);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv?view=msvc-170
    if (1 == __argc)
    {
        ShowHelpThenExit(L"Missing argument: CONFIG_FILE_PATH");
    }

    // Intentional: Skip 0 == i which is path to executable.
    for (int i = 1; i < __argc; ++i)
    {
        if (0 == wcscmp(L"/?", __wargv[i]) || 0 == wcscmp(L"-h", __wargv[i]) || 0 == wcscmp(L"-help", __wargv[i]) || 0 == wcscmp(L"--help", __wargv[i]))
        {
            ShowHelpThenExit(NULL);
        }
    }

    if (__argc > 2)
    {
        ShowHelpThenExit(L"Too many arguments: Expected exactly one (CONFIG_FILE_PATH), but found %d", (__argc - 1));
    }

    const DWORD dwAttr = GetFileAttributes(__wargv[1]);  // [in] LPCWSTR lpFileName
    if (INVALID_FILE_ATTRIBUTES == dwAttr)
    {
        ErrorExitWF(L"GetFileAttributes(%ls)", __wargv[1]);
    }

    if (FILE_ATTRIBUTE_DIRECTORY & dwAttr)
    {
        ErrorExitWF(L"CONFIG_FILE_PATH is a directory: [%ls]", __wargv[1]);
    }

    *lppConfigFilePathWCharArr = __wargv[1];
}
// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI
wWinMain(HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
         __attribute__((unused))
         HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
         __attribute__((unused))
         PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
         __attribute__((unused))
         int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/ns-commctrl-initcommoncontrolsex
    const INITCOMMONCONTROLSEX initCommonControlsEx = {
        // "The size of the structure, in bytes."
        .dwSize = (DWORD) sizeof(INITCOMMONCONTROLSEX),
        // "The set of bit flags that indicate which common control classes will be loaded from the DLL."
        // "Load tab and tooltip control classes."
        .dwICC  = ICC_TAB_CLASSES,
    };

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-initcommoncontrolsex
    if (FALSE == InitCommonControlsEx(&initCommonControlsEx))  // [in] const INITCOMMONCONTROLSEX *picce
    {
        ErrorExitW(L"InitCommonControlsEx");
    }

    Win32SizeGripControlInit(hInstance);

    wchar_t *lpConfigFilePathWCharArr = NULL;
    ParseCommandLineArgs(&lpConfigFilePathWCharArr);

    struct Config config = {0};
    ConfigParseFile(lpConfigFilePathWCharArr,  // _In_  const wchar_t *lpConfigFilePathWCharArr,
                    CP_UTF8,                   // _In_  const UINT     codePage,  // Ex: CP_UTF8
                    &config);                  // _Out_ struct Config *lpConfig);

    global.win.config = config;
    global.win.bIsInitDone = FALSE;
    global.win.layout = (struct Layout) {
        .config = {
            .fontPointSize = 12,
            .fontFaceNameWStr = WSTR_FROM_LITERAL(L"Consolas"),
            .spacing = 5,
            .listBoxMinSize = {.cx = 128, .cy = 256},
            .labelDescWStr = WSTR_FROM_LITERAL(L"Select password to copy to clipboard:"),
            .labelTipWStr = WSTR_FROM_LITERAL(L"Ctrl+C to copy username to clipboard"),
        },
    };
    global.win.layout.dpi = WIN32_DPI_INIT;

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createacceleratortablew
    global.win.hAccel =
        CreateAcceleratorTableW(
            (ACCEL *) ACCEL_ARR,                        // [in] LPACCEL paccel
            sizeof(ACCEL_ARR) / sizeof(ACCEL_ARR[0]));  // [in] int     cAccel

    if (NULL == global.win.hAccel)
    {
        ErrorExitW(L"CreateAcceleratorTableW");
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadcursorw
    const HCURSOR hCursor =
        LoadCursorW(
            // "To use one of the predefined cursors, the application must set the hInstance parameter to NULL and the lpCursorName parameter to one the following values."
            NULL,        // [in, optional] HINSTANCE hInstance
            IDC_ARROW);  // [in]           LPCWSTR   lpCursorName

    if (NULL == hCursor)
    {
        ErrorExitW(L"LoadCursorW(hInstance, IDC_ARROW)");
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexw
    global.wndClassExW = (WNDCLASSEXW) {
        // "The size, in bytes, of this structure. Set this member to sizeof(WNDCLASSEX). Be sure to set this member before calling the GetClassInfoEx function."
        .cbSize = (UINT) sizeof(WNDCLASSEXW),
        // "The class style(s). This member can be any combination of the Class Styles."
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-class-styles
        .style = (UINT) CS_DROPSHADOW,
        // "A pointer to the window procedure. You must use the CallWindowProc function to call the window procedure."
        .lpfnWndProc = (WNDPROC) WindowProc,
        // "The number of extra bytes to allocate following the window-class structure. The system initializes the bytes to zero."
        .cbClsExtra = (int) 0,
        // "The number of extra bytes to allocate following the window instance. The system initializes the bytes to zero."
        // Ref: https://stackoverflow.com/a/65876605/257299
        .cbWndExtra = (int) 1 * sizeof(void *),
        // "A handle to the instance that contains the window procedure for the class."
        // Ref: https://stackoverflow.com/questions/20140117/why-does-createwindow-take-a-hinstance-as-an-argument-if-was-already-provided
        // Ref: https://devblogs.microsoft.com/oldnewthing/20050418-59/?p=35873
        .hInstance = hInstance,
        // "A handle to the class icon. This member must be a handle to an icon resource. If this member is NULL, the system provides a default icon."
        .hIcon = (HICON) 0,
        // "A handle to the class cursor. This member must be a handle to a cursor resource.
        //  If this member is NULL, an application must explicitly set the cursor shape whenever the mouse moves into the application's window.
        .hCursor = hCursor,
        // "A handle to the class background brush. This member can be a handle to the physical brush to be used for painting the background, or it can be a color value.
        //  A color value must be one of the following standard system colors (the value 1 must be added to the chosen color)."
        .hbrBackground = (HBRUSH) (1 + COLOR_BTNFACE),
        // "Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
        //  If you use an integer to identify the menu, use the MAKEINTRESOURCE macro. If this member is NULL, windows belonging to this class have no default menu."
        .lpszMenuName = (LPCWSTR) 0,
        // "A pointer to a null-terminated string or is an atom. If this parameter is an atom,
        //  it must be a class atom created by a previous call to the RegisterClass or RegisterClassEx function.
        //  The atom must be in the low-order word of lpszClassName; the high-order word must be zero.
        //  If lpszClassName is a string, it specifies the window class name. The class name can be any name registered with RegisterClass or RegisterClassEx,
        //  or any of the predefined control-class names.
        //  The maximum length for lpszClassName is 256. If lpszClassName is greater than the maximum length, the RegisterClassEx function will fail."
        .lpszClassName = (LPCWSTR) L"Password Helper",
        // "A handle to a small icon that is associated with the window class.
        //  If this member is NULL, the system searches the icon resource specified by the hIcon member for an icon of the appropriate size to use as the small icon."
        .hIconSm = (HICON) NULL,
    };

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
    global.registerClassExAtom = RegisterClassExW(&global.wndClassExW);
    if (0 == global.registerClassExAtom)
    {
        ErrorExitW(L"RegisterClassW");
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
    // Before this function returns, the following window messages are received by wndproc: WM_GETMINMAXINFO, WM_NCCREATE, WM_NCCALCSIZE, WM_CREATE
    const HWND hWnd =
        CreateWindowExW(
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
            (DWORD) (                   // [in]           DWORD     dwExStyle,
                WS_EX_APPWINDOW         // "Forces a top-level window onto the taskbar when the window is visible."
                | WS_EX_WINDOWEDGE      // "The window has a border with a raised edge."
                | WS_EX_CLIENTEDGE      // "The window has a border with a sunken edge."
                | WS_EX_TOPMOST),       // "The window should be placed above all non-topmost windows and should stay above them, even when the window is deactivated."
            global.wndClassExW.lpszClassName,  // [in, optional] LPCWSTR   lpClassName,
            // caption / title bar text
            global.wndClassExW.lpszClassName,  // [in, optional] LPCWSTR   lpWindowName,
            // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
            (DWORD) (                   // [in]           DWORD     dwStyle,
                WS_OVERLAPPED           // "The window is an overlapped window. An overlapped window has a title bar and a border. Same as the WS_TILED style."
                | WS_CAPTION            // "The window has a title bar (includes the WS_BORDER style)."
                | WS_SYSMENU            // "The window has a window menu on its title bar. The WS_CAPTION style must also be specified."
                | WS_THICKFRAME         // "The window has a sizing border. Same as the WS_SIZEBOX style."
                | WS_MINIMIZEBOX        // "The window has a minimize button. Cannot be combined with the WS_EX_CONTEXTHELP style.
                                        //  The WS_SYSMENU style must also be specified."
                | WS_MAXIMIZEBOX        // "The window has a maximize button. Cannot be combined with the WS_EX_CONTEXTHELP style.
                                        //  The WS_SYSMENU style must also be specified."
//                | WS_VISIBLE            // "The window is initially visible."
                ),
            // "If x is set to CW_USEDEFAULT, the system selects the default position for the window's upper-left corner and ignores the y parameter."
            CW_USEDEFAULT,              // [in]           int       X
            // "If the y parameter is CW_USEDEFAULT, then the window manager calls ShowWindow with the SW_SHOW flag after the window has been created.
            //  If the y parameter is some other value, then the window manager calls ShowWindow with that value as the nCmdShow parameter."
            CW_USEDEFAULT,              // [in]           int       Y
            // "If nWidth is CW_USEDEFAULT, the system selects a default width and height for the window"
            CW_USEDEFAULT,              // [in]           int       nWidth
            // "If the nWidth parameter is set to CW_USEDEFAULT, the system ignores nHeight."
            0,                          // [in]           int       nHeight
            NULL,                       // [in, optional] HWND      hWndParent,
            NULL,                       // [in, optional] HMENU     hMenu,
            hInstance,                  // [in, optional] HINSTANCE hInstance,
            &global.win);               // [in, optional] LPVOID    lpParam

    if (NULL == hWnd)
    {
        ErrorExitWF(L"hWnd: CreateWindowExW(lpClassName[%ls])", global.wndClassExW.lpszClassName);
    }

    global.win.bIsInitDone = TRUE;

    Win32SetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, &global.win, L"SetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, &global.win)");

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msg
    MSG msg = {0};
    while (TRUE)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessagew
        const BOOL bRet = GetMessageW(&msg,  // [out]          LPMSG lpMsg
                                      NULL,  // [in, optional] HWND  hWnd
                                      0,     // [in]           UINT  wMsgFilterMin
                                      0);    // [in]           UINT  wMsgFilterMax
        if (-1 == bRet)
        {
            ErrorExitW(L"GetMessage");
        }

        DEBUG_LOGWF(stdout, L"INFO: GetMessageW(msg{HWND hwnd[%p], UINT message[%u/%ls], WPARAM wParam[%llu], LPARAM lParam[%lld], DWORD time[%u], POINT pt{LONG x[%d], LONG y[%d]}, ...)\n",
                    msg.hwnd, msg.message, Win32MsgToText(msg.message), msg.wParam, msg.lParam, msg.time, msg.pt.x, msg.pt.y);

        if (FALSE == bRet)
        {
            break;  // WM_QUIT received
        }

        // Ref: https://stackoverflow.com/questions/29276275/should-i-call-isdialogmessage-before-translateaccelerator
        // TL;DR: Must call TranslateAcceleratorW() before IsDialogMessageW()
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translateacceleratorw
        // "If the function fails, the return value is zero."  Ignore this part(!): "To get extended error information, call GetLastError."
        if (0 == TranslateAcceleratorW(hWnd,        // [in] HWND   hWnd
                                       global.win.hAccel,  // [in] HACCEL hAccTable
                                       &msg))       // [in] LPMSG  lpMsg
        {
            // Ref: https://stackoverflow.com/a/3299290/257299
            // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-isdialogmessagew
            // "If the message has not been processed, the return value is zero."
            if (0 == IsDialogMessageW(hWnd,   // [in] HWND  hDlg,
                                      &msg))  // [in] LPMSG lpMsg
            {
                // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
                __attribute__((unused))
                const BOOL bRet2 = TranslateMessage(&msg);  // [in] const MSG *lpMsg
                if (FALSE == bRet2)
                {
                    #define DEBUG_BREAKPOINT do { __attribute__((unused)) int dummy = 1; } while (0)
                    DEBUG_BREAKPOINT;
                }
                // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage
                // "The return value specifies the value returned by the window procedure.
                //  Although its meaning depends on the message being dispatched, the return value generally is ignored."
                __attribute__((unused))
                const LRESULT lResult = DispatchMessageW(&msg);  // [in] const MSG *lpMsg
                DEBUG_BREAKPOINT;
            }
            DEBUG_BREAKPOINT;
        }
        DEBUG_BREAKPOINT;
    }
    // Return the exit code to the system from PostQuitMessage()
    return msg.wParam;
}

