#include "socket.h"

namespace soc {
	static const char* defaultIp = "127.0.0.1";
	const int socUDPPortStart = 10100;
	const int socTCPPortStart = 10200;
}

#ifdef WIN32 
#include "socket_win_inl.h"
#endif

namespace soc {

	Socket::Socket()
	:
		m_port(8080),
		m_type{SocketType::TCP},
		m_role{SocketRole::Listener},
		m_imp{nullptr}
	{
	}

	Socket::Socket(int port, SocketType type, SocketRole role, bool isBlocking /* = false */)
		:
		m_port{port},
		m_type{type},
		m_role{role}
	{
		m_imp = new Socket::Impl();
		m_imp->m_isBlocking = isBlocking;
	}

	Socket::~Socket()
	{
		if (m_imp) {
			delete m_imp;
			m_imp = nullptr;
		}
	}

	bool Socket::init()
	{
		return m_imp->init(m_port, m_type, m_role);
	}

	bool Socket::connect() {
		return m_imp->connect();
	}

	bool Socket::shutdown()
	{
		return m_imp->shutdown(m_role);
	}

	int Socket::send(char* buf, int bufLength)
	{
		return m_imp->send(buf, bufLength);
	}

	Socket* Socket::accept(unsigned int timeoutMs)
	{
		Socket::Impl* res = m_imp->accept(timeoutMs);
		if (!res) {
			return nullptr;
		}

		Socket* myRes = new Socket();
		myRes->m_imp = res;
		myRes->m_port = m_port;
		return myRes;
	}

	bool Socket::bind()
	{
		return m_imp->bind();
	}

	int Socket::receive(char* outBuf, int bufLength)
	{
		return m_imp->receive(outBuf, bufLength);
	}

	bool Socket::listen()
	{
		if (m_role != SocketRole::Listener) {
			printf_s("Sender socket trying to listen.");
			return false;
		}
		return m_imp->listen();
	}

	bool initSocLib()
	{
		return initSocLibImpl();
	}

	void shutdownSocLib()
	{
		shutdownSocLibImlp();
	}
}