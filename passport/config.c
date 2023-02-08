#include "config.h"
#include "error_exit.h"
#include "xmalloc.h"
#include "log.h"
#include <assert.h>   // required for assert()
#include <windows.h>

static void
ConfigEntryDynArr_AssertValid(__attribute__((unused))
                              _In_ const struct ConfigEntryDynArr *lpDynArr)
{
    assert(NULL != lpDynArr);
    // Intentional: If ulCapacity is more that *half* of SIZE_MAX, there is probably an unsigned wrap bug.
    assert(lpDynArr->ulCapacity <= SIZE_MAX / 2U);
    assert(lpDynArr->ulSize <= lpDynArr->ulCapacity);
}

static void
ConfigEntryDynArr_IncreaseCapacity(_Inout_ struct ConfigEntryDynArr *lpDynArr)
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

static void
ConfigEntryDynArr_Append(_Inout_ struct ConfigEntryDynArr *lpDynArr,
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

static void
ConfigAssertValid(_In_ const struct Config *lpConfig)
{
    if (0 == lpConfig->shortcutKey.eKeyModifiers && 0 == lpConfig->shortcutKey.dwVkCode)
    {
        ErrorExitW(L"Config file: Missing shortcut key");
    }

    if (0 == lpConfig->dynArr.ulSize)
    {
        ErrorExitW(L"Config file: Found zero username and password entries");
    }
}

void
ConfigParseFile(_In_  const wchar_t *lpConfigFilePathWCharArr,
                _In_  const UINT     codePage,  // Ex: CP_UTF8
                _Out_ struct Config *lpConfig)
{
    LogWF(stdout, L"INFO: Config file: Reading [%ls]...\r\n", lpConfigFilePathWCharArr);
    struct WStr textWStr = {0};
    WStrFileRead(lpConfigFilePathWCharArr, codePage, &textWStr);
    LogWF(stdout, L"INFO: Config file: Read %zd chars\r\n", textWStr.ulSize);

    const struct WStrSplitOptions splitOptions = {
        .iMinTokenCount             = UNLIMITED_MIN_TOKEN_COUNT,
        .iMaxTokenCount             = UNLIMITED_MAX_TOKEN_COUNT,
        .fpNullableWStrConsumerFunc = WStrTrimSpace,
    };

    struct WStrArr lineWStrArr = {0};
    WStrSplitNewLine(&textWStr, &splitOptions, &lineWStrArr);

    BOOL bFirstLine = TRUE;
    struct ShortcutKey shortcutKey = {0};
    struct ConfigEntryDynArr dynArr = {0};

    for (size_t ulLineIndex = 0; ulLineIndex < lineWStrArr.ulSize; ++ulLineIndex)
    {
        const struct WStr *lpLineWStr = lineWStrArr.lpWStrArr + ulLineIndex;

        if (0 == lpLineWStr->ulSize) {
            continue;  // skip blank line
        }

        if (L'#' == lpLineWStr->lpWCharArr[0]) {
            continue;  // skip comment -- begins with '#'
        }

        if (TRUE == bFirstLine)
        {
            bFirstLine = FALSE;
            struct WStr errorWStr = {0};
            if (FALSE == ShortcutKeyTryParseWStr(lpLineWStr, &shortcutKey, &errorWStr))
            {
                ErrorExitWF(L"%ls", errorWStr.lpWCharArr);
            }
        }
        else {
            struct ConfigEntry configEntry = {0};
            ConfigParseLine(ulLineIndex, lpLineWStr, &configEntry);
            ConfigEntryDynArr_Append(&dynArr, &configEntry);
        }
    }

    lpConfig->shortcutKey = shortcutKey;
    lpConfig->dynArr      = dynArr;

    WStrFree(&textWStr);
    WStrArrFree(&lineWStrArr);

    ConfigAssertValid(lpConfig);
    LogWF(stdout, L"INFO: Config file: Read %zd entries\r\n", lpConfig->dynArr.ulSize);
}

void
ConfigParseLine(_In_  const size_t        ulLineIndex,
                _In_  const struct WStr  *lpLineWStr,  // Ex: L"username|password"
                _Out_ struct ConfigEntry *lpConfigEntry)
{
    WStrAssertValid(lpLineWStr);
    assert(NULL != lpConfigEntry);

    const struct WStr delimWStr = WSTR_FROM_LITERAL(L"|");

    const struct WStrSplitOptions splitOptions = {
        .iMinTokenCount             = UNLIMITED_MIN_TOKEN_COUNT,
        .iMaxTokenCount             = UNLIMITED_MAX_TOKEN_COUNT,
        .fpNullableWStrConsumerFunc = WStrTrimSpace,
    };

    // Ex: "username|password" -> ["username", "password"]
    struct WStrArr tokenWStrArr = {};
    WStrSplit(lpLineWStr, &delimWStr, &splitOptions, &tokenWStrArr);

    if (tokenWStrArr.ulSize < 2)
    {
        ErrorExitWF(L"Config file: Failed to find delim [%ls]\r\n"
                    L"Line #%zd: %ls\r\n",
                    delimWStr.lpWCharArr, (1 + ulLineIndex), lpLineWStr->lpWCharArr);
    }

    if (tokenWStrArr.ulSize > 2)
    {
        ErrorExitWF(L"Config file: Found multiple delim [%ls]\r\n"
                    L"Line #%zd: %ls\r\n",
                    delimWStr.lpWCharArr, (1 + ulLineIndex), lpLineWStr->lpWCharArr);
    }

    // Ex: L"username|password" -> L"username"
    const struct WStr *lpUsernameWStr = tokenWStrArr.lpWStrArr + 0;  // Explicit: '+ 0'

    // Ex: L"username|password" -> L"password"
    const struct WStr *lpPasswordWStr = tokenWStrArr.lpWStrArr + 1;

    if (0 == lpUsernameWStr->ulSize)
    {
        ErrorExitWF(L"Config file: Username is empty\r\n"
                    L"Line #%zd: %ls\r\n",
                    (1 + ulLineIndex), lpLineWStr->lpWCharArr);
    }
    else if (0 == lpPasswordWStr->ulSize)
    {
        ErrorExitWF(L"Config file: Password is empty\r\n"
                    L"Line #%zd: %ls\r\n",
                    (1 + ulLineIndex), lpLineWStr->lpWCharArr);
    }

    // Intentional: MUST copy.  Do not assign.  Why?  WStrArrFree() is called next.
    WStrCopyWStr(&lpConfigEntry->usernameWStr, lpUsernameWStr);
    // Intentional: MUST copy.  Do not assign.  Why?  WStrArrFree() is called next.
    WStrCopyWStr(&lpConfigEntry->passwordWStr, lpPasswordWStr);

    WStrArrFree(&tokenWStrArr);
}

