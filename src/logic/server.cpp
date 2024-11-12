#include "server.h"

#include <thread>
#include <memory>

#include "message.h"
#include "../socket/socket.h"

using SocPtr = std::unique_ptr<soc::Socket>;

struct Server::DataReceiver {
	Server* m_server;
	SocPtr m_soc;
	int m_id;

	DataReceiver(Server* c, SocPtr ptr, int id);
	DataReceiver(DataReceiver&& other) noexcept;
	DataReceiver& operator=(DataReceiver&& other) noexcept;

	DataReceiver(const DataReceiver&) = delete;
	DataReceiver& operator=(const DataReceiver&) = delete;

	void operator()();
};

struct Server::DataSender {
	Server* m_server;
	SocPtr m_soc;
	int m_id;

	DataSender(Server* s, SocPtr ptr, int id);
	DataSender(DataSender&& other) noexcept;
	DataSender& operator=(DataSender&& other) noexcept;

	DataSender(const DataSender&) = delete;
	DataSender& operator=(const DataSender&) = delete;

	void operator()();
};

Server::Server()
{
	m_run = 0;
	m_dupesDiscarded = 0;
}

Server::~Server()
{
	// cleanup resources
}

void Server::start(int numberOfReceivers)
{
	// create sliding window, basically array and init
	if (!m_sw.init(16)) {
		printf("Failed to initialize sliding window, aborting.\n");
		return;
	}
	
	// create message countainer with number of pages = threads * 2
	if (!m_msgCont.init(numberOfReceivers, 1024)) {
		printf("Failed to initialize message container, aborting.\n");
		return;
	}

	{
		SocPtr ptr{ std::make_unique<soc::Socket>(
			soc::socTCPPortStart,
			soc::SocketType::TCP,
			soc::SocketRole::Sender) };
		std::thread t = std::thread(DataSender(this, std::move(ptr), 0));
		t.detach();
	}
	
	for (int i = 0; i < numberOfReceivers; ++i) {

		SocPtr ptr{ std::make_unique<soc::Socket>(
			soc::socUDPPortStart + i,
			soc::SocketType::UDP,
			soc::SocketRole::Listener) };
		std::thread t = std::thread(DataReceiver{ this, std::move(ptr), i });
		t.detach();
	}

	m_run = 1;
	m_lastPacketTimestamp.reset();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		if (m_lastPacketTimestamp.hasPassed<std::chrono::seconds>(10)) {
			break;
		}
	}
	m_run = 0;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}


// Data Receiver
Server::DataReceiver::DataReceiver(Server* c, SocPtr ptr, int id)
	: m_server{c}, m_soc{std::move(ptr)}, m_id{id}
{
}

Server::DataReceiver::DataReceiver(DataReceiver&& other) noexcept
	: m_server{nullptr}, m_id {0}
{
	this->operator=(std::move(other));
}

Server::DataReceiver& Server::DataReceiver::operator=(DataReceiver&& other) noexcept
{
	m_server = other.m_server;
	other.m_server = nullptr;
	m_soc = std::move(other.m_soc);
	m_id = other.m_id;
	return *this;
}

void Server::DataReceiver::operator()()
{
	if (!m_soc->init() || !m_soc->bind()) {
		return;
	}

	while (!m_server->m_run) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	while (true) {
		if (!m_server->m_run) {
			return;
		}

		char buffer[64];
		int received = m_soc->receive(buffer, 64);
		if (received < 0) {
			return;
		}
		else if (received == 0) {
			continue;
		}
		
		data::message msg{};
		data::DeserialiseMessage(buffer, &msg);

		{
			sync::lock_guard lock{ m_server->m_slidingWindowLock };
			if (!m_server->m_sw.insert(msg.MessageId)) {
				m_server->m_dupesDiscarded++;
				continue;
			}
		}

		std::string smsg{ data::toString(msg) };
		printf("Received packet size: %d, threadId: %d\n", received, m_id);
		printf("%s\n", smsg.c_str());
		
		m_server->m_msgCont.insert(m_id, msg);
		m_server->m_lastPacketTimestamp.reset();

		if (msg.MessageData == 10) {
			sync::lock_guard lock{ m_server->m_tcpQueueLock };
			m_server->m_tcpQueue.push(msg);
		}
	}
}


// Data Sender
Server::DataSender::DataSender(Server* s, SocPtr ptr, int id)
	:m_server{ s }, m_soc{std::move(ptr)}, m_id{id}
{
}

Server::DataSender::DataSender(Server::DataSender&& other) noexcept
	:m_server{nullptr}, m_id{0}
{
	this->operator=(std::move(other));
}

Server::DataSender& Server::DataSender::operator=(Server::DataSender&& other) noexcept
{
	m_server = other.m_server;
	other.m_server = nullptr;
	m_soc = std::move(other.m_soc);
	m_id = other.m_id;
	return *this;
}

void Server::DataSender::operator()()
{
	if (!m_soc->init() || !m_soc->connect()) {
		return;
	}

	while (!m_server->m_run) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	data::message msg{};

	while (true) {
		if (!m_server->m_run) {
			m_soc->shutdown();
			return;
		}

		{
			sync::lock_guard lock{ m_server->m_tcpQueueLock };
			if (m_server->m_tcpQueue.empty()) {
				continue;
			}
			msg = m_server->m_tcpQueue.pop();
		}


		char buff[sizeof(data::message)];
		data::SerialiseMessage(buff, &msg);

		int result = m_soc->send(buff, sizeof(data::message));
		if (result <= 0) {
			m_soc->shutdown();
			return;
		}

		std::string smsg{ data::toString(msg) };
		printf("Sending message: %s\n", smsg.c_str());
	}
}