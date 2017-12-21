#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "xlog.hpp"

#include "RemuxH264StreamFrameBoundaryTask.hpp"

RemuxH264StreamFrameBoundaryTask::RemuxH264StreamFrameBoundaryTask( std::vector<TSPacket*>* ret, TS::Stream* source, const char* dump_file ) 
	: m_output( ret )
	, m_source( source )
	, m_pts( 0xFFFFFFFFFFFFFFFFUL )
	, m_dts( 0xFFFFFFFFFFFFFFFFUL )
	, m_fd(-1)
	, m_stream_id(0)
	{

	if ( dump_file ) {
		char t[ 1024 ];

		(void)snprintf( t, sizeof(t), "%s.es.h264", dump_file );
		m_fd = open( t, O_CREAT | O_WRONLY | O_TRUNC, 0666 );
	}
}

RemuxH264StreamFrameBoundaryTask::~RemuxH264StreamFrameBoundaryTask() {
	if ( m_fd >= 0 ) (void)close( m_fd );
}

void RemuxH264StreamFrameBoundaryTask::deliver( const uint8_t* ptr, size_t len ) {
//	XLOG_INFO("PTS %llu DTS %llu", m_pts, m_dts );
//	XLOG_HEXDUMP_INFO( ptr, len );
	if ( m_fd >= 0 )
		(void)write( m_fd, ptr, len );
}

bool RemuxH264StreamFrameBoundaryTask::deliverComplete() {
	const uint8_t* ptr = (const uint8_t*)m_buffer.data();
	size_t len = m_buffer.size();
	bool rc = false;

	if ( len >= 6 ) {
		for ( unsigned int i = 3; i < len; i++ ) {
			if ( checkHeader( ptr+i, len-i ) ) {
				deliver( ptr, i );
				rc = true;
				m_buffer.erase( m_buffer.begin(), m_buffer.begin() + i );
				break;
			}
		}
	}

	return rc;	
}

void RemuxH264StreamFrameBoundaryTask::deliverAll() {
	const uint8_t* ptr = (const uint8_t*)m_buffer.data();
	size_t len = m_buffer.size();
	if ( ptr )
		deliver( ptr, len );
	m_buffer.erase( m_buffer.begin(), m_buffer.end() );
}

bool RemuxH264StreamFrameBoundaryTask::checkHeader( const uint8_t* ptr, int len ) {
	if ( ( len >= 4 ) && ( ptr[0] == 0x00 ) && ( ptr[1] == 0x00 ) && ( ptr[2] == 0x00 ) && ( ptr[3] == 0x01 ) ) return true;
	if ( ( len >= 3 ) && ( ptr[0] == 0x00 ) && ( ptr[1] == 0x00 ) && ( ptr[2] == 0x01 ) ) return true;
	return false;
}

void RemuxH264StreamFrameBoundaryTask::pesCallback( void* opaque, PES* pes ) {
	const uint8_t*		ptr;
	size_t				len;
	unsigned long long	pts;
	unsigned long long	dts;

	while ( deliverComplete() ) ;
	
	if ( pes == NULL ) {
		deliverAll();
		return;
	}

	m_stream_id = pes->streamId();

	if ( pes->hasPTS() )
		pts = pes->PTS();
	else
		pts = m_pts;

	if ( pes->hasDTS() )
		dts = pes->DTS();
	else
		dts = 0xFFFFFFFFFFFFFFFFUL;

	ptr = pes->payload( &len );
	if ( ptr == NULL ) {
		XLOG_WARNING("Unexpected empty payload");
		return;
	}

	if ( checkHeader( ptr, len ) ) {
		deliverAll();
		m_pts = pts;
		m_dts = dts;
		m_buffer.insert( m_buffer.end(), ptr, ptr+len );
		while ( deliverComplete() );
	} else {
		m_buffer.insert( m_buffer.end(), ptr, ptr+len );	
		if ( deliverComplete() ) {
			m_pts = pts;
			m_dts = dts;
		}
	}
}

int RemuxH264StreamFrameBoundaryTask::run( std::vector<TSPacket*>* ret, TS::Stream* source, const char* dump_file ) {
	RemuxH264StreamFrameBoundaryTask t( ret, source, dump_file );
	Misc::pesScan( source, &t );
	t.pesCallback( NULL, NULL );
	return 0;
}


#if 0




	for ( unsigned i = 0; i < source->numPackets(); i++ )
		ret->push_back( source->packet(i) );
#endif

