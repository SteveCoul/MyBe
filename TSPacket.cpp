
#include "TSPacket.hpp"

#include <string.h>

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

