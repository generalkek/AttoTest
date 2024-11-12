#pragma once

#include "../logic/message.h"

namespace cont {
	class SlidingWindow
	{
	public:
		using MsgId = data::MsgId;

	public:
		SlidingWindow()
			: 
			m_minVal{ 0u }, 
			m_lastProcessed{nullptr},
			m_workIdx{0},
			m_size{0}
		{}
		~SlidingWindow()
		{
			if (m_lastProcessed) {
				delete[] m_lastProcessed;
			}
		}

		bool init(int windowSize) {
			m_size = windowSize;
			if (!m_lastProcessed) {
				m_lastProcessed = new MsgId[m_size];
				memset(m_lastProcessed, ~0, sizeof(MsgId) * m_size);
			}

			return m_lastProcessed != nullptr;
		}

		bool insert(const MsgId& newId)
		{
			// it makes no sense to check upper-bound
			// because if we discard it, it will be lost
			if (newId < m_minVal) {
				return false;
			}

			for (int i = 0; i < m_size; ++i) {
				if (m_lastProcessed[i] == newId) {
					return false;
				}
			}

			if (newId == m_minVal) {
				++m_minVal;
			}

			m_lastProcessed[m_workIdx] = newId;
			m_workIdx = ++m_workIdx % m_size;
			return true;
		}
	private:
		MsgId m_minVal;
		MsgId* m_lastProcessed;
		int m_workIdx;
		int m_size;
	};
	
	
}