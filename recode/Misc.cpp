
#include <vector>

#include "PES.hpp"
#include "xlog.hpp"

#include "Misc.hpp"

void Misc::pesScan( TS::Stream* stream, Misc::PESCallback* cb ) {
	std::vector<uint8_t> pes;
	for ( unsigned int i = 0; i < stream->numPackets(); i++ ) {
		TSPacket* p = stream->packet(i);

		if ( p->pusi() ) {
			if ( !pes.empty() ) {	
				PES* c = new PES( pes.data(), pes.size() );
				cb->pesCallback( c );
				delete c;
			}
			pes.clear();
		}

		size_t len;
		const uint8_t* r = p->getPayload( &len );
		pes.insert( pes.end(), r, r+len );
	}
};

unsigned long long Misc::mpegTimestampFromBytes( const uint8_t* ptr ) {
	unsigned long long b32_30 = ( ptr[0] >> 1 ) & 7;
	unsigned long long b29_23 = ( ptr[1] );
	unsigned long long b22_15 = ( ptr[2] >> 1 ) & 127;
	unsigned long long b14_8 = ( ptr[3] );
	unsigned long long b7_0 = ( ptr[4] >> 1 ) & 127;

	b32_30 <<= 30;
	b29_23 <<= 23;
	b22_15 <<= 15;
	b14_8 <<= 8;

	return b32_30 | b29_23 | b22_15 | b14_8 | b7_0;

}

