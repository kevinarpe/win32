#ifndef H_COMMON_ERROR_EXIT
#define H_COMMON_ERROR_EXIT

#include <sal.h>      // required for _In_
#include <wchar.h>    // required for wchar_t
#include <stdbool.h>  // required for bool

// TODO: Deprecate me!

/**
 * Ex: "2023-01-31 00:51:45.064 +09:00 ERROR: My error message: Failed with error 1234: Blah, blah, blah"
 * When 0 == GetLastError(), exclude suffix: ": Failed with error ..."
 *
 * @param lpszMsg
 *        may be empty; usually does NOT end with newline: L"\r\n"
 *        Ex: L"My error message"
 */
void
ErrorExitW(// @EmptyStringAllowed
           _In_ const wchar_t *lpszMsg);

/**
 * Ex: "2023-01-31 00:51:45.064 +09:00 ERROR: My error message: Failed with error 1234: Blah, blah, blah"
 * When 0 == GetLastError(), exclude suffix: ": Failed with error ..."
 *
 * @param lpszMsgFmt
 *        may be empty; usually does NOT end with newline: L"\r\n"
 *        Ex: L"my error message: %d %d %c"
 *
 * @param ...
 *        zero or more args used by {@code lpszMsgFmt}
 *        Ex: 123, -456, 'b'
 */
void
ErrorExitWF(// @EmptyStringAllowed
            _In_ const wchar_t *lpszMsgFmt, ...);

/**
 * Ex: "2023-01-31 00:51:45.064 +09:00 ERROR: My error message: Failed with error 1234: Blah, blah, blah"
 * When 0 == GetLastError(), exclude suffix: ": Failed with error ..."
 *
 * @param lpszMsgFmt
 *        may be empty; usually does NOT end with newline: L"\r\n"
 *        Ex: L"my error message: %d %d %c"
 *
 * @param ap
 *        zero or more args used by {@code lpszMsgFmt}
 *        Ex: 123, -456, 'b'
 */
void
ErrorExitWFV(// @EmptyStringAllowed
             _In_ const wchar_t *lpszMsgFmt,
             _In_ va_list        ap);

#endif  // H_COMMON_ERROR_EXIT

