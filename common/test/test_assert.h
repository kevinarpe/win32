#include "wstr.h"
#include <wchar.h>

void AssertWStrArrEqual(_In_ const struct WStrArr  *wstrArr,
                        _In_ const wchar_t        **lppExpectedArr,
                        _In_ const size_t           ulExpectedCount);

