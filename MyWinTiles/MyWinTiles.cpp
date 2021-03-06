
#include "stdafx.h"
#include "MyWinTiles.h"

#define MAX_LOADSTRING 100

HWND appbar = NULL;
HWND hWndProgramManager = NULL;
HWND hWndExceptionWindow = NULL;
HWND hWndInstance = NULL;


unsigned short currentWorkSpaceInfo = 0;

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

APPBARDATA explorerTaskbar;

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


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

	hWndProgramManager = FindWindow(NULL, L"Program Manager");
	hWndExceptionWindow = FindWindow(L"ApplicationFrameWindow", L"");

    while (GetMessage(&msg, nullptr, 0, 0) > 0)
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
				appbar = FindWindow(_T(APPBAR_WINDOW_CLASS), NULL);

				if (appbar != NULL && appbar == hWnd)
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
				HWND hWnd = (HWND) msg.lParam;
				SendCurrentFocusWindowThroughIPC(msg.hwnd, hWnd);

				if (isFocusByHotkey) {
					isFocusByHotkey = FALSE;
					break;
				}

				UpdateFocusIndice(currentWorkSpace, hWnd);
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
				HWND parentTargetWindow = GetParent(targetWindow);

				RemoveWindowFromWorkspace(targetWindow, currentWorkSpace);

				if (parentTargetWindow == NULL) {
					SendMessage(targetWindow, WM_CLOSE, 0, 0);
				}
				else {
					PostMessage(parentTargetWindow, WM_CLOSE, 0, 0);
				}

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
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWITHTO_WORKSPACE_2:
			{
				SwitchToWorkspace(2);
				FocusWindow(2, currentFocusIndice[1]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_3:
			{
				SwitchToWorkspace(3);
				FocusWindow(3, currentFocusIndice[2]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_4:
			{
				SwitchToWorkspace(4);
				FocusWindow(4, currentFocusIndice[3]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_5:
			{
				SwitchToWorkspace(5);
				FocusWindow(5, currentFocusIndice[4]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_6:
			{
				SwitchToWorkspace(6);
				FocusWindow(6, currentFocusIndice[5]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_7:
			{
				SwitchToWorkspace(7);
				FocusWindow(7, currentFocusIndice[6]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}
			case HOTKEY_SWITHTO_WORKSPACE_8:
			{
				SwitchToWorkspace(8);
				FocusWindow(8, currentFocusIndice[7]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWITHTO_WORKSPACE_9:
			{
				SwitchToWorkspace(9);
				FocusWindow(9, currentFocusIndice[8]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWITHTO_WORKSPACE_10:
			{
				SwitchToWorkspace(10);
				FocusWindow(10, currentFocusIndice[9]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
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
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
				break;
			}

			case HOTKEY_SWITHTO_PREVIOUS_WORKSPACE:
			{
				SwitchToPreviousWorkspace();
				FocusWindow(currentWorkSpace, currentFocusIndice[currentWorkSpace - 1]);
				SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
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
					totalWindowInWorkspace[0] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_2:
			{
				if (currentWorkSpace != 2) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 2);
					totalWindowInWorkspace[1] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_3:
			{
				if (currentWorkSpace != 3) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 3);
					totalWindowInWorkspace[2] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_4:
			{
				if (currentWorkSpace != 4) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 4);
					totalWindowInWorkspace[3] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_5:
			{
				if (currentWorkSpace != 5) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 5);
					totalWindowInWorkspace[4] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_6:
			{
				if (currentWorkSpace != 6) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 6);
					totalWindowInWorkspace[5] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_7:
			{
				if (currentWorkSpace != 7) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 7);
					totalWindowInWorkspace[6] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_8:
			{
				if (currentWorkSpace != 8) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 8);
					totalWindowInWorkspace[7] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_9:
			{
				if (currentWorkSpace != 9) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 9);
					totalWindowInWorkspace[8] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_MOVEWINDOW_TO_WORKSPACE_10:
			{
				if (currentWorkSpace != 10) {
					HWND targetWindow = GetForegroundWindow();
					MoveWindowToWorkspaceByID(targetWindow, currentWorkSpace, 10);
					totalWindowInWorkspace[9] += 1;
					SendCurrentWorkspaceInfoThroughIPC(msg.hwnd);
					UpdateCurrentWorkspaceLayout();
					FocusPreviousWindow();
				}
				break;
			}

			case HOTKEY_TOGGLE_EXPLORER_TASKBAR:
			{
				ToggleExplorerTaskbar();
				break;
			}

			case HOTKEY_QUIT_APP:
			{
				UINT dialogStyle = (MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);
				int result = MessageBox(NULL, L"Do you want to quit MyWinTiles?", L"Warnning", dialogStyle);
				if (result == IDYES)
					DestroyWindow(msg.hwnd);
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
	wcex.lpszMenuName = NULL;
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
   else
	   hWndInstance = hWnd;

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   RegisterShellHookWindow(hWnd);
   SHELLHOOK_MSG = RegisterWindowMessage(TEXT("SHELLHOOK"));

   RegisterHotKey(hWnd, HOTKEY_DESTROY_WINDOW, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x51);
   RegisterHotKey(hWnd, HOTKEY_MINIMIZE_WINDOW, MOD | MOD_NOREPEAT, 0x4d);
   RegisterHotKey(hWnd, HOTKEY_MAXIMIZE_WINDOW, MOD | MOD_NOREPEAT, 0x46);
   RegisterHotKey(hWnd, HOTKEY_MINIMIZE_ALL_WINDOW, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x4d);
   RegisterHotKey(hWnd, HOTKEY_MAXIMIZE_ALL_WINDOW, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x46);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_NEXT_WINDOW, MOD | MOD_NOREPEAT, 0x4a);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_PREVIOUS_WINDOW, MOD | MOD_NOREPEAT, 0x4b);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_NEXT_WINDOW, MOD | MOD_NOREPEAT, 0x4c);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_PREVIOUS_WINDOW, MOD | MOD_NOREPEAT, 0x48);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_NEXT_WORKSPACE, MOD | MOD_NOREPEAT, 0x4e);
   RegisterHotKey(hWnd, HOTKEY_SWITHTO_PREVIOUS_WORKSPACE, MOD | MOD_NOREPEAT, 0x50);
   RegisterHotKey(hWnd, HOTKEY_TILE_VERTICAL, MOD | MOD_NOREPEAT, 0xba);
   RegisterHotKey(hWnd, HOTKEY_TILE_HORIZONTAL, MOD | MOD_NOREPEAT, 0x56);
   RegisterHotKey(hWnd, HOTKEY_REFRESH_TILE, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x52);
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
   RegisterHotKey(hWnd, HOTKEY_SWAPWINDOW_NEXT, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x4a);
   RegisterHotKey(hWnd, HOTKEY_SWAPWINDOW_PREVIOUS, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x4b);
   RegisterHotKey(hWnd, HOTKEY_SWAPWINDOW_NEXT, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x4c);
   RegisterHotKey(hWnd, HOTKEY_SWAPWINDOW_PREVIOUS, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x48);
   RegisterHotKey(hWnd, HOTKEY_TOGGLE_EXPLORER_TASKBAR, MOD | MOD_NOREPEAT, VK_F12);
   RegisterHotKey(hWnd, HOTKEY_QUIT_APP, MOD | MOD_SHIFT | MOD_NOREPEAT, 0x45);

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

   //Set default tile mode of workspace 1 to be overlap
   workspaceTileMode[0] = OVERLAP_WINDOW_MODE;

   explorerTaskbar.cbSize = sizeof(APPBARDATA);
   explorerTaskbar.hWnd = FindWindow(_T(EXPLORER_APPBAR_WINDOW_CLASS), NULL);
   explorerTaskbar.uEdge = ABE_BOTTOM;
   SHAppBarMessage(ABM_GETTASKBARPOS, &explorerTaskbar);

   EnumWindows(&InitWorkSpaces_Callback, NULL);
   SendCurrentWorkspaceInfoThroughIPC(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
	{
		DeregisterShellHookWindow(hWnd);

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
		UnregisterHotKey(hWnd, HOTKEY_TOGGLE_EXPLORER_TASKBAR);
		UnregisterHotKey(hWnd, HOTKEY_QUIT_APP);

		PostQuitMessage(0);
		break;
		}

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK InitWorkSpaces_Callback(HWND hWnd, LPARAM lParam)
{
	if (iterator >= MAX_WINDOW_PER_WORKSPACE)
		return FALSE;

	if (!IsValidWindow(hWnd))
		return TRUE;

	if (!IsWindow(hWnd))
		return TRUE;

	if (AddWindowToWorkspace(hWnd, 1)) {
		iterator++;
		SwitchToWorkspace(1);
	}

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
	POINT origin = { 0 };

	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	int width = (rect.right - rect.left);
	int height = (rect.bottom - rect.top);

	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;

	WINDOWPLACEMENT windowPlacement;
	windowPlacement.length = sizeof(WINDOWPLACEMENT);
	windowPlacement.flags = WPF_ASYNCWINDOWPLACEMENT;
	windowPlacement.showCmd = SW_SHOWNOACTIVATE;
	windowPlacement.ptMinPosition = origin;
	windowPlacement.ptMaxPosition = origin;
	windowPlacement.rcNormalPosition = rect;

	for (UINT i = 0; i < MAX_WINDOW_PER_WORKSPACE; ++i) {
		HWND target = NULL;
		target = GetWindowByWorkspaceID(workspaceID, i);

		UINT targetStyle = GetWindowLong(target, GWL_EXSTYLE);

		if ((targetStyle & WS_EX_TOPMOST) != WS_EX_TOPMOST || (targetStyle & WS_EX_TOOLWINDOW) != WS_EX_TOOLWINDOW) {
			SetWindowPlacement(target, &windowPlacement);
		}

		ShowWindowAsync(target, SW_RESTORE);
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
	if (hWnd == hWndInstance)
		return FALSE;

	if (!IsWindowVisible(hWnd))
		return FALSE;

	if (hWnd == hWndProgramManager)
		return FALSE;

	TITLEBARINFO ti;

	ti.cbSize = sizeof(ti);
	GetTitleBarInfo(hWnd, &ti);

	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return FALSE;

	if (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return FALSE;

	if (hWndExceptionWindow == NULL)
	{
		TCHAR targetClassName[] = TEXT("ApplicationFrameWindow");
		TCHAR buffer[23];

		int isSuccess = GetClassName(hWnd, buffer, 23);

		if (_tcscmp(targetClassName, buffer) == 0)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		if (hWnd == hWndExceptionWindow)
			return FALSE;
	}

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
	//Dirty hacks (need to calculate properly)
	if (totalWindowInWorkspace[currentWorkSpace - 1] == 3) {
		TileWindowVerticalThreeSplit(currentAry);
	}
	else {
		TileWindows(
			NULL,
			MDITILE_SKIPDISABLED | MDITILE_VERTICAL | MDITILE_ZORDER,
			NULL,
			MAX_WINDOW_PER_WORKSPACE,
			currentAry
		);
	}
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

void TileWindowVerticalThreeSplit(HWND* currentAry)
{
	POINT origin = { 0 };
	WINDOWPLACEMENT windowPlacement;

	windowPlacement.length = sizeof(WINDOWPLACEMENT);
	windowPlacement.flags = WPF_ASYNCWINDOWPLACEMENT;
	windowPlacement.showCmd = SW_SHOWNOACTIVATE;
	windowPlacement.ptMinPosition = origin;
	windowPlacement.ptMaxPosition = origin;

	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	int width = (rect.right - rect.left);
	int height = (rect.bottom - rect.top);

	RECT topLeft;
	topLeft.left = 0;
	topLeft.top = 0;
	topLeft.right = width / 2;
	topLeft.bottom = height / 2;

	RECT bottomLeft;
	bottomLeft.left = 0;
	bottomLeft.top = topLeft.bottom;
	bottomLeft.right = width / 2;
	bottomLeft.bottom = height;

	RECT topRight;
	topRight.left = topLeft.right;
	topRight.top = topLeft.top;
	topRight.right = width;
	topRight.bottom = height;

	HWND target = NULL;
	RECT rects[3] = { topLeft, bottomLeft, topRight };

	for (UINT i = 0; i < 3; ++i) {
		windowPlacement.rcNormalPosition = rects[i];
		target = GetWindowByWorkspaceID(currentWorkSpace, i);

		if (target != NULL) {
			SetWindowPlacement(target, &windowPlacement);
			ShowWindowAsync(target, SW_SHOWNOACTIVATE);
		}
	}
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
			ShowWindowAsync(hWnd, SW_SHOWNOACTIVATE);
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

void SendCurrentWorkspaceInfoThroughIPC(HWND hWnd)
{
	appbar = FindWindow(_T(APPBAR_WINDOW_CLASS), NULL);

	if (appbar == NULL)
		return;

	currentWorkSpaceInfo = (currentWorkSpace << MAX_WORKSPACE);

	for (UINT i = 0; i < MAX_WORKSPACE; ++i) {
		HWND firstWindow = GetWindowByWorkspaceID((i + 1), 0);
		if (IsWindow(firstWindow)) {
			currentWorkSpaceInfo |= (1 << i);
		}
	}

	COPYDATASTRUCT data;

	data.dwData = APPBAR_UPDATE_CURRENT_WORKSPACE;
	data.cbData = sizeof(unsigned short);
	data.lpData = &currentWorkSpaceInfo;

	SendMessage(appbar, WM_COPYDATA, (WPARAM) hWnd, (LPARAM)(LPVOID) &data);
}

void SendCurrentFocusWindowThroughIPC(HWND currentWindow, HWND focusWindow)
{
	appbar = FindWindow(_T(APPBAR_WINDOW_CLASS), NULL);

	if (appbar == NULL)
		return;

	COPYDATASTRUCT data;

	data.dwData = APPBAR_UPDATE_CURRENT_FOCUS_WINDOW;
	data.cbData = sizeof(HWND);
	data.lpData = &focusWindow;

	SendMessage(appbar, WM_COPYDATA, (WPARAM) currentWindow, (LPARAM)(LPVOID) &data);
}

void ToggleExplorerTaskbar()
{
	HWND taskbar = FindWindow(_T(EXPLORER_APPBAR_WINDOW_CLASS), NULL);

	if (taskbar == NULL)
		return;

	explorerTaskbar.hWnd = taskbar;

	if (IsWindowVisible(taskbar)) {
		explorerTaskbar.lParam = ABS_AUTOHIDE;
		SHAppBarMessage(ABM_SETSTATE, &explorerTaskbar);
		explorerTaskbar.lParam = FALSE;
		SHAppBarMessage(ABM_ACTIVATE, &explorerTaskbar);
		ShowWindow(taskbar, SW_HIDE);
	}
	else {
		explorerTaskbar.lParam = 0;
		SHAppBarMessage(ABM_SETSTATE, &explorerTaskbar);
		explorerTaskbar.lParam = TRUE;
		SHAppBarMessage(ABM_ACTIVATE, &explorerTaskbar);
		ShowWindow(taskbar, SW_SHOW);
	}
}

void UpdateFocusIndice(UINT workspaceID, HWND hWnd)
{
	if (workspaceID > MAX_WORKSPACE)
		return;

	int indice = currentFocusIndice[workspaceID - 1];
	HWND* currentAry = GetWorkspaceByID(workspaceID);

	if (hWnd != currentAry[indice]) {
		for (UINT i = 0; i < MAX_WORKSPACE; ++i) {
			if (currentAry[i] == hWnd) {
				currentFocusIndice[workspaceID - 1] = i;
				break;
			}
		}
	}
}
