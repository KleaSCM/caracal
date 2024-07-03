#include <windows.h>
#include <string>
#include <sstream>
#include "framework.h"
#include "caracal.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Function prototype for appending text to RichTextBox
void AppendTextToRichTextBox(HWND hRichTextBox, const wchar_t* text);

// Function prototype for executing Python script
std::string ExecutePythonScript(const std::string& command);

// Global variables for the controls
HWND hTextBox;
HWND hButton;
HWND hRichTextBox;
HWND hHistoryListBox;
HWND hClearButton;

// Custom font
HFONT hFont;

// Custom button procedure
LRESULT CALLBACK ButtonProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static HBRUSH hBrush = CreateSolidBrush(RGB(50, 50, 50));
	static HBRUSH hHoverBrush = CreateSolidBrush(RGB(70, 70, 70));
	static HBRUSH hClickBrush = CreateSolidBrush(RGB(90, 90, 90));

	switch (message)
	{
	case WM_MOUSEMOVE:
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hWnd;
		TrackMouseEvent(&tme);
		SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hHoverBrush);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_LBUTTONDOWN:
		SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hClickBrush);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_LBUTTONUP:
		SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hHoverBrush);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_MOUSELEAVE:
		SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_CTLCOLORBTN:
		hdc = (HDC)wParam;
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(211, 211, 211)); // Light grey text
		return (LRESULT)hBrush;
	case WM_CTLCOLORSTATIC:
		hdc = (HDC)wParam;
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(211, 211, 211)); // Light grey text
		return (LRESULT)hBrush;
	case WM_CTLCOLOREDIT:
		hdc = (HDC)wParam;
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(211, 211, 211)); // Light grey text
		return (LRESULT)hBrush;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// Custom painting code here
		EndPaint(hWnd, &ps);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CARACAL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CARACAL));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CARACAL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CARACAL);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1000, 600, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Create custom font
	hFont = CreateFontW(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		// Create the TextBox for inputting commands
		hTextBox = CreateWindowW(
			L"EDIT",
			L"",
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOVSCROLL,
			10, 10, 600, 25,
			hWnd,
			NULL,
			hInst,
			NULL);
		SendMessage(hTextBox, WM_SETFONT, (WPARAM)hFont, TRUE);

		// Create the Button to submit the command
		hButton = CreateWindowW(
			L"BUTTON",
			L"Submit",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			620, 10, 150, 25,
			hWnd,
			(HMENU)1,
			hInst,
			NULL);
		SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

		// Create the Clear Button
		hClearButton = CreateWindowW(
			L"BUTTON",
			L"Clear",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			780, 10, 150, 25,
			hWnd,
			(HMENU)2,
			hInst,
			NULL);
		SendMessage(hClearButton, WM_SETFONT, (WPARAM)hFont, TRUE);

		// Create the RichTextBox to display the responses
		hRichTextBox = CreateWindowW(
			L"EDIT",
			L"",
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
			10, 50, 760, 500,
			hWnd,
			NULL,
			hInst,
			NULL);
		SendMessage(hRichTextBox, WM_SETFONT, (WPARAM)hFont, TRUE);

		// Create the ListBox to display command history
		hHistoryListBox = CreateWindowW(
			L"LISTBOX",
			NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
			780, 50, 200, 500,
			hWnd,
			NULL,
			hInst,
			NULL);
		SendMessage(hHistoryListBox, WM_SETFONT, (WPARAM)hFont, TRUE);

		// Set background color
		SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(30, 30, 30)));
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == 1) // Submit button
		{
			// Get the text from the TextBox
			wchar_t buffer[256];
			GetWindowTextW(hTextBox, buffer, sizeof(buffer) / sizeof(buffer[0]));

			// Convert wchar_t buffer to std::string
			std::wstring ws(buffer);
			std::string command(ws.begin(), ws.end());

			// Call Python script and capture its output
			std::string result = ExecutePythonScript(command);

			// Convert std::string result to wchar_t
			std::wstring wresult(result.begin(), result.end());

			// Display the command and result in the RichTextBox
			AppendTextToRichTextBox(hRichTextBox, L"Command Entered:\r\n");
			AppendTextToRichTextBox(hRichTextBox, buffer);
			AppendTextToRichTextBox(hRichTextBox, L"\r\n\nResult:\r\n");
			AppendTextToRichTextBox(hRichTextBox, wresult.c_str());
			AppendTextToRichTextBox(hRichTextBox, L"\r\n\n");

			// Add the command to the history ListBox
			SendMessage(hHistoryListBox, LB_ADDSTRING, 0, (LPARAM)buffer);

			// Clear the TextBox
			SetWindowText(hTextBox, L"");
		}
		else if (LOWORD(wParam) == 2) // Clear button
		{
			// Clear the TextBox and RichTextBox
			SetWindowText(hTextBox, L"");
			SetWindowText(hRichTextBox, L"");
		}
		else
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
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Function to append text to the RichTextBox
void AppendTextToRichTextBox(HWND hRichTextBox, const wchar_t* text)
{
	int len = GetWindowTextLengthW(hRichTextBox);
	SendMessageW(hRichTextBox, EM_SETSEL, (WPARAM)len, (LPARAM)len);
	SendMessageW(hRichTextBox, EM_REPLACESEL, 0, (LPARAM)text);
}

// Function to execute Python script and capture its output
std::string ExecutePythonScript(const std::string& command)
{
	// Prepare command line
	std::wstring cmd = L"python nlp_module.py \"" + std::wstring(command.begin(), command.end()) + L"\"";

	// Create pipes to capture the output
	HANDLE hStdOutRead, hStdOutWrite;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0);
	SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

	// Set up the process start info
	STARTUPINFOW si = { sizeof(STARTUPINFOW) };
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hStdOutWrite;
	si.hStdError = hStdOutWrite;
	PROCESS_INFORMATION pi;

	// Create the process
	if (!CreateProcessW(NULL, &cmd[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(hStdOutWrite);
		CloseHandle(hStdOutRead);
		return "Error: Unable to execute Python script";
	}

	// Close the write end of the pipe before reading from the read end of the pipe
	CloseHandle(hStdOutWrite);

	// Read the output
	char buffer[128];
	DWORD bytesRead;
	std::string result;
	while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead != 0)
	{
		buffer[bytesRead] = '\0';
		result += buffer;
	}

	// Clean up handles
	CloseHandle(hStdOutRead);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return result;
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