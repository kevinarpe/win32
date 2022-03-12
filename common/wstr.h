#ifndef _H_WSTR
#define _H_WSTR

#include "win32.h"
#include <wchar.h>  // required for wchar_t
#include <sal.h>    // required for _Inout_, etc.
#include <windef.h>  // required for UINT

struct WStr
{
    // if 0 == ulSize, lpWCharArr can be NULL or L"" (empty string)
    wchar_t *lpWCharArr;
    // non-negative number of wchars in member 'lpWCharArr', excluding final null char
    size_t   ulSize;
};

struct WStrArr
{
    struct WStr *lpWStrArr;
    size_t       ulSize;
};

void WStrAssertValid(_In_ const struct WStr *lpWStr);

void WStrFree(_Inout_ struct WStr *lpWStr);

void WStrCopyWCharArr(_Inout_ struct WStr   *lpDestWStr,
                      _In_    const wchar_t *lpSrcWCharArr,
                      _In_    const size_t   ulSrcSize);

void WStrCopyWStr(_Inout_ struct WStr       *lpDestWStr,
                  _In_    const struct WStr *lpSrcWStr);

// Maybe: WStrInsert, WStrAppend

enum EWStrTrim
{
    WSTR_LTRIM = 1,
    WSTR_RTRIM = 2,
};

typedef int (*WStrCharPredicateFunc)(_In_ const wchar_t ch);

void WStrTrim(_Inout_ struct WStr                 *lpWStr,
              _In_    const enum EWStrTrim         eWStrTrim,
              _In_    const WStrCharPredicateFunc  fpWStrCharPredicateFunc);

void WStrLTrimSpace(_Inout_ struct WStr *lpWStr);

void WStrRTrimSpace(_Inout_ struct WStr *lpWStr);

void WStrTrimSpace(_Inout_ struct WStr *lpWStr);

void WStrSplit(_In_    const struct WStr *lpWStrText,
               _In_    const struct WStr *lpWStrDelim,
               _In_    const int          iMaxTokenCount,
               _Inout_ struct WStrArr    *lpTokenWStrArr);

void WStrSplitNewLine(_In_    const struct WStr *lpWStrText,
                      _In_    const int          iMaxLineCount,
                      _Inout_ struct WStrArr    *lpTokenWStrArr);

void WStrFileWrite(_In_ const wchar_t     *lpFilePath,
                   _In_ const UINT         codePage,  // Ex: CP_UTF8
                   _In_ const struct WStr *lpWStr);

void WStrFileRead(_In_    const wchar_t *lpFilePath,
                  _In_    const UINT     codePage,  // Ex: CP_UTF8
                  _Inout_ struct WStr   *lpDestWStr);

void WStrArrAssertValid(_In_ const struct WStrArr *lpWStrArr);

void WStrArrFree(_Inout_ struct WStrArr *lpWStrArr);

void WStrArrAlloc(_Inout_ struct WStrArr *lpWStrArr,
                  _In_    const size_t    ulSize);

typedef void (*WStrConsumerFunc)(_Inout_ struct WStr *lpWStr);

void WStrArrForEach(_Inout_ struct WStrArr         *lpWStrArr,
                    _In_    const WStrConsumerFunc  fpWStrConsumerFunc);

#endif  // _H_WSTR

