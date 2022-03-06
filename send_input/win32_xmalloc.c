#include "xmalloc.h"
#include "error_exit.h"
#include <assert.h>
#include <windows.h>

static HANDLE StaticGetProcessHeap()
{
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-getprocessheap
    const HANDLE hProcessHeap = GetProcessHeap();
    if (NULL == hProcessHeap)
    {
        ErrorExit(L"GetProcessHeap()");
    }
    return hProcessHeap;
}

void *xcalloc(size_t ulNumItem, size_t ulSizeOfEachItem)
{
    assert(ulNumItem > 0);
    assert(ulSizeOfEachItem > 0);

    const HANDLE hProcessHeap = StaticGetProcessHeap();

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc
    void *lpData = HeapAlloc(hProcessHeap,
                             HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                             ulNumItem * ulSizeOfEachItem);
    return lpData;
}

void xrealloc(void **lppData, size_t ulNewDataSize)
{
    assert(NULL != lppData);
    assert(NULL != *lppData);
    assert(ulNewDataSize > 0);

    const HANDLE hProcessHeap = StaticGetProcessHeap();

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heaprealloc
    void *lpData = HeapReAlloc(hProcessHeap,
                               HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                               *lppData,
                               ulNewDataSize);
    *lppData = lpData;
}

void xfree(void **lppData)
{
    assert(NULL != lppData);

    if (NULL != *lppData)
    {
        const HANDLE hProcessHeap = StaticGetProcessHeap();

        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapfree
        if (!HeapFree(hProcessHeap, 0, *lppData))
        {
            ErrorExit(L"HeapFree(hProcessHeap, 0, *lppData)");
        }
        *lppData = NULL;
    }
}

