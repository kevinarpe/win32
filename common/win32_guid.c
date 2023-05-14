#include "win32_guid.h"
#include "win32_last_error.h"
#include "win32_errno.h"
#include "log.h"
#include "assertive.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW
#include <combaseapi.h>

void
Win32GuidCreate(_Out_ GUID *lpGuid)
{
    if (false == Win32GuidCreate2(lpGuid,   // _Out_ GUID *lpGuid
                                  stderr))  // _In_  FILE *lpErrorStream
    {
        abort();
    }
}

bool
Win32GuidCreate2(_Out_ GUID *lpGuid,
                 _In_  FILE *lpErrorStream)
{
    assert(NULL != lpGuid);
    assert(NULL != lpErrorStream);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateguid
    const HRESULT h = CoCreateGuid(lpGuid);  // [out] GUID *pguid
    if (S_OK != h)
    {
        Win32LastErrorFPrintFW2(lpErrorStream,                           // _In_  FILE          *lpStream
                                lpErrorStream,                           // _Out_ FILE          *lpErrorStream
                                L"S_OK != CoCreateGuid(&guid): %p", h);  // _In_  const wchar_t *lpMessageFormat, ...
        return false;
    }
    return true;
}

void
Win32GuidToWStr(_In_  GUID        *lpGuid,
                _Out_ struct WStr *lpWStr)
{
    if (false == Win32GuidToWStr2(lpGuid,   // _In_  GUID        *lpGuid
                                  lpWStr,   // _Out_ struct WStr *lpWStr
                                  stderr))  // _In_  FILE        *lpErrorStream
    {
        abort();
    }
}

bool
Win32GuidToWStr2(_In_  const GUID  *lpGuid,
                 _Out_ struct WStr *lpWStr,
                 _In_  FILE        *lpErrorStream)
{
    assert(NULL != lpGuid);
    WStrAssertValid(lpWStr);
    assert(NULL != lpErrorStream);

    // Ex: L"6B29FC40-CA47-1067-B31D-00DD010662DA"
    if (false == WStrSPrintF2(lpWStr,                                                                  // _Inout_ struct WStr  *lpDestWStr
                              lpErrorStream,                                                           // _In_    FILE         *lpErrorStream
                              L"%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",  // _In_    const wchar_t *lpFormatWCharArr
                              lpGuid->Data1,      // %lX: 6B29FC40
                              lpGuid->Data2,      // %hX: CA47
                              lpGuid->Data3,      // %hX: 1067
                              lpGuid->Data4[0],   // %hhX: B3
                              lpGuid->Data4[1],   // %hhX: 1D
                              lpGuid->Data4[2],   // %hhX: 00
                              lpGuid->Data4[3],   // %hhX: DD
                              lpGuid->Data4[4],   // %hhX: 01
                              lpGuid->Data4[5],   // %hhX: 06
                              lpGuid->Data4[6],   // %hhX: 62
                              lpGuid->Data4[7]))  // %hhX: DA
    {
        return false;
    }

    AssertWF(36 == lpWStr->ulSize,                                   // _In_ const bool     bAssertResult
             L"36 == lpWStr->ulSize:%zd, lpWStr->lpWCharArr:[%ls]",  // _In_ const wchar_t *lpMessageFormatWCharArr
             lpWStr->ulSize, lpWStr->lpWCharArr);                    // _In_ ...
    assert(36 == lpWStr->ulSize);
    return true;
}
/*
bool
Win32GuidToWStr2(_In_  GUID        *lpGuid,
                 _Out_ struct WStr *lpWStr,
                 _In_  FILE        *lpErrorStream)
{
    assert(NULL != lpGuid);
    WStrAssertValid(lpWStr);
    assert(lpWStr->ulSize == WIN32_GUID_ZERO_WCHAR_LEN);
    assert(NULL != lpErrorStream);

    // Ex: L"6B29FC40-CA47-1067-B31D-00DD010662DA"
    // "The functions return the number of wide characters written, excluding the terminating null wide character
    //  in case of the functions swprintf() and vswprintf().  They return -1 when an error occurs."
LAST: USE WStrSprintf !!!
    const int nNonNulChar =
        swprintf(lpWStr->lpWCharArr,                                // _Out_ char *buffer
                 // "The functions snprintf() and vsnprintf() write at most size bytes (including the terminating null byte ('\0')) to str."
                 lpWStr->ulSize + LEN_NUL_CHAR,                     // _In_ size_t count
                 L"%lX-%hX-%hX-%hhX%hhX-%hhX%hhX%hhX%hhX%hhX%hhX",  // _In_ const char *format
                 lpGuid->Data1,      // %lX: 6B29FC40
                 lpGuid->Data2,      // %hX: CA47
                 lpGuid->Data3,      // %hX: 1067
                 lpGuid->Data4[0],   // %hhX: B3
                 lpGuid->Data4[1],   // %hhX: 1D
                 lpGuid->Data4[2],   // %hhX: 00
                 lpGuid->Data4[3],   // %hhX: DD
                 lpGuid->Data4[4],   // %hhX: 01
                 lpGuid->Data4[5],   // %hhX: 06
                 lpGuid->Data4[6],   // %hhX: 62
                 lpGuid->Data4[7]);  // %hhX: DA

    if (-1 == nNonNulChar)
    {
        Win32ErrnoFPutWS2(lpErrorStream,                       // _In_  FILE          *lpStream
                          L"Win32GuidToWStr2: swprintf(...)",  // _In_  const wchar_t *lpMessage
                          lpErrorStream);                      // _Out_ FILE          *lpErrorStream
        return false;
    }

    LogWF(stdout, L"nNonNulChar:%d, struct WStr{.lpWCharArr=[%ls], .ulSize=%zd}\r\n",
          nNonNulChar, lpWStr->lpWCharArr, lpWStr->ulSize);

    assert(((size_t) nNonNulChar) == lpWStr->ulSize);
    return true;
}
*/

void
Win32GuidCreateToWStr(_Out_ struct WStr *lpWStr)
{
    if (false == Win32GuidCreateToWStr2(lpWStr,   // _Out_ struct WStr *lpWStr
                                        stderr))  // _In_  FILE        *lpErrorStream
    {
        abort();
    }
}

bool
Win32GuidCreateToWStr2(_Out_ struct WStr *lpWStr,
                       _In_  FILE        *lpErrorStream)
{
    assert(NULL != lpWStr);
    assert(NULL != lpErrorStream);

    GUID guid = {0};
    if (false == Win32GuidCreate2(&guid,           // _Out_ GUID *lpGuid
                                  lpErrorStream))  // _In_  FILE *lpErrorStream
    {
        return false;
    }

    if (false == Win32GuidToWStr2(&guid,           // _In_  const GUID  *lpGuid
                                  lpWStr,          // _Out_ struct WStr *lpWStr
                                  lpErrorStream))  // _In_  FILE        *lpErrorStream
    {
        return false;
    }
    return true;
}

