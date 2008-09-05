#pragma once

namespace sailing {

	template <class _type, int _count>
	class pool {
	public:
		pool() {
			m_seq = (unsigned int)this;
			atom_slist_init(&m_header);
			for(int i=0; i<_count; i++) {
				atom_slist_push(&m_header, &m_array_entry[i]);
			}
			memset(&m_array_seq, 0, sizeof(m_array_seq));
		}
		_type* pop() {
			unsigned int index;
			ATOM_SLIST_ENTRY* entry;

			entry = atom_slist_pop(&m_header);
			if(!entry) return NULL;

			index = entry - m_array_entry;
			m_array_seq[index] = ((index<<16) | ((m_seq++)&0xffff));

			return &m_array_objs[index];
		}
		void push(_type* obj) {
			unsigned int index;
			index = obj - m_array_objs;
			if(obj<m_array_objs || index>=_count)
				return;
			atom_slist_push(&m_header, &m_array_entry[index]);
		}
		_type* get(unsigned int seq) {
			unsigned int index;
			index = (seq>>16);
			if(index>=_count || m_array_seq[index]!=seq)
				return NULL;
			return &m_array_objs[index];
		}
		unsigned int seq(_type* obj) {
			unsigned int index;
			index = obj - m_array_objs;
			if(obj<m_array_objs || index>=_count)
				return 0;
			return m_array_seq[index];
		}
		
	private:
		_type				m_array_objs[_count];
		ATOM_SLIST_ENTRY	m_array_entry[_count];
		unsigned int		m_array_seq[_count];
		ATOM_SLIST_HEADER	m_header;
		unsigned int		m_seq;
	};

}
