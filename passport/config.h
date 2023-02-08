#ifndef H_CONFIG
#define H_CONFIG

#include "wstr.h"
#include "shortcut_key.h"

// TODO: Support comma separate list of hot keys?

struct ConfigEntry
{
    struct WStr usernameWStr;
    struct WStr passwordWStr;
};

struct ConfigEntryDynArr
{
    struct ConfigEntry *lpConfigEntryArr;
    size_t              ulSize;
    size_t              ulCapacity;
};

struct Config
{
    struct ShortcutKey       shortcutKey;
    struct ConfigEntryDynArr dynArr;
};

void ConfigParseFile(_In_  const wchar_t *lpConfigFilePathWCharArr,
                     _In_  const UINT     codePage,  // Ex: CP_UTF8
                     _Out_ struct Config *lpConfig);

// All functions below are public/non-static for testing.
// Ref: https://stackoverflow.com/questions/593414/how-to-test-a-static-function

void ConfigParseLine(_In_  const size_t        ulLineIndex,
                     _In_  const struct WStr  *lpLineWStr,  // Ex: L"username|password"
                     _Out_ struct ConfigEntry *lpConfigEntry);

#endif  // H_CONFIG

