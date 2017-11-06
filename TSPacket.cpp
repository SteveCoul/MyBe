
#include "TSPacket.hpp"

#include <string.h>
#include <unistd.h>

#include "xlog.hpp"

TSPacket::TSPacket( const void* data, bool copy ) {
	m_copied = false;

	if ( copy ) {
		m_copy_data = new unsigned char[ 188 ];
		if ( m_copy_data == NULL ) {
			XLOG_ERROR( "out of memory" );	
		} else {
			memcpy( m_copy_data, data, 188 );
			m_copied = true;
			m_raw_data = (const uint8_t*)m_copy_data;
		}
	} else {
		m_raw_data = (const uint8_t*)data;
	}
}

TSPacket::~TSPacket() {
	if ( m_copied )
		if ( m_copy_data )
			delete[] m_copy_data;
}

bool TSPacket::isvalid() {
	return m_raw_data[0] == 0x47;
}

unsigned int TSPacket::pid() {
	unsigned int rc = ( ( m_raw_data[1] & 0x1F ) << 8 ) | m_raw_data[2];
	return rc;
}

bool TSPacket::pusi() { return ( ( m_raw_data[1] & 0x40 ) == 0x40 ); }
bool TSPacket::hasPayload() { return ( ( m_raw_data[3] & 0x10 ) == 0x10 ); }
bool TSPacket::hasAdaptation() { return ( ( m_raw_data[3] & 0x20 ) == 0x20 ); }

const uint8_t* TSPacket::getPayload( size_t* p_size ) {
	const uint8_t* p;
	size_t l;

	if ( hasPayload() == false ) {
		p = NULL;
		l = 0;
	} else {
		p = m_raw_data + 4;
		l = 184;

		if ( hasAdaptation() ) {
			uint8_t field_len = p[0] + 1;
			p+=field_len;
			l-=field_len;
		}
	}

	if ( p_size ) p_size[0] = l;
	return p;
}

int TSPacket::write( int fd ) {
	return ::write( fd, m_raw_data, 188 );
}

const uint8_t* TSPacket::ptr() {
	return m_raw_data;
}

void TSPacket::changePID(unsigned int new_pid) {
	if ( m_copied ) {
		m_copy_data[1] = ( m_copy_data[1] & 0xE0 ) | ( ( new_pid >> 8 ) & 0x1F );
		m_copy_data[2] = new_pid & 0xFF;
	} else {
		XLOG_ERROR("Ignoring attempt to change read only data" );
	}
}

