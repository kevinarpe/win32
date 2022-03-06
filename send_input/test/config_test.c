#include "config.h"
#include "error_exit.h"
#include <windows.h>  // required for wWinMain()
#include <stdio.h>    // required for printf()
#include <assert.h>   // required for assert()

static void TestConfigParseModifier(_In_ wchar_t                 *lpTokenWCharArr,
                                    _In_ enum EKeyModifier        eModifiers,
                                    _In_ const enum EKeyModifier  eModifiersExpected)
{
    printf("TestConfigParseModifier: (%zd)[%ls] & [%d] -> [%d]\r\n", wcslen(lpTokenWCharArr), lpTokenWCharArr, eModifiers, eModifiersExpected);

    struct WStr tokenWStr = {.lpWCharArr = lpTokenWCharArr, .ulSize = wcslen(lpTokenWCharArr)};

    wchar_t *lpShortcutKeyWCharArr = L"XYZ";
    struct WStr shortcutKeyWStr = {.lpWCharArr = lpShortcutKeyWCharArr, .ulSize = wcslen(lpShortcutKeyWCharArr)};

    const size_t ulLineIndex = 3;

    wchar_t *lpLineWCharArr = L"blah blah blah";
    struct WStr lineWStr = {.lpWCharArr = lpLineWCharArr, .ulSize = wcslen(lpLineWCharArr)};

    ConfigParseModifier(&tokenWStr, &shortcutKeyWStr, ulLineIndex, &lineWStr, &eModifiers);

    assert(eModifiers == eModifiersExpected);
}

static void TestConfigParseShortcutKey(_In_ wchar_t                 *lpShortcutKeyWCharArr,  // Ex: L"Ctrl+Shift+Alt+0x70"
                                       _In_ const enum EKeyModifier  eModifiersExpected,
                                       _In_ const DWORD              dwVkCodeExpected)
{
    printf("TestConfigParseShortcutKey: [%ls]\r\n", lpShortcutKeyWCharArr);

    struct WStr shortcutKeyWStr = {.lpWCharArr = lpShortcutKeyWCharArr, .ulSize = wcslen(lpShortcutKeyWCharArr)};

    const size_t ulLineIndex = 3;

    wchar_t *lpLineWCharArr = L"blah blah blah";
    struct WStr lineWStr = {.lpWCharArr = lpLineWCharArr, .ulSize = wcslen(lpLineWCharArr)};

    struct ShortcutKey shortcutKey = {};
    ConfigParseShortcutKey(&shortcutKeyWStr, ulLineIndex, &lineWStr, &shortcutKey);

    assert(shortcutKey.eModifiers == eModifiersExpected);
    assert(shortcutKey.dwVkCode == dwVkCodeExpected);
}

static void TestConfigParseSendKeys(_In_ wchar_t *lpSendKeysWCharArr)  // Ex: L"username"
{
    printf("TestConfigParseSendKeys: [%ls]\r\n", lpSendKeysWCharArr);

    struct WStr sendKeysWStr = {.lpWCharArr = lpSendKeysWCharArr, .ulSize = wcslen(lpSendKeysWCharArr)};
    struct InputKeyArr inputKeyArr = {};
    ConfigParseSendKeys(&sendKeysWStr, &inputKeyArr);

    assert(inputKeyArr.ulSize == 2U * sendKeysWStr.ulSize);
    size_t ulInputKeyArrIndex = 0;
    for (size_t i = 0; i < sendKeysWStr.ulSize; ++i, ulInputKeyArrIndex += 2)
    {
        const wchar_t wchar = lpSendKeysWCharArr[i];

        INPUT* lpInputKeyDown = inputKeyArr.lpInputKeyArr + ulInputKeyArrIndex;
        INPUT* lpInputKeyUp   = inputKeyArr.lpInputKeyArr + ulInputKeyArrIndex + 1;

        assert(lpInputKeyDown->type == INPUT_KEYBOARD);
        assert(lpInputKeyDown->ki.wVk == 0);
        assert(lpInputKeyDown->ki.wScan == wchar);
        assert(lpInputKeyDown->ki.dwFlags == KEYEVENTF_UNICODE);
    
        assert(lpInputKeyUp->type == INPUT_KEYBOARD);
        assert(lpInputKeyUp->ki.wVk == 0);
        assert(lpInputKeyUp->ki.wScan == wchar);
        assert(lpInputKeyUp->ki.dwFlags == KEYEVENTF_UNICODE | KEYEVENTF_KEYUP);
    }
}

static void TestConfigParseLine(_In_ wchar_t                 *lpLineWCharArr,  // Ex: L"Ctrl+Shift+Alt+0x70|username"
                                _In_ const enum EKeyModifier  eModifiersExpected,
                                _In_ const DWORD              dwVkCodeExpected,
                                _In_ const wchar_t           *lpSendKeysWCharArr)
{
    printf("TestConfigParseLine: [%ls] -> [%d][0x%x][%ls]\r\n", lpLineWCharArr, eModifiersExpected, dwVkCodeExpected, lpSendKeysWCharArr);

    struct WStr lineWStr = {.lpWCharArr = lpLineWCharArr, .ulSize = wcslen(lpLineWCharArr)};

    const size_t ulLineIndex = 3;

    struct ConfigEntry configEntry = {};
    ConfigParseLine(ulLineIndex, &lineWStr, &configEntry);

    assert(configEntry.shortcutKey.eModifiers == eModifiersExpected);
    assert(configEntry.shortcutKey.dwVkCode   == dwVkCodeExpected);
    assert(0 == wcscmp(lpSendKeysWCharArr, configEntry.sendKeysWStr.lpWCharArr));
    assert(wcslen(lpSendKeysWCharArr) == configEntry.sendKeysWStr.ulSize);
    assert(configEntry.inputKeyArr.ulSize == 2U * configEntry.sendKeysWStr.ulSize);
}

static void TestParseConfigFile()
{
    printf("TestParseConfigFile\r\n");

    wchar_t lpConfigWCharArr[] =
L"# Comment\r\n"
L"   # Comment\r\n"
L"       \r\n"
L"\t\t\r\n"
L"\r\n"
L" # Comment\r\n"
L"0x75|abcdef\r\n"
L"\r\n"
L" # Comment\r\n"
L"LCtrl+LShift+LAlt+0x70|password\r\n"
L"\r\n"
L" # Comment\r\n"
L"LShift+LAlt+0x72| username \r\n"
L"\r\n"
L" # Comment\r\n"
L"RCtrl+RAlt+0x72|user東京name\r\n"
L"\r\n"
L" # Comment\r\n"
;
    struct WStr configWStr = {.lpWCharArr = lpConfigWCharArr, .ulSize = wcslen(lpConfigWCharArr)};

    const wchar_t *lpFilePath = L"TestWStr.txt";

    // Intentional: Ignore return value (BOOL)
    DeleteFile(lpFilePath);

    WStrFileWrite(lpFilePath, CP_UTF8, &configWStr);

    struct ConfigEntryDynArr dynArr = {};
    ConfigParseFile(lpFilePath, CP_UTF8, &dynArr);

    // TODO: Assert here!
    assert(dynArr.ulSize == 4);

    // L"0x75|abcdef\r\n"
    assert(dynArr.lpConfigEntryArr[0].shortcutKey.eModifiers == 0);
    assert(dynArr.lpConfigEntryArr[0].shortcutKey.dwVkCode == 0x75);
    assert(0 == wcscmp(dynArr.lpConfigEntryArr[0].sendKeysWStr.lpWCharArr, L"abcdef"));
    assert(dynArr.lpConfigEntryArr[0].inputKeyArr.ulSize == 2U * wcslen(L"abcdef"));

    // L"LCtrl+LShift+LAlt+0x70|password\r\n"
    assert(dynArr.lpConfigEntryArr[1].shortcutKey.eModifiers == (CTRL_LEFT | SHIFT_LEFT | ALT_LEFT));
    assert(dynArr.lpConfigEntryArr[1].shortcutKey.dwVkCode == 0x70);
    assert(0 == wcscmp(dynArr.lpConfigEntryArr[1].sendKeysWStr.lpWCharArr, L"password"));
    assert(dynArr.lpConfigEntryArr[1].inputKeyArr.ulSize == 2U * wcslen(L"password"));

    // L"LShift+LAlt+0x72| username \r\n"
    assert(dynArr.lpConfigEntryArr[2].shortcutKey.eModifiers == (SHIFT_LEFT | ALT_LEFT));
    assert(dynArr.lpConfigEntryArr[2].shortcutKey.dwVkCode == 0x72);
    assert(0 == wcscmp(dynArr.lpConfigEntryArr[2].sendKeysWStr.lpWCharArr, L" username "));
    assert(dynArr.lpConfigEntryArr[2].inputKeyArr.ulSize == 2U * wcslen(L" username "));

    // L"RCtrl+RAlt+0x72|user東京name\r\n"
    assert(dynArr.lpConfigEntryArr[3].shortcutKey.eModifiers == (CTRL_RIGHT | ALT_RIGHT));
    assert(dynArr.lpConfigEntryArr[3].shortcutKey.dwVkCode == 0x72);
    assert(0 == wcscmp(dynArr.lpConfigEntryArr[3].sendKeysWStr.lpWCharArr, L"user東京name"));
    assert(dynArr.lpConfigEntryArr[3].inputKeyArr.ulSize == 2U * wcslen(L"user東京name"));

    // Intentional: Ignore return value (BOOL)
    DeleteFile(lpFilePath);
}

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI wWinMain(HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
                    HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
                    PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
                    int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-error-mode?view=msvc-170
    _set_error_mode(_OUT_TO_STDERR);  // assert to STDERR

    TestConfigParseModifier(L"LCtrl", 0, CTRL_LEFT);
    TestConfigParseModifier(L"RCtrl", 0, CTRL_RIGHT);
    TestConfigParseModifier(L"LShift", 0, SHIFT_LEFT);
    TestConfigParseModifier(L"RShift", 0, SHIFT_RIGHT);
    TestConfigParseModifier(L"LAlt", 0, ALT_LEFT);
    TestConfigParseModifier(L"RAlt", 0, ALT_RIGHT);
    TestConfigParseModifier(L"LCtrl", ALT_LEFT, ALT_LEFT | CTRL_LEFT);
    TestConfigParseModifier(L"RCtrl", ALT_RIGHT, ALT_RIGHT | CTRL_RIGHT);
    TestConfigParseModifier(L"LCtrl", ALT_LEFT, ALT_LEFT | CTRL_LEFT);
    TestConfigParseModifier(L"RCtrl", SHIFT_LEFT, SHIFT_LEFT | CTRL_RIGHT);
    TestConfigParseModifier(L"LCtrl", SHIFT_RIGHT, SHIFT_RIGHT | CTRL_LEFT);
    TestConfigParseModifier(L"RCtrl", SHIFT_RIGHT, SHIFT_RIGHT | CTRL_RIGHT);
    TestConfigParseModifier(L"LAlt", CTRL_LEFT, CTRL_LEFT | ALT_LEFT);
    TestConfigParseModifier(L"RAlt", CTRL_RIGHT, CTRL_RIGHT | ALT_RIGHT);
    TestConfigParseModifier(L"LAlt", CTRL_LEFT, CTRL_LEFT | ALT_LEFT);
    TestConfigParseModifier(L"RShift", CTRL_LEFT, CTRL_LEFT | SHIFT_RIGHT);
    TestConfigParseModifier(L"LShift", CTRL_RIGHT, CTRL_RIGHT | SHIFT_LEFT);
    TestConfigParseModifier(L"RShift", CTRL_RIGHT, CTRL_RIGHT | SHIFT_RIGHT);
    TestConfigParseModifier(L"LCtrl", ALT_LEFT | SHIFT_LEFT, ALT_LEFT | SHIFT_LEFT | CTRL_LEFT);
    TestConfigParseModifier(L"RCtrl", ALT_RIGHT | SHIFT_RIGHT, ALT_RIGHT | SHIFT_RIGHT | CTRL_RIGHT);
    TestConfigParseModifier(L"LCtrl", ALT_LEFT | SHIFT_LEFT, ALT_LEFT | SHIFT_LEFT | CTRL_LEFT);
    TestConfigParseModifier(L"RCtrl", SHIFT_LEFT | ALT_RIGHT, SHIFT_LEFT | ALT_RIGHT | CTRL_RIGHT);
    TestConfigParseModifier(L"LCtrl", SHIFT_RIGHT | ALT_LEFT, SHIFT_RIGHT | ALT_LEFT | CTRL_LEFT);
    TestConfigParseModifier(L"RCtrl", SHIFT_LEFT | ALT_LEFT, SHIFT_LEFT | ALT_LEFT | CTRL_RIGHT);
    TestConfigParseModifier(L"LAlt", CTRL_LEFT | SHIFT_RIGHT, CTRL_LEFT | SHIFT_RIGHT | ALT_LEFT);
    TestConfigParseModifier(L"RAlt", CTRL_RIGHT | SHIFT_LEFT, CTRL_RIGHT | SHIFT_LEFT | ALT_RIGHT);
    TestConfigParseModifier(L"LAlt", CTRL_LEFT | SHIFT_LEFT, CTRL_LEFT | SHIFT_LEFT | ALT_LEFT);
    TestConfigParseModifier(L"RShift", CTRL_LEFT | ALT_LEFT, CTRL_LEFT | ALT_LEFT | SHIFT_RIGHT);
    TestConfigParseModifier(L"LShift", CTRL_RIGHT | ALT_LEFT, CTRL_RIGHT | ALT_LEFT | SHIFT_LEFT);
    TestConfigParseModifier(L"RShift", CTRL_RIGHT | ALT_LEFT, CTRL_RIGHT | ALT_LEFT | SHIFT_RIGHT);

    TestConfigParseShortcutKey(L"0x75", 0, 0x75);
    TestConfigParseShortcutKey(L"0xA5", 0, 0xA5);
    TestConfigParseShortcutKey(L"0xa5", 0, 0xA5);
    TestConfigParseShortcutKey(L"0XA5", 0, 0xA5);
    TestConfigParseShortcutKey(L"0Xa5", 0, 0xA5);
    TestConfigParseShortcutKey(L"  0x75  ", 0, 0x75);
    TestConfigParseShortcutKey(L"LCtrl+RShift+LAlt+0x70", CTRL_LEFT | SHIFT_RIGHT | ALT_LEFT, 0x70);
    TestConfigParseShortcutKey(L" RCtrl +  LShift +  LAlt +  0x70  ", CTRL_RIGHT | SHIFT_LEFT | ALT_LEFT, 0x70);
    TestConfigParseShortcutKey(L"LShift+RAlt+0x72", SHIFT_LEFT | ALT_RIGHT, 0x72);
    TestConfigParseShortcutKey(L"  LShift +  RAlt +  0x72  ", SHIFT_LEFT | ALT_RIGHT, 0x72);
    TestConfigParseShortcutKey(L"RCtrl+LAlt+0x72", CTRL_RIGHT | ALT_LEFT, 0x72);
    TestConfigParseShortcutKey(L"  RCtrl +  LAlt +  0x72  ", CTRL_RIGHT | ALT_LEFT, 0x72);

    TestConfigParseSendKeys(L"username");
    TestConfigParseSendKeys(L"user東京name");

    TestConfigParseLine(L"0x75|abcdef", 0, 0x75, L"abcdef");
    TestConfigParseLine(L"RCtrl+LShift+RAlt+0x70|password", CTRL_RIGHT | SHIFT_LEFT | ALT_RIGHT, 0x70, L"password");
    TestConfigParseLine(L"LShift+LAlt+0x72|username", SHIFT_LEFT | ALT_LEFT, 0x72, L"username");
    TestConfigParseLine(L"RCtrl+RAlt+0x72|user東京name", CTRL_RIGHT | ALT_RIGHT, 0x72, L"user東京name");

    TestParseConfigFile();

    return 0;
}

