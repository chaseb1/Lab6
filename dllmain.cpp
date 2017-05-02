// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "stdio.h"

HHOOK hkb;
KBDLLHOOKSTRUCT kbdStruct;

typedef int(*__stdcall MOVECARDS)(HWND);
typedef int(*__stdcall MainWndProc)(HWND, int, int, int);
typedef int(*__stdcall CleanUp)(HWND);

HACCEL newFreeMenu;
HACCEL origFreeMenu;

void MessageHandler(HWND hWnd, LPMSG uMsg, WPARAM wParam, LPARAM lParam) {

	OutputDebugStringA("Got TestingHandler");
	if (!TranslateAccelerator(hWnd, newFreeMenu, uMsg))
	{
		TranslateMessage(uMsg);
		DispatchMessage(uMsg);
	}
}
HWND FindMyTopMostWindow()
{
	DWORD dwProcID = GetCurrentProcessId();
	HWND hWnd = GetTopWindow(GetDesktopWindow());
	while (hWnd)
	{
		DWORD dwWndProcID = 0;
		GetWindowThreadProcessId(hWnd, &dwWndProcID);
		if (dwWndProcID == dwProcID)
			return hWnd;
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}
	return NULL;
}

void problem_1() {
	DWORD s = (DWORD)GetModuleHandle(NULL);
	DWORD offset = (DWORD)0x00010C04;
	DWORD a = offset + s;
	DWORD old;
	char* ptr = (char*)a;
	VirtualProtect(ptr, 0x31, 0x40, &old);
	char msg[] = "Not in this game.";
	for (int x = 0; x < sizeof(msg) / sizeof(msg[0]); x++)
		ptr[x * 2] = msg[x];
}

void problem_2() {

	// store key in freecell mem
	HKEY* _hkey = (HKEY*)0x10079A4;
	// pointer to registry path
	WCHAR* _pszRegPath = (WCHAR*)0x1001230;
	// pointer to pszWon variable
	WCHAR* _pszWon = (WCHAR*)0x10012A8;
	// create registry key - store in _hkey
	RegCreateKeyW((HKEY)2147483649, _pszRegPath, _hkey);
	// set registry at _pszWon to data
	DWORD data = 1000;
	RegSetValueExW(*_hkey, _pszWon, 0, 3, (LPBYTE)&data, 4);
	// close registry key
	RegCloseKey(*_hkey);
}

void problem_5() {
	HWND hWnd = FindMyTopMostWindow();

	// Set the move index to a nonzero value
	unsigned int oldMoveIndex, newMoveIndex;
	oldMoveIndex = *(unsigned int *)0x01007864;

	*(unsigned int *)0x01007864 = 1;
	newMoveIndex = *(unsigned int *)0x01007864;

	// Set the card count to a zero (winning) value
	unsigned int oldCardCount, newCardCount;
	oldCardCount = *(unsigned int *)0x01007800;

	*(unsigned int *)0x01007800 = 0;
	newCardCount = *(unsigned int *)0x01007800;

	MOVECARDS mcFn = (MOVECARDS)0x01004FC7;
	mcFn(hWnd);
}

LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	char buf[200];

	LPMSG lMsg = (LPMSG)lParam;
	HWND hWnd;
	hWnd = *(HWND *)0x01008374;

	if (TranslateAccelerator(hWnd, newFreeMenu, lMsg) == 0)
	{
		DWORD err = GetLastError();

		TranslateMessage(lMsg);
		DispatchMessage(lMsg);
	}

	return CallNextHookEx(hkb, nCode, wParam, lParam);
}

BOOL InstallHook()
{

	OutputDebugStringA("Attempting to Install Hook");
	HWND hWnd = FindMyTopMostWindow();

	DWORD dwProcID = GetCurrentProcessId();

	DWORD dwThreadID = GetWindowThreadProcessId(hWnd, &dwProcID);


	HWND pgmHwnd;
	pgmHwnd = *(HWND *)0x01008374;


	hkb = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)HookCallback, NULL, dwThreadID);

	OutputDebugStringA("Hooked");

	MSG msg;
	BOOL bRet;

	while (bRet = GetMessage(&msg, NULL, 0, 0))
	{
	}

	if (hkb == NULL)
		return FALSE;
	else
		return TRUE;
}

void newAccelerators(HMODULE self) {


	origFreeMenu = LoadAccelerators(NULL, L"FreeMenu");
	if (origFreeMenu != NULL)
	{
		DestroyAcceleratorTable(origFreeMenu);
	}
	else
	{
		OutputDebugStringA("Failed to load origFreeMenu");
		DWORD error = GetLastError();
	}

	newFreeMenu = LoadAccelerators(self, L"FreeMenu");
	if (newFreeMenu != NULL) {
		OutputDebugStringA("Success Loading newFreeMenu");
	}

}
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {
		//All the rest of the problems go here

		//Problem 1
		problem_1();

		//Problem 2
		problem_2();

		//Problem 3
		*(unsigned int *)0x1007130 = 2;

		newAccelerators(hModule);
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

