#ifndef __TS_hpp__
#define __TS_hpp__

#include <stddef.h>

#include <vector>

#include "TSPacket.hpp"

class TS {
public:
	class Stream {
	friend class TS;
	public:
		unsigned int numPackets();
		TSPacket* packet( unsigned int offset );	/* returned packet is owned by TS */
	protected:
		Stream( std::vector<TSPacket*>* p ) { m_packets = p; }
	private:
		std::vector<TSPacket*>*	m_packets;
	};
public:
	TS( const void* data, size_t length );
	~TS();
	Stream* stream( int pid );
	unsigned int getUnusedPID( int lowest = 0);
	int writePIDStream( int fd, unsigned pid );
	void add( TSPacket* pkt );
private:
	std::vector<TSPacket*>	m_packets;
	std::vector<TSPacket*>	m_packets_by_pid[ 8192 ];
	Stream*					m_stream[ 8192 ];
};

#endif
