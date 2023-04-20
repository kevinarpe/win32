#ifndef H_COMMON_WSTR
#define H_COMMON_WSTR

#include "win32.h"
#include <wchar.h>  // required for wchar_t
#include <sal.h>    // required for _Inout_, etc.
#include <windef.h>  // required for UINT
#include <stdarg.h>  // required for 

/**
 * @param lpDestWCharArr
 *        text is copied to this buffer
 *
 * @param ulDestWCharArrLenPlusNulChar
 *        number of wchar_t's in {@code lpDestWCharArr}, including trailing null char
 *
 * @param lpSrcWCharArr
 *        text is copied from this buffer
 *
 * @param ulWCharCount
 *        number of wchar_t's to copy from {@code lpSrcWCharArr}, excluding trailing null char
 *        <br>if special value {@code _TRUNCATE}, then copy as many wchar_t's as possible
 */
void
SafeWCharArrCopy(_Out_ wchar_t       *lpDestWCharArr,
                 _In_  size_t         ulDestWCharArrLenPlusNullChar,
                 _In_  const wchar_t *lpSrcWCharArr,
                 _In_  const size_t   ulWCharCount);

enum EWStrAlloc
{
    /**
     * {@code struct WStr} is uninitialised.
     *
     * Read  : Yes -- It is safe to read from this string.
     * Write : No  -- It is not safe to write to this string that might be a string literal.
     * Malloc: Yes -- It is safe to malloc new memory and assign to {@code WStr#lpWCharArr}.
     * Free  : No  -- It is not safe to free this string that might be a string literal.
     *
     * Intentional: Value is zero to support: {@code struct WStr wstr = {0};}
     */
    WSTR_ALLOC_UNKNOWN = 0,
    /**
     * {@see struct WStr#lpWCharArr} is a string literal, e.g.,
     * {@code
     * const wchar_t *lpWCharArr = "read-only string literal";
     * struct WStr wstr = {.lpWCharArr = (wchar_t *) lpWCharArr, .ulSize = strlen(lpWCharArr), .allocFlag = WSTR_ALLOC_LITERAL};}
     * // Or:
     * #define STRING_LITERAL "read-only string literal"
     * struct WStr wstr = {.lpWCharArr = (wchar_t *) STRING_LITERAL, .ulSize = sizeof(STRING_LITERAL), .allocFlag = WSTR_ALLOC_LITERAL};}
     *
     * Read  : Yes -- It is safe to read from this string.
     * Write : No  -- It is not safe to write to this string literal.
     * Malloc: No  -- Do not malloc new memory and assign to {@code WStr#lpWCharArr}.
     * Free  : No  -- It is not safe to free this string literal.
     */
    WSTR_ALLOC_LITERAL = 1,
    /**
     * {@see struct WStr#lpWCharArr} is stack-allocated, e.g.,
     * {@code
     * // Read-only string literal "stack-..." is copied into stack-allocated string buffer 'lpWCharArr'.
     * wchar_t lpWCharArr[] = "stack-allocated string buffer";
     * struct WStr wstr = {.lpWCharArr = lpWCharArr, .ulSize = (sizeof(lpWCharArr) / sizeof(wchar_t)) - 1, .allocFlag = WSTR_ALLOC_STACK};}
     *
     * Read  : Yes -- It is safe to read from this string.
     * Write : Yes -- It is safe to write to this stack-allocated string buffer.
     * Malloc: No  -- Do not malloc new memory and assign to {@code WStr#lpWCharArr}.
     * Free  : No  -- It is not safe to free this stack-allocated string buffer.
     */
    WSTR_ALLOC_STACK   = 2,
    /**
     * {@see struct WStr#lpWCharArr} is heap-allocated, e.g.,
     * {@code
     * const wchar_t *lpWCharArr = "heap-allocated string buffer";
     * wchar_t *lpWCharArr2 = malloc(sizeof(wchar_t) * (1 + strlen(lpWCharArr)));
     * strcpy(lpWCharArr2, lpWCharArr);
     * struct WStr wstr = {.lpWCharArr = lpWCharArr2, .ulSize = strlen(lpWCharArr2), .allocFlag = WSTR_ALLOC_HEAP};}
     *
     * Read  : Yes -- It is safe to read from this string.
     * Write : Yes -- It is safe to write to this heap-allocated string buffer.
     * Malloc: Yes -- It is safe to malloc new memory and assign to {@code WStr#lpWCharArr}.
     * Free  : No  -- It is safe to free this heap-allocated string buffer.
     */
    WSTR_ALLOC_HEAP    = 3,
};

struct WStr
{
    /**
     * always terminated with '\0'
     * if 0 == ulSize, lpWCharArr can be NULL or L"" (empty string)
     */
    wchar_t         *lpWCharArr;
    /**
     * non-negative number of wchars in member 'lpWCharArr', excluding final '\0' char
     * usually: wcslen(lpWCharArr)
     */
    size_t           ulSize;
    /** Note: struct WStr wstr = {0} -> WSTR_ALLOC_UNKNOWN */
    enum EWStrAlloc  allocFlag;
};

// Intentional: Cast to (const struct WStr) to make compatible with both initialisation and assignment.
// Ref: https://stackoverflow.com/a/30789420/257299
/**
 * @param lpNullableLiteral
 *        must be a string literal, e.g., NULL or L"" or L"abc"
 *
 * @return const struct WStr
 */
#define WSTR_FROM_LITERAL(/* wchar_t* */ lpNullableLiteral) \
    ((const struct WStr) { \
        .lpWCharArr = (wchar_t *) (lpNullableLiteral), \
        .ulSize     = (NULL == (lpNullableLiteral) ? 0 : ((sizeof(*lpNullableLiteral) / sizeof(wchar_t)) - 1)), \
        .allocFlag  = WSTR_ALLOC_LITERAL \
    })

/**
 * @param wcharArr
 *        must be a string literal, e.g., NULL or L"" or L"abc"
 *
 * @return const struct WStr
 */
#define WSTR_FROM_STACK(/* wchar_t[] */ wcharArr) \
    ((const struct WStr) { \
        .lpWCharArr = (wchar_t *) (wcharArr), \
        .ulSize     = ((sizeof(wcharArr) / sizeof(wchar_t)) - 1), \
        .allocFlag  = WSTR_ALLOC_STACK \
    })

/**
 * @return const struct WStr
 */
#define WSTR_FROM_ANY(/* wchar_t* */ lpNullableValue, /* enum EWStrAlloc */ allocFlag) \
    ((const struct WStr) { \
        .lpWCharArr = (wchar_t *) (lpNullableValue), \
        .ulSize     = (NULL == (lpNullableValue) ? 0 : wcslen((lpNullableValue))), \
        .allocFlag  = allocFlag \
    })

struct WStrArr
{
    struct WStr *lpWStrArr;
    size_t       ulSize;
};

void
WStrAssertValid(_In_ const struct WStr *lpWStr);

void
WStrFree(_Inout_ struct WStr *lpWStr);

bool
WStrIsEmpty(_In_ const struct WStr *lpWStr);

/**
 * To compare against a string literal, try:
 * <pre>{@code
 * struct WStr leftWStr = ...;
 * if (0 == WStrCompare(&leftWStr, &WSTR_FROM_LITERAL(L"my string literal"))) ...
 * }</pre>
 */
int
WStrCompare(_In_ const struct WStr *lpWStrLeft,
            _In_ const struct WStr *lpWStrRight);

void
WStrCopyWCharArr(_Inout_ struct WStr   *lpDestWStr,
                 _In_    const wchar_t *lpSrcWCharArr,
                 _In_    const size_t   ulSrcSize);

void
WStrCopyWStr(_Inout_ struct WStr       *lpDestWStr,
             _In_    const struct WStr *lpSrcWStr);

void
WStrSPrintF(_Inout_ struct WStr   *lpDestWStr,
            // @EmptyStringAllowed
            _In_    const wchar_t *lpFormatWCharArr, ...);

    // Ref: https://learn.microsoft.com/ja-jp/cpp/c-runtime-library/reference/sprintf-sprintf-l-swprintf-swprintf-l-swprintf-l?view=msvc-170
    // "The functions return the number of wide characters written, excluding the terminating null wide character
    //  in case of the functions swprintf() and vswprintf().  They return -1 when an error occurs."
bool
WStrSPrintF2(_Inout_ struct WStr   *lpDestWStr,
             _In_    FILE          *lpErrorStream,
             // @EmptyStringAllowed
             _In_    const wchar_t *lpFormatWCharArr, ...);

void
WStrSPrintFV(_Inout_ struct WStr   *lpDestWStr,
             // @EmptyStringAllowed
             _In_    const wchar_t *lpFormatWCharArr,
             _In_    va_list        ap);

void
WStrSPrintFV(_Inout_ struct WStr   *lpDestWStr,
             // @EmptyStringAllowed
             _In_    const wchar_t *lpFormatWCharArr,
             _In_    va_list        ap);

bool
WStrSPrintFV2(_Inout_ struct WStr   *lpDestWStr,
              _In_    FILE          *lpErrorStream,
              // @EmptyStringAllowed
              _In_    const wchar_t *lpFormatWCharArr,
              _In_    va_list        ap);

/**
 * Concatenate two WStr objects.
 *
 * @param lpDestWStr
 *        destination Wstr.  If non-empty, free, then malloc new WStr.
 * 
 * @param lpSrcWStr
 *        first WStr to concatenate
 * 
 * @param lpSrcWStr2
 *        second WStr to concatenate
 */
void
WStrConcat(_Inout_ struct WStr       *lpDestWStr,
           _In_    const struct WStr *lpSrcWStr,
           _In_    const struct WStr *lpSrcWStr2);

/**
 * Concatenate two or more WStr objects.
 *
 * @param lpDestWStr
 *        destination Wstr.  If non-empty, free, then malloc new WStr.
 * 
 * @param lpSrcWStr
 *        first WStr to concatenate
 * 
 * @param lpSrcWStr2
 *        second WStr to concatenate
 * 
 * @param ...
 *        zero or more additional WStr objects to concatenate.
 *        Last value must be NULL.
 */
void
WStrConcatMany(_Inout_ struct WStr       *lpDestWStr,
               _In_    const struct WStr *lpSrcWStr,
               _In_    const struct WStr *lpSrcWStr2, ...);

// Maybe: WStrInsert, WStrAppend

enum EWStrTrim
{
    WSTR_LTRIM = 1,
    WSTR_RTRIM = 2,
};

typedef int (*WStrCharPredicateFunc)(_In_ const wchar_t ch);

void
WStrTrim(_Inout_ struct WStr                 *lpWStr,
         _In_    const enum EWStrTrim         eWStrTrim,
         _In_    const WStrCharPredicateFunc  fpWStrCharPredicateFunc);

void
WStrLTrimSpace(_Inout_ struct WStr *lpWStr);

void
WStrRTrimSpace(_Inout_ struct WStr *lpWStr);

void
WStrTrimSpace(_Inout_ struct WStr *lpWStr);

extern const int UNLIMITED_MIN_TOKEN_COUNT;
extern const int UNLIMITED_MAX_TOKEN_COUNT;

// Matches WStrTrimSpace(), etc.
typedef void (*WStrConsumerFunc)(_Inout_ struct WStr *lpWStr);

struct WStrSplitOptions
{
    int              iMinTokenCount;  // UNLIMITED_MIN_TOKEN_COUNT for unlimited
    int              iMaxTokenCount;  // UNLIMITED_MAX_TOKEN_COUNT for unlimited
    WStrConsumerFunc fpNullableWStrConsumerFunc;
};

void
WStrSplit(_In_    const struct WStr             *lpWStrText,
          _In_    const struct WStr             *lpWStrDelim,
          _In_    const struct WStrSplitOptions *lpOptions,
          _Inout_ struct WStrArr                *lpTokenWStrArr);

void
WStrSplitNewLine(_In_    const struct WStr             *lpWStrText,
                 _In_    const struct WStrSplitOptions *lpOptions,
                 _Inout_ struct WStrArr                *lpTokenWStrArr);

void
WStrJoin(_In_    const struct WStrArr *lpWStrArr,
         _In_    const struct WStr    *lpDelimWStr,
         _Inout_ struct WStr          *lpDestWStr);

void
WStrFileWrite(_In_ const wchar_t     *lpFilePath,
              _In_ const UINT         codePage,  // Ex: CP_UTF8
              _In_ const struct WStr *lpWStr);

void
WStrFileRead(_In_    const wchar_t *lpFilePath,
             _In_    const UINT     codePage,  // Ex: CP_UTF8
             _Inout_ struct WStr   *lpDestWStr);

void
WStrArrAssertValid(_In_ const struct WStrArr *lpWStrArr);

void
WStrArrFree(_Inout_ struct WStrArr *lpWStrArr);

void
WStrArrAlloc(_Inout_ struct WStrArr *lpWStrArr,
             _In_    const size_t    ulSize);

void
WStrArrCopyWCharArrArr(_Inout_ struct WStrArr  *lpDestWStrArr,
                       _In_    const wchar_t  **lppSrcWCharArrArr,
                       _In_    const size_t     ulSrcCount);

void
WStrArrForEach(_Inout_ struct WStrArr         *lpWStrArr,
               _In_    const WStrConsumerFunc  fpWStrConsumerFunc);

#endif  // H_COMMON_WSTR

