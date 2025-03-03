#include <windows.h>
#include <iostream>
#include <string>
#include <ctime>
#include <cstdio>
#include "HookManager.h"

#define PIPE_NAME "\\\\.\\pipe\\HookPipe"

HANDLE g_hPipe = INVALID_HANDLE_VALUE;
std::string g_mode;
std::string g_param;

HookManager g_hookManager;

DWORD WINAPI PipeClientThread(LPVOID lpParam)
{
	while (true)
	{
		g_hPipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (g_hPipe != INVALID_HANDLE_VALUE)
			break;
		Sleep(100);
	}

	char buffer[512] = { 0 };
	DWORD bytesRead;
	if (ReadFile(g_hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		std::string config(buffer);
		size_t modePos = config.find("MODE:");
		size_t paramPos = config.find(";PARAM:");
		if (modePos != std::string::npos && paramPos != std::string::npos)
		{
			g_mode = config.substr(modePos + 5, paramPos - (modePos + 5));
			size_t endPos = config.find("\n", paramPos);
			if (endPos == std::string::npos)
				endPos = config.length();
			g_param = config.substr(paramPos + 7, endPos - (paramPos + 7));
		}
	}

	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
	if (g_mode == "hide")
	{

	}
	else if (g_mode == "func")
	{

	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//MessageBoxW(NULL, L"DLL has been injected!", L"Success!", MB_OK | MB_ICONINFORMATION);

		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, 0, PipeClientThread, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		if (g_hPipe != INVALID_HANDLE_VALUE)
			CloseHandle(g_hPipe);
		break;
	}
	return TRUE;
}
