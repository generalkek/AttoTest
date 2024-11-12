#include <thread>
#include <chrono>
#include <memory>
#include <string>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../socket/socket.h"
#include "../utils/spinlock.h"
#include "../utils/Random.h"
#include "../logic/message.h"

using MsgId = std::uint64_t;
using aflag = sync::aflag;
using spinlock = sync::spinlock;

class Client {
public:
	Client();
	void start(int numberOfSenders, int maxPacketToSend);

	inline MsgId getMsgId() { return m_id++; }
	inline int getPoolIdx() { return m_msgPoolIdx++ & (s_msgPoolSize - 1); }

	using SocPtr = std::unique_ptr<soc::Socket>;
	using Msg = data::message;
private:
	struct DataSender {
		Client* m_client;
		SocPtr m_soc;
		int m_packSinceDup;

		DataSender(Client *c, SocPtr ptr);
		DataSender(DataSender&& other) noexcept;
		DataSender& operator=(DataSender&& other) noexcept;
		
		DataSender(const DataSender&) = delete;
		DataSender& operator=(const DataSender&) = delete;

		void operator()();
	};

	static constexpr int s_msgPoolSize = 8;

private:
	void resetMsgId() { m_id = 0; } // maybe unused
	
	int m_run;
	sync::spinlock m_flagLock;
	MsgId m_id;
	int m_msgPoolIdx;
	Msg m_messagePool[s_msgPoolSize];
	int m_curPacketSent;
	int m_maxPacketToSend;
	int m_dupFreq;
};

int main() {
	if (!soc::initSocLib()) {
		return -1;
	}

	Client c{};
	c.start(2, 100);
	system("pause");
	soc::shutdownSocLib();
	return 0;
}

Client::Client()
{
	m_run = 0;
	m_id = 0;
	m_msgPoolIdx = 0;
	m_dupFreq = 10;
	memset(m_messagePool, 0, sizeof(m_messagePool));
	m_curPacketSent = 0;
	m_maxPacketToSend = 100;
}

void Client::start(int numberOfSenders, int maxPacketToSend)
{
	m_maxPacketToSend = maxPacketToSend;

	for (int i = 0; i < s_msgPoolSize; ++i) {
		m_messagePool[i] = {
			static_cast<uint16_t>(math::Random(10, 100)),
			static_cast<uint8_t>(math::Random(10, 100)),
			0,
			math::Random(5, 15),
		};
	}

	for (int i = 0; i < numberOfSenders; ++i) {
		
		SocPtr ptr{ std::make_unique<soc::Socket>(
			soc::socUDPPortStart + i,
			soc::SocketType::UDP,
			soc::SocketRole::Sender) };
		std::thread t = std::thread(DataSender{ this, std::move(ptr) });
		t.detach();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	m_run = 1;
	while (m_curPacketSent < m_maxPacketToSend) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	m_run = 0;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

Client::DataSender::DataSender(Client* c, SocPtr ptr)
	:m_client{c}, m_soc{std::move(ptr)}, m_packSinceDup{0}
{
}

Client::DataSender::DataSender(DataSender&& other) noexcept
	:m_client{nullptr}, m_packSinceDup{ 0 }
{
	this->operator=(std::move(other));
}

Client::DataSender& Client::DataSender::operator=(DataSender&& other) noexcept
{
	m_client = other.m_client;
	other.m_client = nullptr;
	m_soc = std::move(other.m_soc);
	m_packSinceDup = other.m_packSinceDup;
	return *this;
}

void Client::DataSender::operator()()
{
	if (!m_soc->init()) {
		return;
	}

	while (!m_client->m_run) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	while (true) {
		if (!m_client->m_run) {
			return;
		}

		MsgId newId = 0;
		int poolIdx = 0;
		if (m_packSinceDup < m_client->m_dupFreq)
		{
			sync::lock_guard lock{ m_client->m_flagLock };
			newId = m_client->getMsgId();
			poolIdx = m_client->getPoolIdx();
			m_client->m_curPacketSent++; // for now it is ok
		}
		else {
			m_packSinceDup %= m_client->m_dupFreq;
			newId = m_client->m_id;
			poolIdx = m_client->getPoolIdx();
		}

		char buf[sizeof(data::message)];
		data::message msg = m_client->m_messagePool[poolIdx];
		msg.MessageId = newId;
		data::SerialiseMessage(buf, &msg);

		int sent = m_soc->send(buf, sizeof(data::message));
		if (sent < 0) {
			return;
		}
		++m_packSinceDup;

		std::string smsg{ data::toString(msg) };
		printf_s("Sent: %s\n", smsg.c_str());
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
		// move packet counter here
	}
}
