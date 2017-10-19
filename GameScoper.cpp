#include "stdafx.h"
#include "windows.h"
#include "iostream"
#include "fstream"
#include "strsafe.h"
//основная либа
#include "magnification.h"
#include "signal.h"

//без этого она не заводится
#pragma comment (lib, "magnification.lib")

using namespace std;

HHOOK mouseHook;
HHOOK _hook_keyboard;
KBDLLHOOKSTRUCT kbdStruct;

bool active; //активен ли зум
bool pressed; //нажат ли прицел
int cScope; //текущий коэффициент увеличения
float scopes[4]; //4 слота для прицеливания
char activationButton;
char cyclingButton;

void getCfg() //загружаем конфиг
{
	char string[20];
	ifstream fin("config.txt");
	fin.getline(string, 20, '\n');
	activationButton = string[strlen(string)-1];
	cout << "activationButton: " << activationButton << endl;
	fin.getline(string, 20, '\n');
	cyclingButton = string[strlen(string) - 1];
	cout << "cyclingButton: " << cyclingButton << endl;
	fin.getline(string, 20, '=');
	for (int i = 0; i < 4; i++) 
	{
		fin.getline(string, 20, ',');
		scopes[i] = atof(string);
		cout << "scope " << i+1 << ": " << scopes[i] << endl;
	}
	fin.close();
}

void SetZoom(float magnificationFactor) //установка зума
{
	if (magnificationFactor >= 1.0)
	{
		int xDlg = (int)((float)GetSystemMetrics(SM_CXSCREEN) * (1.0 - (1.0 / magnificationFactor)) / 2.0);
		int yDlg = (int)((float)GetSystemMetrics(SM_CYSCREEN) * (1.0 - (1.0 / magnificationFactor)) / 2.0);
		MagSetFullscreenTransform(magnificationFactor, xDlg, yDlg);
		BOOL fSuccess = MagSetFullscreenTransform(magnificationFactor, xDlg, yDlg);
		if (!fSuccess)
		{
			cout << "failed to magnify screen!" << endl;
		}
		else
		{
			cout << "zoomed to: " << magnificationFactor << endl;
		}
	}
}

LRESULT __stdcall MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) { //хук на мышь
	if (nCode >= 0)
	{
		switch (wParam)
		{
		case WM_RBUTTONDOWN:
			cout << "right button down" << endl;
			pressed = 1;
			if (active)
			{
				SetZoom(scopes[cScope]);
			}
			break;

		case WM_RBUTTONUP:
			cout << "right button up" << endl;
			pressed = 0;
			SetZoom(1);
			break;
		}
	}
	return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

LRESULT __stdcall HookCallbackKeyboard(int nCode, WPARAM wParam, LPARAM lParam) { //хук на клавиатуру
	if (nCode >= 0)
	{
		kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
		if (wParam == WM_KEYDOWN) {
			char c = MapVirtualKey(kbdStruct.vkCode, 2);
			if (c == activationButton)
			{
				if (active) 
				{
					cout << "zoom is deactivated" << endl;
					SetZoom(1); 
				}
				else
				{
					cout << "zoom is activated" << endl;
				}
				active = !active;
			}
			if (c == cyclingButton)
			{
				if (cScope < 3)
				{
					cScope++;
				}
				else
				{
					cScope = 0;
				}
				cout << "selected scope: " << cScope + 1 << endl;
				if (pressed && active)
				{
					SetZoom(scopes[cScope]);
				}
			}
		}
	}
	return CallNextHookEx(_hook_keyboard, nCode, wParam, lParam);
}

void SetHook() { //активируем хуки
	if (!(_hook_keyboard = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallbackKeyboard, NULL, 0)))
	{
		cout << "failed to install hook on keyboard!" << endl;
	}
	if (!(mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, NULL, 0)))
	{
		cout << "failed to install mouse hook!" << endl;
	}
}

int main()
{
	getCfg();
	MagInitialize();
	SetHook();
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}