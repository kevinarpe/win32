#ifndef H_COMMON_WIN32
#define H_COMMON_WIN32

// https://docs.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers
// https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt

#ifndef UNICODE
#define UNICODE
#endif

// Ref: https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
// These two pairs of undef+define: Enable WideCharToMultiByte(WC_ERR_INVALID_CHARS) 
//#undef  WINVER
//#define WINVER _WIN32_WINNT_WIN7
#define WINVER _WIN32_WINNT_WIN10

#undef  _WIN32_WINNT
#define _WIN32_WINNT WINVER

// Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setsystemcursor
// <<For an application to use any of the OCR_ constants, the constant OEMRESOURCE must be defined before the Windows.h header file is included.>>
#define OEMRESOURCE

// Ref: https://stackoverflow.com/questions/11040133/what-does-defining-win32-lean-and-mean-exclude-exactly
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define LEN_NUL_CHAR 1UL

// Ref: https://learn.microsoft.com/uk-ua/windows-hardware/drivers/ddi/ntstrsafe/nf-ntstrsafe-rtlunicodestringvalidate
// Ref: https://github.com/tpn/winsdk-10/blob/master/Include/10.0.10240.0/km/ntstrsafe.h#L132
#ifndef NTSTRSAFE_UNICODE_STRING_MAX_CCH
#define NTSTRSAFE_UNICODE_STRING_MAX_CCH ((USHRT_MAX) / sizeof(wchar_t))
#endif

// Ref: https://learn.microsoft.com/en-us/windows/win32/learnwin32/dpi-and-device-independent-pixels
// "The standard DPI settings are 100% (96 DPI)"
#define DEFAULT_DPI 96

// Ref: https://wiki.winehq.org/List_Of_Windows_Messages
const wchar_t *
Win32MsgToText(const UINT uMsg);

#endif  // H_COMMON_WIN32

