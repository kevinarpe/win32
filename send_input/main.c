#include "win32.h"
#include "console.h"
#include "log.h"
#include "wstr.h"
#include "error_exit.h"
#include "config.h"
#include <windows.h>
#include <assert.h>
#include <wchar.h>
#include <stdio.h>

struct ConfigEntryDynArr g_configEntryDynArr = {};

enum EKeyModifier g_eKeyModifiers = 0;

// Ref: https://docs.microsoft.com/en-us/windows/console/registering-a-control-handler-function
// Ref: https://docs.microsoft.com/en-us/windows/console/handlerroutine
static BOOL WINAPI HandlerRoutine(__attribute__((unused)) _In_ DWORD dwCtrlType)
{
    Log(stdout, "Handled event: CTRL_C_EVENT, CTRL_BREAK_EVENT, CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT");
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
    ExitProcess(1);
}

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
        struct ConfigEntry *lpConfigEntry = g_configEntryDynArr.lpConfigEntryArr + i;

        if (g_eKeyModifiers != lpConfigEntry->shortcutKey.eModifiers
        ||  dwVkCode        != lpConfigEntry->shortcutKey.dwVkCode)
        {
            continue;
        }

        ++(lpConfigEntry->ulSendKeysCount);
        LogF(stdout, "%zd:SendKeys(%ls)", lpConfigEntry->ulSendKeysCount, lpConfigEntry->sendKeysWStr.lpWCharArr);

        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput
        // Ref: https://stackoverflow.com/questions/32149644/keyboard-input-via-sendinput-win32-api-doesnt-work-hardware-one-does
        // Ref: https://stackoverflow.com/a/71384213/257299
        // Ref: https://batchloaf.wordpress.com/2014/10/02/using-sendinput-to-type-unicode-characters/
        for (size_t j = 0; j < lpConfigEntry->inputKeyArr.ulSize; ++j)
        {
            const UINT cInputs = 1;
            const UINT uSent = SendInput(cInputs,                                       // [in] UINT    cInputs
                                         lpConfigEntry->inputKeyArr.lpInputKeyArr + j,  // [in] LPINPUT pInputs
                                         sizeof(INPUT));                                // [in] int     cbSize
            if (uSent != cInputs)
            {
                ErrorExitF("SendInput([%zd]%ls[index:%zd]): Sent %zd events, but expected %zd\n",
                           lpConfigEntry->sendKeysWStr.ulSize, lpConfigEntry->sendKeysWStr.lpWCharArr, j,
                           uSent, lpConfigEntry->inputKeyArr.ulSize);
            }
        }
        break;
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

static void ShowHelpThenExit(_In_opt_ const char *lpszNulllableErrorMsgFmt, ...)
{
    if (NULL != lpszNulllableErrorMsgFmt)
    {
        puts("\nError: ");

        // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
        va_list ap;
        va_start(ap, lpszNulllableErrorMsgFmt);

        vprintf(lpszNulllableErrorMsgFmt, ap);

        va_end(ap);

        puts("\n");
    }

    printf("\n");
    printf("Usage: %ls CONFIG_FILE_PATH [/?] [-h] [--help]\n", __wargv[0]);
    printf("Register Windows global keyboard shortcuts to send keys, usually username or password.\n");
    printf("\n");
    printf("Required Arguments:\n");
    printf("    CONFIG_FILE_PATH: path to config file\n");
    printf("        Example: \"C:\\src\\path to my config file.txt\"\n");
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
    printf("    /? or -h or --help\n");
    printf("        Show this help page\n");
    printf("\n");

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess
    ExitProcess(1);
}

static void CheckCommandLineArgs(_Out_ wchar_t **lppConfigFilePath)
{
    assert(NULL != lppConfigFilePath);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv?view=msvc-170
    if (1 == __argc)
    {
        ShowHelpThenExit("Missing argument CONFIG_FILE_PATH");
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
        ShowHelpThenExit("Too many arguments: Expected exactly one, but found %d", (__argc - 1));
    }

    *lppConfigFilePath = __wargv[1];
    const DWORD dwAttr = GetFileAttributes(*lppConfigFilePath);
    if (INVALID_FILE_ATTRIBUTES == dwAttr)
    {
        ErrorExitF("GetFileAttributes(%ls)", *lppConfigFilePath);
    }

    if (FILE_ATTRIBUTE_DIRECTORY & dwAttr)
    {
        ShowHelpThenExit("CONFIG_FILE_PATH is a directory: [%ls]", __wargv[0]);
    }
}

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI wWinMain(                        HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
                    __attribute__((unused)) HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
                    __attribute__((unused)) PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
                    __attribute__((unused)) int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    const size_t ulMinConsoleLines = 500;
    const size_t ulMaxConsoleLines = 500;
    RedirectIOToConsole(ulMinConsoleLines, ulMaxConsoleLines);

    const BOOL bIsAdd = TRUE;
    if (!SetConsoleCtrlHandler(HandlerRoutine, bIsAdd))
    {
        ErrorExit("SetConsoleCtrlHandler()");
    }
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
        ErrorExit("SetWindowsHookEx(WH_KEYBOARD_LL, ...)");
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
            ErrorExit("GetMessage");
        }

        if (FALSE == bRet)
        {
            break;  // WM_QUIT received
        }

        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
        __attribute__((unused)) const BOOL    bRet2   = TranslateMessage(&msg);

        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage
        __attribute__((unused)) const LRESULT lResult = DispatchMessage(&msg);

        #define DEBUG_BREAKPOINT do { __attribute__((unused)) int dummy = 1; } while (0)
        DEBUG_BREAKPOINT;
    }

    // Return the exit code to the system from PostQuitMessage()
    return msg.wParam;
}

