#include "Pipe.h"
#include <iostream>

Pipe& Pipe::getInstance() {
	static Pipe instance;
	return instance;
}

HANDLE Pipe::getHandle()
{
	return hPipe;
}

bool Pipe::createServerPipe() {
	hPipe = CreateNamedPipeW(
		pipeName.c_str(),
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1,
		1024,
		1024,
		0,
		NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to create named pipe." << std::endl;
		return false;
	}
	return true;
}

bool Pipe::connectToServer() {
	while (true) {
		hPipe = CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hPipe != INVALID_HANDLE_VALUE)
			return true;
		Sleep(100);
	}
}

bool Pipe::sendMessage(const std::wstring& message) {
	if (hPipe == INVALID_HANDLE_VALUE) return false;

	DWORD bytesWritten;
	return WriteFile(hPipe, ws2s(message).c_str(), static_cast<DWORD>(ws2s(message).size()), &bytesWritten, NULL);
}

bool Pipe::receiveMessage(std::function<void(const std::wstring&)> callback) {
	if (hPipe == INVALID_HANDLE_VALUE) return false;

	char buffer[512] = { 0 };
	DWORD bytesRead;
	if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
		buffer[bytesRead] = '\0';
		callback(s2ws(std::string(buffer)));
		return true;
	}
	return false;
}

void Pipe::closePipe() {
	if (hPipe != INVALID_HANDLE_VALUE) {
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}
