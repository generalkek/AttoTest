#include "socket/socket.h"
#include "logic/server.h"
#include "utils/misc.h"

int main(int argc, char** argv) {
	if (!soc::initSocLib()) {
		return -1;
	}
	
	int targetVal = 10;
	utils::setIfHasParams<int>(argc, argv, "-t", &targetVal);
	
	Server s{ targetVal };
	s.start(2);

	system("pause");
	
	soc::shutdownSocLib();
    return 0;
}