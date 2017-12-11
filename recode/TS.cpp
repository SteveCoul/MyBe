
#include "TS.hpp"

#include "xlog.hpp"

void TS::add( TSPacket* pkt ) {
	m_packets.push_back( pkt );
	m_packets_by_pid[ pkt->pid() ].push_back( pkt );
}

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

unsigned int TS::getUnusedPID( int lowest ) {
	unsigned int rc = 0;
	for ( unsigned i = lowest; i < 8192; i++ ) 
		if ( m_packets_by_pid[i].size() == 0 ) {
			rc = i;
			break;
		}
	return rc;
}

size_t TS::sizePIDStream( unsigned pid ) {
	return m_packets_by_pid[pid].size() * 188;
}

int TS::writePIDStream( int fd, unsigned pid, int skip, int count ) {
	int rc = 0;
	if ( m_packets_by_pid[pid].size() ) {
		int toskip = skip > 0 ? skip : 0;
		int todo = count > 0 ? count : m_packets_by_pid[pid].size() - toskip;
		for ( std::vector<TSPacket*>::const_iterator it = m_packets_by_pid[pid].begin(); it != m_packets_by_pid[pid].end(); ++it ) {
			if ( toskip ) toskip--;
			else if ( todo ) {
				(*it)->write( fd );
				rc += 188;
				todo--;
			}
		}
	}
	return rc;
}
