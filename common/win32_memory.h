#ifndef H_COMMON_WIN32_MEMORY
#define H_COMMON_WIN32_MEMORY

#include <sal.h>      // required for _In_
#include <stdbool.h>  // required for bool
#include <stdio.h>

void
Win32MemoryLocalFree(_In_ void **lppBufferToFree);

bool
Win32MemoryLocalFree2(_In_  void **lppBufferToFree,
                      _Out_ FILE  *lpErrorStream);

#endif  // H_COMMON_WIN32_MEMORY

