#ifndef _H_WIN32
#define _H_WIN32

// https://docs.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers
// https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt

#ifndef UNICODE
#define UNICODE
#endif

// These two pairs of undef+define: Enable WideCharToMultiByte(WC_ERR_INVALID_CHARS) 
#undef  WINVER
#define WINVER _WIN32_WINNT_WIN7

#undef  _WIN32_WINNT
#define _WIN32_WINNT WINVER

// Ref: https://stackoverflow.com/questions/11040133/what-does-defining-win32-lean-and-mean-exclude-exactly
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern const size_t LEN_NULL_CHAR;  // = 1U;

#endif  // _H_WIN32

