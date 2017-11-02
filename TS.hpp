#ifndef __TS_hpp__
#define __TS_hpp__

#include <stddef.h>

#include <vector>

#include "TSPacket.hpp"

class TS {
public:
	TS( const void* data, size_t length );
	~TS();
private:
	std::vector<TSPacket*>	m_packets;
	std::vector<TSPacket*>	m_packets_by_pid[ 8192 ];
};

#endif

