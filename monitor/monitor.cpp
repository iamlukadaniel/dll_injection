#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

#define PIPE_NAME "\\\\.\\pipe\\HookPipe"

DWORD FindProcessId(const std::string& processName) {
	DWORD processId = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe)) {
			do {
				if (_stricmp(pe.szExeFile, processName.c_str()) == 0) {
					processId = pe.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &pe));
		}
		CloseHandle(hSnapshot);
	}
	return processId;
}

bool InjectDLL(DWORD pid, const std::string& dllPath) {
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
		PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
		FALSE, pid);
	if (!hProcess) {
		std::cerr << "Failed to open process " << pid << std::endl;
		return false;
	}
	size_t size = dllPath.size() + 1;
	LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT, PAGE_READWRITE);
	if (!pRemoteMemory) {
		std::cerr << "Failed to allocate memory in target process." << std::endl;
		CloseHandle(hProcess);
		return false;
	}
	if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.c_str(), size, NULL)) {
		std::cerr << "Failed to write data to target process memory." << std::endl;
		VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}
	auto pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (!pLoadLibrary) {
		std::cerr << "Failed to get address of LoadLibraryA." << std::endl;
		VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibrary, pRemoteMemory, 0, NULL);
	if (!hThread) {
		std::cerr << "Failed to create remote thread." << std::endl;
		VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}
	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	return true;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage:" << std::endl;
		std::cerr << "  monitor.exe -pid <PID> -func <function name>" << std::endl;
		std::cerr << "  monitor.exe -name <process name> -func <function name>" << std::endl;
		std::cerr << "  monitor.exe -name <process name> -hide <file name>" << std::endl;
		return 1;
	}

	DWORD targetPid = 0;
	std::string processName;
	std::string mode; // func / hide
	std::string param;

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
		targetPid = FindProcessId(processName);
		if (targetPid == 0) {
			std::cerr << "Process " << processName << " not found." << std::endl;
			return 1;
		}
	}
	if (targetPid == 0) {
		std::cerr << "No valid target process specified." << std::endl;
		return 1;
	}

	std::cout << "Target PID: " << targetPid << std::endl;

	// building DLL absolute path
	char dllPathBuffer[MAX_PATH];
	GetFullPathName("hookdll.dll", MAX_PATH, dllPathBuffer, NULL);
	std::string dllPath(dllPathBuffer);

	// creating named pipe
	HANDLE hPipe = CreateNamedPipeA(
		PIPE_NAME,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1,
		1024,
		1024,
		0,
		NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to create named pipe." << std::endl;
		return 1;
	}
	std::cout << "Named pipe created: " << PIPE_NAME << std::endl;

	std::cout << "Injecting DLL: " << dllPath << std::endl;
	if (!InjectDLL(targetPid, dllPath)) {
		std::cerr << "DLL injection failed." << std::endl;
		CloseHandle(hPipe);
		return 1;
	}
	std::cout << "DLL injected successfully." << std::endl;

	// waiting for DLL to connect to the pipe
	std::cout << "Waiting for DLL to connect to the pipe..." << std::endl;
	BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	if (!connected) {
		std::cerr << "Failed to establish connection on named pipe." << std::endl;
		CloseHandle(hPipe);
		return 1;
	}
	std::cout << "DLL connected to the pipe." << std::endl;

	// sending configuration to DLL: "MODE:<mode>;PARAM:<param>\n"
	std::string configMsg = "MODE:" + mode + ";PARAM:" + param + "\n";
	DWORD bytesWritten;
	if (!WriteFile(hPipe, configMsg.c_str(), (DWORD)configMsg.size(), &bytesWritten, NULL)) {
		std::cerr << "Failed to send configuration to DLL." << std::endl;
		CloseHandle(hPipe);
		return 1;
	}
	std::cout << "Configuration sent to DLL: " << configMsg;

	// reading messages from DLL
	char buffer[512] = { 0 };
	DWORD bytesRead;
	std::cout << "Listening for messages from DLL..." << std::endl;
	while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead != 0) {
		buffer[bytesRead] = '\0';
		std::cout << "[DLL] " << buffer << std::endl;
	}

	CloseHandle(hPipe);
	std::cout << "Pipe closed. Exiting." << std::endl;
	std::cin.get();
	return 0;
}
