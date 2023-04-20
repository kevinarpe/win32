#include "win32_memory.h"
#include "win32.h"
#include "log.h"
#include <assert.h>
#include <stdlib.h>

void
Win32MemoryLocalFree(_In_ void **lppBufferToFree)
{
    if (false == Win32MemoryLocalFree2(lppBufferToFree,  // _In_  void **lppBufferToFree
                                       stderr))          // _Out_ FILE  *lpErrorStream
    {
        abort();
    }
}

bool
Win32MemoryLocalFree2(_In_  void **lppBufferToFree,
                      _Out_ FILE  *lpErrorStream)
{
    assert(NULL != lppBufferToFree);
    assert(NULL != *lppBufferToFree);
    assert(NULL != lpErrorStream);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localfree
    // "If the function succeeds, the return value is NULL.
    //  If the function fails, the return value is equal to a handle to the local memory object."
    if (*lppBufferToFree == LocalFree(*lppBufferToFree))
    {
        LogW(lpErrorStream, L"ERROR: LocalFree(...)");
        return false;
    }
    *lppBufferToFree = NULL;
    return true;
}

