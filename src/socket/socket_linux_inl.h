#pragma once

#ifdef __linux

#include <stdio.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#include "../utils/timer.h"

#define INVALID_SOCKET -1

namespace soc {
static bool s_winSockInitialized = false;
	bool initSocLib() {
		if (s_winSockInitialized) {
			return true;
		}
		s_winSockInitialized = true;
		return true;
	}

	void shutdownSocLib() {}

	class Socket::Impl {
	public:
		Impl();
		~Impl();

		bool init(int port, SocketType type, SocketRole role);
		
		bool connect();
		bool shutdown(SocketRole role);

		bool bind();
		bool listen();
		Socket::Impl* accept(unsigned int timeoutMs);

		int receive(char* buf, int bufLength);
		int send(char* buf, int bufLength);


		static const unsigned int s_defaultTimeoutMs = 5000;

		int m_socket;
		utils::Timer m_timer;
		sockaddr_in m_sockaddr;
		int m_addrlen;
		bool m_isBlocking;
	};

	Socket::Impl::Impl() {
		m_isBlocking = true;
		m_socket = INVALID_SOCKET;
		m_addrlen = 0;
	}

	Socket::Impl::~Impl()
	{
		if (m_socket != INVALID_SOCKET) {
			close(m_socket);
		}
	}

	bool Socket::Impl::init(int port, SocketType type, SocketRole role)
	{
        int socType = type == SocketType::TCP ? SOCK_STREAM : SOCK_DGRAM;
        m_socket = socket(AF_INET, socType, 0);
        if (m_socket == INVALID_SOCKET) {
			LOG_ERROR("Failed to create socket. Error: %d", errno);
			return false;
		}
        
        memset(&m_sockaddr, 0, sizeof(sockaddr_in));

		bool isSender = role == SocketRole::Sender;
		m_sockaddr.sin_family = AF_INET;
		m_sockaddr.sin_port = htons(port);
		m_sockaddr.sin_addr.s_addr = isSender ? inet_addr(defaultIp) : INADDR_ANY;
		
		if (!m_isBlocking) {
            int flags = fcntl(m_socket, F_GETFL, 0);
            if (flags == -1) {
                return true;
            }
            flags |= O_NONBLOCK;
            int setBlockRes = fcntl(m_socket, F_SETFL, flags);
            if (setBlockRes != 0) {
				LOG_ERROR("Failed to set non-blocking mode for socket. Port: %d, error: %d", port, errno);
				return false;
            }
		}

		return true;
	}

	bool Socket::Impl::connect()
	{
		int result = 0;
		int lastError = 0;
		m_timer.reset();
		do {
			result = ::connect(m_socket, 
            reinterpret_cast<sockaddr*>(&m_sockaddr), sizeof(sockaddr));
			if (result == 0) {
				return true;
			}

			if (result < 0) {
				lastError = errno;
				if (lastError != EINPROGRESS && lastError != EALREADY) {
					LOG_ERROR("Failed to connect to the server. Error: %d", lastError);
					return false;
				}

				if (m_timer.hasPassed<utils::millis>(5000)) {
					LOG_ERROR("Failed to connect, no response from server.");
					return false;
				}
			}
		} while (lastError == EINPROGRESS || lastError == EALREADY);

		return true;
	}

	bool Socket::Impl::shutdown(SocketRole role)
	{
		int result = ::shutdown(m_socket, role == SocketRole::Sender ? SHUT_WR : SHUT_RD);
		if (result < 0) {
			LOG_ERROR("Failed to shutdown connection. Error: %d", errno);
			return false;
		}
		return true;
	}

	bool Socket::Impl::bind()
	{
		int result = ::bind(m_socket, reinterpret_cast<sockaddr*>(&m_sockaddr), sizeof(sockaddr));
		if (result < 0) {
			LOG_ERROR("Bind failed. Error: %d", errno);
			return false;
		}
		return true;
	}

	bool Socket::Impl::listen()
	{
		int result = ::listen(m_socket, SOMAXCONN);
		if (result < 0) {
			LOG_ERROR("Listen failed. Error: %d", errno);
			return false;
		}
		return true;
	}

	Socket::Impl* Socket::Impl::accept(unsigned int timeoutMs)
	{
		int lastError = 0;
		int result = INVALID_SOCKET;
		m_timer.reset();
		do {
			result = ::accept(m_socket, nullptr, nullptr);
			if (result == INVALID_SOCKET) {
				lastError = errno;
				if (lastError != EWOULDBLOCK) {
					LOG_ERROR("Failed to accept new connection. Error: %d", lastError);
					return nullptr;
				}
				if (m_timer.hasPassed<utils::millis>(timeoutMs)) {
					return nullptr;
				}
			}
			else {
				break;
			}
		} while (lastError == EWOULDBLOCK);

		Socket::Impl* mySocRes = new Socket::Impl();
		mySocRes->m_socket = result;
		
		if (!m_isBlocking) {
            int flags = fcntl(mySocRes->m_socket, F_GETFL, 0);
            if (flags != -1) {
                flags |= O_NONBLOCK;
                int setBlockRes = fcntl(mySocRes->m_socket, F_SETFL, flags);        
            }
		}
		return mySocRes;
	}

	int Socket::Impl::receive(char* buf, int bufLength)
	{
		int result = 0;
		int lastError = 0;
		m_timer.reset();
		do {
			result = ::recvfrom(m_socket, buf, bufLength, 0, nullptr, nullptr);
			if (result < 0) {
				lastError = errno;
				if (lastError != EWOULDBLOCK) {
					LOG_ERROR("Failed to receive packet. Error: %d\n", errno);
					return 0;
				}
				if (m_timer.hasPassed<utils::millis>(s_defaultTimeoutMs)) {
					return 0;
				}
			}
			else {
				break;
			}
			
		} while (lastError == EWOULDBLOCK);
		
		return result;
	}

	int Socket::Impl::send(char* buf, int bufLength)
	{
		int result = ::sendto(m_socket, buf, bufLength, 0, reinterpret_cast<sockaddr*>(&m_sockaddr), sizeof(sockaddr));
		if (result < 0) {
			LOG_ERROR("Failed to send packet. Error: %d\n", errno);
			return 0;
		}
		return result;
	}
}

#undef INVALID_SOCKET

#endif