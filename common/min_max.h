#ifndef H_MIN_MAX
#define H_MIN_MAX

#include "win32.h"
#include <sal.h>    // required for _Inout_, etc.

SHORT MinShort(_In_ const SHORT a,
               _In_ const SHORT b);

SHORT MaxShort(_In_ const SHORT a,
               _In_ const SHORT b);

#endif  // H_MIN_MAX

