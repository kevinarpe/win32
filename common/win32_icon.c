#include "win32_icon.h"
#include "win32_last_error.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW

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
        Win32LastErrorFPutWSAbort(stderr,                       // _In_ FILE          *lpStream
                                  L"GetIconInfo(hIcon, &ii)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmap
    BITMAP bm = {};
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getobject
    const int bmSize = GetObject(ii.hbmMask, sizeof(bm), &bm);
    if (0 == bmSize)
    {
        Win32LastErrorFPutWSAbort(stderr,                                      // _In_ FILE          *lpStream
                                  L"GetObject(ii.hbmMask, sizeof(bm), &bm)");  // _In_ const wchar_t *lpMessage
    }

    const LONG width = bm.bmWidth;
    assert(width > 0);

    const LONG height = (NULL == ii.hbmColor) ? (bm.bmHeight / 2) : bm.bmHeight;
    assert(height > 0);

    if (NULL != ii.hbmMask)
    {
        if (!DeleteObject(ii.hbmMask))
        {
            Win32LastErrorFPutWSAbort(stderr,                        // _In_ FILE          *lpStream
                                      L"DeleteObject(ii.hbmMask)");  // _In_ const wchar_t *lpMessage
        }
    }

    if (NULL != ii.hbmColor)
    {
        if (!DeleteObject(ii.hbmColor))
        {
            Win32LastErrorFPutWSAbort(stderr,                         // _In_ FILE          *lpStream
                                      L"DeleteObject(ii.hbmColor)");  // _In_ const wchar_t *lpMessage
        }
    }

    lpIconDim->width  = width;
    lpIconDim->height = height;
}

