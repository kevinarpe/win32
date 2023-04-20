#ifndef H_COMMON_WIN32_GUID
#define H_COMMON_WIN32_GUID

#include "wstr.h"

/**
 * Example:
 * // Important: You must copy the literal to a writable char array.
 * wchar_t guidWCharArr[] = GUID_EMPTY_LITERAL;
 * struct WStr guidWStr = WSTR_FROM_WCHAR_ARR(guidWCharArr);
 */
#define WIN32_GUID_ZERO_WCHAR_LITERAL L"00000000-0000-0000-0000-000000000000"

/**
 * 36 (thirty-six): Number of wchar_t's (not including trailing NUL char) in {@link WIN32_GUID_ZERO_WCHAR_LITERAL}
 */
#define WIN32_GUID_ZERO_WCHAR_LEN ((sizeof(WIN32_GUID_ZERO_WCHAR_LITERAL) / sizeof(wchar_t)) - LEN_NUL_CHAR)

/**
 * This is a convenience function to call {@link Win32GuidCreate2(lpGuid)} where {@code lpErrorStream} is {@code stderr}.
 * On error, abort() is called.
 */
void
Win32GuidCreate(_Out_ GUID *lpGuid);

/**
 * @param lpGuid
 *        <pre>{@code
 *        struct {
 *            unsigned long  Data1;
 *            unsigned short Data2;
 *            unsigned short Data3;
 *            unsigned char  Data4[8];
 *        };
 * }</pre>
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32GuidCreate2(_Out_ GUID *lpGuid,
                 _In_  FILE *lpErrorStream);

/**
 * This is a convenience function to call {@link Win32GuidToWStr2()} where {@code lpErrorStream} is {@code stderr}.
 * On error, abort() is called.
 */
void
Win32GuidToWStr(_In_  GUID        *lpGuid,
                _Out_ struct WStr *lpWStr);

/**
 * Example: <pre>{@code
 *     GUID guid = {0};
 *     Win32GuidCreate(&guid);
 *     // Important: You must copy the literal to a writable char array.
 *     wchar_t guidWCharArr[] = GUID_EMPTY_LITERAL;
 *     struct WStr guidWStr = WSTR_FROM_WCHAR_ARR(guidWCharArr);
 *     if (false == Win32GuidToWStr2(&guid, &guidWStr, stderr)) { abort(); }
 *     // use: guidWStr
 * }</pre>
 *
 * @param lpWStr
 *        Ex: {@code L"E7CD34AC-9339-473C-9C78-A7ABC91A5A37"}
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32GuidToWStr2(_In_  const GUID  *lpGuid,
                 _Out_ struct WStr *lpWStr,
                 _In_  FILE        *lpErrorStream);

void
Win32GuidCreateToWStr(_Out_ struct WStr *lpWStr);

bool
Win32GuidCreateToWStr2(_Out_ struct WStr *lpWStr,
                       _In_  FILE        *lpErrorStream);

#endif  // H_COMMON_WIN32_GUID

