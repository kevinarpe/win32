#ifndef H_COMMON_WIN32_TEMP_FILE
#define H_COMMON_WIN32_TEMP_FILE

#include "win32.h"
#include "wstr.h"
#include "win32_file.h"

/**
 * This is a convenience method to call Win32TempGetTempDirPath2(lpTempDirPathWStr, stderr).
 * On error, abort() is called.
 */
void
Win32TempGetTempDirPath(_Out_ struct WStr *lpTempDirPathWStr);

/**
 * Get temporary directory path.
 * Call GetTempPathW(), then call WStrCopyWCharArr() to copy result to 'lpTempDirPathWStr'.
 *
 * @param lpTempDirPathWStr
 *        on successful return, ownership for malloc'd lpTempDirPathWStr->lpWCharArr is passed to caller
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32TempGetTempDirPath2(_Out_ struct WStr *lpTempDirPathWStr,
                         _Out_ FILE        *lpErrorStream);

/**
 * This is a convenience method to call Win32TempCreateTempFileName2(lpPrefixWStr, lpSuffixWStr, stderr).
 * On error, abort() is called.
 */
void
Win32TempCreateTempFileName(// @EmptyStringAllowed
                            _In_  struct WStr *lpPrefixWStr,
                            // @EmptyStringAllowed
                            _In_  struct WStr *lpSuffixWStr,
                            _Out_ struct WStr *lpTempFileNameWStr);

/**
 * Create unique temporary file name.
 * Call Win32GuidCreateToWStr2() to create Win32 GUID, then call WStrConcatMany('lpPrefixWStr', GUID, 'lpSuffixWStr')
 * to copy result to 'lpTempFileNameWStr'.
 *
 * @param lpPrefixWStr
 *        file name prefix
 *        Ex: {@code L"prefix_"}
 *
 * @param lpSuffixWStr
 *        file name suffix
 *        Ex: {@code L"_suffix.tmp"}
 *
 * @param lpTempFileNameWStr
 *        result is copied to {@code lpTempFileNameWStr->lpWCharArr}
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32TempCreateTempFileName2(// @EmptyStringAllowed
                             _In_  struct WStr *lpPrefixWStr,
                             // @EmptyStringAllowed
                             _In_  struct WStr *lpSuffixWStr,
                             _Out_ struct WStr *lpTempFileNameWStr,
                             _Out_ FILE        *lpErrorStream);

/**
 * This is a convenience method to call Win32TempCreateTempFilePath2(lpPrefixWStr, lpSuffixWStr, stderr).
 * On error, abort() is called.
 */
void
Win32TempCreateTempFilePath(// @Nullable
                            _In_  struct WStr *lpNullableTempDirPathWStr,
                            // @EmptyStringAllowed
                            _In_  struct WStr *lpPrefixWStr,
                            // @EmptyStringAllowed
                            _In_  struct WStr *lpSuffixWStr,
                            _Out_ struct WStr *lpTempFilePathWStr);

/**
 * Create path to unique temporary file path.
 * Combine Win32TempGetTempDirPath2() and Win32TempCreateTempFileName2().
 *
 * @param lpNullableTempDirPathWStr
 *        path to directory for temp file
 *        if {@code NULL}, then replace with result from Win32TempGetTempDirPath2()
 *
 * @param lpPrefixWStr
 *        file name prefix
 *        Ex: {@code L"prefix_"}
 *
 * @param lpSuffixWStr
 *        file name suffix
 *        Ex: {@code L"_suffix.tmp"}
 *
 * @param lpTempFileNameWStr
 *        result is copied to {@code lpTempFileNameWStr->lpWCharArr}
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32TempCreateTempFilePath2(// @Nullable
                             _In_  struct WStr *lpNullableTempDirPathWStr,
                             // @EmptyStringAllowed
                             _In_  struct WStr *lpPrefixWStr,
                             // @EmptyStringAllowed
                             _In_  struct WStr *lpSuffixWStr,
                             _Out_ struct WStr *lpTempFilePathWStr,
                             _Out_ FILE        *lpErrorStream);

void
Win32TempCreateFile(// @Nullable
                    _In_  struct WStr      *lpNullableTempDirPathWStr,
                    // @EmptyStringAllowed
                    _In_  struct WStr      *lpPrefixWStr,
                    // @EmptyStringAllowed
                    _In_  struct WStr      *lpSuffixWStr,
                    _Out_ struct Win32File *lpFile);

/**
 * Call Win32TempCreateTempFilePath2(), then call 
 */
bool
Win32TempCreateFile2(// @Nullable
                     _In_  struct WStr      *lpNullableTempDirPathWStr,
                     // @EmptyStringAllowed
                     _In_  struct WStr      *lpPrefixWStr,
                     // @EmptyStringAllowed
                     _In_  struct WStr      *lpSuffixWStr,
                     _Out_ struct Win32File *lpFile,
                     _Out_ FILE             *lpErrorStream);

void
Win32TempLogWF(_In_ struct Win32File *lpFile,
               // @EmptyStringAllowed
               _In_ const wchar_t    *lpMessageFormat,
               ...);

bool
Win32TempLogWF2(_In_    struct Win32File *lpFile,
                _Inout_ FILE             *lpErrorStream,
                // @EmptyStringAllowed
                _In_    const wchar_t    *lpMessageFormat,
                ...);

void
Win32TempLogWFV(_In_ struct Win32File *lpFile,
                // @EmptyStringAllowed
                _In_ const wchar_t    *lpMessageFormat,
                _In_ va_list           ap);

/**
 * Precondition: assert(NULL != lpFile->stream.lpFile);
 *
 * Read full contents of 'lpFile', then call 'LogWFV(lpErrorStream, lpMessageFormat, ap)',
 * then print L": ", contents of 'lpFile', then newline.
 *
 * @param lpFile
 *        temp file with error message
 */
bool
Win32TempLogWFV2(_In_    struct Win32File *lpFile,
                 _Inout_ FILE             *lpErrorStream,
                 // @EmptyStringAllowed
                 _In_    const wchar_t    *lpMessageFormat,
                 _In_    va_list           ap);

#endif  // H_COMMON_WIN32_TEMP_FILE

