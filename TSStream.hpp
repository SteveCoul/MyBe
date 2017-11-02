#ifndef __TSStream_hpp__
#define __TSStream_hpp__

#include <stddef.h>

#include <vector>

#include "TSPacket.hpp"

class TSStream {
public:
	TSStream( const void* data, size_t length );
	~TSStream();
private:
	std::vector<TSPacket*>	m_packets;
	std::vector<TSPacket*>	m_packets_by_pid[ 8192 ];
};

#endif

