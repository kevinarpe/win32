#ifndef H_COMMON_WIN32_IMAGE
#define H_COMMON_WIN32_IMAGE

#include "win32.h"

HANDLE
Win32LoadSharedImage(_In_ const wchar_t *lpszImageResource,   // Ex: IDI_APPLICATION or MAKEINTRESOURCE(OCR_NORMAL)
                     _In_ const char    *lpszImageIdName,     // Ex: "IDI_APPLICATION" or "MAKEINTRESOURCE(OCR_NORMAL)"
                     _In_ const UINT     imageType,           // Ex: IMAGE_ICON
                     _In_ const char    *lpszImageTypeName);  // Ex: "IMAGE_ICON"

#endif  // H_COMMON_WIN32_IMAGE

