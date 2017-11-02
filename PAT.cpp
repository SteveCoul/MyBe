
#include "PAT.hpp"

#include "xlog.hpp"

PAT::PAT( const uint8_t* data, size_t len ) 
	: m_data( data )
	, m_len( len )
	, m_valid( false ) {

	const uint8_t*p = data;
	
	/* skip pointer field */
	len = len - ( p[0] + 1 );
	p = p + p[0] + 1;

	if ( p[0] != 0x00 ) {
		XLOG_WARNING( "Invalid PAT, non 0 tableID" );
	} else if ( ( p[1] & 0x80 ) == 0 ) {
		XLOG_WARNING( "Invalid PAT, SSI not set" );
	} else if ( ( p[1] & 0x40 ) != 0 ) {
		XLOG_WARNING( "Invalid PAT, private bit set" );
	} else if ( ( p[1] & 0x30 ) != 0x30 ) {
		XLOG_WARNING( "Invalid PAT, reserved bits not set" );
	} else if ( ( p[1] & 0x0C ) != 0 ) {
		XLOG_WARNING( "Invalid PAT, reserved section length bits set" );
	} else {
		len = ( ( p[1] & 0x3 ) << 8 ) | p[2];
		p = p + 3;	
		len-=4;	/* ignore CRC */

		m_tsid = ( p[0] << 8 ) | p[1];
		/* ignore reserved bits, version, current/next */
		if ( ( p[3] != 0 ) || ( p[4] != 0 ) ) {
			XLOG_WARNING( "Invalid PAT, parser can only handle single sections and we got %u out of %u", p[3], p[4] );
		} else {
			p+=5;
			len-=5;

			// FIXME - most of the above is generic section header stuff 

			while ( len >= 4 ) {
				unsigned int program_id = ( p[2] & 0x1F ) << 8 | p[3];
				p+=4;
				len-=4;
				m_pmt_pids.push_back( program_id );
			}

			if ( len != 0 ) {
				XLOG_WARNING( "Ignore extraneous data bytes in PAT" );
			}

			XLOG_INFO( "PAT contained %u programs", m_pmt_pids.size() );
			m_valid = true;
		}
	}
}

bool PAT::isvalid() {
	return m_valid;
}

void PAT::dump() {
	XLOG_HEXDUMP_INFO( m_data, m_len );
}

unsigned int PAT::numPrograms() {
	if ( !m_valid ) return 0;
	return m_pmt_pids.size();
}

