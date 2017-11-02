#ifndef __PMT_hpp__
#define __PMT_hpp__

#include <stddef.h>
#include <stdint.h>

#include <vector>

class PMT {
public:
	class ElementaryStream {
	friend class PMT;
	public:
	private:
		ElementaryStream( int type, int pid ) : m_type(type), m_pid( pid ) { }
	private:
		int m_type;
		int m_pid;
	};
public:
	PMT( const uint8_t* data, size_t len );
	~PMT();
	bool isvalid();
	void dump();
	unsigned int streamsOfType( int type );
	unsigned int getPidFirstOfType( int type );
private:
	const uint8_t*  m_data;
	size_t			m_len;
	bool			m_valid;
	std::vector<ElementaryStream*>m_streams;
};

#endif

