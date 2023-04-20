#ifndef H_COMMON_WIN32_FILE
#define H_COMMON_WIN32_FILE

#include "win32.h"
#include "wstr.h"
#include <stdbool.h>
#include <stdio.h>

struct Win32FileCreateIndirect
{
    struct WStr           filePathWStr;
    DWORD                 dwDesiredAccess;
    DWORD                 dwShareMode;
    // @Nullable
    LPSECURITY_ATTRIBUTES lpSecurityAttributes;
    DWORD                 dwCreationDisposition;
    DWORD                 dwFlagsAndAttributes;
    // @Nullable
    HANDLE                hTemplateFile;
};

struct Win32FileDescriptor
{
    // See: _open_osfhandle()
    // Ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/open-osfhandle?view=msvc-170
    int flags;
    // Invalid is -1
    int fd;
};

enum EWin32FileStreamMode
{
    /* L"r"   Opens for reading. If the file doesn't exist or can't be found, the fopen call fails. */
    FSM_READ        = 1,
    /* L"w"   Opens an empty file for writing. If the given file exists, its contents are destroyed. */
    FSM_WRITE       = 2,
    /* L"a"   Opens for writing at the end of the file (appending). Creates the file if it doesn't exist. */
    FSM_APPEND      = 3,
    /* L"r+"  Opens for both reading and writing. The file must exist. */
    FSM_READ_PLUS   = 4,
    /* L"w+"  Opens an empty file for both reading and writing. If the file exists, its contents are destroyed. */
    FSM_WRITE_PLUS  = 5,
    /* L"a+"  Opens for reading and appending. Creates the file if it doesn't exist. */
    FSM_APPEND_PLUS = 6,
};

struct Win32FileStream
{
    enum EWin32FileStreamMode  mode;
    // Invalid is NULL
    FILE                      *lpFile;
};

struct Win32File
{
    struct Win32FileCreateIndirect indirect;
    // Invalid is NULL
    HANDLE                         hFile;
    struct Win32FileDescriptor     fd;
    struct Win32FileStream         stream;
};

/**
 * This is a convenience method to call Win32FileCreate2(lpFile, stderr).
 * On error, abort() is called.
 */
void
Win32FileCreate(_Inout_ struct Win32File *lpFile);

/**
 * Call CreateFileW().
 *
 * @param lpFile
 *        lpFile->indirect must be populated before calling this function
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32FileCreate2(_Inout_ struct Win32File *lpFile,
                 _Out_   FILE             *lpErrorStream);

/**
 * This is a convenience method to call Win32FileGetFileDescriptor2(lpFile, flags, stderr).
 * On error, abort() is called.
 */
void
Win32FileGetFileDescriptor(_Out_ struct Win32File *lpFile,
                           _In_  const int         flags);

/**
 * Calls _open_osfhandle() to create file descriptor.
 * Ownership is transferred from file handle to file descriptor.
 *
 * @param flags
 *        bitwise-OR of zero or more flags:
 *        _O_APPEND: "Positions a file pointer to the end of the file before every write operation."
 *                   If file exists and this flag is missing, the file is truncated to zero bytes.
 *        _O_RDONLY: "Opens the file for reading only."
 *                   If this flag is missing, the file is open as read/write.
 *        _O_TEXT  : "Opens the file in text (translated) mode."
 *                   Use this to write 8-bit chars, e.g., C type 'char'.
 *                   If this flag is present, then reading and writing '\n' is translated to "\r\n".
 *                   If this flag is missing, then reading and writing will be exactly the bytes from the stream.
 *        _O_WTEXT : "Opens the file in Unicode (translated UTF-16) mode."
 *                   Use this to write 16-bit chars, e.g., C type 'wchar_t'.
 *                   If this flag is present, then reading and writing L'\n' is translated to L"\r\n".
 *                   If this flag is missing, then reading and writing will be exactly the bytes from the stream.
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32FileGetFileDescriptor2(_Out_ struct Win32File *lpFile,
                            _In_  const int         flags,
                            _Out_ FILE             *lpErrorStream);

/**
 * This is a convenience method to call Win32FileGetFileStream2(lpFile, lpModeWCharArr, stderr).
 * On error, abort() is called.
 */
void
Win32FileGetFileStream(_Inout_ struct Win32File *lpFile,
                       _In_    const wchar_t    *lpModeWCharArr);

/**
 * Prequisite: lpFile->fd >= 0: Call Win32FileGetFileDescriptor2() before this method.
 * Calls _wfdopen() to create file stream.
 * Ownership is transferred from file descriptor to file stream.
 *
 * @param lpModeWCharArr
 *        L"r"   Opens for reading. If the file doesn't exist or can't be found, the fopen call fails.
 *        L"w"   Opens an empty file for writing. If the given file exists, its contents are destroyed.
 *        L"a"   Opens for writing at the end of the file (appending). Creates the file if it doesn't exist.
 *        L"r+"  Opens for both reading and writing. The file must exist.
 *        L"w+"  Opens an empty file for both reading and writing. If the file exists, its contents are destroyed.
 *        L"a+"  Opens for reading and appending. Creates the file if it doesn't exist.
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32FileGetFileStream2(_Inout_ struct Win32File *lpFile,
                        _In_    const wchar_t    *lpModeWCharArr,
                        _Out_   FILE             *lpErrorStream);

void
Win32FileReadAll(_Inout_ struct Win32File *lpFile,
                 _Inout_ struct WStr      *lpWStr);

bool
Win32FileReadAll2(_Inout_ struct Win32File *lpFile,
                  _Inout_ struct WStr      *lpWStr,
                  _Out_   FILE             *lpErrorStream);

/**
 * This is a convenience method to call Win32FileClose2(lpFile, stderr).
 * On error, abort() is called.
 */
void
Win32FileClose(_In_ struct Win32File *lpFile);

/**
 * If NULL != lpFile->lpFile, then call: fclose(lpFile->lpFile)
 * If lpFile->fd >= 0,        then call: _close(lpFile->fd)
 * If NULL != lpFile->hFile,  then call: CloseHandle(lpFile->hFile)
 *
 * @param lpErrorStream
 *        stream to print errors
 *        usually 'stderr' (from <stdio.h>), but may be any valid stream
 *
 * @return true on success
 *         false on failure and error printed to {@code lpErrorStream}
 */
bool
Win32FileClose2(_In_  struct Win32File *lpFile,
                _Out_ FILE             *lpErrorStream);

#endif  // H_COMMON_WIN32_FILE

