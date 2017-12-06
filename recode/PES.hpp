
#ifndef __PES_hpp__
#define __PES_hpp__

#include <stdint.h>

class PES { 
public:
	PES( const uint8_t* ptr, size_t len );
	~PES();
	void dump();
private:
	const uint8_t*	m_ptr;
	size_t			m_len;
};

#endif

