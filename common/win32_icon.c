#include "win32_icon.h"
#include "error_exit.h"
#include <assert.h>

void
Win32IconDimensionsAssertValue(__attribute__((unused))
                               _In_ const struct Win32IconDimensions *lpIconDim)
{
    assert(NULL != lpIconDim);

    assert(lpIconDim->width  > 0);
    assert(lpIconDim->height > 0);

    // Intentional: struct BITMAP uses (signed) LONG: int32_t -> Do not allow larger
    assert(lpIconDim->width  <= INT32_MAX);
    assert(lpIconDim->height <= INT32_MAX);
}

// Ref: "How do I get the dimensions of a cursor or icon?" -> https://devblogs.microsoft.com/oldnewthing/20101020-00/?p=12493
void
Win32IconGetDimensions(_In_  const HICON                 hIcon,
                       _Out_ struct Win32IconDimensions *lpIconDim)
{
    assert(NULL != hIcon);
    assert(NULL != lpIconDim);

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-geticoninfo
    ICONINFO ii = {};
    if (!GetIconInfo(hIcon, &ii))
    {
        ErrorExitW(L"GetIconInfo(hIcon, &ii)");
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmap
    BITMAP bm = {};
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getobject
    const int bmSize = GetObject(ii.hbmMask, sizeof(bm), &bm);
    if (0 == bmSize)
    {
        ErrorExitW(L"GetObject(ii.hbmMask, sizeof(bm), &bm)");
    }

    const LONG width = bm.bmWidth;
    assert(width > 0);

    const LONG height = (NULL == ii.hbmColor) ? (bm.bmHeight / 2) : bm.bmHeight;
    assert(height > 0);

    if (NULL != ii.hbmMask)
    {
        if (!DeleteObject(ii.hbmMask))
        {
            ErrorExitW(L"DeleteObject(ii.hbmMask)");
        }
    }

    if (NULL != ii.hbmColor)
    {
        if (!DeleteObject(ii.hbmColor))
        {
            ErrorExitW(L"DeleteObject(ii.hbmColor)");
        }
    }

    lpIconDim->width  = width;
    lpIconDim->height = height;
}

