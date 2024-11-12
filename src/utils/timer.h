#pragma once

#include <chrono>

namespace utils {

	using millis = std::chrono::milliseconds;
	using secs = std::chrono::seconds;

	class Timer {
	public:

		using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
	
	public:
		Timer()
		{
			m_timestamp = std::chrono::system_clock::now();
		}
		~Timer() {}

		void reset()
		{
			m_timestamp = std::chrono::system_clock::now();
		}

		template <typename Dur = millis>
		bool hasPassed(unsigned int timePassed)
		{
			auto now = std::chrono::system_clock::now();
			return std::chrono::duration_cast<Dur>(now - m_timestamp).count() > timePassed;
		}

	private:
		TimePoint m_timestamp;
	};
}