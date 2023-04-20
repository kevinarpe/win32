#ifndef H_COMMON_LOG
#define H_COMMON_LOG

#include <sal.h>    // required for _In_
#include <wchar.h>  // required for wchar_t
#include <stdio.h>  // required for FILE

#ifdef NDEBUG
    #define DEBUG_LOGW(/* FILE * */ fp, /* @EmptyStringAllowed const wchar_t * */ lpszMsg) \
        ((void) 0)

    #define DEBUG_LOGWF(/* FILE * */ fp, /* @EmptyStringAllowed const wchar_t * */ lpszMsgFmt, ...) \
        ((void) 0)

    #define DEBUG_LOGWFV(/* FILE * */ fp, /* @EmptyStringAllowed const wchar_t * */ lpszMsgFmt, /* va_list */ ap) \
        ((void) 0)
#else
    #define DEBUG_LOGW(/* FILE * */ fp, /* @EmptyStringAllowed const wchar_t * */ lpszMsg) \
        LogW(fp, lpszMsg)

    #define DEBUG_LOGWF(/* FILE * */ fp, /* @EmptyStringAllowed const wchar_t * */ lpszMsgFmt, ...) \
        LogWF(fp, lpszMsgFmt, __VA_ARGS__)

    #define DEBUG_LOGWFV(/* FILE * */ fp, /* @EmptyStringAllowed const wchar_t * */ lpszMsgFmt, /* va_list */ ap) \
        LogWFV(fp, lpszMsgFmt, ap)
#endif  // NDEBUG

/**
 * fprintf() timestamp, then fputws() message.  Newline must be explicitly included.
 * Example timestamp: "2022-03-10 22:17:47.123 +09:00 "
 *
 * @param fp
 *        usually stdout or stderr
 *
 * @param lpszMsg
 *        may be empty; usually ends with newline: L"\r\n"
 *        ex: L"my log message\r\n"
 */
void
LogW(_In_ FILE          *fp,
     // @EmptyStringAllowed
     _In_ const wchar_t *lpszMsg);

/**
 * fprintf() timestamp, then vfwprintf() message.  Newline must be explicitly included.
 * Example timestamp: "2022-03-10 22:17:47.123 +09:00 "
 *
 * @param fp
 *        usually stdout or stderr
 *
 * @param lpszMsg
 *        may be empty; usually ends with newline: L"\r\n"
 *        ex: L"my log message: %d %d %c\r\n"
 *
 * @param ...
 *        zero or more args for lpszMsgFmt
 *        ex: 5, 7, 'z'
 */
void
LogWF(_In_ FILE          *fp,
      // @EmptyStringAllowed
      _In_ const wchar_t *lpszMsgFmt,
      _In_ ...);

/**
 * fprintf() timestamp, then vfwprintf() message.  Newline must be explicitly included.
 * Example timestamp: "2022-03-10 22:17:47.123 +09:00 "
 *
 * @param fp
 *        usually stdout or stderr
 *
 * @param lpszMsg
 *        may be empty; usually ends with newline: L"\r\n"
 *        ex: L"my log message: %d %d %c\r\n"
 *
 * @param ap
 *        zero or more args for lpszMsgFmt
 *        ex: 5, 7, 'z'
 */
void
LogWFV(_In_ FILE          *fp,
       // @EmptyStringAllowed
       _In_ const wchar_t *lpszMsgFmt,
       _In_ va_list        ap);

#endif  // H_COMMON_LOG

