#undef UNICODE
#define MOVETIMER 1
#define EDIT1 1001
#define EDIT2 1002
#define RADIO1 1003 
#define RADIO2 1004
#define DIALOG 101
#define COLORBUTTON 1005
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <time.h>
#include <stdlib.h>
#include <set>
#include <stdlib.h>
#include <time.h>

using namespace std;

COLORREF _color;
int _perMove;
const int _dim = 30;
POINT _pos;
set<POINT> _trace;

bool isInRange(int dim, int x, int y);
void DrawMatrix(HDC& hdc, int x, int y);
void DrawTrace(HDC& hdc, int x, int y);
BOOL InitApplication(HINSTANCE hinstance);
BOOL InitInstance(HINSTANCE hinstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prevHinstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	srand(time(NULL));
	if (!InitApplication(hinstance))
	{
		MessageBox(NULL, "Unable to Init App", "Error", MB_OK);
		return FALSE;
	}

	if (!InitInstance(hinstance, nCmdShow))
	{
		MessageBox(NULL, "Unable to Init Instance", "Error", MB_OK);
		return FALSE;
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}


void DrawMatrix(HDC& hdc, int x, int y)
{
	double perX = (double)x / _dim, perY = (double)y / _dim;

	for (int i = 0; i < _dim; i++)
	{
		MoveToEx(hdc, 0, perY * i, NULL);
		LineTo(hdc, x, perY* i);

		MoveToEx(hdc, perX * i, 0, NULL);
		LineTo(hdc, perX * i, y);
	}

	RECT r;
	SetRect(&r, perX* _pos.x, perY* _pos.y, perX* (_pos.x + 1), perY* (_pos.y + 1));

	HBRUSH newBrush = CreateSolidBrush(_color);
	FillRect(hdc, &r, newBrush);
	DeleteObject(newBrush);
}

void DrawTrace(HDC& hdc, int x, int y)
{
	double perX = (double)x / _dim, perY = (double)y / _dim;
	HBRUSH brush = CreateSolidBrush(RGB(195, 195, 195));

	for_each(_trace.begin(), _trace.end(), [&hdc, perX, perY, &brush](POINT pt)
	{
		RECT r;
		SetRect(&r, pt.x * perX, pt.y * perY, (pt.x + 1) * perX, (pt.y + 1) * perY);
		FillRect(hdc, &r, brush);

	});
	DeleteObject(brush);
}

BOOL InitApplication(HINSTANCE hinstance)
{
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hinstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_CROSS);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "Snake";
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wndclass))
	{
		MessageBox(NULL, "Cannot register class", "Error", MB_OK);
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK InitDlg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static HWND editx, edity, radio1, radio2, color;
	string s;
	switch (message)
	{
	case WM_INITDIALOG:
		editx = GetDlgItem(hwnd, EDIT1);
		edity = GetDlgItem(hwnd, EDIT2);
		radio1 = GetDlgItem(hwnd, RADIO1);
		radio2 = GetDlgItem(hwnd, RADIO2);
		color = GetDlgItem(hwnd, COLORBUTTON);

		s = to_string(_dim / 2);
		SetWindowText(edity, s.data());
		SetWindowText(editx, s.data());

		CheckRadioButton(hwnd, RADIO1, RADIO2, RADIO1);
		_perMove = 1;
		break;
	case WM_COMMAND:
		switch (wparam)
		{
		case IDOK:
			char buff[256];
			GetWindowText(editx, buff, 255);

			if (atoi(buff) >= _dim || atoi(buff) < 0)
			{
				MessageBox(hwnd, "Incorrect position", "Error", MB_OK);
				return FALSE;
			}

			_pos.x = atoi(buff);

			GetWindowText(edity, buff, 255);

			if (atoi(buff) >= _dim || atoi(buff) < 0)
			{
				MessageBox(hwnd, "Incorrect position", "Error", MB_OK);
				return FALSE;
			}

			_pos.y = atoi(buff);

			EndDialog(hwnd, 1);
			break;
		case COLORBUTTON:
			CHOOSECOLOR ccs;
			COLORREF acrCustClr[16];
			ccs.lStructSize = sizeof(CHOOSECOLOR);
			ccs.hwndOwner = hwnd;
			ccs.rgbResult = RGB(255, 255, 255);
			ccs.Flags = CC_RGBINIT | CC_FULLOPEN;
			ccs.lpCustColors = (LPDWORD)acrCustClr;
			ccs.rgbResult = _color;
			if (ChooseColor(&ccs))
			{
				_color = ccs.rgbResult;
			}
			break;
		case RADIO2:
			_perMove = 2;
			break;
		case RADIO1:
			_perMove = 1;
		}
		break;
	}
	return FALSE;
}

inline bool operator<(const POINT& lhs, const POINT& rhs)
{
	static int i = 0;
	i++;
	return (i % 2) ? true : false;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static int x, y;
	static int ticks, intersactions;
	static HDC hdc;
	PAINTSTRUCT ps;
	switch (message)
	{
	case WM_CREATE:
		srand(time(NULL));
		ticks = 0;
		intersactions = 0;
		int a; a = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(DIALOG), hwnd, InitDlg);
		SetTimer(hwnd, MOVETIMER, 100, NULL);
		break;

	case WM_SIZE:
		x = LOWORD(lparam);
		y = HIWORD(lparam);
		break;
	case WM_TIMER:
		switch (wparam)
		{
		case MOVETIMER:
		{
			ticks++;

			vector<vector<bool>> boolField(_dim);
			vector<char> directions;

			for (int i = 0; i < _dim; i++)
				boolField[i] = vector<bool>(_dim);

			boolField[_pos.x][_pos.y] = true;

			if (!isInRange(_dim, _pos.x + _perMove, _pos.y) || !isInRange(_dim, _pos.x - _perMove, _pos.y)
				|| !isInRange(_dim, _pos.x, _pos.y - _perMove) || !isInRange(_dim, _pos.x, _pos.y + _perMove) || ticks > 300)
			{
				KillTimer(hwnd, MOVETIMER);
				string msg;
				msg = (ticks > 300) ? "Timeout" : "Border reached : " + to_string(ticks - 2) + "steps";

				MessageBox(NULL, msg.data(), "Finished", MB_OK);
				return TRUE;
			}

			if (isInRange(_dim, _pos.x + _perMove, _pos.y)) directions.push_back('r');
			if (isInRange(_dim, _pos.x - _perMove, _pos.y)) directions.push_back('l');
			if (isInRange(_dim, _pos.x, _pos.y - _perMove)) directions.push_back('u');
			if (isInRange(_dim, _pos.x, _pos.y + _perMove)) directions.push_back('d');

			POINT tracePoint;
			tracePoint.x = _pos.x;
			tracePoint.y = _pos.y;


			int t = directions.size();
			t = rand() % t;

			char sign = directions[t];
			switch (sign)
			{
			case 'r':
				for (int j = 0; j < _perMove; j++)
				{
					tracePoint.x += 1;
					_trace.insert(tracePoint);
				}
				_pos.x += _perMove;
				break;
			case 'l':
				for (int j = 0; j < _perMove; j++)
				{
					tracePoint.x -= 1;
					_trace.insert(tracePoint);
				}
				_pos.x -= _perMove;
				break;
			case 'u':
				for (int j = 0; j < _perMove; j++)
				{
					tracePoint.y -= 1;
					_trace.insert(tracePoint);
				}
				_pos.y -= _perMove;
				break;
			case 'd':
				for (int j = 0; j < _perMove; j++)
				{
					tracePoint.y += 1;
					_trace.insert(tracePoint);
				}
				_pos.y += _perMove;
				break;
			}
			InvalidateRect(hwnd, NULL, true);
		}
		break;
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		DrawTrace(hdc, x, y);
		DrawMatrix(hdc, x, y);
		EndPaint(hwnd, &ps);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		KillTimer(hwnd, MOVETIMER);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	return FALSE;
}


BOOL InitInstance(HINSTANCE hinstance, int nCmdShow)
{
	HWND hwnd;
	hwnd = CreateWindow(
		"Snake",
		"Snake",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		700,
		700,
		NULL,
		NULL,
		hinstance,
		NULL);

	if (!hwnd)
		return FALSE;
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	return TRUE;
}

bool isInRange(int dim, int x, int y)
{
	return (x >= 0 && x <= dim - 1 && y >= 0 && y <= dim - 1) ? true : false;
}