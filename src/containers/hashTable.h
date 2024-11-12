#pragma once

#include <cstring> // memset
#include <type_traits>

namespace cont {

	template <
		typename Type, 
		typename Hasher,
		typename KeyFunc,
		typename Equality
		/*std::enable_if_t<std::conjunction_v<
			std::_Invoke_traits<Hasher>::_Is_invocable_r,
			std::is_same<std::_Invoke_traits<Hasher>::type, std::size_t(const Type&)>
		>, int> = 0*/
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
			pointer target = _get(val);
			if (_isNull(target) || _isDeleted(target)) {
				m_size++;
			}
			*target = val;
		}


		bool has(const_reference val)
		{
			pointer ptr = _get(val);
			return !_isNull(ptr) && !_isDeleted(ptr);
		}

		void erase(const_reference val)
		{
			if (!has(val)) {
				return;
			}

			pointer p = _get(val);
			_markDeleted(p);
			m_size--;
		}

		reference get(const_reference val)
		{
			pointer ptr = _get(val);
			if (_isNull(ptr) && !_isDeleted(ptr)) {
				*ptr = Type{val};
				m_size++;
			}
			return *ptr;
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

		pointer _get(const_reference val)
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
			
				idx = helperIndex(idx + i);
				target = m_table + idx;
			}

			return target;
		}

		std::size_t index(const std::size_t  val)
		{
			return m_hasher(val) & m_maxSize - 1;
		}

		std::size_t helperIndex(const std::size_t val)
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