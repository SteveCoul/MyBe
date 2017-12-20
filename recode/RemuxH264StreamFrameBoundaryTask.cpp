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

	if ( dump_file )
		m_fd = open( dump_file, O_CREAT | O_WRONLY | O_TRUNC, 0666 );
}

RemuxH264StreamFrameBoundaryTask::~RemuxH264StreamFrameBoundaryTask() {
	if ( m_fd >= 0 )
		(void)close( m_fd );
}

void RemuxH264StreamFrameBoundaryTask::deliver( const uint8_t* ptr, size_t len ) {
//	XLOG_INFO("PTS %llu DTS %llu", m_pts, m_dts );
//	XLOG_HEXDUMP_INFO( ptr, len );
//	if ( m_fd >= 0 )
//			(void)write( m_fd, ptr, len );


	if (1) {
		uint8_t output[ len + 21 ];
		output[0] = 0x00;
		output[1] = 0x00;
		output[2] = 0x01;
		output[3] = m_stream_id;
		if ( len > 65535 ) {
			output[4] = 0;
			output[5] = 0;
		} else {
			output[4] = ( ( len >> 8 ) & 255 );
			output[5] = len & 255;
		}
		output[6] = 0xC0 | 0x04;
		output[7] = 0x80 | 0x40 | 0x02;
		output[8] = 10;
		// PTS
		output[9] = 0xFF;
		output[10] = 0xFF;
		output[11] = 0xFF;
		output[12] = 0xFF;
		output[13] = 0xFF;
		// DTS
		output[14] = 0xFF;
		output[15] = 0xFF;
		output[16] = 0xFF;
		output[17] = 0xFF;
		output[18] = 0xFF;
		
		memmove( output+19, ptr, len );

		output[ len+19 ] = 0xFF;
		output[ len+20 ] = 0xFF;
		if ( m_fd >= 0 )
			(void)write( m_fd, output, len+21 );
	}
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
	if ( ptr == NULL ) return;

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

