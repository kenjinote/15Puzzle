﻿#include<windows.h>
#include"resource.h"

TCHAR szClassName[] = TEXT("15Puzzle");

#define BITMAP_WIDTH 100
#define BITMAP_HEIGHT 99
#define WM_CLICK (WM_APP+100)
#define WM_RANDOM (WM_APP+101)

HHOOK g_hHook;

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HCBT_ACTIVATE)
	{
		UnhookWindowsHookEx(g_hHook);
		const HWND hMessageBox = (HWND)wParam;
		const HWND hParentWnd = GetParent(hMessageBox);
		RECT rectMessageBox, rectParentWnd;
		GetWindowRect(hMessageBox, &rectMessageBox);
		GetWindowRect(hParentWnd, &rectParentWnd);
		SetWindowPos(
			hMessageBox,
			hParentWnd,
			(rectParentWnd.right + rectParentWnd.left - rectMessageBox.right + rectMessageBox.left) >> 1,
			(rectParentWnd.bottom + rectParentWnd.top - rectMessageBox.bottom + rectMessageBox.top) >> 1,
			0,
			0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int table[16];
	static HBITMAP hBitmap[15];
	int i, j, index;
	PAINTSTRUCT ps;
	HDC hdc, hdc_mem;
	switch (msg)
	{
	case WM_CREATE:
		for (i = 0; i < 15; ++i)
		{
			table[i] = i;
			hBitmap[i] = LoadBitmap(
				((LPCREATESTRUCT)lParam)->hInstance,
				MAKEINTRESOURCE(IDB_BITMAP1 + i)
			);
		}
		table[15] = 15;
		srand(GetTickCount());
		SendMessage(hWnd, WM_RANDOM, 0, 0);
		break;
	case WM_RANDOM:
		for (i = 0; i < 1000; ++i)
			SendMessage(hWnd, WM_CLICK, rand() % 16, 0);
		InvalidateRect(hWnd, 0, 0);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		hdc_mem = CreateCompatibleDC(hdc);
		for (i = 0; i < 16; ++i)
		{
			if (table[i] == 15)
			{
				RECT rect;
				SetRect(
					&rect,
					i % 4 * BITMAP_WIDTH,
					i / 4 * BITMAP_HEIGHT,
					i % 4 * BITMAP_WIDTH + BITMAP_WIDTH,
					i / 4 * BITMAP_WIDTH + BITMAP_HEIGHT
				);
				ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, 0, 0, 0);
			}
			else
			{
				SelectObject(
					hdc_mem,
					hBitmap[table[i]]
				);
				BitBlt(
					hdc,
					i % 4 * BITMAP_WIDTH,
					i / 4 * BITMAP_HEIGHT,
					BITMAP_WIDTH,
					BITMAP_HEIGHT,
					hdc_mem,
					0,
					0,
					SRCCOPY
				);
			}
		}
		DeleteDC(hdc_mem);
		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		SendMessage(
			hWnd,
			WM_CLICK,
			LOWORD(lParam) / BITMAP_WIDTH + HIWORD(lParam) / BITMAP_HEIGHT * 4,
			1
		);
		break;
	case WM_KEYDOWN:
		for (i = 0; i < 16; ++i)
			if (table[i] == 15)
				break;
		switch (wParam)
		{
		case VK_LEFT:
			SendMessage(hWnd, WM_CLICK, i + 1, 1);
			break;
		case VK_RIGHT:
			SendMessage(hWnd, WM_CLICK, i - 1, 1);
			break;
		case VK_UP:
			SendMessage(hWnd, WM_CLICK, i + 4, 1);
			break;
		case VK_DOWN:
			SendMessage(hWnd, WM_CLICK, i - 4, 1);
			break;
		}
		break;
	case WM_CLICK:
		index = (int)wParam;
		if (table[index] == 15 || index < 0 || index>15)
			break;
		for (i = 0; i < 4; ++i)
		{
			if (index % 4 > i&&table[index - (i + 1)] == 15)//move left
			{
				for (j = i; j >= 0; --j)table[index - (j + 1)] = table[index - j];
				table[index] = 15;
				break;
			}
			else if (index > 3 + 4 * i&&table[index - 4 * (i + 1)] == 15)//move up
			{
				for (j = i; j >= 0; --j)table[index - 4 * (j + 1)] = table[index - 4 * j];
				table[index] = 15;
				break;
			}
			else if (index % 4 < 3 - i&&table[index + (i + 1)] == 15)//move right
			{
				for (j = i; j >= 0; --j)table[index + (j + 1)] = table[index + j];
				table[index] = 15;
				break;
			}
			else if (index < 12 - i * 4 && table[index + 4 * (i + 1)] == 15)//move down
			{
				for (j = i; j >= 0; --j)table[index + 4 * (j + 1)] = table[index + 4 * j];
				table[index] = 15;
				break;
			}
		}
		if (lParam&&table[index] == 15)
		{
			InvalidateRect(hWnd, 0, 0);
			if (index == 15)
			{
				for (i = 0; i < 16; ++i)
					if (table[i] != i)
						return 0;
				g_hHook = SetWindowsHookEx(
					WH_CBT,
					CBTProc,
					NULL,
					GetCurrentThreadId()
				);
				if (IDYES == MessageBox(
					hWnd,
					TEXT("もう一度行いますか?"),
					TEXT("完成!"),
					MB_YESNO)
					)
					SendMessage(hWnd, WM_RANDOM, 0, 0);
			}
		}
		break;
	case WM_DESTROY:
		for (i = 0; i < 15; ++i)
		{
			DeleteObject(hBitmap[i]);
		}
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	const WNDCLASS wndclass = {
		0,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	RECT rect;
	SetRect(&rect, 0, 0, BITMAP_WIDTH * 4, BITMAP_HEIGHT * 4);
	AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, 0);
	const HWND hWnd = CreateWindow(
		szClassName,
		TEXT("15 Puzzle"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT,
		0,
		rect.right - rect.left,
		rect.bottom - rect.top,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
