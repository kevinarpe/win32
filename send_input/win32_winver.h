#ifndef _H_WIN32_WINVER
#define _H_WIN32_WINVER

// https://docs.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers
// https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt

#ifndef UNICODE
#define UNICODE
#endif

#undef  WINVER
#define WINVER _WIN32_WINNT_WIN7

#undef  _WIN32_WINNT
#define _WIN32_WINNT WINVER

#endif  // _H_WIN32_WINVER

