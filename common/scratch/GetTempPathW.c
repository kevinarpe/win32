#include "win32.h"
#include "wstr.h"
#include <fileapi.h>
#include <stdbool.h>

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI
wWinMain(__attribute__((unused))
         HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
         __attribute__((unused))
         HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
         __attribute__((unused))
         PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
         __attribute__((unused))
         int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    static wchar_t C[] = L"abc";
    wprintf(L"[%ls]: %d\r\n", C, sizeof(C) / sizeof(*C));
/*
    __attribute__((unused))
    bool x = true;
    const DWORD r = GetTempPathW(0,      // [in]  DWORD  nBufferLength,
                                 NULL);  // [out] LPWSTR lpBuffer
    // r:19
    wprintf(L"r:%u\n", r);
    wchar_t buf[18 + 1 - 1] = {0};
    const DWORD r2 = GetTempPathW(sizeof(buf) / sizeof(buf[0]),      // [in]  DWORD  nBufferLength,
                                  buf);  // [out] LPWSTR lpBuffer
    // r2:18, [C:\users\kca\Temp\]
    wprintf(L"r2:%u, [%ls]\n", r2, buf);
*/
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
    const DWORD dwLastError = GetLastError();
    // Ref: https://github.com/dotnet/runtime/blob/3b63eb1346f1ddbc921374a5108d025662fb5ffd/src/coreclr/utilcode/posterror.cpp#L113
    #define LP_BUFFER_WCHAR_ARR_LEN 512
    static wchar_t lpBufferWCharArr[LP_BUFFER_WCHAR_ARR_LEN] = L"%0";
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-makelangid
    const DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagew
    const DWORD dwStrLen =
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
//                       | FORMAT_MESSAGE_IGNORE_INSERTS,  // [in] DWORD dwFlags
//                       | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,  // [in] DWORD dwFlags
                       NULL,                             // [in, optional] LPCVOID lpSource
//                       dwLastError,                      // [in] DWORD dwMessageId
                       (DWORD) 3,                        // [in] DWORD dwMessageId
                       dwLanguageId,                     // [in] DWORD dwLanguageId
                       lpBufferWCharArr,                 // [out] LPWSTR lpBuffer
                       LP_BUFFER_WCHAR_ARR_LEN,          // [in] DWORD nSize
                       NULL);                            // [in, optional] va_list *Arguments
    wprintf(L"Last error: %u: %ls|%u|%u|%u(Len: %u)\r\n", dwLastError, lpBufferWCharArr, lpBufferWCharArr[dwStrLen - 3], lpBufferWCharArr[dwStrLen - 2], lpBufferWCharArr[dwStrLen - 1], dwStrLen);
    return 0;
}

