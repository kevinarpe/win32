#ifndef H_COMMON_XMALLOC
#define H_COMMON_XMALLOC

#include "win32.h"
#include <stddef.h>  // required for size_t

// These function names are inspired by Git.

void *
xcalloc(_In_ const size_t ulNumItem,
        _In_ const size_t ulSizeOfEachItem);

void
xrealloc(_Inout_ void         **lppData,
         _In_    const size_t   ulNewDataSize);

void
xfree(_Inout_ void **lppData);

#endif  // H_COMMON_XMALLOC

