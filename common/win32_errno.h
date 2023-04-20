#ifndef H_COMMON_WIN32_ERRNO
#define H_COMMON_WIN32_ERRNO

#include "wstr.h"

/**
 * Get error message text for a Win32 errno.
 *
 * @param lastErrno
 *        usually {@code errno}
 *        Ex: EFAULT/14 -> L"Bad address"
 *
 * @param lpWStr
 *        output: pointer to error message
 *        ownership is transferred to caller for lpWStr->lpWCharArr
 */
void
Win32ErrnoGetWStr(_In_    const int    lastErrno,
                  _Inout_ struct WStr *lpWStr);

/**
 * This is a convenience method to call {@code Win32ErrnoFPutW2(lpStream, stderr)}.
 * On error, abort() is called.
 */
void
Win32ErrnoFPutW(_In_ FILE *lpStream);

/**
 * This is a convenience method to call {@code Win32ErrnoFPutW(lpStream, stderr)},
 * then call {@code abort()}.
 */
void
Win32ErrnoFPutWAbort(_In_ FILE *lpStream);

/**
 * Get error message text for 'errno', then log to {@code lpStream}.
 * <br>Example printed text: L"2022-03-10 22:17:47.123 +09:00 ERROR: errno(3/0x0003): Path not found\r\n"
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
Win32ErrnoFPutW2(_In_  FILE *lpStream,
                 _Out_ FILE *lpErrorStream);

/**
 * This is a convenience method to call {@code Win32ErrnoFPutWS2(lpStream, lpMessage, stderr)}.
 * On error, abort() is called.
 */
void
Win32ErrnoFPutWS(_In_ FILE          *lpStream,
                 // @EmptyStringAllowed
                 _In_ const wchar_t *lpMessage);

/**
 * This is a convenience method to call {@code Win32ErrnoFPutWS(lpStream, lpMessage)},
 * then call {@code abort()}.
 */
void
Win32ErrnoFPutWSAbort(_In_ FILE          *lpStream,
                      // @EmptyStringAllowed
                      _In_ const wchar_t *lpMessage);

/**
 * Get error message text for 'errno', then log to {@code lpStream}.
 * <br>Example printed text: L"2022-03-10 22:17:47.123 +09:00 ERROR: Failed to open file: errno(3/0x0003): Path not found\r\n"
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
Win32ErrnoFPutWS2(_In_  FILE          *lpStream,
                  // @EmptyStringAllowed
                  _In_  const wchar_t *lpMessage,
                  _Out_ FILE          *lpErrorStream);

/**
 * This is a convenience method to call {@code Win32ErrnoFPrintFWV2(lpStream, lpMessageFormat, ap, stderr)}.
 * On error, abort() is called.
 */
void
Win32ErrnoFPrintFW(_In_ FILE          *lpStream,
                   // @EmptyStringAllowed
                   _In_ const wchar_t *lpMessageFormat, ...);

/**
 * This is a convenience method to call {@code Win32ErrnoFPrintFWV2(lpStream, lpMessageFormat, ap, stderr)},
 * then call {@code abort()}.
 */
void
Win32ErrnoFPrintFWAbort(_In_ FILE          *lpStream,
                        // @EmptyStringAllowed
                        _In_ const wchar_t *lpMessageFormat, ...);

/**
 * This is a convenience method to call {@code Win32ErrnoFPrintFWV2(lpStream, lpMessageFormat, ap, lpErrorStream)}.
 */
bool
Win32ErrnoFPrintFW2(_In_  FILE          *lpStream,
                    _Out_ FILE          *lpErrorStream,
                    // @EmptyStringAllowed
                    _In_  const wchar_t *lpMessageFormat, ...);

/**
 * This is a convenience method to call {@code Win32ErrnoFPrintFWV2(lpStream, lpMessageFormat, ap, stderr)}.
 */
void
Win32ErrnoFPrintFWV(_In_  FILE          *lpStream,
                    // @EmptyStringAllowed
                    _In_  const wchar_t *lpMessageFormat,
                    _In_  va_list        ap);

/**
 * This is a convenience method to call {@code Win32ErrnoFPrintFWV2(lpStream, lpMessageFormat, ap, stderr)},
 * then call {@code abort()}.
 */
void
Win32ErrnoFPrintFWVAbort(_In_  FILE          *lpStream,
                         // @EmptyStringAllowed
                         _In_  const wchar_t *lpMessageFormat,
                         _In_  va_list        ap);

/**
 * Get error message text for 'errno', then log to {@code lpStream}.
 * <br>Example printed text: L"2022-03-10 22:17:47.123 +09:00 ERROR: Failed to open file [C:\\path\\to\\data.txt]: errno(3/0x0003): Path not found\r\n"
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
Win32ErrnoFPrintFWV2(_In_  FILE          *lpStream,
                     // @EmptyStringAllowed
                     _In_  const wchar_t *lpMessageFormat,
                     _In_  va_list        ap,
                     _Out_ FILE          *lpErrorStream);

#endif  // H_COMMON_WIN32_ERRNO

