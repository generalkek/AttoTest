#pragma once

#include <cstring>
#include <functional>
#include "message.h"


#include "../containers/slidingWindow.h"
#include "../containers/pagedTable.h"
#include "../containers/queue.h"
#include "../utils/spinlock.h"
#include "../utils/timer.h"

class Server {
public:
	Server(int tv);
	~Server();

	void start(int numberOfReceivers);

	using SLock = sync::spinlock;
	using MsgId = data::MsgId;
	using SW = cont::SlidingWindow;
	using MsgCont = cont::PagedTable<data::message, MsgId, data::MessageHasher, data::MessageKey, std::equal_to<MsgId>>;
	using Queue = cont::Queue<data::message>;
	using Timer = utils::Timer;
private:
	struct DataReceiver;
	struct DataSender;

private:
	int m_run;
	SLock m_slidingWindowLock;
	SW m_sw;
	
	MsgCont m_msgCont;

	int m_targetVal;
	SLock m_tcpQueueLock;
	Queue m_tcpQueue;
	
	int m_dupesDiscarded;
	Timer m_lastPacketTimestamp;
};