#include <thread>
#include <chrono>
#include <memory>
#include <string>

#include <stdlib.h>

#include "../utils/misc.h"
#include "../utils/log.h"

#include "../socket/socket.h"
#include "../utils/spinlock.h"
#include "../utils/Random.h"
#include "../logic/message.h"

using MsgId = std::uint64_t;
using aflag = sync::aflag;
using spinlock = sync::spinlock;

class Client {
public:
	Client(int tv, int delay);
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
	
	int m_targetVal;
	int m_run;
	sync::spinlock m_flagLock;
	MsgId m_id;
	int m_msgPoolIdx;
	Msg m_messagePool[s_msgPoolSize];
	int m_packetDelayInMicrosecs;
	int m_curPacketSent;
	int m_maxPacketToSend;
	int m_dupFreq;
};

int main(int argc, char** argv) {
	if (!soc::initSocLib()) {
		return -1;
	}

	int targetVal = 10;
	int numOfPacketsToSend = 100;
	int m_packetDelayInMicrosecs = 2000;
	utils::setIfHasParams<int>(argc, argv, "-t", &targetVal);
	utils::setIfHasParams<int>(argc, argv, "-ps", &numOfPacketsToSend);
	utils::setIfHasParams<int>(argc, argv, "-pdm", &m_packetDelayInMicrosecs);

	Client c{ targetVal, m_packetDelayInMicrosecs };
	c.start(2, numOfPacketsToSend);
	system("pause");
	soc::shutdownSocLib();
	return 0;
}

Client::Client(int tv, int delay)
{
	m_targetVal = tv;
	m_run = 0;
	m_id = 0;
	m_msgPoolIdx = 0;
	m_dupFreq = 10;
	memset(m_messagePool, 0, sizeof(m_messagePool));
	m_curPacketSent = 0;
	m_maxPacketToSend = 100;
	m_packetDelayInMicrosecs = delay;
}

void Client::start(int numberOfSenders, int maxPacketToSend)
{
	m_maxPacketToSend = maxPacketToSend;

	LOG_INFO("Client starts with next params:");
	LOG_INFO("target value: %d", m_targetVal);
	LOG_INFO("numbef of packets to send: %d", m_maxPacketToSend);
	LOG_INFO("delay to send packet: %d (in microseconds)", m_packetDelayInMicrosecs);

	// force at least one item to has desired value
	m_messagePool[0] = {
			static_cast<uint16_t>(math::Random(10, 100)),
			static_cast<uint8_t>(math::Random(10, 100)),
			0,
			static_cast<uint64_t>(m_targetVal)
	};

	for (int i = 1; i < s_msgPoolSize; ++i) {
		m_messagePool[i] = {
			static_cast<uint16_t>(math::Random(10, 100)),
			static_cast<uint8_t>(math::Random(10, 100)),
			0,
			math::Random(m_targetVal - 5, m_targetVal + 15),
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
	LOG_INFO("UDP DataSender initialized. It will start sending messages soon.");

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
			m_client->m_curPacketSent++;
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
		LOG_DEBUG("Sent: %s", smsg.c_str());
		std::this_thread::sleep_for(std::chrono::microseconds(m_client->m_packetDelayInMicrosecs));
	}
}
