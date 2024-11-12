#pragma once
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <string>

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "../utils/timer.h"

namespace soc {

	static bool s_winSockInitialized = false;
	bool initSocLibImpl() {
		if (s_winSockInitialized) {
			return true;
		}

		WSADATA wsaData;
		int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (res != 0) {
			printf_s("WSAStartup failed. Error: %d\n", res);
			return false;
		}
		s_winSockInitialized = true;
		return true;
	}

	void shutdownSocLibImlp() {
		WSACleanup();
	}

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

		SOCKET m_socket;
		utils::Timer m_timer;
		sockaddr* m_sockaddr;
		int m_addrlen;
		bool m_isBlocking;
	};

	Socket::Impl::Impl() {
		m_isBlocking = true;
		m_socket = INVALID_SOCKET;
		m_sockaddr = nullptr;
		m_addrlen = 0;
	}

	Socket::Impl::~Impl()
	{
		if (m_sockaddr) {
			delete m_sockaddr;
			m_sockaddr = nullptr;
		}

		if (m_socket != INVALID_SOCKET) {
			closesocket(m_socket);
		}
	}

	bool Socket::Impl::init(int port, SocketType type, SocketRole role)
	{
		struct addrinfo hints, * addrInfo;
		ZeroMemory(&hints, sizeof(hints));
		bool isSender = role == SocketRole::Sender;
		hints.ai_family = AF_INET;
		hints.ai_socktype = type == SocketType::TCP ? SOCK_STREAM : SOCK_DGRAM;
		hints.ai_protocol = type == SocketType::TCP ? IPPROTO_TCP : IPPROTO_UDP;
		hints.ai_flags = !isSender ? AI_PASSIVE : 0;

		const std::string sport{ std::to_string(port) };
		const char* ipPtr =  isSender  ? defaultIp : nullptr;

		int result = getaddrinfo(ipPtr, sport.c_str(), &hints, &addrInfo);
		if (result != 0) {
			printf_s("Failed to get socket address, port: %d, error: %d\n", port, result);
			return false;
		}

		m_socket = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
		if (m_socket == INVALID_SOCKET) {
			printf_s("Failed to create socket. Error: %d\n", WSAGetLastError());
			freeaddrinfo(addrInfo);
			return false;
		}
		m_addrlen = static_cast<int>(addrInfo->ai_addrlen);
		m_sockaddr = new sockaddr(*addrInfo->ai_addr);
		freeaddrinfo(addrInfo);

		if (!m_sockaddr) {
			printf_s("[ERROR]""Failed to allocate memory for sockaddr.\n" );
			return false;
		}

		printf_s("[INFO]""Successfully created socket. " __FUNCTION__ " line:%d\n", __LINE__);
		if (!m_isBlocking) {
			u_long mode = 1;
			result = ioctlsocket(m_socket, FIONBIO, &mode);
			if (result != 0) {
				printf_s("Failed to set non-blocking mode for socket. Port: %d, error: %d", port, WSAGetLastError());
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
			result = ::connect(m_socket, m_sockaddr, m_addrlen);
			if (result == 0) {
				return true;
			}

			if (result == SOCKET_ERROR) {
				lastError = WSAGetLastError();
				if (lastError != WSAEWOULDBLOCK && lastError != WSAEALREADY) {
					printf_s("Failed to connect to the server. Error: %d\n", lastError);
					return false;
				}

				if (m_timer.hasPassed<utils::millis>(5000)) {
					printf_s("Failed to connect, no response from server.");
					return false;
				}
			}
		} while (lastError == WSAEWOULDBLOCK || lastError == WSAEALREADY);

		return true;
	}

	bool Socket::Impl::shutdown(SocketRole role)
	{
		int result = ::shutdown(m_socket, role == SocketRole::Sender ? SD_SEND : SD_RECEIVE);
		if (result == SOCKET_ERROR) {
			printf_s("Failed to shutdown connection. Error: %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	bool Socket::Impl::bind()
	{
		int result = ::bind(m_socket, m_sockaddr, m_addrlen);
		if (result == SOCKET_ERROR) {
			printf_s("Bind failed. Error: %d", WSAGetLastError());
			return false;
		}
		return true;
	}

	bool Socket::Impl::listen()
	{
		int result = ::listen(m_socket, SOMAXCONN);
		if (result == SOCKET_ERROR) {
			printf_s("Listen failed. Error: %d", WSAGetLastError());
			return false;
		}
		return true;
	}

	Socket::Impl* Socket::Impl::accept(unsigned int timeoutMs)
	{
		int lastError = 0;
		SOCKET result = INVALID_SOCKET;
		m_timer.reset();
		do {
			result = ::accept(m_socket, nullptr, nullptr);
			if (result == INVALID_SOCKET) {
				lastError = WSAGetLastError();
				if (lastError != WSAEWOULDBLOCK) {
					printf_s("Failed to accept new connection. Error: %d", lastError);
					return nullptr;
				}
				if (m_timer.hasPassed<utils::millis>(timeoutMs)) {
					return nullptr;
				}
			}
			else {
				break;
			}
		} while (lastError == WSAEWOULDBLOCK);

		Socket::Impl* mySocRes = new Socket::Impl();
		mySocRes->m_socket = result;
		
		if (!m_isBlocking) {
			u_long mode = 1;
			result = ioctlsocket(mySocRes->m_socket, FIONBIO, &mode);
			if (result != 0) {
				printf_s("Failed to set non-blocking mode for accepted socket. Error: %d\n", WSAGetLastError());
				return false;
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
			if (result == SOCKET_ERROR) {
				lastError = WSAGetLastError();
				if (lastError != WSAEWOULDBLOCK) {
					printf_s("Failed to receive packet. Error: %d\n", WSAGetLastError());
					return 0;
				}
				if (m_timer.hasPassed<utils::millis>(s_defaultTimeoutMs)) {
					return 0;
				}
			}
			else {
				break;
			}
			
		} while (lastError == WSAEWOULDBLOCK);
		
		return result;
	}

	int Socket::Impl::send(char* buf, int bufLength)
	{
		int result = ::sendto(m_socket, buf, bufLength, 0, m_sockaddr, m_addrlen);
		if (result == SOCKET_ERROR) {
			printf_s("Failed to send packet. Error: %d\n", WSAGetLastError());
			return 0;
		}
		return result;
	}
}