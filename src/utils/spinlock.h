#pragma once
#include <atomic>

namespace sync {

	using aflag = std::atomic_flag;

	class spinlock
	{
		std::atomic_flag m_flag;
	public:
		spinlock() :
			m_flag ATOMIC_FLAG_INIT
		{}
		
		void lock()
		{
			while (m_flag.test_and_set(std::memory_order_acquire));
		}
		
		void unlock()
		{
			m_flag.clear(std::memory_order_release);
		}
	};

	class lock_guard
	{
		spinlock& m_lock;

	public:
		lock_guard(spinlock& lock)
			: m_lock{ lock }
		{
			m_lock.lock();
		}

		~lock_guard() {
			m_lock.unlock();
		}
	};
}