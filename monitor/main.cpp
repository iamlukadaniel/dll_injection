#include "Monitor/Monitor.h"

int main(int argc, char* argv[]) {
    Monitor monitor(argc, argv);
    monitor.run();
    return 0;
}
