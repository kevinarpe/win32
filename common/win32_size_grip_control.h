#ifndef H_COMMON_WIN32_SIZE_GRIP_CONTROL
#define H_COMMON_WIN32_SIZE_GRIP_CONTROL

#include "win32.h"
#include "wstr.h"

// Assumes 100% 96 DPI: 1px margin, plus 5x 2px squares
extern const UINT8    WIN32_SIZE_GRIP_CONTROL_WIDTH_AND_HEIGHT;  // = 17;
extern const wchar_t *WIN32_SIZE_GRIP_CONTROL_CLASS_NAMEW;       // = L"WIN32_SIZE_GRIP_CONTROL";

enum EWin32SizeGripControlOrientation
{
    // SGC: SizeGripControl
    WIN32_SGC_BOTTOM_LEFT  = 1,
    WIN32_SGC_BOTTOM_RIGHT = 2,
};

struct Win32SizeGripControlCreateParams
{
    enum EWin32SizeGripControlOrientation orientation;
};

void
Win32SizeGripControlInit(_In_ const HINSTANCE hInstance);

#endif  // H_COMMON_WIN32_SIZE_GRIP_CONTROL

