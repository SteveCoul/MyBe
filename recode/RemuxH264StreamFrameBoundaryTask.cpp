#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "Misc.hpp"
#include "xlog.hpp"

#include "RemuxH264StreamFrameBoundaryTask.hpp"

RemuxH264StreamFrameBoundaryTask::RemuxH264StreamFrameBoundaryTask( std::string title, std::vector<TSPacket*>* ret, TS::Stream* source, const char* dump_file ) 
	: m_title( title )
	, m_output( ret )
	, m_source( source )
	, m_pts( 0xFFFFFFFFFFFFFFFFUL )
	, m_dts( 0xFFFFFFFFFFFFFFFFUL )
	, m_fd(-1)
	, m_stream_id(0)
	, m_counter(0)
	{

	if ( dump_file ) {
		m_fd = open( dump_file, O_CREAT | O_WRONLY | O_TRUNC, 0666 );
	}
}

RemuxH264StreamFrameBoundaryTask::~RemuxH264StreamFrameBoundaryTask() {
	if ( m_fd >= 0 ) (void)close( m_fd );
}

static
const uint8_t* makeTSPacket( uint8_t* output, unsigned pid, unsigned* p_cc, const uint8_t* source, size_t* plen ) {
	size_t len = plen[0];

	output[0] = 0x47;
	output[1] = ( pid >> 8 ) & 0x1F;
	output[2] = pid & 255;

	if ( len >= 184 ) {
		output[3] = 0x10;
	} else {
		output[3] = 0x30;
	}
	output[3] |= ( (p_cc[0] & 0x0F) );
	p_cc[0] = ( ( p_cc[0] + 1 ) & 0x0F );

	if ( len >= 184 ) {
		memmove( output+4, source, 184 );
		source+=184;
		len-=184;
	} else if ( len == 183 ) {
		output[4] = 0;
		memmove( output+5, source, len );
		len = 0;
	} else {
		int skip = 184 - len;
		skip--;
		output[4] = skip;
		output[5] = 0;
		skip--;
		if ( skip > 0 ) memset( output+6, 0xFF, skip );
		memmove( output+6+skip, source, len );
		len = 0;
	}

	plen[0] = len;

//	XLOG_HEXDUMP_INFO( output, 188 );
	return source;
}

void RemuxH264StreamFrameBoundaryTask::deliver( const uint8_t* ptr, size_t len ) {
//	XLOG_INFO("PTS %llu DTS %llu", m_pts, m_dts );
//	XLOG_HEXDUMP_INFO( ptr, len );

	XLOG_INFO("% 4d] NALU %s PTS %llu, type 0x%02X, size %u", m_counter++, m_title.c_str(), m_pts, ptr[2] == 0x01 ? ptr[3] : ptr[4], (unsigned)len );

	// FIXME 
	m_dts = 0xFFFFFFFFFFFFFFFF;

	std::vector<uint8_t>* pes = Misc::makeUnboundPESSegment( ptr, len, m_pts, m_dts, m_stream_id );
	if ( pes == NULL ) {
		XLOG_ERROR("Failed to make PES stream");
		return;
	}

	unsigned PID=m_source->pid();
	unsigned CC=0;
	uint8_t packet[188];

	ptr = pes->data();
	len = pes->size();
	
	ptr = makeTSPacket( packet, PID, &CC, ptr, &len );
	packet[1] |= 0x40;
	if ( m_fd >= 0 )
		(void)write( m_fd, packet, 188 );
	m_output->push_back( new TSPacket( packet, true ) );

	while ( len >= 184 ) {
		ptr = makeTSPacket( packet, PID, &CC, ptr, &len );
		if ( m_fd >= 0 )
			(void)write( m_fd, packet, 188 );
		m_output->push_back( new TSPacket( packet, true ) );

	}
	if ( len != 0 ) {
		ptr = makeTSPacket( packet, PID, &CC, ptr, &len );
		if ( m_fd >= 0 )
			(void)write( m_fd, packet, 188 );
		m_output->push_back( new TSPacket( packet, true ) );
	}

	delete pes;
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

//	XLOG_INFO("--------------------------");

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

int RemuxH264StreamFrameBoundaryTask::run( std::string title, std::vector<TSPacket*>* ret, TS::Stream* source, const char* dump_file ) {
	RemuxH264StreamFrameBoundaryTask t( title, ret, source, dump_file );
	Misc::pesScan( source, &t );
	t.pesCallback( NULL, NULL );
	return 0;
}

