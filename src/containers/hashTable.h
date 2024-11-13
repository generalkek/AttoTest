#pragma once

#include <cstring> // memset
#include <type_traits>

namespace cont {

	template <
		typename Type, 
		typename KeyType,
		typename Hasher,
		typename KeyFunc,
		typename Equality
	>
	class HashTable
	{
	public:
		using value_type = Type;
		using pointer = Type*;
		using reference = Type&;
		using const_reference = const Type&;
		using hash = Hasher;
		using key = KeyFunc;
		using equal = Equality;

		static const unsigned int s_null = 0xffffffff;
		static const unsigned int s_tombstone = 0xfffffffe;

	public:
		HashTable()
			:
			m_hasher{}, 
			m_table {nullptr},
			m_size{ 0u }, m_maxSize{ 1024u }
		{}

		~HashTable() {
			if (m_table) {
				delete[] m_table;
			}
		}

		bool init(int tableSize)
		{
			m_maxSize = tableSize;
			m_table = new Type[m_maxSize];
			if (!m_table) {
				return false;
			}
			memset(m_table, s_null, tableSize * sizeof(value_type));
			return true;
		}

		void clear()
		{
			m_size = 0;
			if (m_table) {
				memset(m_table, s_null, m_maxSize * sizeof(value_type));
			}
		}

		void insert(const_reference val)
		{
			pointer target = _getFreeOrMe(val);
			if (_isNull(target) || _isDeleted(target)) {
				m_size++;
			}
			*target = val;
		}


		bool has(KeyType val)
		{
			return _find(val) != nullptr;
		}

		bool erase(const_reference val)
		{
			if (!has(m_key(val))) {
				return false;
			}

			pointer p = _find(m_key(val));
			_markDeleted(p);
			m_size--;
			return true;
		}

		pointer get(KeyType val)
		{
			return _find(val);
		}

		float loadFactor()
		{
			return static_cast<float>(m_size) / static_cast<float>(m_maxSize);
		}

	private:

		inline void _markDeleted(pointer p)
		{
			*reinterpret_cast<unsigned int*>(p) = s_tombstone;
		}

		inline bool _isNull(pointer p)
		{
			return *reinterpret_cast<unsigned int*>(p) == s_null;
		}

		inline bool _isDeleted(pointer p)
		{
			return *reinterpret_cast<unsigned int*>(p) == s_tombstone;
		}

		pointer _getFreeOrMe(const_reference val)
		{
			auto idx = index(m_key(val));
			pointer target = m_table + idx;
			if (_isNull(target) || _isDeleted(target)) {
				return target;
			}

			for (int i = 1; i < m_maxSize && !_isNull(target); ++i) {
				if (_isDeleted(target)) {
					return target;
				}

				//if (target->MessageId == val.MessageId) {
				if (m_equal(m_key(*target), m_key(val))) {
					return target;
				}
			
				idx = index(idx + i);
				target = m_table + idx;
			}

			return target;
		}

		pointer _find(const KeyType& val)
		{
			auto idx = index(val);
			pointer target = m_table + idx;

			if (_isNull(target)) {
				return nullptr;
			}

			if (!_isDeleted(target) && m_equal(val, m_key(*target))) {
				return target;
			}

			for (int i = 1; i < m_maxSize; ++i) {
				auto idx_ = index(idx + i);
				target = m_table + idx_;

				if (_isNull(target)) {
					return nullptr;
				}

				if (_isDeleted(target)) {
					continue;
				}

				if (m_equal(val, m_key(*target))) {
					return target;
				}
			}

			return nullptr;
		}

		std::size_t index(const std::size_t  val)
		{
			return m_hasher(val) & m_maxSize - 1;
		}

	private:
		hash m_hasher;
		key m_key;
		equal m_equal;
		pointer m_table;
		std::size_t m_size;
		std::size_t m_maxSize;
	};
}