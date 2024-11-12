#include "socket.h"

namespace soc {
	static const char* defaultIp = "127.0.0.1";
	const int socUDPPortStart = 10100;
	const int socTCPPortStart = 10200;
}

#include "../utils/log.h"

#ifdef WIN32 
#include "socket_win_inl.h"
#elif __linux
#include "socket_linux_inl.h"
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
		bool res = m_imp->connect();
		if (res && m_type == SocketType::TCP) {
			LOG_INFO("TCP socket successfully connected on port: %d", m_port);
		}
		return res;
	}

	bool Socket::shutdown()
	{	
		bool res = m_imp->shutdown(m_role);
		if (res) {
			LOG_INFO("Connection cut on port %d", m_port);
		}
		return res;
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
		LOG_INFO("New connection accepted at port: %d", m_port);
		return myRes;
	}

	bool Socket::bind()
	{
		bool res = m_imp->bind();
		if (res && m_type == SocketType::UDP) {
			LOG_INFO("UDP socket is listening port: %d", m_port);
		}
		return res;
	}

	int Socket::receive(char* outBuf, int bufLength)
	{
		return m_imp->receive(outBuf, bufLength);
	}

	bool Socket::listen()
	{
		if (m_role != SocketRole::Listener) {
			LOG_ERROR("Sender socket trying to listen.");
			return false;
		}
		bool res = m_imp->listen();
		if (res) {
			const char* stype = m_type == SocketType::TCP ? "TCP" : "UDP";
			LOG_INFO("%s socket is listening port: %d", stype, m_port);
		}
		return res;
	}
}