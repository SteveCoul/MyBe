#ifndef __RemuxH264StreamFrameBoundaryTask_hpp__
#define __RemuxH264StreamFrameBoundaryTask_hpp__

#include <vector>
#include "Misc.hpp"
#include "TS.hpp"

class RemuxH264StreamFrameBoundaryTask : public Misc::PESCallback {
public:
	static int run( std::vector<TSPacket*>* ret, TS::Stream* source, const char* dump_file = NULL );
private:
	virtual void pesCallback( void* opaque, PES* pes );
	RemuxH264StreamFrameBoundaryTask( std::vector<TSPacket*>* ret, TS::Stream* source, const char* dump_file );
	~RemuxH264StreamFrameBoundaryTask();
	bool deliverComplete();
	void deliverAll();
	bool checkHeader( const uint8_t* ptr, int len );
	void deliver( const uint8_t* ptr, size_t len );
private:
	std::vector<TSPacket*>*		m_output;
	TS::Stream*					m_source;
	std::vector<uint8_t>		m_buffer;
	unsigned long long			m_pts;
	unsigned long long			m_dts;
	int							m_fd;
	unsigned int				m_stream_id;
};

#endif

