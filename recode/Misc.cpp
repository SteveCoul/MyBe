
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


