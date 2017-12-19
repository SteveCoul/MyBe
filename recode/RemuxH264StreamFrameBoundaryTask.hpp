#ifndef __RemuxH264StreamFrameBoundaryTask_hpp__
#define __RemuxH264StreamFrameBoundaryTask_hpp__

#include <vector>
#include "Misc.hpp"
#include "TS.hpp"

class RemuxH264StreamFrameBoundaryTask : public Misc::PESCallback {
public:
	static int run( std::vector<TSPacket*>* ret, TS::Stream* source );
private:
	virtual void pesCallback( void* opaque, PES* pes );
	RemuxH264StreamFrameBoundaryTask( std::vector<TSPacket*>* ret, TS::Stream* source );
	~RemuxH264StreamFrameBoundaryTask();
	/**
	 * Deliver all complete NALUs in buffer. Don't assume a NALU is complete if it runs off the end of the buffer.
	 */
	void deliverComplete();

	/* 
	 *Deliver the existing buffer as a NALA.
	 */
	void deliverAll();
	bool checkHeader( const uint8_t* ptr, int len );
	int findHeader( const uint8_t* ptr, int len );

	void deliver( const uint8_t* ptr, size_t len, unsigned long long pts );	
private:
	std::vector<TSPacket*>*		m_output;
	TS::Stream*					m_source;
	std::vector<uint8_t>		m_buffer;
	unsigned long long			m_pts;
};

#endif

