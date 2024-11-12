#pragma once

namespace cont {
	template<typename T>
	class Queue
	{
	public:
		Queue()
			:
			m_head(new node),
			m_tail(m_head),
			m_size{0}
		{}
		
		~Queue()
		{
			while (node * old_head = m_head)
			{
				m_head = old_head->next;
				delete old_head;
			}
		}

		Queue(const Queue& other) = delete;
		Queue& operator=(const Queue& other) = delete;

		bool empty() { return m_size == 0; }

		T pop()
		{
			node* const old_head = m_head;
			m_head = old_head->next;
			T res{ old_head->data };
			delete old_head;
			m_size--;
			return res;
		}

		void push(T val)
		{
			T new_data{ val };
			node* p = new node;
			node* const old_tail = m_tail;
			old_tail->data = new_data;
			old_tail->next = p;
			m_tail = p;
			m_size++;
		}

	private:
		struct node
		{
			T data; // store val, as we going to store only PODs
			node* next;
			node() :
				next(nullptr)
			{}
		};

		node* m_head;
		node* m_tail;
		int m_size;
	};
}