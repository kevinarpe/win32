#ifndef _H_XMALLOC
#define _H_XMALLOC

#include <stddef.h>  // required for size_t

// These function names are inspired by Git.

void *xcalloc(size_t ulNumItem, size_t ulSizeOfEachItem);

void xrealloc(void **lppData, size_t ulNewDataSize);

void xfree(void **lppData);

#endif  // _H_XMALLOC

