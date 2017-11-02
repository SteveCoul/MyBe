
#include "TSStream.hpp"

#include "xlog.hpp"

TSStream::TSStream( const void* data, size_t length ) {
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
}

TSStream::~TSStream() {
	for ( std::vector<TSPacket*>::const_iterator it = m_packets.begin(); it != m_packets.end(); ++it )
		delete *it;
}

