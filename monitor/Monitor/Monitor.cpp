#include "Monitor.h"
#include <iostream>

Monitor::Monitor(int argc, char* argv[]) : targetPid(0) {
	parseArguments(argc, argv);
}

void Monitor::parseArguments(int argc, char* argv[]) {
	std::wstring processName;

	for (int i = 1; i < argc; i++) {
		std::wstring arg = s2ws(argv[i]);
		if (arg == L"-pid" && i + 1 < argc) {
			targetPid = atoi(argv[++i]);
		}
		else if (arg == L"-name" && i + 1 < argc) {
			processName = s2ws(argv[++i]);
		}
		else if (arg == L"-func" && i + 1 < argc) {
			mode = L"func";
			param = s2ws(argv[++i]);
		}
		else if (arg == L"-hide" && i + 1 < argc) {
			mode = L"hide";
			param = s2ws(argv[++i]);
		}
	}

	if (targetPid == 0 && !processName.empty()) {
		targetPid = ProcessManager::findProcessId(processName);
	}
}

void Monitor::run() {
	Pipe& pipe = Pipe::getInstance();

	wchar_t dllPathBuffer[MAX_PATH];
	GetFullPathNameW(L"hookdll.dll", MAX_PATH, dllPathBuffer, NULL);
	std::wstring dllPath(dllPathBuffer);

	if (!pipe.createServerPipe() || !ProcessManager::injectDLL(targetPid, dllPath)) {
		return;
	}

	HANDLE hPipe = pipe.getHandle();
	std::cout << "Waiting for DLL to connect..." << std::endl;
	BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	if (!connected) {
		std::cerr << "Failed to establish connection on named pipe." << std::endl;
		CloseHandle(hPipe);
		return;
	}
	std::cout << "DLL connected to the pipe." << std::endl;


	std::wcout << L"Sending command to DLL: " << L"MODE:" + mode + L";PARAM:" + param << std::endl;
	std::wstring message = L"MODE:" + mode + L";PARAM:" + param;
	pipe.sendMessage(message);

	std::cout << "Listening for messages from DLL..." << std::endl;
	while(pipe.receiveMessage([](const std::wstring& msg) {
		std::wcout << L"[DLL] " << msg << std::endl;
	}));

	pipe.closePipe();
	std::cout << "Pipe closed. Exiting." << std::endl;
	std::cin.get();
}
