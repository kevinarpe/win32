#include "xmalloc.h"
#include "win32_last_error.h"
#include <assert.h>
#include <windows.h>

static HANDLE
StaticGetProcessHeap()
{
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-getprocessheap
    const HANDLE hProcessHeap = GetProcessHeap();
    if (NULL == hProcessHeap)
    {
        Win32LastErrorFPutWSAbort(stderr,                // _In_ FILE          *lpStream
                                  L"GetProcessHeap()");  // _In_ const wchar_t *lpMessage
    }
    return hProcessHeap;
}

void *
xcalloc(_In_ const size_t ulNumItem,
        _In_ const size_t ulSizeOfEachItem)
{
    assert(ulNumItem > 0);
    assert(ulSizeOfEachItem > 0);

    const HANDLE hProcessHeap = StaticGetProcessHeap();

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc
    void *lpData = HeapAlloc(hProcessHeap,                                 // [in] HANDLE hHeap
                             HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,  // [in] DWORD  dwFlags
                             ulNumItem * ulSizeOfEachItem);                // [in] SIZE_T dwBytes
    return lpData;
}

void
xrealloc(_Inout_ void         **lppData,
         _In_    const size_t   ulNewDataSize)
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

void
xfree(_Inout_ void **lppData)
{
    assert(NULL != lppData);

    if (NULL != *lppData)
    {
        const HANDLE hProcessHeap = StaticGetProcessHeap();

        // Ref: https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapfree
        if (!HeapFree(hProcessHeap, 0, *lppData))
        {
            Win32LastErrorFPrintFWAbort(stderr,                            // _In_ FILE          *lpStream
                                        L"HeapFree(hProcessHeap:%p, 0, *lppData:%p/%p)",  // _In_ const wchar_t *lpMessageFormat
                                        hProcessHeap, lppData, *lppData);  // ...
        }
        *lppData = NULL;
    }
}

