#include "win32_size_grip_control.h"
#include "win32_last_error.h"
#include "log.h"
#include "win32_dpi.h"
#include "win32_hwnd.h"
#include "xmalloc.h"
#include <windowsx.h>

// extern
const UINT8    WIN32_SIZE_GRIP_CONTROL_WIDTH_AND_HEIGHT = 17;
// extern
const wchar_t *WIN32_SIZE_GRIP_CONTROL_CLASS_NAMEW      = L"WIN32_SIZE_GRIP_CONTROL";

static struct Global
{
    WNDCLASSEXW wndClassExW;
}
global = {0};

// Width and height of each small square inside the component.
// Always multiple of 2 and inner square is exactly half.
static const LONG SQUARE_LEN = 2;

struct Window
{
    HWND                                    hWnd;
    BOOL                                    bIsDpiChanging;
    struct Win32DPI                         dpi;
    HCURSOR                                 hCursorActive;
    HCURSOR                                 hCursorPrev;
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-createstructw
    CREATESTRUCTW                           createStruct;
    struct Win32SizeGripControlCreateParams createParams;
    POINT                                   prevLeftMouseButtonDownPoint;
};

// Used by Set/GetWindowLongPtrW(...)
static const int WINDOW_LONG_PTR_INDEX = 0;

// Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-nccreate
static void
Win32SizeGripControlWindowProc_WM_NCCREATE(_In_ const HWND   hWnd,
                                           __attribute__((unused))
                                           _In_ const WPARAM wParam,
                                           __attribute__((unused))
                                           _In_ const LPARAM lParam)
{
    struct Window *lpWin = xcalloc(1, sizeof(struct Window));

    *lpWin = (struct Window) {
        .hWnd           = hWnd,
        .bIsDpiChanging = FALSE,
    };
    Win32DPIGet(&lpWin->dpi, hWnd);

    Win32SetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, lpWin,
                           L"Win32SizeGripControlWindowProc_WM_NCCREATE: SetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX, lpWin)");
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-create
static void
Win32SizeGripControlWindowProc_WM_CREATE(_In_ const HWND   hWnd,
                                         __attribute__((unused))
                                         _In_ const WPARAM wParam,
                                         _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                                                  L"Win32SizeGripControlWindowProc_WM_CREATE: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-createstructw
    lpWin->createStruct = *(CREATESTRUCTW *) lParam;
    if (NULL == lpWin->createStruct.lpCreateParams)
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_CREATE: CREATESTRUCTW.lpCreateParams is NULL: Expected struct Win32SizeGripControlCreateParams");  // _In_ const wchar_t *lpMessage
    }

    lpWin->createParams = *(struct Win32SizeGripControlCreateParams *) lpWin->createStruct.lpCreateParams;

    if (WIN32_SGC_BOTTOM_LEFT != lpWin->createParams.orientation && WIN32_SGC_BOTTOM_RIGHT != lpWin->createParams.orientation)
    {
        Win32LastErrorFPrintFWAbort(stderr,                                                                           // _In_ FILE          *lpStream
                                    L"Win32SizeGripControlCreateParams.orientation: Expected WIN32_SGC_BOTTOM_LEFT:%d or WIN32_SGC_BOTTOM_RIGHT:%d, but found: %d",                                  // _In_ const wchar_t *lpMessageFormat
                                    WIN32_SGC_BOTTOM_LEFT, WIN32_SGC_BOTTOM_RIGHT, lpWin->createParams.orientation);  // _In_ ...
    }

    const wchar_t *lpCursorName = (WIN32_SGC_BOTTOM_LEFT == lpWin->createParams.orientation) ? IDC_SIZENESW : IDC_SIZENWSE;

//LAST: LEFT SIDE: MOUSE OVER SET CURSOR IS BUGGY!

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadcursorw
    lpWin->hCursorActive =
        LoadCursorW(
            // "To use one of the predefined cursors, the application must set the hInstance parameter to NULL
            //  and the lpCursorName parameter to one the following values."
            NULL,           // [in, optional] HINSTANCE hInstance
            lpCursorName);  // [in]           LPCWSTR   lpCursorName

    if (NULL == lpWin->hCursorActive)
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_NCCREATE: LoadCursorW(NULL, IDC_SIZENWSE)");  // _In_ const wchar_t *lpMessage
    }
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/gdi/wm-paint
static void
Win32SizeGripControlWindowProc_WM_PAINT(_In_ const HWND   hWnd,
                                        __attribute__((unused))
                                        _In_ const WPARAM wParam,
                                        __attribute__((unused))
                                        _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                                                  L"Win32SizeGripControlWindowProc_WM_PAINT: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");

    // Ref: https://www.codeproject.com/Articles/617212/Custom-Controls-in-Win-API-The-Painting
    // Ref: https://learn.microsoft.com/en-us/windows/win32/gdi/painting-and-drawing-messages
    // Ref: https://stackoverflow.com/questions/51950182/how-can-i-keep-reusing-hbitmap-and-hdc-continually
    // Ref: HTHEME GetWindowTheme(HWND hWnd): https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/nf-uxtheme-getwindowtheme
    // Ref: HBRUSH GetThemeSysColorBrush(HTHEME, int iColorId): https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/nf-uxtheme-getthemesyscolorbrush

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-paintstruct
    PAINTSTRUCT ps = {0};
    const HDC hDC = BeginPaint(hWnd,  // [in]  HWND          hWnd
                               &ps);  // [out] LPPAINTSTRUCT lpPaint
    if (NULL == hDC)
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: BeginPaint(hWnd, &ps)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclientrect
    RECT rect = {0};
    if (0 == GetClientRect(hWnd,    // [in]  HWND   hWnd,
                           &rect))  // [out] LPRECT lpRect
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: GetClientRect(hWnd, &rect)");  // _In_ const wchar_t *lpMessage
    }

    const LONG width  = rect.right - rect.left;
    const LONG height = rect.bottom - rect.top;

    // Ref: https://www.codeproject.com/Articles/617212/Custom-Controls-in-Win-API-The-Painting
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createcompatibledc
    const HDC hDCMem = CreateCompatibleDC(ps.hdc);  // [in] HDC hdc
    if (NULL == hDCMem)
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: CreateCompatibleDC(...)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createcompatiblebitmap
    const HBITMAP hBitmap = CreateCompatibleBitmap(ps.hdc,  // [in] HDC hdc
                                             width,      // [in] int cx
                                             height);    // [in] int cy
    if (NULL == hBitmap)
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: CreateCompatibleBitmap(...)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectobject
    const HBITMAP hBitmapPrev = SelectObject(hDCMem,    // [in] HDC     hdc
                                             hBitmap);  // [in] HGDIOBJ h
    if (NULL == hBitmapPrev)
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: SelectObject(hDCMem, hBitmap)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-fillrect
    if (0 == FillRect(hDCMem,                             // [in] HDC        hDC
//                      &lpPs->rcPaint,                     // [in] const RECT *lprc
                      &rect,                              // [in] const RECT *lprc
                      global.wndClassExW.hbrBackground))  // [in] HBRUSH     hbr
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: FillRect(hDCMem, &rect, global.wndClassExW.hbrBackground)");  // _In_ const wchar_t *lpMessage
    }

    // WIN32_SGC_BOTTOM_LEFT
    // Draw: *:square, #:order of drawing
    //     *1
    //     *2  *3
    //     *4  *5  *6
    //     *7  *8  *9  *10
    //
    // WIN32_SGC_BOTTOM_RIGHT
    // Draw: *:square, #:order of drawing
    //                 *1
    //             *3  *2
    //         *6  *5  *4
    //     *10 *9  *8  *7
    //
    // Each square: -:light, +:dark
    // +-
    // --
    //
    // 200% scaling (2 x 96 DPI -> 192 DPI)
    // ++--
    // ++--
    // ----
    // ----

    const UINT scale          = MulDiv(1,                         // [in] int nNumber
                                       lpWin->dpi.dpi,            // [in] int nNumerator
                                       // Ref: https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged
                                       // "The base value of DPI is defined as USER_DEFAULT_SCREEN_DPI which is set to 96."
                                       USER_DEFAULT_SCREEN_DPI);  // [in] int nDenominator

    const LONG minLen         = min(width, height);
    const LONG squareLen      = SQUARE_LEN * scale;
    const LONG innerSquareLen = squareLen / 2;
    const LONG margin         = innerSquareLen;
    const LONG maxSquareCount = (minLen - margin) / (squareLen + margin);
    const BOOL isBottomLeft   = (WIN32_SGC_BOTTOM_LEFT == lpWin->createParams.orientation);

    for (LONG i = 1; i <= maxSquareCount; ++i)
    {
        const LONG lTop = margin + ((i - 1) * (squareLen + margin));
        for (LONG j = 1; j <= i; ++j)
        {
            const LONG lLeft = isBottomLeft ? (margin + ((j - 1) * (squareLen + margin))) : (minLen - (j * (squareLen + margin)));
            const RECT r = {.top    = lTop,
                            .left   = lLeft,
                            .bottom = lTop  + squareLen,
                            .right  = lLeft + squareLen,};

            if (0 == FillRect(hDCMem,                              // [in] HDC         hDC
                              &r,                                  // [in] const RECT *lprc
                              (HBRUSH) (1 + COLOR_BTNHIGHLIGHT)))  // [in] HBRUSH      hbr
            {
                Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                          L"Win32SizeGripControlWindowProc_WM_PAINT: FillRect(hDCMem, &r, (HBRUSH) (1 + COLOR_BTNHIGHLIGHT))");  // _In_ const wchar_t *lpMessage
            }

            const RECT r2 = {.top    = lTop,
                             .left   = lLeft,
                             .bottom = lTop  + innerSquareLen,
                             .right  = lLeft + innerSquareLen,};

            if (0 == FillRect(hDCMem,                           // [in] HDC         hDC
                              &r2,                              // [in] const RECT *lprc
                              (HBRUSH) (1 + COLOR_BTNSHADOW)))  // [in] HBRUSH      hbr
            {
                Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                          L"Win32SizeGripControlWindowProc_WM_PAINT: FillRect(hDCMem, &r2, (HBRUSH) (1 + COLOR_BTNSHADOW))");  // _In_ const wchar_t *lpMessage
            }
            __attribute__((unused)) int dummy = 1;
        }
        __attribute__((unused)) int dummy = 1;
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-bitblt
    if (0 == BitBlt(ps.hdc,  // [in] HDC   hdc
                    ps.rcPaint.left,  // [in] int   x
                    ps.rcPaint.top,   // [in] int   y
                    width,  // [in] int   cx
                    height,  // [in] int   cy
                    hDCMem,  // [in] HDC   hdcSrc
                    0,  // [in] int   x1
                    0,  // [in] int   y1
                    SRCCOPY))  //  [in] DWORD rop
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: BitBlt(...)");  // _In_ const wchar_t *lpMessage
    }

    if (NULL == SelectObject(hDCMem,        // [in] HDC     hdc
                             hBitmapPrev))  // [in] HGDIOBJ h
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: SelectObject(hDCMem, hBitmapPrev)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-deleteobject
    if (FALSE == DeleteObject(hBitmap))  // [in] HGDIOBJ ho
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: DeleteObject(hBitmap)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-deletedc
    if (FALSE == DeleteDC(hDCMem))  // [in] HDC hdc
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: DeleteDC(hDCMem)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
    if (!EndPaint(hWnd,  // [in] HWND               hWnd
                  &ps))  // [in] const PAINTSTRUCT *lpPaint
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_PAINT: EndPaint(hWnd, &ps)");  // _In_ const wchar_t *lpMessage
    }
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged
static void
Win32SizeGripControlWindowProc_WM_DPICHANGED(_In_ const HWND   hWnd,
                                             __attribute__((unused))
                                             _In_ const WPARAM wParam,
                                             __attribute__((unused))
                                             _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                                                  L"Win32SizeGripControlWindowProc_WM_DPICHANGED: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    Win32DPIGet(&lpWin->dpi, hWnd);
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged-beforeparent
static void
Win32SizeGripControlWindowProc_WM_DPICHANGED_BEFOREPARENT(_In_ const HWND   hWnd,
                                                          __attribute__((unused))
                                                          _In_ const WPARAM wParam,
                                                          __attribute__((unused))
                                                          _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                                                  L"Win32SizeGripControlWindowProc_WM_DPICHANGED_BEFOREPARENT: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    Win32DPIGet(&lpWin->dpi, hWnd);
    lpWin->bIsDpiChanging = TRUE;
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged-afterparent
static void
Win32SizeGripControlWindowProc_WM_DPICHANGED_AFTERPARENT(_In_ const HWND   hWnd,
                                                          __attribute__((unused))
                                                          _In_ const WPARAM wParam,
                                                          __attribute__((unused))
                                                          _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                                                  L"Win32SizeGripControlWindowProc_WM_DPICHANGED_AFTERPARENT: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    lpWin->bIsDpiChanging = FALSE;
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttondown
static void
Win32SizeGripControlWindowProc_WM_LBUTTONDOWN(_In_ const HWND   hWnd,
                                              __attribute__((unused))
                                              _In_ const WPARAM wParam,
                                              __attribute__((unused))
                                              _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                                                  L"Win32SizeGripControlWindowProc_WM_LBUTTONDOWN: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
//    DEBUG_LOGWF(stdout, L"Win32SizeGripControlWindowProc_WM_LBUTTONDOWN: B: lpWin->prevLeftMouseButtonDownPoint: x: %ld, y: %ld\r\n", lpWin->prevLeftMouseButtonDownPoint.x, lpWin->prevLeftMouseButtonDownPoint.y);
    lpWin->prevLeftMouseButtonDownPoint = (POINT) { .x = GET_X_LPARAM(lParam), .y = GET_Y_LPARAM(lParam) };
//    DEBUG_LOGWF(stdout, L"Win32SizeGripControlWindowProc_WM_LBUTTONDOWN: A: lpWin->prevLeftMouseButtonDownPoint: x: %ld, y: %ld\r\n", lpWin->prevLeftMouseButtonDownPoint.x, lpWin->prevLeftMouseButtonDownPoint.y);
}

__attribute__((unused))
static HWND
GetTopLevelWindow(_In_ const HWND hWnd)
{
    // Ref: https://stackoverflow.com/a/62593995/257299

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getparent
    // "Remarks: To obtain a window's owner window, instead of using GetParent, use GetWindow with the GW_OWNER flag.
    //  To obtain the parent window and not the owner, instead of using GetParent, use GetAncestor with the GA_PARENT flag."
/*
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindow
    const HWND hWndOwner = GetWindow(hWnd,       // [in] HWND hWnd,
                                     GW_OWNER);  // [in] UINT uCmd
    if (NULL != hWndOwner)
    {
//        DEBUG_LOGWF(stdout, L"Win32SizeGripControl: GetWindow(hWnd, GW_OWNER): %p\r\n", hWndOwner);
        return hWndOwner;
    }
*/
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getancestor
    const HWND hWndParent = GetAncestor(hWnd,        // [in] HWND hwnd,
                                        // "Retrieves the parent window. This does not include the owner, as it does with the GetParent function."
                                        GA_PARENT);  // [in] UINT gaFlags
    if (NULL == hWndParent)
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControl: GetAncestor(hWnd, GA_PARENT)");  // _In_ const wchar_t *lpMessage
    }
//    DEBUG_LOGWF(stdout, L"Win32SizeGripControl: GetAncestor(hWnd, GA_PARENT): %p\r\n", hWndParent);
    return hWndParent;
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
static void
Win32SizeGripControlWindowProc_WM_MOUSEMOVE(_In_ const HWND   hWnd,
                                            __attribute__((unused))
                                            _In_ const WPARAM wParam,
                                            _In_ const LPARAM lParam)
{
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                                                  L"Win32SizeGripControlWindowProc_WM_MOUSEMOVE: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
    if (!(wParam & MK_LBUTTON))
    {
        return;
    }
    const POINT p = { .x = GET_X_LPARAM(lParam), .y = GET_Y_LPARAM(lParam) };
    const LONG widthDelta  = p.x - lpWin->prevLeftMouseButtonDownPoint.x;
    const LONG heightDelta = p.y - lpWin->prevLeftMouseButtonDownPoint.y;
    if (0 == widthDelta && 0 == heightDelta)
    {
        DEBUG_LOGW(stdout, L"Win32SizeGripControlWindowProc_WM_MOUSEMOVE: 0 == widthDelta && 0 == heightDelta\r\n");
        return;
    }

//    const HWND hWndTopLevel = GetTopLevelWindow(hWnd);
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getancestor
    const HWND hWndTopLevel = GetAncestor(hWnd,        // [in] HWND hwnd,
                                          // "Retrieves the parent window. This does not include the owner, as it does with the GetParent function."
                                          GA_PARENT);  // [in] UINT gaFlags
    if (NULL == hWndTopLevel)
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_MOUSEMOVE: GetAncestor(hWnd, GA_PARENT)");  // _In_ const wchar_t *lpMessage
    }

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setlasterror
    SetLastError(0);  // [in] DWORD dwErrCode

    POINT p2 = p;
    if (0 == MapWindowPoints(hWnd,          // [in]      HWND    hWndFrom
                             hWndTopLevel,  // [in]      HWND    hWndTo
                             &p2,           // [in, out] LPPOINT lpPoints
                             1)             // [in]      UINT    cPoints
        && 0 != GetLastError())
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_MOUSEMOVE: MapWindowPoints");  // _In_ const wchar_t *lpMessage
    }

    RECT winRect = {0};
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowrect
    if (0 == GetWindowRect(hWndTopLevel,  // [in]  HWND   hWnd
                           &winRect))     // [out] LPRECT lpRect
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_MOUSEMOVE: GetWindowRect(hWndTopLevel, &winRect)");  // _In_ const wchar_t *lpMessage
    }
    DEBUG_LOGWF(stdout, L"Win31SizeGripControlWindowProc_WM_MOUSEMOVE: B: hWndTopLevel: %p, x: %ld, y: %ld, width: %ld, height: %ld, p.x: %ld->%ld, p.y: %ld->%ld, dw: %ld, dh: %ld\r\n",
                hWndTopLevel, winRect.left, winRect.top, winRect.right - winRect.left, winRect.bottom - winRect.top, p.x, p2.x, p.y, p2.y, widthDelta, heightDelta);

    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos
    if (!SetWindowPos(hWndTopLevel,                                // [in]           HWND hWnd
                      NULL,                                        // [in, optional] HWND hWndInsertAfter
                      // "The new position of the left side of the window, in client coordinates."
                      // Note: For top-level windows (NULL == hWndParent), "client coordinates" are actually "screen coordinates".
                      winRect.left,                                // [in]           int  X
                      winRect.top,                                 // [in]           int  Y
                      // "The new width of the window, in pixels."
                      winRect.right - winRect.left + widthDelta,   // [in]           int  cx
                      // "The new height of the window, in pixels."
                      winRect.bottom - winRect.top + heightDelta,  // [in]           int  cy
                      SWP_NOACTIVATE | SWP_NOZORDER))              // [in]           UINT uFlags
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_MOUSEMOVE: SetWindowPos(hWndTopLevel, ...)");  // _In_ const wchar_t *lpMessage
    }                                                                                                                                                                       

    RECT winRect2 = {0};
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowrect
    if (0 == GetWindowRect(hWndTopLevel,  // [in]  HWND   hWnd
                           &winRect2))    // [out] LPRECT lpRect
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_MOUSEMOVE: GetWindowRect(hWndTopLevel, &winRect2)");  // _In_ const wchar_t *lpMessage
    }
    DEBUG_LOGWF(stdout, L"Win32SizeGripControlWindowProc_WM_MOUSEMOVE: A: hWndTopLevel: %p, x: %ld, y: %ld, width: %ld, height: %ld\r\n",
                hWndTopLevel, winRect2.left, winRect2.top, winRect2.right - winRect2.left, winRect2.bottom - winRect2.top);

    lpWin->prevLeftMouseButtonDownPoint = p;
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttonup
static void
Win32SizeGripControlWindowProc_WM_LBUTTONUP(_In_ const HWND   hWnd,
                                            __attribute__((unused))
                                            _In_ const WPARAM wParam,
                                            __attribute__((unused))
                                            _In_ const LPARAM lParam)
{
    __attribute__((unused))
    struct Window *lpWin = Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                                                  L"Win32SizeGripControlWindowProc_WM_LBUTTONUP: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");
}

// Ref: https://learn.microsoft.com/en-us/windows/win32/menurc/wm-setcursor
static void
Win32SizeGripControlWindowProc_WM_SETCURSOR(_In_ const HWND   hWnd,
                                            __attribute__((unused))
                                            _In_ const WPARAM wParam,
                                            __attribute__((unused))
                                            _In_ const LPARAM lParam)
{
    struct Window *lpWin =
        Win32GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX,
                               L"Win32SizeGripControlWindowProc_WM_SETCURSOR: GetWindowLongPtrW(hWnd, WINDOW_LONG_PTR_INDEX)");

//    DEBUG_LOGWF(stdout, L"Win32SizeGripControlWindowProc_WM_SETCURSOR: hWnd: %p, (HWND) wParam: %p\r\n", hWnd, (HWND) wParam);
    POINT p = {0};
    if (FALSE == GetCursorPos(&p))  // [out] LPPOINT lpPoint
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_SETCURSOR: GetCursorPos");  // _In_ const wchar_t *lpMessage
    }

    SetLastError(0);
    if (0 == MapWindowPoints(HWND_DESKTOP,  // [in]      HWND    hWndFrom
                             hWnd,          // [in]      HWND    hWndTo
                             &p,            // [in, out] LPPOINT lpPoints
                             1)             // [in]      UINT    cPoints
        && 0 != GetLastError())
    {
        Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                  L"Win32SizeGripControlWindowProc_WM_SETCURSOR: MapWindowPoints");  // _In_ const wchar_t *lpMessage
    }

    BOOL isActiveCursor = FALSE;
    if (WIN32_SGC_BOTTOM_LEFT == lpWin->createParams.orientation)
    {
        isActiveCursor = (p.x <= p.y);
DEBUG_LOGWF(stdout, L"Win32SizeGripControlWindowProc_WM_SETCURSOR: WIN32_SGC_BOTTOM_LEFT: isActiveCursor:%s = (y:%d <= x:%d)\r\n",
            (isActiveCursor ? "TRUE" : "FALSE"), p.y, p.x);
    }
    else  // if (WIN32_SGC_BOTTOM_RIGHT == lpWin->createParams.orientation)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclientrect
        RECT rect = {0};
        if (0 == GetClientRect(hWnd,    // [in]  HWND   hWnd,
                               &rect))  // [out] LPRECT lpRect
        {
            Win32LastErrorFPutWSAbort(stderr,  // _In_ FILE          *lpStream
                                      L"Win32SizeGripControlWindowProc_WM_SETCURSOR: WIN32_SGC_BOTTOM_RIGHT: GetClientRect(hWnd, &rect)");  // _In_ const wchar_t *lpMessage
        }

        const LONG height = rect.bottom - rect.top;
        isActiveCursor = (p.x >= (height - p.y));
DEBUG_LOGWF(stdout, L"Win32SizeGripControlWindowProc_WM_SETCURSOR: WIN32_SGC_BOTTOM_RIGHT: isActiveCursor:%s = ((height:%d - p.y:%d) <= p.x:%d)\r\n",
            (isActiveCursor ? "TRUE" : "FALSE"), height, p.y, p.x);
    }

    if (isActiveCursor)
    {
        lpWin->hCursorPrev = SetCursor(lpWin->hCursorActive);
DEBUG_LOGWF(stdout, L"Win32SizeGripControlWindowProc_WM_SETCURSOR: SetCursor: %p -> %p\r\n", lpWin->hCursorPrev, lpWin->hCursorActive);
    }
    else
    {
        if (NULL != lpWin->hCursorPrev)
        {
            __attribute__((unused))
            const HCURSOR hCursorPrev = SetCursor(lpWin->hCursorPrev);
DEBUG_LOGWF(stdout, L"Win32SizeGripControlWindowProc_WM_SETCURSOR: SetCursor: %p -> %p\r\n", lpWin->hCursorPrev, lpWin->hCursorActive);
            lpWin->hCursorPrev = NULL;
        }
        else
        {
DEBUG_LOGW(stdout, L"NULL == lpWin->hCursorPrev\r\n");
        }
    }
}

// Ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
static LRESULT CALLBACK
Win32SizeGripControlWindowProc(_In_ const HWND   hWnd,
                               _In_ const UINT   uMsg,
                               _In_ const WPARAM wParam,
                               _In_ const LPARAM lParam)
{
    DEBUG_LOGWF(stdout, L"INFO: Win32SizeGripControlWindowProc(HWND hWnd[%p], UINT uMsg[%u/%ls], WPARAM wParam[%llu], LPARAM lParam[%lld])\r\n",
                hWnd, uMsg, Win32MsgToText(uMsg), wParam, lParam);
    switch (uMsg)
    {
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-nccreate
        case WM_NCCREATE:
        {
            Win32SizeGripControlWindowProc_WM_NCCREATE(hWnd, wParam, lParam);
            // "If an application processes this message, it should return TRUE to continue creation of the window."
            return TRUE;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-create
        case WM_CREATE:
        {
            Win32SizeGripControlWindowProc_WM_CREATE(hWnd, wParam, lParam);
            // "If an application processes this message, it should return zero to continue creation of the window."
            return 0;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-erasebkgnd
        case WM_ERASEBKGND:
        {
            // "An application should return nonzero if it erases the background; otherwise, it should return zero."
            return 0;
        }
        // Ref: https://learn.microsoft.com/en-us/windows/win32/gdi/wm-paint
        case WM_PAINT:
        {
            Win32SizeGripControlWindowProc_WM_PAINT(hWnd, wParam, lParam);
            // "An application returns zero if it processes this message."
            return 0;
        }
        // Intentional: Do not listen for WM_DPICHANGED.  This message is only sent to top-level windows.
        case WM_DPICHANGED:
        {
            Win32SizeGripControlWindowProc_WM_DPICHANGED(hWnd, wParam, lParam);
            // "An application returns zero if it processes this message."
            return 0;
        }
        case WM_DPICHANGED_BEFOREPARENT:
        {
            Win32SizeGripControlWindowProc_WM_DPICHANGED_BEFOREPARENT(hWnd, wParam, lParam);
            // "This value is unused and ignored by the system."
            return 0;
        }
        case WM_DPICHANGED_AFTERPARENT:
        {
            Win32SizeGripControlWindowProc_WM_DPICHANGED_AFTERPARENT(hWnd, wParam, lParam);
            // "This value is unused and ignored by the system."
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            Win32SizeGripControlWindowProc_WM_LBUTTONDOWN(hWnd, wParam, lParam);
            // "An application returns zero if it processes this message."
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            Win32SizeGripControlWindowProc_WM_MOUSEMOVE(hWnd, wParam, lParam);
            // "An application returns zero if it processes this message."
            return 0;
        }
        case WM_LBUTTONUP:
        {
            Win32SizeGripControlWindowProc_WM_LBUTTONUP(hWnd, wParam, lParam);
            // "An application returns zero if it processes this message."
            return 0;
        }
        case WM_SETCURSOR:
        {
            Win32SizeGripControlWindowProc_WM_SETCURSOR(hWnd, wParam, lParam);
            // "An application returns zero if it processes this message."
            return 0;
        }
    }
//    DEBUG_LOGW(stdout, L"Win32SizeGripControlWindowProc: DefWindowProc(...)\r\n");
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-defwindowprocw
    const LRESULT x = DefWindowProc(hWnd, uMsg, wParam, lParam);
    return x;
}

void
Win32SizeGripControlInit(_In_ const HINSTANCE hInstance)
{
    // Ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexw
    global.wndClassExW = (WNDCLASSEXW) {
        // "The size, in bytes, of this structure. Set this member to sizeof(WNDCLASSEX). Be sure to set this member before calling the GetClassInfoEx function."
        .cbSize = (UINT) sizeof(WNDCLASSEXW),
        // "The class style(s). This member can be any combination of the Class Styles."
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-class-styles
        .style = (UINT) 0,
        // "A pointer to the window procedure. You must use the CallWindowProc function to call the window procedure."
        .lpfnWndProc = (WNDPROC) Win32SizeGripControlWindowProc,
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
        .lpszClassName = (LPCWSTR) WIN32_SIZE_GRIP_CONTROL_CLASS_NAMEW,
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

