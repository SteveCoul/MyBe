
#include "TS.hpp"

#include "xlog.hpp"

TS::TS( const void* data, size_t length ) {
	while ( length >= 188 ) {
		TSPacket* pkt = new TSPacket( data, 188 );
		if ( pkt->isvalid()	== false ) {
			XLOG_WARNING( "Invalid TSPacket/Frame, stop processing" );
			break;
		}

		m_packets.push_back( pkt );
		m_packets_by_pid[ pkt->pid() ].push_back( pkt );

		data = (const void*)( ((unsigned char*)data) + 188 );
		length -= 188;
	}
	
	if ( length != 0 ) 
		XLOG_WARNING( "Excess of data at end of stream" );

	for ( int i = 0; i < 8192; i++ ) m_stream[i] = new Stream( &m_packets_by_pid[ i ] );
}

TS::~TS() {
	for ( std::vector<TSPacket*>::const_iterator it = m_packets.begin(); it != m_packets.end(); ++it )
		delete *it;
	for ( int i = 0; i < 8192; i++ ) delete m_stream[i];
}

TS::Stream* TS::stream( int pid ) { 
	if ( ( pid < 0 ) || ( pid > 8191 ) ) return NULL;
	return m_stream[ pid ];
}

unsigned int TS::Stream::numPackets() {
	return m_packets->size();
}

TSPacket* TS::Stream::packet( unsigned int offset ) {
	if ( offset >= m_packets->size() ) return NULL;
	return m_packets->at( offset );
}
