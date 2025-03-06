#ifndef PIPE_H
#define PIPE_H

#include <windows.h>
#include <string>
#include <functional>

class Pipe {
public:
	static Pipe& getInstance();
	HANDLE getHandle();

	bool createServerPipe();
	bool connectToServer();
	bool sendMessage(const std::string& message);
	bool receiveMessage(std::function<void(const std::string&)> callback);
	void closePipe();


private:
	Pipe() = default;
	Pipe(const Pipe&) = delete;
	Pipe& operator=(const Pipe&) = delete;

	HANDLE hPipe = INVALID_HANDLE_VALUE;
	const std::string pipeName = "\\\\.\\pipe\\HookPipe";
};

#endif // PIPE_H
