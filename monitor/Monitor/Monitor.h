#ifndef MONITOR_H
#define MONITOR_H

#include <string>
#include "../ProcessManager/ProcessManager.h"
#include "../../common/Pipe.h"

class Monitor {
public:
	Monitor(int argc, char* argv[]);
	void run();

private:
	DWORD targetPid;
	std::string mode;
	std::string param;

	void parseArguments(int argc, char* argv[]);
};

#endif // MONITOR_H
