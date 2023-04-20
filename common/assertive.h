#ifndef H_COMMON_ASSERTIVE
#define H_COMMON_ASSERTIVE

#include <sal.h>      // required for _In_
#include <stdbool.h>  // required for bool
#include <wchar.h>    // required for wchar_t
#include <stdarg.h>   // required for va_list

/**
 * If {@code bAssertResult} is {@code false}, then log message via {@link LogWF()}
 * to {@link stderr} and call {@link abort()}.
 * Sample log message: L"Assert failed: a > b\r\n"
 *
 * @param bAssertResult
 *        boolean condition to assert is true
 *        Ex: assume 'a' and 'b' are ints: {@code a > b}
 *
 * @param lpMessageWCharArr
 *        message format string (excluding trailing newline) to log when assert fails
 *        Ex: {@code L"a > b"}
 */
void
AssertW(_In_ const bool     bAssertResult,
        // @EmptyStringAllowed
        _In_ const wchar_t *lpMessageWCharArr);

/**
 * If {@code bAssertResult} is {@code false}, then log message via {@link LogWF()}
 * to {@link stderr} and call {@link abort()}.
 * Sample log message: L"Assert failed: a:87 > b:42\r\n"
 *
 * @param bAssertResult
 *        boolean condition to assert is true
 *        Ex: assume 'a' and 'b' are ints: {@code a > b}
 *
 * @param lpMessageFormatWCharArr
 *        message format string (excluding trailing newline) to log when assert fails
 *        Ex: {@code L"a:%d > b:%d"}
 *
 * @param ...
 *        zero or more arguments for {@code lpMessageFormatWCharArr}
 *        Ex: {@code 87, 42}
 */
void
AssertWF(_In_ const bool     bAssertResult,
         // @EmptyStringAllowed
         _In_ const wchar_t *lpMessageFormatWCharArr,
         _In_ ...);

void
AssertWFV(_In_ const bool     bAssertResult,
          // @EmptyStringAllowed
          _In_ const wchar_t *lpMessageFormatWCharArr,
          _In_ va_list        ap);

#endif  // H_COMMON_ASSERTIVE

