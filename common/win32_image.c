#include "win32_image.h"
#include "win32_last_error.h"

HANDLE
Win32LoadSharedImageByResource(_In_ const wchar_t *lpszImageResource,  // Ex: IDI_APPLICATION
                               _In_ const char    *lpszImageIdName,    // Ex: "IDI_APPLICATION"
                               _In_ const UINT     imageType,          // Ex: IMAGE_ICON
                               _In_ const char    *lpszImageTypeName)  // Ex: "IMAGE_ICON"
{
    // Ref: https://stackoverflow.com/questions/65213494/how-to-register-a-window-with-the-default-cursor-using-loadimage-instead-of-lo
    // Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadimagew
    const HANDLE h = LoadImageW(NULL,               // [in, optional] HINSTANCE hInst
                                lpszImageResource,  // [in] LPCWSTR name
                                imageType,          // [in] UINT type
                                0,                  // [in] int cx
                                0,                  // [in] int cy
                                LR_SHARED);         // [in] UINT fuLoad
    if (NULL == h)
    {
        Win32LastErrorFPrintFWAbort(stderr,                                                             // _In_ FILE          *lpStream
                                    L"LoadImageW(NULL, %ls[%ls], %u:%s, 0, 0, LR_SHARED)",              // _In_ const wchar_t *lpMessageFormat
                                    lpszImageResource, lpszImageIdName, imageType, lpszImageTypeName);  // ...
    }
    return h;
}

