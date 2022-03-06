#ifndef _H_ERROR_EXIT
#define _H_ERROR_EXIT

#include <wchar.h>  // required for wchar_t

#define g_ulErrorMsgBufferSize ((size_t) 1024)
extern wchar_t g_lpErrorMsgBuffer[];

void ErrorExit(const wchar_t *lpszFunction);

#endif  // _H_ERROR_EXIT

