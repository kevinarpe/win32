#include "win32_guid.h"
#include "win32.h"
#include "assertive.h"
#include <stdio.h>    // required for printf()
#include <assert.h>   // required for assert()

void
TestWin32GuidToWStr2()
{
    printf("TestWin32GuidToWStr2()\r\n");

    GUID guid = {0};
    if (false == Win32GuidCreate2(&guid,    // _Out_ GUID *lpGuid
                                  stderr))  // _In_  FILE *lpErrorStream
    {
        abort();
    }
/*
typedef struct _GUID {
  unsigned long  Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char  Data4[8];
} GUID;
*/
    const bool char_is_signed = ((char) -1) < 0;
    const bool wchar_is_signed = ((wchar_t) -1) < 0;
    printf("char_is_signed? %s, wchar_is_signed? %s, guid{.Data1=%ld, .Data2=%hd, .Data3=%hd, .Data4={%d,%d,%d,%d,%d,%d,%d,%d}}\r\n",
           char_is_signed ? "true" : "false", wchar_is_signed ? "true" : "false", guid.Data1, guid.Data2, guid.Data3,
           guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

    struct WStr guidWStr = {0};
    if (false == Win32GuidToWStr2(&guid,      // _In_  GUID        *lpGuid
                                  &guidWStr,  // _Out_ struct WStr *lpWStr
                                  stderr))    // _In_  FILE        *lpErrorStream
    {
        abort();
    }

    GUID guid2 = {.Data1=1863342379, .Data2=11529, .Data3=17566, .Data4={186,178,156,158,88,39,32,9}};

    struct WStr guidWStr2 = {0};
    if (false == Win32GuidToWStr2(&guid2,      // _In_  GUID        *lpGuid
                                  &guidWStr2,  // _Out_ struct WStr *lpWStr
                                  stderr))     // _In_  FILE        *lpErrorStream
    {
        abort();
    }

    const int result = WStrCompare(&guidWStr2, &WSTR_FROM_LITERAL(L"6F10592B-2D09-449E-BAB2-9C9E58272009"));
    AssertWF(0 == result,                    // _In_ const bool     bAssertResult
             L"0 == %d:WStrCompare(&guidWStr2:[%ls], &WSTR_FROM_LITERAL(L\"6F10592B-2D09-449E-BAB2-9C9E58272009\")",  // _In_ const wchar_t *lpMessageFormatWCharArr
             result, guidWStr2.lpWCharArr);  // _In_ ...
}

// Ref: https://stackoverflow.com/a/13872211/257299
// Ref: https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
int WINAPI wWinMain(__attribute__((unused)) HINSTANCE hInstance,      // The operating system uses this value to identify the executable (EXE) when it is loaded in memory.
                    __attribute__((unused)) HINSTANCE hPrevInstance,  // ... has no meaning. It was used in 16-bit Windows, but is now always zero.
                    __attribute__((unused)) PWSTR     lpCmdLine,      // ... contains the command-line arguments as a Unicode string.
                    __attribute__((unused)) int       nCmdShow)       // ... is a flag that says whether the main application window will be minimized, maximized, or shown normally.
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-error-mode?view=msvc-170
    _set_error_mode(_OUT_TO_STDERR);  // assert to STDERR

    TestWin32GuidToWStr2();

    return 0;
}

