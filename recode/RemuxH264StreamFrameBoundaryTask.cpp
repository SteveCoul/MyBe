
#include "xlog.hpp"

#include "RemuxH264StreamFrameBoundaryTask.hpp"

RemuxH264StreamFrameBoundaryTask::RemuxH264StreamFrameBoundaryTask( std::vector<TSPacket*>* ret, TS::Stream* source ) 
	: m_output( ret )
	, m_source( source )
	, m_pts( 0xFFFFFFFFFFFFFFFFUL )
	{
}

RemuxH264StreamFrameBoundaryTask::~RemuxH264StreamFrameBoundaryTask() {
}

void RemuxH264StreamFrameBoundaryTask::deliver( const uint8_t* ptr, size_t len, unsigned long long pts ) {
	XLOG_INFO("PTS %llu", pts );
	XLOG_HEXDUMP_INFO( ptr, len );
}

void RemuxH264StreamFrameBoundaryTask::deliverComplete() {
	const uint8_t* ptr = (const uint8_t*)m_buffer.data();
	size_t len = m_buffer.size();

	if ( len < 6 ) return;

	int offset = findHeader( ptr+3, len-3 );
	if ( offset < 0 ) return;

	deliver( ptr, offset, m_pts );

	m_buffer.erase( m_buffer.begin(), m_buffer.begin()+(offset-1) );
	deliverComplete();
}

void RemuxH264StreamFrameBoundaryTask::deliverAll() {
	const uint8_t* ptr = (const uint8_t*)m_buffer.data();
	size_t len = m_buffer.size();
	if ( ptr )
		deliver( ptr, len, m_pts );
}

bool RemuxH264StreamFrameBoundaryTask::checkHeader( const uint8_t* ptr, int len ) {
	if ( ( len >= 4 ) && ( ptr[0] == 0x00 ) && ( ptr[1] == 0x00 ) && ( ptr[2] == 0x00 ) && ( ptr[3] == 0x01 ) ) return true;
	if ( ( len >= 3 ) && ( ptr[0] == 0x00 ) && ( ptr[1] == 0x00 ) && ( ptr[2] == 0x01 ) ) return true;
	return false;
}

int RemuxH264StreamFrameBoundaryTask::findHeader( const uint8_t* ptr, int len ) {
	int offset = 0;
	while ( len > 0 ) {
		if ( checkHeader( ptr, len ) ) return offset;
		// FIXME optimize by jumping to next 0x00
		offset++;
		ptr++;	
		len--;
	}
	return -1;
}
	
void RemuxH264StreamFrameBoundaryTask::pesCallback( void* opaque, PES* pes ) {
	const uint8_t*		ptr;
	size_t				len;

	deliverComplete();
	
	if ( pes == NULL ) {
		deliverAll();
		return;
	}

	ptr = pes->payload( &len );
	if ( ptr == NULL ) return;

	if ( checkHeader( ptr, len ) ) 
		deliverAll();

	if ( len < 4 ) {
		m_buffer.insert( m_buffer.end(), ptr, ptr+len );
		return;
	}

	int offset = findHeader( ptr+3, len-3 );
	if ( offset > 0 ) {
		m_buffer.insert( m_buffer.end(), ptr+3, ptr+3+offset );
		deliverComplete();
		deliverAll();
		m_buffer.insert( m_buffer.end(), ptr+offset, ptr+len );
		if ( pes->hasPTS() )
			m_pts = pes->PTS();
		deliverComplete();
	}
}

int RemuxH264StreamFrameBoundaryTask::run( std::vector<TSPacket*>* ret, TS::Stream* source ) {
	RemuxH264StreamFrameBoundaryTask t( ret, source );
	Misc::pesScan( source, &t );
	t.pesCallback( NULL, NULL );
	return 0;
}


#if 0




	for ( unsigned i = 0; i < source->numPackets(); i++ )
		ret->push_back( source->packet(i) );
#endif

