#pragma once

#include "../utils/spinlock.h"
#include "hashTable.h"

namespace cont {
	template <typename Type, typename Hasher, typename KeyFunc, typename Equality>
	class PagedTable {
	public:
		PagedTable()
			:
			m_pages{nullptr},
			m_activePages{nullptr},
			m_pageLocks{nullptr},
			m_pageSize{1024u},
			m_numberOfPages{4}
		{}

		~PagedTable()
		{
			if (m_pages) {
				delete[] m_pages;
			}

			if (m_activePages) {
				delete[] m_activePages;
			}

			if (m_pageLocks) {
				delete[] m_pageLocks;
			}
		}


	public:
		using Table = HashTable<Type, Hasher, KeyFunc, Equality>;

		bool init(int numberOfPages, int pageSize)
		{
			// double buffering
			m_numberOfPages = numberOfPages;
			m_pageSize = pageSize;

			m_pages = new Table[m_numberOfPages * 2];
			m_activePages = new int[numberOfPages];
			m_pageLocks = new sync::spinlock[numberOfPages] ;

			if (!m_pages || !m_activePages) {
				return false;
			}

			for (int i = 0; i < m_numberOfPages * 2; ++i) {
				m_pages[i].init(m_pageSize);
			}

			for (int i = 0; i < m_numberOfPages; ++i) {
				m_activePages[i] = i;
			}

			return true;
		}

		void insert(int pageIdx, const Type& val)
		{
			sync::lock_guard lock{ m_pageLocks[pageIdx] };
			int targetPage = m_activePages[pageIdx];
			Table& t = m_pages[targetPage];
			t.insert(val);
			if (t.loadFactor() > 0.8) {

				// if we are here it means that table is full
				// and we can pass it to another thread which persists it somewhere
				// by simply placing table index in the queue and notifying cond_var
				// p.s. it isn't implemented here, as it is not required by task. Just notes to a reader.
				_changeActivePage(pageIdx);
				t.clear();
			}
		}

		bool has(const Type& val)
		{
			for (int i = 0; i < m_numberOfPages; ++i) {
				sync::lock_guard lock{ m_pageLocks[i] };
				int activeIdx = m_activePages[i];
				// here we only care about active pages
				// as inactive are passed somewhere else
				Table& t = m_pages[activeIdx];
				if (t.has(val)) {
					return true;
				}
			}
			return false;
		}

		Type* get(const Type& val)
		{
			for (int i = 0; i < m_numberOfPages; ++i) {
				sync::lock_guard lock{ m_pageLocks[i] };
				int activeIdx = m_activePages[i];
				// here we only care about active pages
				// as inactive are passed somewhere else
				Table& t = m_pages[activeIdx];
				if (t.has(val)) {
					return t.get(val);
				}
			}
			return nullptr;
		}

	private:
		inline void _changeActivePage(int pageIdx)
		{
			m_activePages[pageIdx] = (pageIdx + m_numberOfPages) & (m_numberOfPages * 2 - 1);
		}

	private:

		Table* m_pages;
		int* m_activePages;
		sync::spinlock* m_pageLocks;
		int m_pageSize;
		int m_numberOfPages;
	};
}