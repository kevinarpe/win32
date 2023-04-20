#ifndef H_COMMON_WIN32_LAST_ERROR
#define H_COMMON_WIN32_LAST_ERROR

#include "wstr.h"

/**
 * This is a convenience method to call {@code Win32LastErrorGetWStr2(dwLastError, lpWStr, stderr)}.
 * On error, abort() is called.
 */
void
Win32LastErrorGetWStr(_In_    const DWORD  dwLastError,
                      _Inout_ struct WStr *lpWStr);

/**
 * Get error message text for a Win32 error code returned by {@see GetLastError()}.
 *
 * @param dwLastError
 *        usually result of {@code GetLastError()}
 *        Ex: 3 -> L"Path not found"
 *
 * @param lpWStr
 *        output: pointer to error message
 *        ownership is transferred to caller for lpWStr->lpWCharArr
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 *         Ex: L"Path not found"
 */
bool
Win32LastErrorGetWStr2(_In_    const DWORD  dwLastError,
                       _Inout_ struct WStr *lpWStr,
                       _Out_   FILE        *lpErrorStream);

/**
 * This is a convenience method to call {@code Win32LastErrorFPutW2(lpStream, stderr)}.
 * On error, abort() is called.
 */
void
Win32LastErrorFPutW(_In_ FILE *lpStream);

/**
 * This is a convenience method to call {@code Win32LastErrorFPutW(lpStream, stderr)},
 * then call {@code abort()}.
 */
void
Win32LastErrorFPutWAbort(_In_ FILE *lpStream);

/**
 * Call {@see GetLastError()}, get error message text, then log to {@code lpStream}.
 * <br>Example printed text: L"2022-03-10 22:17:47.123 +09:00 ERROR: Win32 Last Error: 3/0x3: Path not found\r\n"
 *
 * @param lpStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32LastErrorFPutW2(_In_  FILE *lpStream,
                     _Out_ FILE *lpErrorStream);

/**
 * This is a convenience method to call {@code Win32LastErrorFPutWS2(lpStream, lpMessage, stderr)}.
 * On error, abort() is called.
 */
void
Win32LastErrorFPutWS(_In_ FILE          *lpStream,
                     // @EmptyStringAllowed
                     _In_ const wchar_t *lpMessage);

/**
 * This is a convenience method to call {@code Win32LastErrorFPutWS(lpStream, lpMessage)},
 * then call {@code abort()}.
 */
void
Win32LastErrorFPutWSAbort(_In_ FILE          *lpStream,
                          // @EmptyStringAllowed
                          _In_ const wchar_t *lpMessage);

/**
 * Call {@see GetLastError()}, get error message text, then log to {@code lpStream}.
 * <br>Example printed text: L"2022-03-10 22:17:47.123 +09:00 ERROR: Failed to open file: Win32 Last Error: 3/0x3: Path not found\r\n"
 *
 * @param lpStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @param lpMessage
 *        message to log before error message text
 *        Ex: {@code L"Failed to open file"}
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32LastErrorFPutWS2(_In_  FILE          *lpStream,
                      // @EmptyStringAllowed
                      _In_  const wchar_t *lpMessage,
                      _Out_ FILE          *lpErrorStream);

/**
 * This is a convenience method to call {@code Win32LastErrorFPrintFWV2(lpStream, lpMessageFormat, ap, stderr)}.
 * On error, abort() is called.
 */
void
Win32LastErrorFPrintFW(_In_ FILE          *lpStream,
                       // @EmptyStringAllowed
                       _In_ const wchar_t *lpMessageFormat,
                       _In_ ...);

/**
 * This is a convenience method to call {@code Win32LastErrorFPrintFWV2(lpStream, lpMessageFormat, ap, stderr)},
 * then call {@code abort()}.
 */
void
Win32LastErrorFPrintFWAbort(_In_ FILE          *lpStream,
                            // @EmptyStringAllowed
                            _In_ const wchar_t *lpMessageFormat,
                            _In_ ...);

/**
 * This is a convenience method to call {@code Win32LastErrorFPrintFWV2(lpStream, lpMessageFormat, ap, lpErrorStream)}.
 */
bool
Win32LastErrorFPrintFW2(_In_  FILE          *lpStream,
                        _Out_ FILE          *lpErrorStream,
                        // @EmptyStringAllowed
                        _In_  const wchar_t *lpMessageFormat,
                        _In_  ...);

/**
 * This is a convenience method to call {@code Win32LastErrorFPrintFWV2(lpStream, lpMessageFormat, ap, stderr)}.
 */
void
Win32LastErrorFPrintFWV(_In_  FILE          *lpStream,
                       // @EmptyStringAllowed
                       _In_  const wchar_t *lpMessageFormat,
                       _In_  va_list        ap);

/**
 * This is a convenience method to call {@code Win32LastErrorFPrintFWV2(lpStream, lpMessageFormat, ap, stderr)},
 * then call {@code abort()}.
 */
void
Win32LastErrorFPrintFWVAbort(_In_  FILE          *lpStream,
                            // @EmptyStringAllowed
                            _In_  const wchar_t *lpMessageFormat,
                            _In_  va_list        ap);

/**
 * Call {@see GetLastError()}, get error message text, then log to {@code lpStream}.
 * <br>Example printed text: L"2022-03-10 22:17:47.123 +09:00 ERROR: Failed to open file [C:\\path\\to\\data.txt]: Win32 Last Error: 3/0x3: Path not found\r\n"
 *
 * @param lpStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @param lpMessageFormat
 *        format string for {@code fprintf(lpMessageFormat, ...)} to log before error message text
 *        Ex: {@code L"Failed to open file: [%ls]"}
 *
 * @param ap
 *        zero or more arguments to be used with {@code fprintf(lpMessageFormat, ...)}
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32LastErrorFPrintFWV2(_In_  FILE          *lpStream,
                        // @EmptyStringAllowed
                        _In_  const wchar_t *lpMessageFormat,
                        _In_  va_list        ap,
                        _Out_ FILE          *lpErrorStream);

#endif  // H_COMMON_WIN32_LAST_ERROR

