#include <Windows.h>
#include <mshtml.h>
#include <tchar.h>
#include <iostream>

// Global variables
HWND g_hMainWindow;
HWND g_hAddressBar;
HWND g_hWebView;
IWebBrowser2* g_pWebBrowser; // Global pointer to IWebBrowser2 interface

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitializeMainWindow(HINSTANCE hInstance);
void CreateBrowserComponents(HWND hWnd);
void NavigateToURL(const wchar_t* url);
void UpdateNavigationButtons();
void NavigateBack();
void NavigateForward();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CoInitialize(NULL); // Initialize COM for using IWebBrowser2

    InitializeMainWindow(hInstance);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize(); // Uninitialize COM before exiting
    return static_cast<int>(msg.wParam);
}

void InitializeMainWindow(HINSTANCE hInstance) {
    const wchar_t CLASS_NAME[] = L"YewGamerBrowserClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // White background

    RegisterClass(&wc);

    g_hMainWindow = CreateWindowEx(
        0,                          // Optional window styles
        CLASS_NAME,                 // Window class name
        L"YewGamer Browser",       // Window title
        WS_OVERLAPPEDWINDOW,        // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, // Size and position
        NULL,                       // Parent window
        NULL,                       // Menu
        hInstance,                  // Instance handle
        NULL                        // Additional application data
    );

    if (g_hMainWindow == NULL) {
        return;
    }

    ShowWindow(g_hMainWindow, SW_SHOWDEFAULT);
    UpdateWindow(g_hMainWindow);

    CreateBrowserComponents(g_hMainWindow);
}

void CreateBrowserComponents(HWND hWnd) {
    g_hAddressBar = CreateWindowEx(
        0, L"EDIT", NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        60, 0, 600, 30,
        hWnd, NULL, NULL, NULL
    );

    // Create navigation buttons
    CreateWindow(L"BUTTON", L"<", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 50, 30, hWnd, (HMENU)1, NULL, NULL);

    CreateWindow(L"BUTTON", L">", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        30, 0, 50, 30, hWnd, (HMENU)2, NULL, NULL);

    g_hWebView = CreateWindowEx(
        0, L"Internet Explorer_Server", NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        0, 30, 800, 570,
        hWnd, NULL, NULL, NULL
    );

    // Initialize IWebBrowser2 interface
    CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (void**)&g_pWebBrowser);

    // Navigate to a default URL
    NavigateToURL(L"https://example.com");
}

void NavigateToURL(const wchar_t* url) {
    if (g_pWebBrowser) {
        VARIANT ve;
        VariantInit(&ve);
        ve.vt = VT_BSTR;
        ve.bstrVal = SysAllocString(url);
        g_pWebBrowser->Navigate2(&ve, 0, 0, 0, 0);
        VariantClear(&ve);

        // Update address bar with the new URL
        SetWindowText(g_hAddressBar, url);

        // Update navigation buttons state
        UpdateNavigationButtons();
    }
}

void UpdateNavigationButtons() {
    if (g_pWebBrowser) {
        // Enable or disable back button based on navigation history
        EnableWindow(GetDlgItem(g_hMainWindow, 1), g_pWebBrowser->CanGoBack() ? TRUE : FALSE);

        // Enable or disable forward button based on navigation history
        EnableWindow(GetDlgItem(g_hMainWindow, 2), g_pWebBrowser->CanGoForward() ? TRUE : FALSE);
    }
}

void NavigateBack() {
    if (g_pWebBrowser) {
        g_pWebBrowser->GoBack();
        UpdateNavigationButtons();
    }
}

void NavigateForward() {
    if (g_pWebBrowser) {
        g_pWebBrowser->GoForward();
        UpdateNavigationButtons();
    }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_COMMAND:
            // Handle button clicks
            switch (LOWORD(wParam)) {
                case 1: // Back button
                    NavigateBack();
                    break;
                case 2: // Forward button
                    NavigateForward();
                    break;
                default:
                    break;
            }
            break;
        case WM_DESTROY:
            if (g_pWebBrowser) {
                g_pWebBrowser->Release();
                g_pWebBrowser = nullptr;
            }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
