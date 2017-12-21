
#include <vector>

#include "PES.hpp"
#include "xlog.hpp"

#include "Misc.hpp"

void Misc::pesScan( TS::Stream* stream, Misc::PESCallback* cb, void* opaque ) {
	std::vector<uint8_t> pes;
	std::vector<uint32_t> pes_offset;

	for ( unsigned int i = 0; i < stream->numPackets(); i++ ) {
		TSPacket* p = stream->packet(i);

		if ( p->pusi() ) {
			if ( !pes.empty() ) {	
				PES* c = new PES( pes.data(), pes.size(), pes_offset.data() );
				cb->pesCallback( opaque, c );
				delete c;
			}
			pes.clear();
			pes_offset.clear();
		}

		size_t len;
		const uint8_t* r = p->getPayload( &len );
		pes.insert( pes.end(), r, r+len );

		unsigned int offset = ( i * 188 ) + p->headerLen();
		for ( unsigned int j = 0; j < len; j++ ) {
			pes_offset.push_back( offset++ );
		}
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

std::vector<uint8_t>* Misc::makeUnboundPESSegment( const uint8_t* ptr, size_t len, unsigned long long pts, unsigned long long dts, unsigned int m_stream_id ) {
	std::vector<uint8_t>* rc = new std::vector<uint8_t>();
	if ( rc != NULL ) {
		rc->push_back( 0x00 );
		rc->push_back( 0x00 );
		rc->push_back( 0x01 );
		rc->push_back( m_stream_id );
		rc->push_back( 0x00 ); rc->push_back( 0x00 );		/* size = 0 */
		rc->push_back( 0xC0 | 0x00 | 0x00 | 0x04 | 0x00 | 0x00 );	/* mpeg2, non-scrambled, non-priorty, aligned, nocopyright, original */
		rc->push_back( 0xC0 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 );	/* PTS+DTS, no-escr, no-esrate, no-dsm, no-additional, no-crc, no-extension */
		rc->push_back( 0x0A );								/* header-size */
		rc->push_back( 0x30 | ( ( ( pts >> 30 ) & 0x07 ) << 1 ) | 0x01 );
		rc->push_back( ( pts >> 23 ) & 255 );
		rc->push_back( ( ( pts >> 15 ) & 0x7F ) << 1 );
		rc->push_back( ( pts >> 7 ) & 255 );
		rc->push_back( ( ( pts & 0x7F ) << 1 ) | 0x01 );
		rc->push_back( 0x01 | ( ( ( dts >> 30 ) & 0x07 ) << 1 ) | 0x01 );
		rc->push_back( ( dts >> 23 ) & 255 );
		rc->push_back( ( ( dts >> 15 ) & 0x7F ) << 1 );
		rc->push_back( ( dts >> 7 ) & 255 );
		rc->push_back( ( ( dts & 0x7F ) << 1 ) | 0x01 );
		rc->insert( rc->end(), ptr, ptr+len );
	}
	return rc;
}

