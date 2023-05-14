#include "win32_fileattr.h"
#include "xmalloc.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW
#include <windows.h>
#include <stdio.h>

void
Win32FileAttrMaskVisitEach(_In_    const DWORD                   dwFileAttrMask,
                           _In_    const Win32FileAttrVisitFunc  fpFileAttrVisitFunc,
                           _Inout_ void                         *lpData)
{
    assert(NULL != fpFileAttrVisitFunc);
    assert(NULL != lpData);

    if (INVALID_FILE_ATTRIBUTES == dwFileAttrMask)
    {
        fpFileAttrVisitFunc(INVALID_FILE_ATTRIBUTES, L"INVALID_FILE_ATTRIBUTES", lpData);
        return;
    }

#define M_FILE_ATTR(fileAttr) \
    do { \
        if (fileAttr & dwFileAttrMask) \
        { \
            if (VISIT_NEXT_NO == fpFileAttrVisitFunc(fileAttr, L ## #fileAttr, lpData)) \
            { \
                return; \
            } \
        } \
    } \
    while (0)

    // Source: winnt.h
    M_FILE_ATTR(FILE_ATTRIBUTE_READONLY);  // 0x1
    M_FILE_ATTR(FILE_ATTRIBUTE_HIDDEN);  // 0x2
    M_FILE_ATTR(FILE_ATTRIBUTE_SYSTEM);  // 0x4
    M_FILE_ATTR(FILE_ATTRIBUTE_DIRECTORY);  // 0x10
    M_FILE_ATTR(FILE_ATTRIBUTE_ARCHIVE);  // 0x20
    M_FILE_ATTR(FILE_ATTRIBUTE_DEVICE);  // 0x40
    M_FILE_ATTR(FILE_ATTRIBUTE_NORMAL);  // 0x80
    M_FILE_ATTR(FILE_ATTRIBUTE_TEMPORARY);  // 0x100
    M_FILE_ATTR(FILE_ATTRIBUTE_SPARSE_FILE);  // 0x200
    M_FILE_ATTR(FILE_ATTRIBUTE_REPARSE_POINT);  // 0x400
    M_FILE_ATTR(FILE_ATTRIBUTE_COMPRESSED);  // 0x800
    M_FILE_ATTR(FILE_ATTRIBUTE_OFFLINE);  // 0x1000
    M_FILE_ATTR(FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);  // 0x2000
    M_FILE_ATTR(FILE_ATTRIBUTE_ENCRYPTED);  // 0x4000
//    M_FILE_ATTR(FILE_ATTRIBUTE_INTEGRITY_STREAM);  // 0x8000
    #ifdef FILE_ATTRIBUTE_INTEGRITY_STREAM
        M_FILE_ATTR(FILE_ATTRIBUTE_INTEGRITY_STREAM);
    #else
        #define FILE_ATTRIBUTE_INTEGRITY_STREAM 0x8000
        M_FILE_ATTR(FILE_ATTRIBUTE_INTEGRITY_STREAM);
        #undef FILE_ATTRIBUTE_INTEGRITY_STREAM
    #endif
    M_FILE_ATTR(FILE_ATTRIBUTE_VIRTUAL);  // 0x10000
//    M_FILE_ATTR(FILE_ATTRIBUTE_NO_SCRUB_DATA);  // 0x20000
    #ifdef FILE_ATTRIBUTE_NO_SCRUB_DATA
        M_FILE_ATTR(FILE_ATTRIBUTE_NO_SCRUB_DATA);
    #else
        #define FILE_ATTRIBUTE_NO_SCRUB_DATA 0x20000
        M_FILE_ATTR(FILE_ATTRIBUTE_NO_SCRUB_DATA);
        #undef FILE_ATTRIBUTE_NO_SCRUB_DATA
    #endif
    // Undoc'd, but exists in winnt.h
//    M_FILE_ATTR(FILE_ATTRIBUTE_EA);  // 0x
    #ifdef FILE_ATTRIBUTE_RECALL_ON_OPEN
        M_FILE_ATTR(FILE_ATTRIBUTE_RECALL_ON_OPEN);
    #else
        #define FILE_ATTRIBUTE_RECALL_ON_OPEN 0x40000
        M_FILE_ATTR(FILE_ATTRIBUTE_RECALL_ON_OPEN);
        #undef FILE_ATTRIBUTE_RECALL_ON_OPEN
    #endif

    #ifdef FILE_ATTRIBUTE_PINNED
        M_FILE_ATTR(FILE_ATTRIBUTE_PINNED);
    #else
        #define FILE_ATTRIBUTE_PINNED 0x80000
        M_FILE_ATTR(FILE_ATTRIBUTE_PINNED);
        #undef FILE_ATTRIBUTE_PINNED
    #endif

    #ifdef FILE_ATTRIBUTE_UNPINNED
        M_FILE_ATTR(FILE_ATTRIBUTE_UNPINNED);
    #else
        #define FILE_ATTRIBUTE_UNPINNED 0x100000
        M_FILE_ATTR(FILE_ATTRIBUTE_UNPINNED);
        #undef FILE_ATTRIBUTE_UNPINNED
    #endif

    #ifdef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
        M_FILE_ATTR(FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS);
    #else
        #define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x400000
        M_FILE_ATTR(FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS);
        #undef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
    #endif

#undef M_FILE_ATTR
}

static enum EVisitNext
StaticFileAttrVisitCount(__attribute__ ((unused)) _In_    const DWORD    dwFileAttr,
                         __attribute__ ((unused)) _In_    const wchar_t *lpFileAttrWCharArr,
                                                  _Inout_ void          *lpData)
{
    size_t *lpulCount = (size_t *) lpData;
    ++(*lpulCount);

    return VISIT_NEXT_YES;
}

struct AppendData
{
    struct WStrArr *lpWStrArr;
    size_t ulNextIndex;
};

static enum EVisitNext
StaticFileAttrVisitAppend(__attribute__ ((unused))
                          _In_    const DWORD    dwFileAttr,
                          _In_    const wchar_t *lpFileAttrWCharArr,
                          _Inout_ void          *lpData)
{
    struct AppendData *appendData = (struct AppendData *) lpData;
    struct WStr fileAttrWStr = {};
    WStrCopyWCharArr(&fileAttrWStr, lpFileAttrWCharArr, wcslen(lpFileAttrWCharArr));

    struct WStr *lpWStr = appendData->lpWStrArr->lpWStrArr + appendData->ulNextIndex;
    *lpWStr = fileAttrWStr;
    ++(appendData->ulNextIndex);

    return VISIT_NEXT_YES;
}

void
Win32FileAttrMaskToWStrArr(_In_    const DWORD     dwFileAttrMask,
                           _Inout_ struct WStrArr *lpWStrArr)
{
    assert(NULL != lpWStrArr);

    WStrArrFree(lpWStrArr);

    size_t ulCount = 0;
    Win32FileAttrMaskVisitEach(dwFileAttrMask, &StaticFileAttrVisitCount, &ulCount);

    WStrArrAlloc(lpWStrArr, ulCount);
    struct AppendData appendData = {.lpWStrArr = lpWStrArr, .ulNextIndex = 0};
    
    Win32FileAttrMaskVisitEach(dwFileAttrMask, &StaticFileAttrVisitAppend, &appendData);
    assert(lpWStrArr->ulSize == appendData.ulNextIndex);
}

