#include "win32_message_box.h"
#include "win32_dpi.h"
#include "win32_last_error.h"
#include "xmalloc.h"
#include "win32_hwnd.h"
#include "win32_icon.h"
#include "win32_text.h"
#include "win32_layout.h"
#include "log.h"
#include <assert.h>

// extern
const wchar_t *WIN32_MESSAGE_BOX_CLASS_NAMEW = L"WIN32_MESSAGE_BOX";

static struct Global
{
    WNDCLASSEXW wndClassExW;
}
global = {0};

// Space between child window and edge of dialog
const UINT MARGIN = 10;

struct ButtonRectArr
{
    RECT   *lpButtonRectArr;
    size_t  ulSize;
};

struct Layout
{   
//    struct LayoutConfig     config;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-createstructw
//    CREATESTRUCTW           createStruct;
    struct Win32DPI         dpi;
    // Ex: 5 * 144 / 96 = 7.5 -> 8
    UINT                    scaledMargin;
    struct Win32IconDimensions iconDim;
    // Ex: 128 * (144 / 96) = 192
//    SIZE                    listBoxScaledMinSize;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logfontw
//    LOGFONTW                logFont;
    HFONT                   hFont;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-textmetricw
//    TEXTMETRICW             fontMetrics;
    // Ex: L"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
//    struct WStr             fontSampleWStr;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/windef/ns-windef-size
    // Ex: 930x33           
//    SIZE                    fontSampleSize;
//    struct Win32MonitorInfo primaryMonitorInfo;
    // Coordinates are relative to 'primaryMonitorInfo'.  .left & .top are x & y coordinates.
    RECT           windowNonClientRect;
    RECT           iconRect;
    RECT           labelRect;
    struct ButtonRectArr    buttonRectArr;
};

struct Window
{
    HWND                               hWnd;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-createstructw
    CREATESTRUCTW                      createStruct;
    struct Win32MessageBoxCreateParams createParams;
    HACCEL              hAccel;
    struct Layout       layout;
    BOOL                               bIsDpiChanging;
    struct Win32DPI                    dpi;
    HICON               hIcon;
    HWND                hStaticMessage;
//    UINT                cButtons;
    HWND               *pHButtonArr;
};

// Used by Set/GetWindowLongPtrW(...)
static const int WINDOW_LONG_PTR_INDEX = 0;

// TODO: Fix window resize so there is *zero* extra work.
static void
WindowLayoutInit(_Inout_ struct Window *lpWin)
{
    assert(NULL != lpWin);

//LAST: Read compiler error msgs

//    struct Win32MessageBoxCreateParams *lpCreateParams = lpCreateStruct->lpCreateParams;
    struct Layout *lpLayout = &lpWin->layout;

    Win32DPIGet(&lpLayout->dpi, lpWin->hWnd);

    // Ex: 10 * 144 / 96 = 15
    lpLayout->scaledMargin = MulDiv(MARGIN,                    // [in] int nNumber
                                    lpLayout->dpi.dpi,         // [in] int nNumerator
                                    USER_DEFAULT_SCREEN_DPI);  // [in] int nDenominator

    struct Win32IconDimensions iconDim = {0};  // = { .width = 0, .height = 0 };
    if (NULL != lpWin->createParams.lpNullableIconName)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadiconw
        lpWin->hIcon = LoadIconW(global.wndClassExW.hInstance,              // [in, optional] HINSTANCE hInstance
                                 lpWin->createParams.lpNullableIconName);   // [in]           LPCWSTR   lpIconName

        Win32IconGetDimensions(lpWin->hIcon,  // _In_  const HICON                 hIcon
                               &iconDim);     // _Out_ struct Win32IconDimensions *lpIconDim
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfontindirectw
    lpLayout->hFont = CreateFontIndirectW(&lpWin->createParams.logFont);
    if (NULL == lpLayout->hFont)
    {
        Win32LastErrorFPrintFWAbort(stderr,                                                                         // _In_ FILE          *lpStream
                                    L"CreateFontIndirectW(lfHeight:%ld, lfFaceName[%ls])",                          // _In_ const wchar_t *lpMessageFormat
                                    lpWin->createParams.logFont.lfHeight, lpWin->createParams.logFont.lfFaceName);  // _In_ ...
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc
    HDC hDC = GetDC(lpWin->hWnd);  // [in] HWND hWnd
    if (NULL == hDC)
    {
        Win32LastErrorFPutWSAbort(stderr,                  // _In_ FILE          *lpStream
                                  L"GetDC(lpWin->hWnd)");  // _In_ const wchar_t *lpMessage
    }

//win32_font.h
//Win32LogFontWEquals

    SIZE messageTextSize = {0};
    Win32TextCalcSize(hDC,                                                     // _In_  HDC                hDC
                      lpLayout->hFont,                                         // _In_  HFONT              hFont
                      (const struct WStr *) &lpWin->createParams.messageWStr,  // _In_  const struct WStr *lpWStr
                      (UINT) (                                                 // _In_  UINT               format
                          DT_CALCRECT  // "Determines the width and height of the rectangle.
                                       //  If there is only one line of text, DrawText modifies the right side of the rectangle
                                       //  so that it bounds the last character in the line.
                                       //  DrawText returns the height of the formatted text but does not draw the text."
                          | DT_NOPREFIX),  // "Turns off processing of prefix characters.
                                           //  Normally, DrawText interprets the mnemonic-prefix character & as a directive to underscore the character that follows,
                                           //  and the mnemonic-prefix characters && as a directive to print a single &.
                                           //  By specifying DT_NOPREFIX, this processing is turned off."
                      &messageTextSize);                                       // _Out_ SIZE              *lpSize

    struct Win32LayoutDefaultButtonSize defaultButtonSize = {0};
    Win32LayoutCalcDefaultButtonSize(hDC,                  // _In_  HDC                                  hDC
                                     lpLayout->hFont,      // _In_  HFONT                                hFont
                                     &defaultButtonSize);  // _Out_ struct Win32LayoutDefaultButtonSize *lpDefaultButtonSize

    size_t allButtonsWidth = 0;
    // Assumption: All buttons will be same height; most buttons will be same width -- a few will be wider.
    SIZE *lpButtonSizeArr =
        xcalloc(lpWin->createParams.buttonArr.ulSize,  // _In_ const size_t ulNumItem
                sizeof(SIZE));                     // _In_ const size_t ulSizeOfEachItem

    for (size_t i = 0; i < lpWin->createParams.buttonArr.ulSize; ++i)
    {
        struct Win32MessageBoxButton *lpButton = lpWin->createParams.buttonArr.lpButtonArr + i;
        SIZE *lpButtonSize = lpButtonSizeArr + i;

        Win32LayoutCalcButtonSize(hDC,                        // _In_  HDC                                  hDC
                                  &lpButton->buttonTextWStr,  // _In_  struct WStr                         *lpButtonText
                                  &defaultButtonSize,         // _In_  struct Win32LayoutDefaultButtonSize *lpDefaultButtonSize
                                  lpButtonSize);              // _Out_ SIZE                                *lpButtonSize
        if (allButtonsWidth > 0)
        {
            allButtonsWidth += lpLayout->scaledMargin;
        }
        allButtonsWidth += lpButtonSize->cx;
    }

    // There is tension between: (1) optional icon + message and (2) buttons.
    // Calc width of both, then select max.
    const size_t width  = lpLayout->scaledMargin + iconDim.width + (0 == iconDim.width ? 0 : 2 * lpLayout->scaledMargin) + messageTextSize.cx + lpLayout->scaledMargin;
    const size_t width2 = lpLayout->scaledMargin + allButtonsWidth + lpLayout->scaledMargin;

    lpLayout->windowNonClientRect.right = max(width, width2);

    const size_t height  = lpLayout->scaledMargin + iconDim.height + (0 == iconDim.height ? 0 : 2 * lpLayout->scaledMargin) + lpButtonSizeArr[0].cy + lpLayout->scaledMargin;
    const size_t height2 = lpLayout->scaledMargin + messageTextSize.cy + (2 * lpLayout->scaledMargin) + lpButtonSizeArr[0].cy + lpLayout->scaledMargin;

    lpLayout->windowNonClientRect.bottom = max(height, height2);

    lpLayout->buttonRectArr.ulSize = lpWin->createParams.buttonArr.ulSize;

    lpLayout->buttonRectArr.lpButtonRectArr =
        xcalloc(lpLayout->buttonRectArr.ulSize,  // _In_ const size_t ulNumItem
                sizeof(SIZE));                   // _In_ const size_t ulSizeOfEachItem
/*
+--------------------------------------------------------+
|      10                                                |
|    +------------+        +------------------------+    |
| 10 | (optional) |   20   | message                | 10 |
|    | icon       |        |                        |    |
|    +------------+        |                        |    |
|                          |                        |    |
|                          +------------------------+    |
|                                                        |
|                                             20         |
|                                                        |
|                    +----------+        +----------+    |
|                    |   Yes    |   20   |    No    |    |
|                    +----------+        +----------+    |
|                                             10         |
+--------------------------------------------------------+
*/
    LONG nextButtonRight = lpLayout->windowNonClientRect.right - lpLayout->scaledMargin;

    for (size_t i = 0; i < lpWin->createParams.buttonArr.ulSize; ++i)
    {
        SIZE *lpButtonSize = lpButtonSizeArr + i;
        RECT *lpButtonRect = lpLayout->buttonRectArr.lpButtonRectArr + i;

        lpButtonRect->left   = nextButtonRight - lpButtonSize->cx;
        lpButtonRect->top    = lpLayout->windowNonClientRect.bottom - lpLayout->scaledMargin - lpButtonSize->cy;
        lpButtonRect->right  = nextButtonRight;
        lpButtonRect->bottom = lpLayout->windowNonClientRect.bottom - lpLayout->scaledMargin;

        nextButtonRight     -= lpButtonSize->cx + (2 * lpLayout->scaledMargin);
    }

    lpLayout->iconRect.left   = lpLayout->scaledMargin;
    lpLayout->iconRect.top    = lpLayout->scaledMargin;
    lpLayout->iconRect.right  = lpLayout->iconRect.left + iconDim.width;
    lpLayout->iconRect.bottom = lpLayout->iconRect.top  + iconDim.height;

    lpLayout->labelRect.left   = lpLayout->iconRect.right + (0 == iconDim.width ? 0 : 2 * lpLayout->scaledMargin);
    lpLayout->labelRect.top    = lpLayout->scaledMargin;
    lpLayout->labelRect.right  = lpLayout->labelRect.left + messageTextSize.cx;
    lpLayout->labelRect.bottom = lpLayout->labelRect.top  + messageTextSize.cy;

    xfree((void **) &lpButtonSizeArr);  // _Inout_ void **lppData
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-nccreate
static void
Win32MessageBoxWindowProc_WM_NCCREATE(_In_ const HWND   hWnd,
                                      __attribute__((unused))
                                      _In_ const WPARAM wParam,
                                      __attribute__((unused))
                                      _In_ const LPARAM lParam)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-createstructw
    CREATESTRUCTW *lpCreateStruct = (CREATESTRUCTW *) lParam;
    if (NULL == lpCreateStruct->lpCreateParams)
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32MessageBoxWindowProc_WM_NCCREATE: CREATESTRUCTW.lpCreateParams is NULL: Expected struct Win32MessageBoxCreateParams");  // _In_ const wchar_t *lpMessage
    }
    struct Win32MessageBoxCreateParams *lpCreateParams = (struct Win32MessageBoxCreateParams *) lpCreateStruct->lpCreateParams;
    // TODO: Do asserts on lpCreateParams.  Example: Positive number of buttons!
    WStrAssertValid(&lpCreateParams->messageWStr);
    assert(lpCreateParams->buttonArr.ulSize > 0);
    assert(DEFAULT_BUTTON_IS_DISABLED == lpCreateParams->nDefaultButtonId
            || (lpCreateParams->nDefaultButtonId >= 0
                && ((size_t) lpCreateParams->nDefaultButtonId) < lpCreateParams->buttonArr.ulSize));
    assert(ESCAPE_BUTTON_IS_DISABLED == lpCreateParams->nEscapeButtonId
            || (lpCreateParams->nEscapeButtonId >= 0
                && ((size_t) lpCreateParams->nEscapeButtonId) < lpCreateParams->buttonArr.ulSize));

    struct Window *lpWin = xcalloc(1, sizeof(struct Window));
    *lpWin = (struct Window) {
        .hWnd           = hWnd,
        // Intentional: Value copy.  Ownership of lpCreateStruct is not passed.
        .createStruct   = *lpCreateStruct,
        // Intentional: Value copy.  Ownership of lpCreateParams is not passed.
        .createParams   = *lpCreateParams,
        .bIsDpiChanging = FALSE,
    };
    Win32DPIGet(&lpWin->dpi, hWnd);

    Win32SetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, lpWin,
                           L"Win32MessageBoxWindowProc_WM_NCCREATE: SetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, lpWin)");
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-create
static void
Win32MessageBoxWindowProc_WM_CREATE(_In_ const HWND   hWnd,
                                    __attribute__((unused))
                                    _In_ const WPARAM wParam,
                                    __attribute__((unused))
                                    _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                                                  L"Win32MessageBoxWindowProc_WM_CREATE: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    WindowLayoutInit(lpWin);

//LAST: Call CreateWindow for widgets in lpWin!
}

// Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
static LRESULT CALLBACK
Win32MessageBoxWindowProc(_In_ const HWND   hWnd,
                          _In_ const UINT   uMsg,
                          _In_ const WPARAM wParam,
                          _In_ const LPARAM lParam)
{   
    DEBUG_LOGWF(stdout, L"INFO: Win32MessageBoxWindowProc(HWND hWnd[%p], UINT uMsg[%u/%ls], WPARAM wParam[%llu], LPARAM lParam[%lld])\r\n",
                hWnd, uMsg, Win32MsgToText(uMsg), wParam, lParam);
    switch (uMsg)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-nccreate
        case WM_NCCREATE:
        {   
            Win32MessageBoxWindowProc_WM_NCCREATE(hWnd, wParam, lParam);
            // "If an application processes this message, it should return TRUE to continue creation of the window."
            return TRUE;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-create
        case WM_CREATE:
        {
//LAST: Compile works, but finish clean-up.
            Win32MessageBoxWindowProc_WM_CREATE(hWnd, wParam, lParam);
            // "If an application processes this message, it should return zero to continue creation of the window."
            return 0;
        }
    }
//    DEBUG_LOGW(stdout, L"Win32SizeGripControlWindowProc: DefWindowProc(...)\r\n");
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-defwindowprocw
    const LRESULT x = DefWindowProc(hWnd, uMsg, wParam, lParam);
    return x;
}

void
Win32MessageBoxInit(_In_ const HINSTANCE hInstance)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexw
    global.wndClassExW = (WNDCLASSEXW) {
        // "The size, in bytes, of this structure. Set this member to sizeof(WNDCLASSEX). Be sure to set this member before calling the GetClassInfoEx function."
        .cbSize = (UINT) sizeof(WNDCLASSEXW),
        // "The class style(s). This member can be any combination of the Class Styles."
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-class-styles
        .style = (UINT) (
            CS_DROPSHADOW),  // "Enables the drop shadow effect on a window."
        // "A pointer to the window procedure. You must use the CallWindowProc function to call the window procedure."
        .lpfnWndProc = (WNDPROC) Win32MessageBoxWindowProc,
        // "The number of extra bytes to allocate following the window-class structure. The system initializes the bytes to zero."
        .cbClsExtra = (int) 0,
        // "The number of extra bytes to allocate following the window instance. The system initializes the bytes to zero."
        // Ref: https://stackoverflow.com/a/65876605/257299
        .cbWndExtra = (int) (1 + WINDOW_LONG_PTR_INDEX) * sizeof(void *),
        // "A handle to the instance that contains the window procedure for the class."
        // Ref: https://stackoverflow.com/questions/20140117/why-does-createwindow-take-a-hinstance-as-an-argument-if-was-already-provided
        // Ref: https://devblogs.microsoft.com/oldnewthing/20050418-59/?p=35873
        .hInstance = hInstance,
        // "A handle to the class icon. This member must be a handle to an icon resource. If this member is NULL, the system provides a default icon."
        .hIcon = (HICON) 0,
        // "A handle to the class cursor. This member must be a handle to a cursor resource.
        //  If this member is NULL, an application must explicitly set the cursor shape whenever the mouse moves into the application's window.
        .hCursor = (HCURSOR) NULL,
        // "A handle to the class background brush. This member can be a handle to the physical brush to be used for painting the background, or it can be a color value.
        //  A color value must be one of the following standard system colors (the value 1 must be added to the chosen color)."
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsyscolor
        // "COLOR_BTNFACE: Windows 10 or greater: This value is not supported."
        .hbrBackground = (HBRUSH) (1 + COLOR_BTNFACE),  // COLOR_BTNHIGHLIGHT, COLOR_BTNSHADOW
        // "Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
        //  If you use an integer to identify the menu, use the MAKEINTRESOURCE macro. If this member is NULL, windows belonging to this class have no default menu."
        .lpszMenuName = (LPCWSTR) 0,
        // "A pointer to a null-terminated string or is an atom. If this parameter is an atom,
        //  it must be a class atom created by a previous call to the RegisterClass or RegisterClassEx function.
        //  The atom must be in the low-order word of lpszClassName; the high-order word must be zero.
        //  If lpszClassName is a string, it specifies the window class name. The class name can be any name registered with RegisterClass or RegisterClassEx,
        //  or any of the predefined control-class names.
        //  The maximum length for lpszClassName is 256. If lpszClassName is greater than the maximum length, the RegisterClassEx function will fail."
        .lpszClassName = (LPCWSTR) WIN32_MESSAGE_BOX_CLASS_NAMEW,
        // "A handle to a small icon that is associated with the window class.
        //  If this member is NULL, the system searches the icon resource specified by the hIcon member for an icon of the appropriate size to use as the small icon."
        .hIconSm = (HICON) NULL,
    };

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
    const ATOM registerClassExAtom = RegisterClassExW(&global.wndClassExW);
    if (0 == registerClassExAtom)
    {
        Win32LastErrorFPutWSAbort(stderr,                                    // _In_ FILE          *lpStream
                                  L"Win32SizeGripControl: RegisterClassW");  // _In_ const wchar_t *lpMessage
    }
}

