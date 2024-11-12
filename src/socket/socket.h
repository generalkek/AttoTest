#pragma once

namespace soc {
	enum class SocketType : char {
		UDP,
		TCP
	};

	enum class SocketRole : char {
		Sender,
		Listener
	};

	extern const int socUDPPortStart; // 10100
	extern const int socTCPPortStart; // 10200

	bool initSocLib();
	void shutdownSocLib();

	class Socket {
	public:
		Socket(int port, SocketType type, SocketRole role, bool isBlocking = false);
		~Socket();

		bool init();
		bool connect();
		bool shutdown();
		
		Socket* accept(unsigned int timeoutMs);
		bool bind();
		bool listen();

		int send(char* buf, int bufLength);
		int receive(char* outBuf, int bufLength);
	private:
		Socket();
		class Impl;

	private:
		Impl* m_imp;
		int m_port;
		SocketType m_type;
		SocketRole m_role;
	};
}