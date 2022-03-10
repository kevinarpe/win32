#include "min_max.h"

SHORT MinShort(_In_ const SHORT a,
               _In_ const SHORT b)
{
    if (a < b) {
        return a;
    }
    else {
        return b;
    }
}

SHORT MaxShort(_In_ const SHORT a,
               _In_ const SHORT b)
{
    if (a > b) {
        return a;
    }
    else {
        return b;
    }
}

