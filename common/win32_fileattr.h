#ifndef H_COMMON_WIN32_FILEATTR
#define H_COMMON_WIN32_FILEATTR

#include "win32.h"
#include "visit.h"
#include "wstr.h"
#include <wchar.h>  // required for wchar_t
#include <sal.h>    // required for _Inout_, etc.

typedef enum EVisitNext
(*Win32FileAttrVisitFunc)(_In_    const DWORD    dwFileAttr,
                          _In_    const wchar_t *lpFileAttrWCharArr,
                          _Inout_ void          *lpData);

void
Win32FileAttrMaskVisitEach(_In_    const DWORD              dwFileAttrMask,
                           _In_    const Win32FileAttrVisitFunc  fpFileAttrVisitFunc,
                           _Inout_ void                    *lpData);

void
Win32FileAttrMaskToWStrArr(_In_    const DWORD     dwFileAttrMask,
                           _Inout_ struct WStrArr *lpWStrArr);

#endif  // H_COMMON_WIN32_FILEATTR

