#include "xmalloc.h"
#include "error_exit.h"
#include <assert.h>
#include <stdlib.h>

void *xcalloc(size_t ulNumItem, size_t ulSizeOfEachItem)
{
    assert(ulNumItem > 0);
    assert(ulSizeOfEachItem > 0);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/calloc?view=msvc-170
    void *lpData = calloc(ulNumItem, ulSizeOfEachItem);
    if (NULL == lpData)
    {
        const int iErrno = errno;
        _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                     L"calloc(ulNumItem=%zd, ulSizeOfEachItem=%zd) failed: %ls",
                     ulNumItem, ulSizeOfEachItem, _wcserror(iErrno));
        ErrorExit(g_lpErrorMsgBuffer);
    }
    return lpData;
}

void xrealloc(void **lppData, size_t ulNewDataSize)
{
    assert(NULL != lppData);
    assert(NULL != *lppData);
    assert(ulNewDataSize > 0);

    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/realloc?view=msvc-170
    void *lpData = realloc(*lppData, ulNewDataSize);
    if (NULL == lpData)
    {
        const int iErrno = errno;
        _snwprintf_s(g_lpErrorMsgBuffer, g_ulErrorMsgBufferSize, g_ulErrorMsgBufferSize,
                     L"realloc(ulNewDataSize=%zd) failed: %ls",
                     ulNewDataSize, _wcserror(iErrno));
        ErrorExit(g_lpErrorMsgBuffer);
    }
    *lppData = lpData;
}

void xfree(void **lppData)
{
    assert(NULL != lppData);

    if (NULL != *lppData)
    {
        // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/free?view=msvc-170
        free(*lppData);
        *lppData = NULL;
    }
}

