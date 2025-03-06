#include "Monitor.h"
#include <iostream>

Monitor::Monitor(int argc, char* argv[]) : targetPid(0) {
	parseArguments(argc, argv);
}

void Monitor::parseArguments(int argc, char* argv[]) {
	std::string processName;

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "-pid" && i + 1 < argc) {
			targetPid = atoi(argv[++i]);
		}
		else if (arg == "-name" && i + 1 < argc) {
			processName = argv[++i];
		}
		else if (arg == "-func" && i + 1 < argc) {
			mode = "func";
			param = argv[++i];
		}
		else if (arg == "-hide" && i + 1 < argc) {
			mode = "hide";
			param = argv[++i];
		}
	}

	if (targetPid == 0 && !processName.empty()) {
		targetPid = ProcessManager::findProcessId(processName);
	}
}

void Monitor::run() {
	Pipe& pipe = Pipe::getInstance();

	char dllPathBuffer[MAX_PATH];
	GetFullPathName("hookdll.dll", MAX_PATH, dllPathBuffer, NULL);
	std::string dllPath(dllPathBuffer);

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


	std::cout << "Sending command to DLL: " << "MODE:" + mode + ";PARAM:" + param << std::endl;
	std::string message = "MODE:" + mode + ";PARAM:" + param;
	pipe.sendMessage(message);

	std::cout << "Listening for messages from DLL..." << std::endl;
	while(pipe.receiveMessage([](const std::string& msg) {
		std::cout << "[DLL] " << msg << std::endl;
	}));

	pipe.closePipe();
	std::cout << "Pipe closed. Exiting." << std::endl;
	std::cin.get();
}
