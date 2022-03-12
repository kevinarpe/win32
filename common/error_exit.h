#ifndef _H_ERROR_EXIT
#define _H_ERROR_EXIT

#include <sal.h>    // required for _In_

void ErrorExit(_In_ const char *lpszMsg);

void ErrorExitF(_In_ const char *lpszMsgFmt, ...);

#endif  // _H_ERROR_EXIT

