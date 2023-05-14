#ifndef H_COMMON_WIN32_CLIPBOARD
#define H_COMMON_WIN32_CLIPBOARD

#include "wstr.h"

/**
 * This is a convenience function to call {@link Win32ClipboardClear()}.
 * If result is {@code false}, call {@link abort()}.
 */
void
Win32ClipboardClearAbort();

/**
 * This is a convenience function to call {@link Win32ClipboardClear2()}
 * where {@code lpErrorStream} is {@code stderr}.
 */
bool
Win32ClipboardClear();

/**
 * @param lpErrorStream
 *        on error, message is logged to this stream
 *
 * @return {@code false} on failure to clear clipboard
 */
bool
Win32ClipboardClear2(_Out_ FILE *lpErrorStream);

/**
 * This is a convenience function to call {@link Win32ClipboardReadWStr()}.
 * If result is {@code false}, call {@link abort()}.
 */
void
Win32ClipboardReadWStrAbort(// @EmptyStringAllowed
                            _Inout_ struct WStr *lpWStr);

/**
 * This is a convenience function to call {@link Win32ClipboardReadWStr2()}
 * where {@code lpErrorStream} is {@code stderr}.
 */
bool
Win32ClipboardReadWStr(// @EmptyStringAllowed
                       _Inout_ struct WStr *lpWStr);

/**
 * @param lpWStr
 *        pointer to result; may be empty string upon return
 *
 * @param lpErrorStream
 *        on error, message is logged to this stream
 *
 * @return {@code false} on error
 */
bool
Win32ClipboardReadWStr2(// @EmptyStringAllowed
                        _Inout_ struct WStr *lpWStr,
                        _Out_   FILE        *lpErrorStream);

/**
 * This is a convenience function to call {@link Win32ClipboardWriteWStr()}.
 * If result is {@code false}, call {@link abort()}.
 */
void
Win32ClipboardWriteWStrAbort(// @Nullable
                             _In_ HWND               hNullableWnd,
                             _In_ const struct WStr *lpWStr);

/**
 * This is a convenience function to call {@link Win32ClipboardReadWStr2()}
 * where {@code lpErrorStream} is {@code stderr}.
 */
bool
Win32ClipboardWriteWStr(// @Nullable
                        _In_  HWND               hNullableWnd,
                        _In_  const struct WStr *lpWStr);

/**
 * @param hNullableWnd
 *        nullable HWND to pass to {@link OpenClipboard()}
 *
 * @param lpWStr
 *        text to write to clipboard
 *
 * @param lpErrorStream
 *        on error, message is logged to this stream
 *
 * @return {@code false} on error
 */
bool
Win32ClipboardWriteWStr2(// @Nullable
                         _In_  HWND               hNullableWnd,
                         _In_  const struct WStr *lpWStr,
                         _Out_ FILE              *lpErrorStream);

#endif  // H_COMMON_WIN32_CLIPBOARD

