#include "win32_file.h"
#include "win32_last_error.h"
#include "log.h"
#include "win32_errno.h"
#include "xmalloc.h"
#include "assertive.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW
#include <io.h>

bool
Win32FileExists(_In_ const wchar_t *lpFilePath)
{
    assert(NULL != lpFilePath);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileattributesw
    const DWORD dwAttrib = GetFileAttributesW(lpFilePath);  // [in] LPCWSTR lpFileName
    const bool x = (INVALID_FILE_ATTRIBUTES != dwAttrib) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
    return x;
}

void
Win32FileCreate(_Inout_ struct Win32File *lpFile)
{
    if (false == Win32FileCreate2(lpFile,   // _Inout_ struct Win32File *lpFile
                                  stderr))  // _Out_   FILE             *lpErrorStream
    {
        abort();
    }
}

bool
Win32FileCreate2(_Inout_ struct Win32File *lpFile,
                 _Out_   FILE             *lpErrorStream)
{
    assert(NULL != lpFile);
    assert(NULL != lpErrorStream);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
    lpFile->hFile =
        CreateFileW(lpFile->indirect.filePathWStr.lpWCharArr,  // [in]           LPCWSTR               lpFileName
                    lpFile->indirect.dwDesiredAccess,          // [in]           DWORD                 dwDesiredAccess
                    lpFile->indirect.dwShareMode,              // [in]           DWORD                 dwShareMode
                    lpFile->indirect.lpSecurityAttributes,     // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes
                    lpFile->indirect.dwCreationDisposition,    // [in]           DWORD                 dwCreationDisposition
                    lpFile->indirect.dwFlagsAndAttributes,     // [in]           DWORD                 dwFlagsAndAttributes
                    lpFile->indirect.hTemplateFile);           // [in, optional] HANDLE                hTemplateFile

    // "If the function fails, the return value is INVALID_HANDLE_VALUE. To get extended error information, call GetLastError."
    if (INVALID_HANDLE_VALUE == lpFile->hFile)
    {
        Win32LastErrorFPrintFW2(lpErrorStream,                                                   // _In_  FILE          *lpStream
                                lpErrorStream,                                                   // _Out_ FILE          *lpErrorStream
                                L"Win32FileCreate2: Failed to open file by file handle: [%ls]",  // _In_  const wchar_t *lpMessageFormat
                                lpFile->indirect.filePathWStr.lpWCharArr);                       // ...
        return false;
    }

    lpFile->fd.flags       = 0;
    lpFile->fd.fd          = -1;
    lpFile->stream.mode[0] = 0;
    lpFile->stream.emode   = 0;
    lpFile->stream.lpFile  = NULL;
    return true;
}

void
Win32FileGetFileDescriptor(_Inout_ struct Win32File *lpFile,
                           _In_    const int         flags)
{
    if (false == Win32FileGetFileDescriptor2(lpFile,   // _Inout_ struct Win32File *lpFile
                                             flags,    // _In_    const int         flags
                                             stderr))  // _Out_   FILE             *lpErrorStream
    {
        abort();
    }
}

bool
Win32FileGetFileDescriptor2(_Inout_ struct Win32File *lpFile,
                            _In_    const int         flags,
                            _Out_   FILE             *lpErrorStream)
{
    assert(NULL != lpFile);
    assert(NULL != lpErrorStream);

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/open-osfhandle?view=msvc-170
    lpFile->fd.fd = _open_osfhandle((intptr_t) lpFile->hFile,  // _In_ intptr_t osfhandle
                                    flags);                    // _In_ int      flags
    if (-1 == lpFile->fd.fd)
    {
        Win32LastErrorFPrintFW2(lpErrorStream,                                                                         // _In_  FILE          *lpStream
                                lpErrorStream,                                                                          // _Out_ FILE          *lpErrorStream
                                L"Win32FileGetFileDescriptor2: File [%ls]: -1 == _open_osfhandle(hFile:%p, flags:%d)",  // _In_  const wchar_t *lpMessageFormat
                                lpFile->indirect.filePathWStr.lpWCharArr, lpFile->hFile, flags);                        // _In_  ...
        return false;
    }

    lpFile->fd.flags = flags;
    return true;
}

void
Win32FileGetFileStream(_Inout_ struct Win32File *lpFile,
                       _In_    const wchar_t    *lpModeWCharArr)
{
    if (false == Win32FileGetFileStream2(lpFile,          // _Inout_ struct Win32File *lpFile
                                         lpModeWCharArr,  // _In_    const wchar_t    *lpModeWCharArr
                                         stderr))         // _Out_   FILE             *lpErrorStream
    {
        abort();
    }
}

static void
StaticWin32FileGetMode(_In_  const wchar_t             *lpModeWCharArr,
                       _Out_ enum EWin32FileStreamMode *lpMode,
                       _Out_ FILE                      *lpErrorStream)
{
    assert(NULL != lpModeWCharArr);
    assert(NULL != lpMode);
    assert(NULL != lpErrorStream);

    if (0 == wcscmp(L"r", lpModeWCharArr))
    {
        *lpMode = FSM_READ;
    }
    else if (0 == wcscmp(L"w", lpModeWCharArr))
    {
        *lpMode = FSM_WRITE;
    }
    else if (0 == wcscmp(L"a", lpModeWCharArr))
    {
        *lpMode = FSM_APPEND;
    }
    else if (0 == wcscmp(L"r+", lpModeWCharArr))
    {
        *lpMode = FSM_READ_PLUS;
    }
    else if (0 == wcscmp(L"w+", lpModeWCharArr))
    {
        *lpMode = FSM_WRITE_PLUS;
    }
    else if (0 == wcscmp(L"a+", lpModeWCharArr))
    {
        *lpMode = FSM_APPEND_PLUS;
    }
    else
    {
        AssertWF(false,                                                       // _In_ const bool     bAssertResult
                 L"StaticWin32FileGetMode: _wfdopen(): Unknown mode: [%ls]",  // _In_ const wchar_t *lpMessageFormatWCharArr
                 lpModeWCharArr);                                             // _In_ ...
    }
}

bool
Win32FileGetFileStream2(_Inout_ struct Win32File *lpFile,
                        _In_    const wchar_t    *lpModeWCharArr,
                        _Out_   FILE             *lpErrorStream)
{
    assert(NULL != lpFile);
    assert(lpFile->fd.fd >= 0);
    assert(NULL != lpModeWCharArr);
    AssertWF(2 <= wcslen(lpModeWCharArr),               // _In_ const bool     bAssertResult
             L"2 <= %zd:wcslen(lpModeWCharArr:[%ls])",  // _In_ const wchar_t *lpMessageFormatWCharArr
             wcslen(lpModeWCharArr), lpModeWCharArr);   // _In_ ...
    assert(NULL != lpErrorStream);

    enum EWin32FileStreamMode emode = 0;
    StaticWin32FileGetMode(lpModeWCharArr,  // _In_  const wchar_t             *lpModeWCharArr
                           &emode,          // _Out_ enum EWin32FileStreamMode *lpMode
                           lpErrorStream);  // _Out_ FILE                      *lpErrorStream

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=msvc-170
    lpFile->stream.lpFile = _wfdopen(lpFile->fd.fd,    // int fd
                                     lpModeWCharArr);  // const wchar_t *mode
    if (NULL == lpFile->stream.lpFile)
    {
        Win32LastErrorFPrintFW2(lpErrorStream,                                                             // _In_  FILE          *lpStream
                                lpErrorStream,                                                             // _Out_ FILE          *lpErrorStream
                                L"Win32FileGetFileStream2: File [%ls]: _wfdopen(%d, \"%ls\")",             // _In_  const wchar_t *lpMessageFormat
                                lpFile->indirect.filePathWStr.lpWCharArr, lpFile->fd.fd, lpModeWCharArr);  // _In_  ...
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-wcscpy-mbscpy?view=msvc-170
    wcscpy(lpFile->stream.mode,  // wchar_t *strDestination,
           lpModeWCharArr);      // const wchar_t *strSource
    lpFile->stream.emode = emode;
    return true;
}

void
Win32FileReadAll(_Inout_ struct Win32File *lpFile,
                 _Inout_ struct WStr      *lpWStr)
{
    if (false == Win32FileReadAll2(lpFile,   // _Inout_ struct Win32File *lpFile
                                   lpWStr,   // _Inout_ struct WStr      *lpWStr
                                   stderr))  // _Out_   FILE             *lpErrorStream
    {
        abort();
    }
}

bool
Win32FileReadAll2(_Inout_ struct Win32File *lpFile,
                  _Inout_ struct WStr      *lpWStr,
                  _Out_   FILE             *lpErrorStream)
{
    assert(NULL != lpFile);
    assert(lpFile->fd.fd >= 0);
    assert(NULL != lpFile->stream.lpFile);
    assert(NULL != lpWStr);
    WStrFree(lpWStr);
    assert(NULL != lpErrorStream);

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fseek-fseeki64?view=msvc-170
    if (0 != fseek(lpFile->stream.lpFile,  // FILE *stream
                   // "Number of bytes from origin."
                   (long) 0,               // long offset
                   // "	End of file."
                   SEEK_END))              // int origin
    {
        Win32LastErrorFPutWS2(lpErrorStream,   // _In_  FILE          *lpStream
                              L"Win32FileReadAll2: 0 != fseek(lpFile, 0, SEEK_END)",  // _In_  const wchar_t *lpMessage
                              lpErrorStream);  // _Out_ FILE          *lpErrorStream
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/ftell-ftelli64?view=msvc-170
    const __int64 currFilePosByte = _ftelli64(lpFile->stream.lpFile);  // FILE *stream
    if (-1 == currFilePosByte)
    {
        Win32LastErrorFPutWS2(lpErrorStream,   // _In_  FILE          *lpStream
                              L"Win32FileReadAll2: -1 == _ftelli64(lpFile->stream.lpFile)",  // _In_  const wchar_t *lpMessage
                              lpErrorStream);  // _Out_ FILE          *lpErrorStream
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fseek-fseeki64?view=msvc-170
    if (0 != fseek(lpFile->stream.lpFile,  // FILE *stream
                   // "Number of bytes from origin."
                   (long) 0,               // long offset
                   // "Initial position.  Beginning of file."
                   SEEK_SET))              // int origin
    {
        Win32LastErrorFPutWS2(lpErrorStream,   // _In_  FILE          *lpStream
                              L"Win32FileReadAll2: 0 != fseek(lpFile->stream.lpFile, 0, SEEK_SET)",  // _In_  const wchar_t *lpMessage
                              lpErrorStream);  // _Out_ FILE          *lpErrorStream
        return false;
    }

    // Note: If the file system is full (out of space), it is possible to write *part* of one wchar_t,
    // e.g., 1 byte of a 2 byte (Win32) wchar_t.  This statement will round to the number of whole values of wchar_t.
    lpWStr->ulSize     = currFilePosByte / sizeof(wchar_t);
    lpWStr->lpWCharArr = xcalloc(lpWStr->ulSize + LEN_NUL_CHAR,  // _In_ const size_t ulNumItem
                                 sizeof(wchar_t));               // _In_ const size_t ulSizeOfEachItem

    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fread?view=msvc-170
    const size_t wcharCount = fread(lpWStr->lpWCharArr,             // void *buffer
                                    // "Item size in bytes."
                                    sizeof(wchar_t),                // size_t size
                                    // "Maximum number of items to be read."
                                    lpWStr->ulSize + LEN_NUL_CHAR,  // size_t count
                                    lpFile->stream.lpFile);         // FILE *stream

    if (wcharCount < lpWStr->ulSize)
    {
        Win32LastErrorFPrintFW2(lpErrorStream,  // _In_  FILE          *lpStream
                                lpErrorStream,  // _Out_ FILE          *lpErrorStream
                                L"Win32FileReadAll2: currFilePosByte:%lld != %zd:fread(lpWStr->lpWCharArr:[%ls], sizeof(wchar_t):%zd, currFilePosByte:%lld + LEN_NUL_CHAR:%d, lpFile->stream.lpFile)",  // _In_  const wchar_t *lpMessage
                                currFilePosByte, wcharCount, lpWStr->lpWCharArr, sizeof(wchar_t), currFilePosByte, LEN_NUL_CHAR);  // _In_  ...

        xfree((void **) &lpWStr->lpWCharArr);  // _Inout_ void **lppData
        return false;
    }

    return true;
}

void
Win32FileClose(_In_ struct Win32File *lpFile)
{
    if (false == Win32FileClose2(lpFile,   // _In_  struct Win32File *lpFile
                                 stderr))  // _Out_ FILE             *lpErrorStream
    {
        abort();
    }
}

bool
Win32FileClose2(_In_  struct Win32File *lpFile,
                _Out_ FILE             *lpErrorStream)
{
    assert(NULL != lpFile);
    assert(NULL != lpErrorStream);

    if (NULL != lpFile->stream.lpFile)
    {
        // Ref(_open_osfhandle): https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/open-osfhandle?view=msvc-170
        // "The _open_osfhandle call transfers ownership of the Win32 file handle to the file descriptor.
        //  To close a file opened by using _open_osfhandle, call _close. The underlying OS file handle
        //  is also closed by a call to _close. Don't call the Win32 function CloseHandle on the original handle.
        //  If the file descriptor is owned by a FILE * stream, then a call to fclose closes both the file descriptor
        //  and the underlying handle. In this case, don't call _close on the file descriptor or CloseHandle on the original handle."

        // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fclose-fcloseall?view=msvc-170
        if (0 != fclose(lpFile->stream.lpFile))  // FILE *stream
        {
            Win32ErrnoFPrintFW2(lpErrorStream,                                                   // _In_  FILE          *lpStream
                                lpErrorStream,                                                   // _Out_ FILE          *lpErrorStream
                                L"Win32FileClose2: Failed to close file by FILE stream: [%ls]",  // _In_  const wchar_t *lpMessageFormat
                                lpFile->indirect.filePathWStr.lpWCharArr);                       // _In_  ...
            return false;
        }
    }
    else if (lpFile->fd.fd >= 0)
    {
        // Ref(_wfdopen): https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=msvc-170
        // "File descriptors passed into _fdopen are owned by the returned FILE * stream.
        //  If _fdopen is successful, don't call _close on the file descriptor.
        //  Calling fclose on the returned FILE * also closes the file descriptor."

        // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/close?view=msvc-170
        if (0 != _close(lpFile->fd.fd))  // int fd
        {
            Win32ErrnoFPrintFW2(lpErrorStream,                                                       // _In_  FILE          *lpStream
                                lpErrorStream,                                                       // _Out_ FILE          *lpErrorStream
                                L"Win32FileClose2: Failed to close file by file descriptor: [%ls]",  // _In_  const wchar_t *lpMessageFormat
                                lpFile->indirect.filePathWStr.lpWCharArr);                           // _In_  ...
            return false;
        }
    }
    else if (NULL != lpFile->hFile)
    {
        // Ref(CreateFileW): https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
        // "When an application is finished using the object handle returned by CreateFile,
        //  use the CloseHandle function to close the handle."

        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
        if (0 == CloseHandle(lpFile->hFile))  // [in] HANDLE hObject
        {
            Win32LastErrorFPrintFW2(lpErrorStream,                                                   // _In_  FILE          *lpStream,
                                    lpErrorStream,                                                   // _Out_ FILE          *lpErrorStream,
                                    L"Win32FileClose2: Failed to close file by file handle: [%ls]",  // _In_  const wchar_t *lpMessageFormat
                                    lpFile->indirect.filePathWStr.lpWCharArr);                       // _In_  ...
            return false;
        }
    }

    lpFile->hFile          = NULL;
    lpFile->fd.flags       = 0;
    lpFile->fd.fd          = -1;
    lpFile->stream.mode[0] = 0;
    lpFile->stream.emode   = 0;
    lpFile->stream.lpFile  = NULL;
    return true;
}

void
Win32FileDelete(_In_ const wchar_t *lpFilePath)
{
    if (false == Win32FileDelete2(lpFilePath,  // _In_  const wchar_t *lpFilePath
                                  stderr))     // _Out_ FILE          *lpErrorStream
    {
        abort();
    }
}

bool
Win32FileDelete2(_In_  const wchar_t *lpFilePath,
                 _Out_ FILE          *lpErrorStream)
{
    assert(NULL != lpFilePath);
    assert(NULL != lpErrorStream);

    if (0 == DeleteFileW(lpFilePath))  // [in] LPCWSTR lpFileName
    {
        Win32LastErrorFPrintFW2(lpErrorStream,                    // _In_  FILE          *lpStream,
                                lpErrorStream,                    // _Out_ FILE          *lpErrorStream,
                                L"Failed to delete file: [%ls]",  // _In_  const wchar_t *lpMessageFormat
                                lpFilePath);                      // _In_  ...
        return false;
    }

    return true;
}

