#include "win32_winver.h"
#include "wstr.h"
#include "error_exit.h"
#include "config.h"
#include <windows.h>
#include <assert.h>
#include <wchar.h>
#include <stdio.h>

const int SHORT_MOST_SIGNIFICANT_BIT = 0x8000;

struct ConfigEntryDynArr g_configEntryDynArr = {};

enum EKeyModifier g_eKeyModifiers = 0;

static void UpdateEKeyModifiers(_In_ const BOOL              bIsKeyUp,
                                _In_ const enum EKeyModifier eKeyModifier)
{
    if (bIsKeyUp)
    {
        // When key up, remove flag 'eKeyModifier'
        g_eKeyModifiers &= ~eKeyModifier;
    }
    else
    {
        // When key down, add flag 'eKeyModifier'
        g_eKeyModifiers |= eKeyModifier;
    }
}

static void HandleKeyUp(_In_ const DWORD dwVkCode)
{
    for (size_t i = 0; i < g_configEntryDynArr.ulSize; ++i)
    {
        const struct ConfigEntry *lpConfigEntry = g_configEntryDynArr.lpConfigEntryArr + i;

        if (g_eKeyModifiers == lpConfigEntry->shortcutKey.eModifiers
        &&  dwVkCode        == lpConfigEntry->shortcutKey.dwVkCode)
        {
            printf("SendKeys(%ls)\r\n", lpConfigEntry->sendKeysWStr.lpWCharArr);
            // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput
            // Ref: https://stackoverflow.com/questions/32149644/keyboard-input-via-sendinput-win32-api-doesnt-work-hardware-one-does
            const UINT uSent = SendInput(lpConfigEntry->inputKeyArr.ulSize,
                                         lpConfigEntry->inputKeyArr.lpInputKeyArr,
                                         sizeof(INPUT));

            if (uSent != lpConfigEntry->inputKeyArr.ulSize)
            {
                _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                             L"SendInput([%zd]%ls): Sent %zd events, but expected %zd\r\n",
                             lpConfigEntry->sendKeysWStr.ulSize, lpConfigEntry->sendKeysWStr.lpWCharArr,
                             uSent, lpConfigEntry->inputKeyArr.ulSize);

                ErrorExit(g_lpErrorMsgBuffer);
            }
            break;
        }
    }
}

// Ref: https://docs.microsoft.com/en-us/windows/win32/winmsg/lowlevelkeyboardproc
// Ref(_In_): https://docs.microsoft.com/en-us/cpp/code-quality/understanding-sal
static LRESULT CALLBACK LowLevelKeyboardProc(_In_ int    nCode,
                                             _In_ WPARAM wParam,  // Any of: WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, or WM_SYSKEYUP
                                             _In_ LPARAM lParam)
{
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-kbdllhookstruct
    const KBDLLHOOKSTRUCT* info = (KBDLLHOOKSTRUCT*) lParam;
    const BOOL bIsInjected = (0 != (info->flags & LLKHF_INJECTED));
    if (HC_ACTION == nCode && !bIsInjected)
    {
        // Is this key stroke a function key (F1, etc.) or a numpad key?
//        const BOOL bIsExtended = (0 != (info->flags & LLKHF_EXTENDED));

        // If TRUE, then 'wParam' will be WM_SYSKEYDOWN or WM_SYSKEYUP
//        const BOOL bIsAltDown = (0 != (info->flags & LLKHF_ALTDOWN));

        // If TRUE, then key is released (up).  If FALSE, then key is pressed (down).
        const BOOL bIsKeyUp = (0 != (info->flags & LLKHF_UP));

        switch (info->vkCode)
        {
            case VK_LSHIFT:
            {
                UpdateEKeyModifiers(bIsKeyUp, SHIFT_LEFT);
                break;
            }
            case VK_RSHIFT:
            {
                UpdateEKeyModifiers(bIsKeyUp, SHIFT_RIGHT);
                break;
            }
            case VK_LCONTROL:
            {
                UpdateEKeyModifiers(bIsKeyUp, CTRL_LEFT);
                break;
            }
            case VK_RCONTROL:
            {
                UpdateEKeyModifiers(bIsKeyUp, CTRL_RIGHT);
                break;
            }
            case VK_LMENU:
            {
                UpdateEKeyModifiers(bIsKeyUp, ALT_LEFT);
                break;
            }
            case VK_RMENU:
            {
                UpdateEKeyModifiers(bIsKeyUp, ALT_RIGHT);
                break;
            }
            default:
            {
                // Captain Obvious says: Only sent inputs on shortcut key *UP*.
                if (bIsKeyUp)
                {
                    HandleKeyUp(info->vkCode);
                }
                break;  // Explicit
            }
        }
    }
    const LRESULT x = CallNextHookEx((HHOOK) 0, nCode, wParam, lParam);
    return x;
}

static void ShowHelpThenExit(_In_opt_ const wchar_t *lpNulllableErrorWCharArr)
{
    if (NULL != lpNulllableErrorWCharArr)
    {
        printf("\r\nError: %ls\r\n", lpNulllableErrorWCharArr);
    }

    printf("\r\n");
    printf("Usage: %ls CONFIG_FILE_PATH [/?] [-h] [--help]\r\n", __wargv[0]);
    printf("Register Windows global keyboard shortcuts to send keys, usually username or password.\r\n");
    printf("\r\n");
    printf("Required Arguments:\r\n");
    printf("    CONFIG_FILE_PATH: path to config file\r\n");
    printf("        Example: \"C:\\src\\path to my config file.txt\"\r\n");
    printf("\r\n");
    printf("        Config file format:\r\n");
    printf("\r\n");
    printf("            Line format: <shortcut-key>|<send-keys-text>\r\n");
    printf("            <shortcut-key> format: {L/RCtrl+}{L/RShift+}{L/RAlt+}<virtual-key-code>\r\n");
    printf("\r\n");
    printf("            ... where {LCtrl+} and {RCtrl+} are optional left/right Control key indicators,\r\n");
    printf("                e.g., LCtrl+0x70 for LCtrl+F1 or RCtrl+0x71 for RCtrl+F2\r\n");
    printf("\r\n");
    printf("            ... where {LShift+} and {RShift+} are optional left/right Shift key indicators,\r\n");
    printf("                e.g., LShift+0x70 for LShift+F1 or RShift+0x71 for RShift+F2\r\n");
    printf("\r\n");
    printf("            ... where {LAlt+} and {RAlt+} are optional left/right Alt key indicators,\r\n");
    printf("                e.g., LAlt+0x70 for LAlt+F1 or RAlt+0x71 for RAlt+F2\r\n");
    printf("\r\n");
    printf("            ... where L/RCtrl, L/RShift, L/RAlt modifiers may be combined,\r\n");
    printf("                e.g., LCtrl+LShift+LAlt+0x70 for LCtrl+LShift+LAlt+F1\r\n");
    printf("\r\n");
    printf("            ... where <virtual-key-code> is hexidecimal code (with 0x prefix) for Win32 virtual-key code,\r\n");
    printf("                e.g., 0x70 for F1\r\n");
    printf("                Read more here: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes\r\n");
    printf("\r\n");
    printf("            Whitespace is ignored, except in <send-keys-text>.\r\n");
    printf("            Empty lines are ignored.\r\n");
    printf("            If first character is '#', then entire line is treated as a comment and ignored.\r\n");
    printf("\r\n");
    printf("            Example(1): # This is a comment.\r\n");
    printf("            Example(2): LCtrl+LShift+LAlt+0x70|username\r\n");
    printf("                        ... will send input 'username'  for keyboard shortcut: LCtrl+LShift+LAlt+F1\r\n");
    printf("            Example(3): LCtrl+LShift+LAlt+0x71|P*assw0rd\r\n");
    printf("                        ... will send input 'P*assw0rd' for keyboard shortcut: LCtrl+LShift+LAlt+F2\r\n");
    printf("\r\n");
    printf("Optional Arguments:\r\n");
    printf("    /? or -h or --help\r\n");
    printf("        Show this help page\r\n");
    printf("\r\n");

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
    ExitProcess(1);
}

static void CheckCommandLineArgs(_Out_ wchar_t **lppConfigFilePath)
{
    assert(NULL != lppConfigFilePath);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv?view=msvc-170
    if (1 == __argc)
    {
        ShowHelpThenExit(L"Missing argument CONFIG_FILE_PATH");
    }

    // Intentional: Skip 0 == i which is path to executable.
    for (int i = 1; i < __argc; ++i)
    {
        if (0 == wcscmp(L"/?", __wargv[i]) || 0 == wcscmp(L"-h", __wargv[i]) || 0 == wcscmp(L"--help", __wargv[i]))
        {
            ShowHelpThenExit(NULL);
        }
    }

    if (__argc > 2)
    {
        _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                     L"Too many arguments: Expected exactly one, but found %d", (__argc - 1));
        ShowHelpThenExit(g_lpErrorMsgBuffer);
    }

    *lppConfigFilePath = __wargv[1];
    const DWORD dwAttr = GetFileAttributes(*lppConfigFilePath);
    if (INVALID_FILE_ATTRIBUTES == dwAttr)
    {
        ErrorExit(L"GetFileAttributes");
    }

    if (FILE_ATTRIBUTE_DIRECTORY & dwAttr)
    {
        _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                     L"CONFIG_FILE_PATH is a directory: [%ls]", __wargv[0]);
        ShowHelpThenExit(g_lpErrorMsgBuffer);
    }
}

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI wWinMain(HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
                    HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
                    PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
                    int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    wchar_t *lpConfigFilePath = NULL;
    CheckCommandLineArgs(&lpConfigFilePath);

    ConfigParseFile(lpConfigFilePath, CP_UTF8, &g_configEntryDynArr);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowshookexw
    const HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL,        // [in] int       idHook
                                         LowLevelKeyboardProc,  // [in] HOOKPROC  lpfn
                                         hInstance,             // [in] HINSTANCE hmod
                                         (DWORD) 0);            // [in] DWORD     dwThreadId
    if (NULL == hHook)
    {
        ErrorExit(L"SetWindowsHookEx(WH_KEYBOARD_LL, ...)");
    }

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

