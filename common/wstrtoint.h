#ifndef H_COMMON_STRTOINT
#define H_COMMON_STRTOINT

#include "wstr.h"
#include "windef.h"  // required for BOOL

// Ref: https://en.cppreference.com/w/c/types/integer
#include <stdint.h>  // required for uint8_t

BOOL
WStrTryParseToUInt8(_In_    const struct WStr *lpWStr,
                    _In_    const int          base,
                    _Out_   uint8_t           *lpUInt8,
                    _Inout_ FILE              *fp,
                    _In_    const wchar_t     *lpszMsgFmt, ...);

#endif  // H_COMMON_STRTOINT

