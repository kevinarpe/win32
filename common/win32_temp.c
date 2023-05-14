#include "win32_temp.h"
#include "win32_guid.h"
#include "win32_last_error.h"
#include "log.h"
#include <assert.h>  // required for assert
#include <stdlib.h>  // required for assert on MinGW
#include <objbase.h>

void
Win32TempGetTempDirPath(_Out_ struct WStr *lpWStr)
{
    if (false == Win32TempGetTempDirPath2(lpWStr,   // _Out_ struct WStr *lpWStr
                                          stderr))  // _Out_ FILE        *lpErrorStream
    {
        abort();
    }
}

bool
Win32TempGetTempDirPath2(_Out_ struct WStr *lpWStr,
                         _Out_ FILE        *lpErrorStream)
{
    WStrAssertValid(lpWStr);
    assert(NULL != lpErrorStream);

    wchar_t tempDirPathWChar[MAX_PATH + LEN_NUL_CHAR] = {0};

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-gettemppathw
    // Note: GetTempPath2W does not currently exist in WINE!
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-gettemppath2w
    // If buffer is too small, 'result' is required buffer len, including trailing NUL char.
    // If buffer is large enough, 'result' is strlen, excluding trailing NUL char.
    // For example, if successful 'result' is 18, passing NULL will return 19: 18 + NUL char.
    const DWORD result = GetTempPathW(MAX_PATH + LEN_NUL_CHAR,  // [in]  DWORD  nBufferLength
                                      tempDirPathWChar);                      // [out] LPWSTR lpBuffer

    // "If the function fails, the return value is zero. To get extended error information, call GetLastError."
    if (0 == result)
    {
        Win32LastErrorFPutWS2(lpErrorStream,     // _In_  FILE          *lpStream
                              L"GetTempPath2W",  // _In_  const wchar_t *lpMessage
                              lpErrorStream);    // _Out_ FILE          *lpErrorStream
        return false;
    }
    // "If the return value is greater than nBufferLength, the return value is the length, in TCHARs, of the buffer required to hold the path."
    else if (result > MAX_PATH)
    {
        Win32LastErrorFPrintFW2(lpErrorStream,                                                      // _In_  FILE          *lpStream
                                lpErrorStream,                                                      // _Out_ FILE          *lpErrorStream
                                L"GetTempPath2W: Required strlen [%u] exceeds buffer strlen [%u]",  // _In_  const wchar_t *lpMessageFormat
                                result - 1, MAX_PATH);                                              // ...
        return false;
    }

    WStrCopyWCharArr(lpWStr,            // _Inout_ struct WStr   *lpDestWStr
                     tempDirPathWChar,  // _In_    const wchar_t *lpSrcWCharArr
                     result);           // _In_    const size_t   ulSrcSize
    return true;
}

void
Win32TempCreateTempFileName(// @EmptyStringAllowed
                            _In_  struct WStr *lpPrefixWStr,
                            // @EmptyStringAllowed
                            _In_  struct WStr *lpSuffixWStr,
                            _Out_ struct WStr *lpTempFileNameWStr)
{
    if (false == Win32TempCreateTempFileName2(lpPrefixWStr,        // _In_  struct WStr *lpPrefixWStr
                                              lpSuffixWStr,        // _In_  struct WStr *lpSuffixWStr
                                              lpTempFileNameWStr,  // _Out_ struct WStr *lpTempFileNameWStr
                                              stderr))             // _Out_ FILE        *lpErrorStream
    {
        abort();
    }
}

bool
Win32TempCreateTempFileName2(// @EmptyStringAllowed
                             _In_  struct WStr *lpPrefixWStr,
                             // @EmptyStringAllowed
                             _In_  struct WStr *lpSuffixWStr,
                             _Out_ struct WStr *lpTempFileNameWStr,
                             _Out_ FILE        *lpErrorStream)
{
    struct WStr guidWStr = {0};
    if (false == Win32GuidCreateToWStr2(&guidWStr,       // _Out_ struct WStr *lpWStr
                                        lpErrorStream))  // _In_  FILE        *lpErrorStream
    {
        return false;
    }

    WStrConcatMany(lpTempFileNameWStr,  // _Inout_ struct WStr       *lpDestWStr
                   lpPrefixWStr,        // _In_    const struct WStr *lpSrcWStr
                   &guidWStr,           // _In_    const struct WStr *lpSrcWStr2
                   lpSuffixWStr);       // ...

    WStrFree(&guidWStr);
    return true;
}

void
Win32TempCreateTempFilePath(// @Nullable
                            _In_  struct WStr *lpNullableTempDirPathWStr,
                            // @EmptyStringAllowed
                            _In_  struct WStr *lpPrefixWStr,
                            // @EmptyStringAllowed
                            _In_  struct WStr *lpSuffixWStr,
                            _Out_ struct WStr *lpTempFilePathWStr)
{
    if (false == Win32TempCreateTempFilePath2(lpNullableTempDirPathWStr,  // _In_  struct WStr *lpNullableTempDirPathWStr
                                              lpPrefixWStr,               // _In_  struct WStr *lpPrefixWStr
                                              lpSuffixWStr,               // _In_  struct WStr *lpSuffixWStr
                                              lpTempFilePathWStr,         // _Out_ struct WStr *lpTempFilePathWStr
                                              stderr))                    // _Out_ FILE        *lpErrorStream
    {
        abort();
    }
}

bool
Win32TempCreateTempFilePath2(// @Nullable
                             _In_  struct WStr *lpNullableTempDirPathWStr,
                             // @EmptyStringAllowed
                             _In_  struct WStr *lpPrefixWStr,
                             // @EmptyStringAllowed
                             _In_  struct WStr *lpSuffixWStr,
                             _Out_ struct WStr *lpTempFilePathWStr,
                             _Out_ FILE        *lpErrorStream)
{
    if (NULL != lpNullableTempDirPathWStr)
    {
        WStrAssertValid(lpNullableTempDirPathWStr);
    }
    WStrAssertValid(lpPrefixWStr);
    WStrAssertValid(lpSuffixWStr);
    assert(NULL != lpErrorStream);

    struct WStr *lpTempDirPathWStr = lpNullableTempDirPathWStr;

    struct WStr tempDirPathWStr2 = {0};
    if (NULL == lpNullableTempDirPathWStr)
    {
        if (false == Win32TempGetTempDirPath2(&tempDirPathWStr2,  // _Out_ struct WStr *lpTempDirPathWStr
                                              lpErrorStream))     // _Out_ FILE        *lpErrorStream
        {
            return false;
        }
        // Note: Must free 'tempDirPathWStr2' before return.
        lpTempDirPathWStr = &tempDirPathWStr2;
    }

    bool result = false;

    struct WStr tempFileNameWStr = {0};
    if (false == Win32TempCreateTempFileName2(lpPrefixWStr,       // _In_  struct WStr *lpPrefixWStr
                                              lpSuffixWStr,       // _In_  struct WStr *lpSuffixWStr
                                              &tempFileNameWStr,  // _Out_ struct WStr *lpTempFileNameWStr
                                              lpErrorStream))     // _Out_ FILE        *lpErrorStream
    {
        result = false;  // explicit
        goto cleanup;
    }

    WStrConcatMany(lpTempFilePathWStr,         // _Inout_ struct WStr       *lpDestWStr
                   lpTempDirPathWStr,          // _In_    const struct WStr *lpSrcWStr
                   &WSTR_FROM_LITERAL(L"\\"),  // _In_    const struct WStr *lpSrcWStr2
                   &tempFileNameWStr);         // ...
    result = true;
cleanup:
    if (NULL != tempDirPathWStr2.lpWCharArr)
    {
        WStrFree(&tempDirPathWStr2);
    }
    if (NULL != tempFileNameWStr.lpWCharArr)
    {
        WStrFree(&tempFileNameWStr);
    }
    return result;
}

void
Win32TempCreateFile(// @Nullable
                    _In_  struct WStr      *lpNullableTempDirPathWStr,
                    // @EmptyStringAllowed
                    _In_  struct WStr      *lpPrefixWStr,
                    // @EmptyStringAllowed
                    _In_  struct WStr      *lpSuffixWStr,
                    _Out_ struct Win32File *lpFile)
{
    if (false == Win32TempCreateFile2(lpNullableTempDirPathWStr,  // _In_  struct WStr      *lpNullableTempDirPathWStr
                                      lpPrefixWStr,               // _In_  struct WStr      *lpPrefixWStr
                                      lpSuffixWStr,               // _In_  struct WStr      *lpSuffixWStr
                                      lpFile,                     // _Out_ struct Win32File *lpFile
                                      stderr))                    // _Out_ FILE             *lpErrorStream
    {
        abort();
    }
}

bool
Win32TempCreateFile2(// @Nullable
                     _In_  struct WStr      *lpNullableTempDirPathWStr,
                     // @EmptyStringAllowed
                     _In_  struct WStr      *lpPrefixWStr,
                     // @EmptyStringAllowed
                     _In_  struct WStr      *lpSuffixWStr,
                     _Out_ struct Win32File *lpFile,
                     _Out_ FILE             *lpErrorStream)
{
    assert(NULL != lpPrefixWStr);
    assert(NULL != lpSuffixWStr);
    assert(NULL != lpFile);
    assert(NULL != lpErrorStream);

    if (false == Win32TempCreateTempFilePath2(lpNullableTempDirPathWStr,       // _In_  struct WStr *lpNullableTempDirPathWStr
                                              lpPrefixWStr,                    // _In_  struct WStr *lpPrefixWStr
                                              lpSuffixWStr,                    // _In_  struct WStr *lpSuffixWStr
                                              &lpFile->indirect.filePathWStr,  // _Out_ struct WStr *lpTempFilePathWStr
                                              lpErrorStream))                  // _Out_ FILE        *lpErrorStream
    {
        return false;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/secauthz/generic-access-rights
    lpFile->indirect.dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    // "Enables subsequent open operations on a file or device to request read access.
    //  Otherwise, no process can open the file or device if it requests read access.
    lpFile->indirect.dwShareMode = FILE_SHARE_READ;
    lpFile->indirect.lpSecurityAttributes = NULL;
    // "Creates a new file, only if it does not already exist.
    //  If the specified file exists, the function fails and the last-error code is set to ERROR_FILE_EXISTS (80)."
    lpFile->indirect.dwCreationDisposition = CREATE_NEW;
    lpFile->indirect.dwFlagsAndAttributes =
        // "The file is read only. Applications can read the file, but cannot write to or delete it."
        FILE_ATTRIBUTE_READONLY
        // "The file is being used for temporary storage."
        | FILE_ATTRIBUTE_TEMPORARY
        // "The file is to be deleted immediately after all of its handles are closed,
        //  which includes the specified handle and any other open or duplicated handles.
        | FILE_FLAG_DELETE_ON_CLOSE;
    lpFile->indirect.hTemplateFile = NULL;

    if (false == Win32FileCreate2(lpFile,          // _Out_ struct Win32File *lpFile
                                  lpErrorStream))  // _Out_ FILE             *lpErrorStream
    {
        return false;
    }
    return true;
}

void
Win32TempLogWF(_In_ struct Win32File *lpFile,
               // @EmptyStringAllowed
               _In_ const wchar_t    *lpMessageFormat,
               ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpMessageFormat);

    if (false == Win32TempLogWFV2(lpFile,           // _In_    struct Win32File *lpFile
                                  stderr,           // _Inout_ FILE             *lpErrorStream
                                  lpMessageFormat,  // _In_    const wchar_t    *lpMessageFormat
                                  ap))              // _In_    va_list           ap
    {
        abort();
    }
    va_end(ap);
}

bool
Win32TempLogWF2(_In_    struct Win32File *lpFile,
                _Inout_ FILE             *lpErrorStream,
                // @EmptyStringAllowed
                _In_    const wchar_t    *lpMessageFormat,
                ...)
{
    // Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170
    va_list ap;
    va_start(ap, lpMessageFormat);

    const bool x = 
        Win32TempLogWFV2(lpFile,           // _In_    struct Win32File *lpFile
                         lpErrorStream,    // _Inout_ FILE             *lpErrorStream
                         lpMessageFormat,  // _In_    const wchar_t    *lpMessageFormat
                         ap);              // _In_    va_list           ap
    va_end(ap);
    return x;
}

void
Win32TempLogWFV(_In_    struct Win32File *lpFile,
                // @EmptyStringAllowed
                _In_    const wchar_t    *lpMessageFormat,
                _In_    va_list           ap)
{
    if (false == Win32TempLogWFV2(lpFile,           // _In_    struct Win32File *lpFile
                                  stderr,           // _Inout_ FILE             *lpErrorStream
                                  lpMessageFormat,  // _In_    const wchar_t    *lpMessageFormat
                                  ap))              // _In_    va_list           ap
    {
        abort();
    }
}

bool
Win32TempLogWFV2(_In_    struct Win32File *lpFile,
                 _Inout_ FILE             *lpErrorStream,
                 // @EmptyStringAllowed
                 _In_    const wchar_t    *lpMessageFormat,
                 _In_    va_list           ap)
{
    assert(NULL != lpFile);
    assert(NULL != lpFile->stream.lpFile);
    assert(NULL != lpErrorStream);
    assert(NULL != lpMessageFormat);

    struct WStr wstr = {0};
    if (false == Win32FileReadAll2(lpFile,          // _Inout_ struct Win32File *lpFile
                                   &wstr,           // _Inout_ struct WStr      *lpWStr
                                   lpErrorStream))  // _Out_   FILE             *lpErrorStream
    {
        return false;
    }

    LogWFV(lpErrorStream, lpMessageFormat, ap);
    fputws(L": ", lpErrorStream);
    fputws(wstr.lpWCharArr, lpErrorStream);
    fputws(L"\r\n", lpErrorStream);

    WStrFree(&wstr);

    return true;
}

