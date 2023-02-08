#ifndef H_COMMON_WSTR
#define H_COMMON_WSTR

#include "win32.h"
#include <wchar.h>  // required for wchar_t
#include <sal.h>    // required for _Inout_, etc.
#include <windef.h>  // required for UINT
#include <stdarg.h>  // required for 

void
SafeWCharArrCopy(_Out_ wchar_t       *lpDestWCharArr,                // dest wstr ptr
                 _In_  size_t         ulDestWCharArrLenPlusNulChar,  // dest wstr length (including null char)
                 _In_  const wchar_t *lpSrcWCharArr,                 // source wstr ptr
                 _In_  const size_t   ulWCharCount);                 // wchar count to be copied (excluding null char)

struct WStr
{
    // always terminated with '\0'
    // if 0 == ulSize, lpWCharArr can be NULL or L"" (empty string)
    wchar_t *lpWCharArr;
    // non-negative number of wchars in member 'lpWCharArr', excluding final '\0' char
    // usually: wcslen(lpWCharArr)
    size_t   ulSize;
};

// Intentional: Cast to (const struct WStr) to make compatible with both initialisation and assignment.
// Ref: https://stackoverflow.com/a/30789420/257299
/**
 * @param lpNullableLiteral
 *        must be a string literal, e.g., NULL or L"" or L"abc"
 *
 * @return const struct WStr
 */
#define WSTR_FROM_LITERAL(/* const wchar_t */ lpNullableLiteral) \
    ((const struct WStr) { \
        .lpWCharArr = (wchar_t *) (lpNullableLiteral), \
        .ulSize     = (NULL == (lpNullableLiteral) ? 0 : ((sizeof(lpNullableLiteral) / sizeof(wchar_t)) - 1)) \
    })
//    ((const struct WStr) {.lpWCharArr = (wchar_t *) (lpLiteral), .ulSize = NULL == (lpLiteral) ? 0 : wcslen((lpLiteral))})

/**
 * @param lpNullableValue
 *        must be a string variable
 *        Ex: const wchar_t *lpWCharArr = NULL;
 *            const struct WStr wstr = WSTR_FROM_VALUE(lpWCharArr);
 *        Ex: const wchar_t *lpWCharArr = L"";
 *            const struct WStr wstr = WSTR_FROM_VALUE(lpWCharArr);
 *        Ex: const wchar_t *lpWCharArr = L"abc";
 *            const struct WStr wstr = WSTR_FROM_VALUE(lpWCharArr);
 *
 * @return const struct WStr
 */
#define WSTR_FROM_VALUE(/* const wchar_t */ lpNullableValue) \
    ((const struct WStr) { \
        .lpWCharArr = (wchar_t *) (lpNullableValue), \
        .ulSize     = (NULL == (lpNullableValue) ? 0 : wcslen((lpNullableValue))) \
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

BOOL
WStrIsEmpty(_In_ const struct WStr *lpWStr);

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
WStrSprintf(_Inout_ struct WStr   *lpDestWStr,
            // @EmptyStringAllowed
            _In_    const wchar_t *lpFormatWCharArr, ...);

void
WStrSprintfV(_Inout_ struct WStr   *lpDestWStr,
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

