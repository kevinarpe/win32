#include "win32_winver.h"
#include "error_exit.h"
#include <windows.h>
#include <assert.h>
#include <wchar.h>
#include <stdio.h>

const SHORT SHORT_MOST_SIGNIFICANT_BIT = 0x8000;

enum EKeyModifier
{
    SHIFT_LEFT  = 1,
    SHIFT_RIGHT = 2,

    CTRL_LEFT   = 4,
    CTRL_RIGHT  = 8,

    ALT_LEFT    = 16,
    ALT_RIGHT   = 32,
};

enum EKeyModifier eKeyModifiers = 0;

// Ref: https://docs.microsoft.com/en-us/windows/win32/winmsg/lowlevelkeyboardproc
// Ref(_In_): https://docs.microsoft.com/en-us/cpp/code-quality/understanding-sal
LRESULT CALLBACK LowLevelKeyboardProc(_In_ int    nCode,
                                      _In_ WPARAM wParam,
                                      _In_ LPARAM lParam)
{
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-kbdllhookstruct
    const KBDLLHOOKSTRUCT* info = (KBDLLHOOKSTRUCT*) lParam;
    const BOOL bIsInjected = (0 != (info->flags & LLKHF_INJECTED));
    if (HC_ACTION == nCode && !bIsInjected)
    {
        const char *lpWParamDesc = "<unknown>";
        switch (wParam)
        {
            case WM_SYSKEYDOWN:
            {
                lpWParamDesc = "WM_SYSKEYDOWN";
                break;
            }
            case WM_SYSKEYUP:
            {
                lpWParamDesc = "WM_SYSKEYUP";
                break;
            }
            // Ref: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
            case WM_KEYDOWN:
            {
                lpWParamDesc = "WM_KEYDOWN";
                break;
            }
            // Ref: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keyup
            case WM_KEYUP:
            {
                lpWParamDesc = "WM_KEYUP";
                break;
            }
        }

        const BOOL bIsExtended = (0 != (info->flags & LLKHF_EXTENDED));
        const BOOL bIsAltDown = (0 != (info->flags & LLKHF_ALTDOWN));
        const BOOL bIsKeyUp = (0 != (info->flags & LLKHF_UP));

        switch (info->vkCode)
        {
            case VK_LSHIFT:
            {
                if (bIsKeyUp)
                {
                    eKeyModifiers &= ~SHIFT_LEFT;
                }
                else
                {
                    eKeyModifiers |= SHIFT_LEFT;
                }
                break;
            }
            case VK_RSHIFT:
            {
                if (bIsKeyUp)
                {
                    eKeyModifiers &= ~SHIFT_RIGHT;
                }
                else
                {
                    eKeyModifiers |= SHIFT_RIGHT;
                }
                break;
            }
            case VK_LCONTROL:
            {
                if (bIsKeyUp)
                {
                    eKeyModifiers &= ~CTRL_LEFT;
                }
                else
                {
                    eKeyModifiers |= CTRL_LEFT;
                }
                break;
            }
            case VK_RCONTROL:
            {
                if (bIsKeyUp)
                {
                    eKeyModifiers &= ~CTRL_RIGHT;
                }
                else
                {
                    eKeyModifiers |= CTRL_RIGHT;
                }
                break;
            }
            case VK_LMENU:
            {
                if (bIsKeyUp)
                {
                    eKeyModifiers &= ~ALT_LEFT;
                }
                else
                {
                    eKeyModifiers |= ALT_LEFT;
                }
                break;
            }
            case VK_RMENU:
            {
                if (bIsKeyUp)
                {
                    eKeyModifiers &= ~ALT_RIGHT;
                }
                else
                {
                    eKeyModifiers |= ALT_RIGHT;
                }
                break;
            }
            default:
            {
            }
        }

        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getasynckeystate
        const SHORT sShiftKeyState = GetAsyncKeyState(VK_SHIFT);
        const BOOL  bIsShiftKeyDown = (sShiftKeyState & SHORT_MOST_SIGNIFICANT_BIT) ? TRUE : FALSE;

        const SHORT sLShiftKeyState = GetAsyncKeyState(VK_LSHIFT);
        const BOOL  bIsLShiftKeyDown = (sLShiftKeyState & SHORT_MOST_SIGNIFICANT_BIT) ? TRUE : FALSE;

        const SHORT sRShiftKeyState = GetAsyncKeyState(VK_RSHIFT);
        const BOOL  bIsRShiftKeyDown = (sRShiftKeyState & SHORT_MOST_SIGNIFICANT_BIT) ? TRUE : FALSE;

        const SHORT sCtrlKeyState = GetAsyncKeyState(VK_CONTROL);
        const BOOL  bIsCtrlKeyDown = (sCtrlKeyState & SHORT_MOST_SIGNIFICANT_BIT) ? TRUE : FALSE;

        const SHORT sLCtrlKeyState = GetAsyncKeyState(VK_LCONTROL);
        const BOOL  bIsLCtrlKeyDown = (sLCtrlKeyState & SHORT_MOST_SIGNIFICANT_BIT) ? TRUE : FALSE;

        const SHORT sRCtrlKeyState = GetAsyncKeyState(VK_RCONTROL);
        const BOOL  bIsRCtrlKeyDown = (sRCtrlKeyState & SHORT_MOST_SIGNIFICANT_BIT) ? TRUE : FALSE;

        const SHORT sAltKeyState = GetAsyncKeyState(VK_MENU);
        const BOOL  bIsAltKeyDown = (sAltKeyState & SHORT_MOST_SIGNIFICANT_BIT) ? TRUE : FALSE;

        const SHORT sLAltKeyState = GetAsyncKeyState(VK_LMENU);
        const BOOL  bIsLAltKeyDown = (sLAltKeyState & SHORT_MOST_SIGNIFICANT_BIT) ? TRUE : FALSE;

        const SHORT sRAltKeyState = GetAsyncKeyState(VK_RMENU);
        const BOOL  bIsRAltKeyDown = (sRAltKeyState & SHORT_MOST_SIGNIFICANT_BIT) ? TRUE : FALSE;

        const DWORD dwThreadId = GetCurrentThreadId();

        printf("dwThreadId: %d, eKeyModifiers: %d, wParam: %s, vkCode: 0x%X, IsExtended? %d, IsAltDown? %d, IsKeyUp? %d, Shift? %d, LShift? %d, RShift? %d, Ctrl? %d, LCtrl? %d, RCtrl? %d, Alt? %d, LAlt? %d, RAlt? %d\r\n",
               dwThreadId, eKeyModifiers, lpWParamDesc, info->vkCode, bIsExtended, bIsAltDown, bIsKeyUp,
               bIsShiftKeyDown, bIsLShiftKeyDown, bIsRShiftKeyDown,
               bIsCtrlKeyDown, bIsLCtrlKeyDown, bIsRCtrlKeyDown,
               bIsAltKeyDown, bIsLAltKeyDown, bIsRAltKeyDown);
        if (bIsKeyUp)
        {
            printf("\r\n");
        }
/*
        printf("Shift? %d, Ctrl? %d, Alt? %d\n", bIsShiftKeyDown, bIsCtrlKeyDown, bIsAltKeyDown);
        switch (wParam)
        {
            case WM_SYSKEYDOWN:
            {
                printf("WM_SYSKEYDOWN\n");
                break;  // Call: CallNextHookEx()
            }
            case WM_SYSKEYUP:
            {
                printf("WM_SYSKEYUP\n");
                break;  // Call: CallNextHookEx()
            }
            // Ref: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
            case WM_KEYDOWN:
            {
                printf("WM_KEYDOWN\n");
                break;  // Call: CallNextHookEx()
            }
            // Ref: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keyup
            case WM_KEYUP:
            {
                printf("WM_KEYUP\n");
                break;  // Call: CallNextHookEx()
            }
        }
*/
    }
    const LRESULT x = CallNextHookEx((HHOOK) 0, nCode, wParam, lParam);
    return x;
}

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI wWinMain(HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
                    HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
                    PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
                    int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowshookexw
    const HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL,        // [in] int       idHook
                                         LowLevelKeyboardProc,  // [in] HOOKPROC  lpfn
                                         hInstance,             // [in] HINSTANCE hmod
                                         (DWORD) 0);            // [in] DWORD     dwThreadId
    if (NULL == hHook)
    {
        ErrorExit(L"SetWindowsHookEx(WH_KEYBOARD_LL, ...)");
    }

    const DWORD dwThreadId = GetCurrentThreadId();
    const DWORD z = HSHELL_WINDOWREPLACED;

    printf("Press any key combination to watch low level keyboard events\r\n");
    printf("Press Ctrl+C in this terminal to exit\r\n");
    printf("dwThreadId: %d\r\n", dwThreadId);
    printf("\r\n");

    MSG msg = {};
    while (TRUE)
    {
        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage
        const BOOL bRet = GetMessage(&msg,  // [out]          LPMSG lpMsg
                                     NULL,  // [in, optional] HWND  hWnd
                                     0,     // [in]           UINT  wMsgFilterMin
                                     0);    // [in]           UINT  wMsgFilterMax
        if (-1 == bRet)
        {
            ErrorExit(L"GetMessage");
        }

        if (FALSE == bRet)
        {
            break;  // WM_QUIT received
        }

        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
        const BOOL bRet2 = TranslateMessage(&msg);
        const LRESULT lResult = DispatchMessage(&msg);
        int dummy = 1;  // @DebugBreakpoint
    }
    // Return the exit code to the system from PostQuitMessage()
    return msg.wParam;
}

