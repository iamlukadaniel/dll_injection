#ifndef PIPE_H
#define PIPE_H

#include <windows.h>
#include <string>
#include <functional>
#include "StringConverter.h"

class Pipe {
public:
	static Pipe& getInstance();
	HANDLE getHandle();

	bool createServerPipe();
	bool connectToServer();
	bool sendMessage(const std::wstring& message);
	bool receiveMessage(std::function<void(const std::wstring&)> callback);
	void closePipe();


private:
	Pipe() = default;
	Pipe(const Pipe&) = delete;
	Pipe& operator=(const Pipe&) = delete;

	HANDLE hPipe = INVALID_HANDLE_VALUE;
	const std::wstring pipeName = L"\\\\.\\pipe\\HookPipe";
};

#endif // PIPE_H
