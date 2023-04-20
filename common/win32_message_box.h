#ifndef H_COMMON_WIN32_MESSAGE_BOX
#define H_COMMON_WIN32_MESSAGE_BOX

#include "win32.h"
#include "wstr.h"

// TODO: Find a way to allow callers to control the 'body' of the message box.
// Example: L"WC_LINK"/SysLink: https://learn.microsoft.com/en-us/windows/win32/controls/syslink-control-entry

extern const wchar_t *WIN32_MESSAGE_BOX_CLASS_NAMEW;  // = L"WIN32_MESSAGE_BOX";
//#define WIN32_MESSAGE_BOX_CLASS_NAMEW L"WIN32_MESSAGE_BOX"

// Ref: https://learn.microsoft.com/en-us/windows/win32/api/commctrl/ns-commctrl-taskdialog_button
struct Win32MessageBoxButton
{
    int         nButtonId;
    struct WStr buttonTextWStr;
};

struct Win32MessageBoxButtonArr
{
    struct Win32MessageBoxButton *lpButtonArr;
    size_t                        ulSize;
};

#define DEFAULT_BUTTON_IS_DISABLED ((int) -1)
#define ESCAPE_BUTTON_IS_DISABLED  ((int) -1)

// TODO: Add feature flags for "centering" the dialog.  Center over parent?  Left/Right/Top/Bottom of parent?  Center of main monitor?  Precise coordinate?

struct Win32MessageBoxCreateParams
{
//    HWND                          hWndParent;
//    wchar_t                      *pWindowTitleWCharArr;
//    // @Nullable
//    HICON                           hIcon;
    // Ex: MAKEINTRESOURCE(IDI_INFORMATION)
    // @Nullable
    wchar_t                         *lpNullableIconName;
    struct WStr                      messageWStr;
    struct Win32MessageBoxButtonArr  buttonArr;
    // Index to buttonArr when Enter/Return key is pressed.  To disable this feature, use: DEFAULT_BUTTON_IS_DISABLED
    int                              nDefaultButtonId;
    // Index to buttonArr when Escape key is pressed.        To disable this feature, use: ESCAPE_BUTTON_IS_DISABLED
    int                              nEscapeButtonId;
    LOGFONTW                         logFont;  // TODO: Allow "NULL" for system font
};

void
Win32MessageBoxInit(_In_ const HINSTANCE hInstance);

// TODO: Instead of WS_STATICW, use read-only edit control.
// Ref: https://stackoverflow.com/a/6320749/257299

// TODO: Support Ctrl+C to copy text from message box

// Modal vs non-modal!
// https://stackoverflow.com/questions/734674/creating-a-win32-modal-window-with-createwindow
// https://devblogs.microsoft.com/oldnewthing/20040227-00/?p=40463
// https://devblogs.microsoft.com/oldnewthing/20050218-00/?p=36413

// Centered over parent or not?  Or absolute location?

// How to support <b>, <u>, <i> tags?

#endif  // H_COMMON_WIN32_MESSAGE_BOX

