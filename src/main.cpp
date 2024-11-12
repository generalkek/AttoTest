#include "socket/socket.h"
#include "logic/server.h"

int main() {
	if (!soc::initSocLib()) {
		return -1;
	}
	
	Server s{};
	s.start(2);

	system("pause");
	
	soc::shutdownSocLib();
    return 0;
}