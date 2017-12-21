
#ifndef __Misc_hpp__
#define __Misc_hpp__

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "PES.hpp"
#include "TS.hpp"

class Misc {
public:
	class PESCallback {
	public:
		virtual void pesCallback( void* opaque, PES* pes ) = 0;
	};
private:
	Misc() { }
public:
	static void pesScan( TS::Stream* stream, Misc::PESCallback* cb, void* opaque = NULL );
	static unsigned long long mpegTimestampFromBytes( const uint8_t* ptr );
	static std::vector<uint8_t>* makeUnboundPESSegment( const uint8_t* ptr, size_t len, unsigned long long pts, unsigned long long dts, unsigned int m_stream_id );
};

#endif

