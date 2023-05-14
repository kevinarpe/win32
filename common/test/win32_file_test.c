#include "win32_file.h"
#include "assertive.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>

static void
TestWin32FileCreate()
{
    puts("TestWin32FileCreate\r\n");

    struct WStr filePathWStr = WSTR_FROM_LITERAL(L"TestWin32FileCreate.txt");
    struct Win32File win32File = {0};
    win32File.indirect =
        (struct Win32FileCreateIndirect) {
            .filePathWStr          = filePathWStr,
            .dwDesiredAccess       = (DWORD) (GENERIC_READ | GENERIC_WRITE),
            .dwShareMode           = (DWORD) FILE_SHARE_READ,
            .lpSecurityAttributes  = (LPSECURITY_ATTRIBUTES) NULL,
            .dwCreationDisposition = (DWORD) CREATE_ALWAYS,
            .dwFlagsAndAttributes  = (DWORD) FILE_ATTRIBUTE_NORMAL,
            .hTemplateFile         = (HANDLE) NULL,
        };

    Win32FileCreate(&win32File);  // _Inout_ struct Win32File *lpFile

    const bool exists = Win32FileExists(filePathWStr.lpWCharArr);  // _In_ const wchar_t *lpFilePath

    AssertWF(exists,                    // _In_ const bool     bAssertResult
             L"After Win32FileCreate: Win32FileExists(filePathWStr.lpWCharArr[%ls])",  // _In_ const wchar_t *lpMessageFormatWCharArr
             filePathWStr.lpWCharArr);  // _In_ ...

    Win32FileGetFileDescriptor(&win32File,  // _Inout_ struct Win32File *lpFile
                               _O_WTEXT);   // _In_    const int         flags

    AssertWF(win32File.fd.flags == _O_WTEXT,  // _In_ const bool     bAssertResult
             L"After Win32FileGetFileDescriptor: win32File.fd.flags:%d == _O_WTEXT:%d",  // _In_ const wchar_t *lpMessageFormatWCharArr
             win32File.fd.flags, _O_WTEXT);   // _In_ ...

    Win32FileGetFileStream(&win32File,  // _Inout_ struct Win32File *lpFile
                           // "Opens for both reading and writing. The file must exist."
                           L"r+");       // _In_    const wchar_t    *lpModeWCharArr

    AssertWF(0 == wcscmp(L"r+", win32File.stream.mode),  // _In_ const bool     bAssertResult
             L"After Win32FileGetFileStream: 0 == %d:wcscmp(L\"r+\", win32File.stream.mode)",  // _In_ const wchar_t *lpMessageFormatWCharArr
             wcscmp(L"r+", win32File.stream.mode));      // _In_ ...

    AssertWF(FSM_READ_PLUS == win32File.stream.emode,  // _In_ const bool     bAssertResult
             L"After Win32FileGetFileStream: FSM_READ_PLUS:%d == %d:win32File.stream.emode",  // _In_ const wchar_t *lpMessageFormatWCharArr
             FSM_READ_PLUS, win32File.stream.emode);   // _In_ ...

    struct WStr textWStr = WSTR_FROM_LITERAL(L"abc123");
    const size_t ulCount = fwrite(textWStr.lpWCharArr,       // const void *buffer
                                  sizeof(wchar_t),           // size_t size
                                  textWStr.ulSize,           // size_t count
                                  win32File.stream.lpFile);  // FILE *stream

    AssertWF(ulCount == textWStr.ulSize,                      // _In_ const bool     bAssertResult
             L"fwrite: ulCount:%zd == textWStr->ulSize:%zd",  // _In_ const wchar_t *lpMessageFormatWCharArr
             ulCount, textWStr.ulSize);                       // _In_ ...

    struct WStr textWStr2 = {0};
    Win32FileReadAll(&win32File,   // _Inout_ struct Win32File *lpFile
                     &textWStr2);  // _Inout_ struct WStr      *lpWStr

    AssertWF(0 == wcscmp(textWStr.lpWCharArr, textWStr2.lpWCharArr),    // _In_ const bool     bAssertResult
             L"textWStr.lpWCharArr[%ls] != textWStr2.lpWCharArr[%ls]",  // _In_ const wchar_t *lpMessageFormatWCharArr
             textWStr.lpWCharArr, textWStr2.lpWCharArr);                // _In_ ...

    Win32FileClose(&win32File);  // _In_ struct Win32File *lpFile

    const bool exists2 = Win32FileExists(filePathWStr.lpWCharArr);  // _In_ const wchar_t *lpFilePath

    AssertWF(exists2,                   // _In_ const bool     bAssertResult
             L"After Win32FileClose: Win32FileExists(filePathWStr.lpWCharArr[%ls])",  // _In_ const wchar_t *lpMessageFormatWCharArr
             filePathWStr.lpWCharArr);  // _In_ ...

    Win32FileDelete(win32File.indirect.filePathWStr.lpWCharArr);  // _In_ const wchar_t *lpFilePath

    const bool exists3 = Win32FileExists(filePathWStr.lpWCharArr);  // _In_ const wchar_t *lpFilePath

    AssertWF(false == exists3,          // _In_ const bool     bAssertResult
             L"Win32FileDelete: Win32FileExists(filePathWStr.lpWCharArr[%ls])",  // _In_ const wchar_t *lpMessageFormatWCharArr
             filePathWStr.lpWCharArr);  // _In_ ...
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

    TestWin32FileCreate();
    return 0;
}

