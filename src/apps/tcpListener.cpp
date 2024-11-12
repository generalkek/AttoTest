#include <stdio.h>
#include "../socket/socket.h"
#include "../logic/message.h"

int main() {
	if (!soc::initSocLib()) {
		return -1;
	}
	
	soc::Socket s1{soc::socTCPPortStart, soc::SocketType::TCP, soc::SocketRole::Listener, true};
	if (!s1.init() || !s1.bind() || !s1.listen()) {
		return -1;
	}
	
	while (true) {
		soc::Socket* newConnection = s1.accept(60000);
		if (newConnection) {
			char buf[64];
			int receivedBytes = 1;
			while (receivedBytes > 0) {
				receivedBytes = newConnection->receive(buf, 64);
				if (receivedBytes) {
					data::message msg{};
					data::DeserialiseMessage(buf, &msg);
					std::string smsg{ data::toString(msg) };
					printf_s("Recieved: %d, Data: %s\n", receivedBytes, smsg.c_str());
				}
			}
			newConnection->shutdown();
			delete newConnection;
		}
		else {
			break;
		}
	}
	system("pause");
	soc::shutdownSocLib();
	return 0;
}