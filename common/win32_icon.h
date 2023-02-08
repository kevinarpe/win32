#ifndef H_COMMON_WIN32_ICON
#define H_COMMON_WIN32_ICON

#include "win32.h"
#include <stdint.h>  // required for uint32_t

struct Win32IconDimensions
{
    // Note: struct BITMAP uses (signed) LONG: int32_t
    // However, official docs say width and height are always positive!
    uint32_t width;
    uint32_t height;
};

void
Win32IconDimensionsAssertValue(_In_ const struct Win32IconDimensions *lpIconDim);

void
Win32IconGetDimensions(_In_  const HICON                 hIcon,
                       _Out_ struct Win32IconDimensions *lpIconDim);

#endif  // H_COMMON_WIN32_ICON

