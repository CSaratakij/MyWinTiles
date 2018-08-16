
#include "stdafx.h"
#include "MyWinTiles.h"

#define APPBAR_WINDOW_CLASS "TESTBAR"

#define MAX_LOADSTRING 100
#define MOD MOD_ALT

#define MAX_WORKSPACE 10
#define MAX_WINDOW_PER_WORKSPACE 20

#define HOTKEY_DESTROY_WINDOW 1001

#define HOTKEY_MINIMIZE_WINDOW 1002
#define HOTKEY_MAXIMIZE_WINDOW 1003
#define HOTKEY_MAXIMIZE_ALL_WINDOW 1004
#define HOTKEY_MINIMIZE_ALL_WINDOW 1005

#define HOTKEY_SWITHTO_NEXT_WINDOW 1006
#define HOTKEY_SWITHTO_PREVIOUS_WINDOW 1007

#define HOTKEY_TILE_VERTICAL 1008
#define HOTKEY_TILE_HORIZONTAL 1009
#define HOTKEY_REFRESH_TILE 1010

#define HOTKEY_OPEN_TERMINAL 1011

#define HOTKEY_SWAPWINDOW_NEXT 1012
#define HOTKEY_SWAPWINDOW_PREVIOUS 1013

#define HOTKEY_SWITHTO_WORKSPACE_1 2001
#define HOTKEY_SWITHTO_WORKSPACE_2 2002
#define HOTKEY_SWITHTO_WORKSPACE_3 2003
#define HOTKEY_SWITHTO_WORKSPACE_4 2004
#define HOTKEY_SWITHTO_WORKSPACE_5 2005
#define HOTKEY_SWITHTO_WORKSPACE_6 2006
#define HOTKEY_SWITHTO_WORKSPACE_7 2007
#define HOTKEY_SWITHTO_WORKSPACE_8 2008
#define HOTKEY_SWITHTO_WORKSPACE_9 2009
#define HOTKEY_SWITHTO_WORKSPACE_10 2010

#define HOTKEY_SWITHTO_NEXT_WORKSPACE 2011
#define HOTKEY_SWITHTO_PREVIOUS_WORKSPACE 2012

#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_1 5001
#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_2 5002
#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_3 5003
#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_4 5004
#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_5 5005
#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_6 5006
#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_7 5007
#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_8 5008
#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_9 5009
#define HOTKEY_MOVEWINDOW_TO_WORKSPACE_10 5010

#define OVERLAP_WINDOW_MODE 0x02

#define SWAPWINDOW_NEXT 0x01
#define SWAPWINDOW_PREVIOUS 0x02

#define HOTKEY_CLOSE_TESTBAR 8000
#define HOTKEY_TOGGLE_EXPLORER_TASKBAR 8001

HWND bar = NULL;

struct WORKSPACE
{
	UINT id;
	UINT focusWindowID;
	UINT currentTotalWindow;
	HWND* windows;
};

int currentFocusIndice[MAX_WORKSPACE];
int totalWindowInWorkspace[MAX_WORKSPACE];

UINT SHELLHOOK_MSG;
UINT currentWorkSpace = 1;
UINT previousWorkSpace = 1;
UINT iterator = 0;

UINT workspaceTileMode[MAX_WORKSPACE];
BOOL isFocusByHotkey = FALSE;

//Replace this with 2 dimension array
HWND windowOfWorkSpace1[MAX_WINDOW_PER_WORKSPACE];
HWND windowOfWorkSpace2[MAX_WINDOW_PER_WORKSPACE];
HWND windowOfWorkSpace3[MAX_WINDOW_PER_WORKSPACE];
HWND windowOfWorkSpace4[MAX_WINDOW_PER_WORKSPACE];
HWND windowOfWorkSpace5[MAX_WINDOW_PER_WORKSPACE];
HWND windowOfWorkSpace6[MAX_WINDOW_PER_WORKSPACE];
HWND windowOfWorkSpace7[MAX_WINDOW_PER_WORKSPACE];
HWND windowOfWorkSpace8[MAX_WINDOW_PER_WORKSPACE];
HWND windowOfWorkSpace9[MAX_WINDOW_PER_WORKSPACE];
HWND windowOfWorkSpace10[MAX_WINDOW_PER_WORKSPACE];

BOOL CALLBACK InitWorkSpaces_Callback(HWND, LPARAM);
BOOL CALLBACK MaximizeAllWindows(HWND, LPARAM);
BOOL CALLBACK MinimizeAllWindows(HWND, LPARAM);

HWND GetWindowByWorkspaceID(UINT, UINT);
HWND* GetWorkspaceByID(UINT);
BOOL IsValidWindow(HWND);
BOOL AddWindowToWorkspace(HWND, UINT);
BOOL MoveWindowToWorkspaceByID(HWND, UINT, UINT);
BOOL SwitchToWorkspace(UINT);
BOOL RemoveWindowFromWorkspace(HWND, UINT);

void SwitchToNextWorkspace();
void SwitchToPreviousWorkspace();
void TileWindowVertical();
void TileWindowHorizontal();
void HideWorkspaceByID(UINT);
void ShowWorkspaceByID(UINT);
void UpdateCurrentWorkspaceLayout();
void UpdateWorkspaceLayout(UINT);
void FocusWindow(UINT, UINT);
void FocusNextWindow();
void FocusPreviousWindow();
void RefreshWorkspace(UINT);
void UpdateTotalWindowInWorkspace(UINT);
void SwapCurrentFocusWindow(UINT, UINT);
void SendCurrentWorkspaceThroughIPC(HWND hWnd);

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MYWINTILES, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYWINTILES));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0) != 0)
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		if (msg.message == SHELLHOOK_MSG) {
			switch (msg.wParam) {
			case HSHELL_WINDOWCREATED:
			{
				HWND hWnd = (HWND)msg.lParam;
				bar = FindWindow(_T(APPBAR_WINDOW_CLASS), NULL);

				if (bar == hWnd)
					break;

				if (!IsWindow(hWnd) && IsValidWindow(hWnd))
					hWnd = GetLastActivePopup(hWnd);

				BOOL isCanAddWindow = AddWindowToWorkspace(hWnd, currentWorkSpace);

				if (isCanAddWindow) {
					int totalWindow = totalWindowInWorkspace[currentWorkSpace - 1];
					totalWindow += 1;

					totalWindowInWorkspace[currentWorkSpace - 1] = totalWindow;
					currentFocusIndice[currentWorkSpace - 1] = (totalWindow - 1);
				}

				UpdateCurrentWorkspaceLayout();
				break;
			}

			case HSHELL_WINDOWDESTROYED:
			{
				//Performance hog in 3, 2 , 1
				HWND hWnd = (HWND) msg.lParam;
				BOOL isCanRemove = FALSE;
				isCanRemove = RemoveWindowFromWorkspace(hWnd, currentWorkSpace);
				if (isCanRemove) {
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HSHELL_RUDEAPPACTIVATED:
			{
				if (isFocusByHotkey) {
					isFocusByHotkey = FALSE;
					break;
				}

				HWND hWnd = (HWND) msg.lParam;

				int indice = currentFocusIndice[currentWorkSpace - 1];
				HWND* currentAry = GetWorkspaceByID(currentWorkSpace);

				if (hWnd != currentAry[indice]) {
					for (UINT i = 0; i < MAX_WORKSPACE; ++i) {
						if (currentAry[i] == hWnd) {
							currentFocusIndice[currentWorkSpace - 1] = i;
							break;
						}
					}
				}

				break;
			}

			default:
				break;
			}
		}
		else if (msg.message == WM_HOTKEY) {

			switch (msg.wParam)
			{
			case HOTKEY_MINIMIZE_WINDOW:
			{
				workspaceTileMode[currentWorkSpace - 1] = OVERLAP_WINDOW_MODE;
				HWND targetWindow = GetForegroundWindow();
				ShowWindowAsync(targetWindow, SW_MINIMIZE);
				break;
			}
			case HOTKEY_MINIMIZE_ALL_WINDOW:
			{
				workspaceTileMode[currentWorkSpace - 1] = OVERLAP_WINDOW_MODE;
				EnumWindows(&MinimizeAllWindows, NULL);
				break;
			}

			case HOTKEY_MAXIMIZE_WINDOW:
			{
				workspaceTileMode[currentWorkSpace - 1] = OVERLAP_WINDOW_MODE;
				HWND targetWindow = GetForegroundWindow();
				ShowWindowAsync(targetWindow, SW_MAXIMIZE);
				break;
			}

			case HOTKEY_MAXIMIZE_ALL_WINDOW:
			{
				workspaceTileMode[currentWorkSpace - 1] = OVERLAP_WINDOW_MODE;
				EnumWindows(&MaximizeAllWindows, currentWorkSpace);
				break;
			}

			case HOTKEY_DESTROY_WINDOW:
			{
				HWND targetWindow = GetForegroundWindow();
				RemoveWindowFromWorkspace(targetWindow, currentWorkSpace);

				SendMessage(targetWindow, WM_CLOSE, 0, 0);
				UpdateCurrentWorkspaceLayout();

				FocusPreviousWindow();
				break;
			}

			case HOTKEY_TILE_VERTICAL:
			{
				workspaceTileMode[currentWorkSpace - 1] = MDITILE_VERTICAL;
				TileWindowVertical();
				break;
			}

			case HOTKEY_TILE_HORIZONTAL:
			{
				workspaceTileMode[currentWorkSpace - 1] = MDITILE_HORIZONTAL;
				TileWindowHorizontal();
				break;
			}

			case HOTKEY_REFRESH_TILE:
			{
				RefreshWorkspace(currentWorkSpace);
				break;
			}

			case HOTKEY_OPEN_TERMINAL:
			{
				break;
			}

			case HOTKEY_SWITHTO_WORKSPACE_1:
			{
				SwitchToWorkspace(1);
				FocusWindow(1, currentFocusIndice[0]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWITHTO_WORKSPACE_2:
			{
				SwitchToWorkspace(2);
				FocusWindow(2, currentFocusIndice[1]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_3:
			{
				SwitchToWorkspace(3);
				FocusWindow(3, currentFocusIndice[2]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_4:
			{
				SwitchToWorkspace(4);
				FocusWindow(4, currentFocusIndice[3]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_5:
			{
				SwitchToWorkspace(5);
				FocusWindow(5, currentFocusIndice[4]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_6:
			{
				SwitchToWorkspace(6);
				FocusWindow(6, currentFocusIndice[5]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_7:
			{
				SwitchToWorkspace(7);
				FocusWindow(7, currentFocusIndice[6]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_8:
			{
				SwitchToWorkspace(8);
				FocusWindow(8, currentFocusIndice[7]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWITHTO_WORKSPACE_9:
			{
				SwitchToWorkspace(9);
				FocusWindow(9, currentFocusIndice[8]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWITHTO_WORKSPACE_10:
			{
				SwitchToWorkspace(10);
				FocusWindow(10, currentFocusIndice[9]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWITHTO_NEXT_WINDOW:
			{
				FocusNextWindow();
				break;
			}

			case HOTKEY_SWITHTO_PREVIOUS_WINDOW:
			{
				FocusPreviousWindow();
				break;
			}

			case HOTKEY_SWITHTO_NEXT_WORKSPACE:
			{
				SwitchToNextWorkspace();
				FocusWindow(currentWorkSpace, currentFocusIndice[currentWorkSpace - 1]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWITHTO_PREVIOUS_WORKSPACE:
			{
				SwitchToPreviousWorkspace();
				FocusWindow(currentWorkSpace, currentFocusIndice[currentWorkSpace - 1]);
				SendCurrentWorkspaceThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWAPWINDOW_NEXT:
			{
				SwapCurrentFocusWindow(currentWorkSpace, SWAPWINDOW_NEXT);
				break;
			}

			case HOTKEY_SWAPWINDOW_PREVIOUS:
			{
				SwapCurrentFocusWindow(currentWorkSpace, SWAPWINDOW_PREVIOUS);
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_1:
			{
				if (currentWorkSpace != 1) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 1);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[0] += 1;
					FocusPreviousWindow();
				}
				break;
			}
			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_2:
			{
				if (currentWorkSpace != 2) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 2);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[1] += 1;
					FocusPreviousWindow();
				}
				break;
			}
			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_3:
			{
				if (currentWorkSpace != 3) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 3);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[2] += 1;
					FocusPreviousWindow();
				}
				break;
			}
			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_4:
			{
				if (currentWorkSpace != 4) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 4);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[3] += 1;
					FocusPreviousWindow();
				}
				break;
			}
			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_5:
			{
				if (currentWorkSpace != 5) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 5);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[4] += 1;
					FocusPreviousWindow();
				}
				break;
			}
			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_6:
			{
				if (currentWorkSpace != 6) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 6);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[5] += 1;
					FocusPreviousWindow();
				}
				break;
			}
			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_7:
			{
				if (currentWorkSpace != 7) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 7);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[6] += 1;
					FocusPreviousWindow();
				}
				break;
			}
			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_8:
			{
				if (currentWorkSpace != 8) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 8);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[7] += 1;
					FocusPreviousWindow();
				}
				break;
			}
			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_9:
			{
				if (currentWorkSpace != 9) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 9);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[8] += 1;
					FocusPreviousWindow();
				}
				break;
			}
			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_10:
			{
				if (currentWorkSpace != 10) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 10);
					UpdateCurrentWorkspaceLayout();
					totalWindowInWorkspace[9] += 1;
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_CLOSE_TESTBAR:
			{
				HWND testbar = FindWindow(_T(APPBAR_WINDOW_CLASS), NULL);
				SendMessage(testbar, WM_CLOSE, NULL, NULL);
				break;
			}

			case HOTKEY_TOGGLE_EXPLORER_TASKBAR:
			{
				HWND taskbar = FindWindow(_T("Shell_TrayWnd"), NULL);

				if (taskbar == NULL)
					break;

				if (IsWindowVisible(taskbar))
					ShowWindow(taskbar, SW_HIDE);
				else
					ShowWindow(taskbar, SW_SHOW);

				break;
			}

			default:
				break;
			}
		}
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYWINTILES));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MYWINTILES);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   // Register Shell Hook here...
   RegisterShellHookWindow(hWnd);
   SHELLHOOK_MSG = RegisterWindowMessage(TEXT("SHELLHOOK"));

   // Register Hotkey
   RegisterHotKey(hWnd, HOTKEY_DESTROY_WINDOW, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x51);
   RegisterHotKey(hWnd, HOTKEY_MINIMIZE_WINDOW, MOD | MOD_NOREPEAT, 0x4d);
   RegisterHotKey(hWnd, HOTKEY_MAXIMIZE_WINDOW, MOD | MOD_NOREPEAT, 0x46);
   RegisterHotKey(hWnd, HOTKEY_MINIMIZE_ALL_WINDOW, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x4d);
   RegisterHotKey(hWnd, HOTKEY_MAXIMIZE_ALL_WINDOW, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x46);

   RegisterHotKey(hWnd, HOTKEY_SWITHTO_NEXT_WINDOW, MOD | MOD_NOREPEAT, 0x4a); //Mod + j
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_PREVIOUS_WINDOW, MOD | MOD_NOREPEAT, 0x4b); //Mod + k

   RegisterHotKey(hWnd, HOTKEY_SWITHTO_NEXT_WINDOW, MOD | MOD_NOREPEAT, 0x4c); //Mod + l
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_PREVIOUS_WINDOW, MOD | MOD_NOREPEAT, 0x48); //Mod + h

   RegisterHotKey(hWnd, HOTKEY_SWITHTO_NEXT_WORKSPACE, MOD | MOD_NOREPEAT, 0x4e);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_PREVIOUS_WORKSPACE, MOD | MOD_NOREPEAT, 0x50);

   RegisterHotKey(hWnd, HOTKEY_TILE_VERTICAL, MOD | MOD_NOREPEAT, 0xba);
   RegisterHotKey(hWnd, HOTKEY_TILE_HORIZONTAL, MOD | MOD_NOREPEAT, 0x56);
   RegisterHotKey(hWnd, HOTKEY_REFRESH_TILE, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x52); //Mod + shift + R

   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_1, MOD | MOD_NOREPEAT, 0x31);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_2, MOD | MOD_NOREPEAT, 0x32);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_3, MOD | MOD_NOREPEAT, 0x33);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_4, MOD | MOD_NOREPEAT, 0x34);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_5, MOD | MOD_NOREPEAT, 0x35);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_6, MOD | MOD_NOREPEAT, 0x36);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_7, MOD | MOD_NOREPEAT, 0x37);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_8, MOD | MOD_NOREPEAT, 0x38);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_9, MOD | MOD_NOREPEAT, 0x39);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_10, MOD | MOD_NOREPEAT, 0x30);

   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_1, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x31);
   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_2, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x32);
   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_3, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x33);
   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_4, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x34);
   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_5, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x35);
   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_6, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x36);
   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_7, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x37);
   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_8, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x38);
   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_9, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x39);
   RegisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_10, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x30);

   RegisterHotKey(hWnd, HOTKEY_OPEN_TERMINAL, MOD | MOD_NOREPEAT, 0x0d);

   RegisterHotKey(hWnd, HOTKEY_SWAPWINDOW_NEXT, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x4a); //Mod + shift + j
   RegisterHotKey(hWnd, HOTKEY_SWAPWINDOW_PREVIOUS, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x4b); //Mod + shift + k

   RegisterHotKey(hWnd, HOTKEY_SWAPWINDOW_NEXT, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x4c); //Mod + shift + l
   RegisterHotKey(hWnd, HOTKEY_SWAPWINDOW_PREVIOUS, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x48); //Mod + shift + h
   
   RegisterHotKey(hWnd, HOTKEY_CLOSE_TESTBAR, MOD | MOD_NOREPEAT, VK_F11);
   RegisterHotKey(hWnd, HOTKEY_TOGGLE_EXPLORER_TASKBAR, MOD | MOD_NOREPEAT, VK_F12);

   for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {
	   windowOfWorkSpace1[i] = NULL;
	   windowOfWorkSpace2[i] = NULL;
	   windowOfWorkSpace3[i] = NULL;
	   windowOfWorkSpace4[i] = NULL;
	   windowOfWorkSpace5[i] = NULL;
	   windowOfWorkSpace6[i] = NULL;
	   windowOfWorkSpace7[i] = NULL;
	   windowOfWorkSpace8[i] = NULL;
	   windowOfWorkSpace9[i] = NULL;
	   windowOfWorkSpace10[i] = NULL;
   }

   for (UINT i = 0; i < MAX_WORKSPACE; ++i) {
	   workspaceTileMode[i] = MDITILE_VERTICAL;
	   currentFocusIndice[i] = 0;
	   totalWindowInWorkspace[i] = 0;
   }

   EnumWindows(&InitWorkSpaces_Callback, NULL);
   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
	{
		DeregisterShellHookWindow(hWnd);

	   // UnRegister Hotkey
		UnregisterHotKey(hWnd, HOTKEY_DESTROY_WINDOW);

		UnregisterHotKey(hWnd, HOTKEY_MINIMIZE_WINDOW);
		UnregisterHotKey(hWnd, HOTKEY_MAXIMIZE_WINDOW);

		UnregisterHotKey(hWnd, HOTKEY_MINIMIZE_ALL_WINDOW);
		UnregisterHotKey(hWnd, HOTKEY_MAXIMIZE_ALL_WINDOW);

		UnregisterHotKey(hWnd, HOTKEY_TILE_VERTICAL);
		UnregisterHotKey(hWnd, HOTKEY_TILE_HORIZONTAL);
		UnregisterHotKey(hWnd, HOTKEY_REFRESH_TILE);

		UnregisterHotKey(hWnd, HOTKEY_OPEN_TERMINAL);

		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_NEXT_WINDOW);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_PREVIOUS_WINDOW);

		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_NEXT_WORKSPACE);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_PREVIOUS_WORKSPACE);

		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_1);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_2);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_3);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_4);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_5);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_6);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_7);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_8);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_9);
		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_WORKSPACE_10);

		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_1);
		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_2);
		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_3);
		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_4);
		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_5);
		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_6);
		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_7);
		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_8);
		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_9);
		UnregisterHotKey(hWnd, HOTKEY_MOVEWINDOW_TO_WORKSPACE_10);

		UnregisterHotKey(hWnd, HOTKEY_SWITHTO_NEXT_WINDOW);
		UnregisterHotKey(hWnd, HOTKEY_SWAPWINDOW_PREVIOUS);

		UnregisterHotKey(hWnd, HOTKEY_CLOSE_TESTBAR);
		UnregisterHotKey(hWnd, HOTKEY_TOGGLE_EXPLORER_TASKBAR);

        PostQuitMessage(0);
        break;
	}

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

BOOL CALLBACK InitWorkSpaces_Callback(HWND hWnd, LPARAM lParam)
{
	if (iterator >= MAX_WINDOW_PER_WORKSPACE)
		return FALSE;

	if (!IsValidWindow(hWnd))
		return TRUE;

	if (!IsWindow(hWnd))
		return TRUE;

	if (AddWindowToWorkspace(hWnd, 1))
		iterator++;

	return TRUE;
}

BOOL CALLBACK MinimizeAllWindows(HWND hWnd, LPARAM lParam)
{
	for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {
		HWND target = NULL;
		target = GetWindowByWorkspaceID(currentWorkSpace, i);
		ShowWindowAsync(target, SW_MINIMIZE);
	}

	return FALSE;
}

BOOL CALLBACK MaximizeAllWindows(HWND hWnd, LPARAM lParam)
{
	int workspaceID = (int)lParam;

	for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {
		HWND target = NULL;
		target = GetWindowByWorkspaceID(workspaceID, i);
		ShowWindow(target, SW_MAXIMIZE);
	}

	return FALSE;
}

HWND GetWindowByWorkspaceID(UINT workspace, UINT id)
{
	if (workspace > MAX_WORKSPACE || id > MAX_WINDOW_PER_WORKSPACE - 1)
		return NULL;

	HWND window = NULL;

	switch (workspace) {
	case 1:
		window = windowOfWorkSpace1[id];
		break;

	case 2:
		window = windowOfWorkSpace2[id];
		break;

	case 3:
		window = windowOfWorkSpace3[id];
		break;

	case 4:
		window = windowOfWorkSpace4[id];
		break;

	case 5:
		window = windowOfWorkSpace5[id];
		break;

	case 6:
		window = windowOfWorkSpace6[id];
		break;

	case 7:
		window = windowOfWorkSpace7[id];
		break;

	case 8:
		window = windowOfWorkSpace8[id];
		break;

	case 9:
		window = windowOfWorkSpace9[id];
		break;

	case 10:
		window = windowOfWorkSpace10[id];
		break;

	default:
		break;
	}

	return window;
}

HWND* GetWorkspaceByID(UINT id)
{
	if (id > MAX_WORKSPACE)
		return NULL;

	HWND* workspace = NULL;

	if (id == 1)
		workspace = windowOfWorkSpace1;

	else if (id == 2)
		workspace = windowOfWorkSpace2;

	else if (id == 3)
		workspace = windowOfWorkSpace3;

	else if (id == 4)
		workspace = windowOfWorkSpace4;

	else if (id == 5)
		workspace = windowOfWorkSpace5;

	else if (id == 6)
		workspace = windowOfWorkSpace6;

	else if (id == 7)
		workspace = windowOfWorkSpace7;

	else if (id == 8)
		workspace = windowOfWorkSpace8;

	else if (id == 9)
		workspace = windowOfWorkSpace9;

	else if (id == 10)
		workspace = windowOfWorkSpace10;

	return workspace;
}

BOOL IsValidWindow(HWND hWnd)
{
	if (!IsWindowVisible(hWnd))
		return FALSE;

	TITLEBARINFO ti;

	ti.cbSize = sizeof(ti);
	GetTitleBarInfo(hWnd, &ti);

	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return FALSE;

	if (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return FALSE;

	return TRUE;
}

BOOL AddWindowToWorkspace(HWND hWnd, UINT id)
{
	if (id > MAX_WORKSPACE)
		return FALSE;

	HWND* ary = GetWorkspaceByID(id);

	if (ary != NULL) {
		for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {

			if (ary[i] == hWnd)
				return FALSE;

			if (ary[i] == NULL) {
				ary[i] = hWnd;
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL SwitchToWorkspace(UINT id)
{
	if (id > MAX_WORKSPACE)
		return FALSE;

	previousWorkSpace = currentWorkSpace;
	currentWorkSpace = id;

	if (currentWorkSpace != previousWorkSpace)
		HideWorkspaceByID(previousWorkSpace);

	ShowWorkspaceByID(currentWorkSpace);
	return TRUE;
}

BOOL RemoveWindowFromWorkspace(HWND hWnd, UINT workspaceID)
{
	if (workspaceID > MAX_WORKSPACE)
		return FALSE;

	HWND* currentAry = GetWorkspaceByID(workspaceID);

	for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {

		if (currentAry[i] == hWnd) {
			currentAry[i] = NULL;

			//Re-arrange window in workspace
			//(Worst Performance -> Consider tree structure)
			for (UINT j = i; j < MAX_WINDOW_PER_WORKSPACE - 1; ++j) {
				currentAry[j] = currentAry[j + 1];
			}

			totalWindowInWorkspace[workspaceID - 1] -= 1;
			return TRUE;
		}
	}

	return FALSE;
}

void SwitchToNextWorkspace()
{
	UINT targetWorkspace = (currentWorkSpace + 1 > MAX_WORKSPACE) ? 1 : (currentWorkSpace + 1);
	SwitchToWorkspace(targetWorkspace);
}

void SwitchToPreviousWorkspace()
{
	UINT targetWorkspace = (currentWorkSpace - 1 < 1) ? MAX_WORKSPACE : (currentWorkSpace - 1);
	SwitchToWorkspace(targetWorkspace);
}

BOOL MoveWindowToWorkspaceByID(HWND hWnd, UINT currentWorkSpaceID, UINT targetWorkspaceID)
{
	if (!IsValidWindow(hWnd))
		return FALSE;

	BOOL isCanInsert = FALSE;
	HWND* targetAry = GetWorkspaceByID(targetWorkspaceID);

	for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {
		if (targetAry[i] == hWnd) {
			RemoveWindowFromWorkspace(hWnd, currentWorkSpaceID);
			break;
		}
		else if (targetAry[i] == NULL) {
			targetAry[i] = hWnd;
			ShowWindowAsync(hWnd, SW_MINIMIZE);
			isCanInsert = TRUE;
			break;
		}
	}

	if (!isCanInsert)
		return FALSE;

	RemoveWindowFromWorkspace(hWnd, currentWorkSpaceID);
	return TRUE;
}

void UpdateCurrentWorkspaceLayout()
{
	UpdateWorkspaceLayout(currentWorkSpace);
}

void UpdateWorkspaceLayout(UINT workspace)
{
	if (workspace > MAX_WORKSPACE)
		return;

	if (workspaceTileMode[workspace - 1] == MDITILE_VERTICAL)
		TileWindowVertical();

	else if (workspaceTileMode[workspace - 1] == MDITILE_HORIZONTAL)
		TileWindowHorizontal();

	else if (workspaceTileMode[workspace - 1] == OVERLAP_WINDOW_MODE)
		EnumWindows(&MaximizeAllWindows, workspace);
}

void TileWindowVertical()
{
	HWND* currentAry = GetWorkspaceByID(currentWorkSpace);
	TileWindows(
		NULL,
		MDITILE_SKIPDISABLED | MDITILE_VERTICAL | MDITILE_ZORDER,
		NULL,
		MAX_WINDOW_PER_WORKSPACE,
		currentAry
	);
}

void TileWindowHorizontal()
{
	HWND* currentAry = GetWorkspaceByID(currentWorkSpace);
	TileWindows(
		NULL,
		MDITILE_SKIPDISABLED | MDITILE_HORIZONTAL | MDITILE_ZORDER,
		NULL,
		MAX_WINDOW_PER_WORKSPACE,
		currentAry
	);
}

void HideWorkspaceByID(UINT id)
{
	if (id > MAX_WORKSPACE)
		return;

	HWND* previousWindows = GetWorkspaceByID(id);

	for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {
		HWND hWnd = previousWindows[i];

		if (!IsWindow(hWnd)) {
			previousWindows[i] = NULL;
			UpdateTotalWindowInWorkspace(id);
		}

		if (workspaceTileMode[id - 1] == OVERLAP_WINDOW_MODE)
			ShowWindow(hWnd, SW_MINIMIZE);
		else
			ShowWindowAsync(hWnd, SW_MINIMIZE);
	}
}

void ShowWorkspaceByID(UINT id)
{
	if (id > MAX_WORKSPACE)
		return;

	HWND* currentWindows = GetWorkspaceByID(id);

	for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {
		HWND hWnd = currentWindows[i];
		if (IsWindow(hWnd))
			ShowWindow(hWnd, SW_SHOWNOACTIVATE);
		else
			currentWindows[i] = NULL;
	}

	RefreshWorkspace(id);
}

void FocusWindow(UINT workspace, UINT id)
{
	if (workspace > MAX_WORKSPACE)
		return;

	if (id > MAX_WINDOW_PER_WORKSPACE - 1)
		return;

	isFocusByHotkey = TRUE;

	HWND focusWindow = GetWindowByWorkspaceID(workspace, id);
	SetForegroundWindow(focusWindow);
}

void FocusNextWindow()
{
	int indice = currentFocusIndice[currentWorkSpace - 1];

	indice = (indice + 1) > totalWindowInWorkspace[currentWorkSpace - 1] - 1 ? 0 : (indice + 1);
	currentFocusIndice[currentWorkSpace - 1] = indice;

	FocusWindow(currentWorkSpace, indice);
}

void FocusPreviousWindow()
{
	int indice = currentFocusIndice[currentWorkSpace - 1];

	indice = (indice - 1) < 0 ? totalWindowInWorkspace[currentWorkSpace - 1] - 1 : (indice - 1);
	currentFocusIndice[currentWorkSpace - 1] = indice;

	FocusWindow(currentWorkSpace, indice);
}

void RefreshWorkspace(UINT workspace)
{
	if (workspace > MAX_WORKSPACE)
		return;

	UpdateTotalWindowInWorkspace(workspace);
	int totalWindow = totalWindowInWorkspace[workspace - 1];

	if (currentFocusIndice[workspace - 1] > totalWindow - 1)
		currentFocusIndice[workspace - 1] = 0;

	UpdateWorkspaceLayout(workspace);
}

void UpdateTotalWindowInWorkspace(UINT workspace)
{
	if (workspace > MAX_WORKSPACE)
		return;

	int totalWindow = 0;

	for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {
		HWND window = GetWindowByWorkspaceID(workspace, i);
		if (IsWindow(window) && IsValidWindow(window))
			totalWindow += 1;
	}

	totalWindowInWorkspace[workspace - 1] = totalWindow;
}

void SwapCurrentFocusWindow(UINT workspace, UINT swapType)
{
	switch (swapType) {
	case SWAPWINDOW_NEXT:
	{
		int indice = currentFocusIndice[workspace - 1];
		int totalWindow = totalWindowInWorkspace[workspace - 1];

		if (indice >= totalWindow - 1)
			return;

		HWND* currentAry = GetWorkspaceByID(workspace);
		HWND temp = currentAry[indice];

		currentAry[indice] = currentAry[indice + 1];
		currentAry[indice + 1] = temp;

		currentFocusIndice[currentWorkSpace - 1] = indice + 1;

		UpdateCurrentWorkspaceLayout();
		FocusWindow(workspace, indice + 1);

		break;
	}
	case SWAPWINDOW_PREVIOUS:
	{
		int indice = currentFocusIndice[workspace - 1];

		if (indice <= 0)
			return;

		HWND* currentAry = GetWorkspaceByID(workspace);
		HWND temp = currentAry[indice];

		currentAry[indice] = currentAry[indice - 1];
		currentAry[indice - 1] = temp;

		currentFocusIndice[currentWorkSpace - 1] = indice - 1;

		UpdateCurrentWorkspaceLayout();
		FocusWindow(workspace, indice - 1);

		break;
	}
	default:
		break;
	}
}

void SendCurrentWorkspaceThroughIPC(HWND hWnd)
{
	ULONG updateCurrentWorkspace = 1;
	HWND testbar = FindWindow(_T(APPBAR_WINDOW_CLASS), NULL);

	if (testbar == NULL)
		return;

	COPYDATASTRUCT data;

	data.dwData = updateCurrentWorkspace;
	data.cbData = sizeof(UINT);
	data.lpData = &currentWorkSpace;

	SendMessage(testbar, WM_COPYDATA, (WPARAM) hWnd, (LPARAM)(LPVOID) &data);
}
