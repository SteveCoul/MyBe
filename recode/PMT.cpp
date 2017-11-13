
#include "PMT.hpp"

#include "xlog.hpp"

PMT::~PMT() {
	for ( std::vector<ElementaryStream*>::const_iterator it = m_streams.begin(); it != m_streams.end(); ++it )
		delete *it;
}

PMT::PMT( const uint8_t* data, size_t len ) 
	: m_data( data )
	, m_len( len )
	, m_valid( false ) {

	const uint8_t*p = data;
	
	/* skip pointer field */
	len = len - ( p[0] + 1 );
	p = p + p[0] + 1;

	if ( p[0] != 0x02 ) {
		XLOG_WARNING( "Invalid PMT, tableID not 2" );
	} else if ( ( p[1] & 0x80 ) == 0 ) {
		XLOG_WARNING( "Invalid PMT, SSI not set" );
	} else if ( ( p[1] & 0x40 ) != 0 ) {
		XLOG_WARNING( "Invalid PMT, private bit set" );
	} else if ( ( p[1] & 0x30 ) != 0x30 ) {
		XLOG_WARNING( "Invalid PMT, reserved bits not set" );
	} else if ( ( p[1] & 0x0C ) != 0 ) {
		XLOG_WARNING( "Invalid PMT, reserved section length bits set" );
	} else {
		len = ( ( p[1] & 0x3 ) << 8 ) | p[2];
		p = p + 3;	
		len-=4;	/* ignore CRC */

		/* ignore table extension id */
		/* ignore reserved bits, version, current/next */
		if ( ( p[3] != 0 ) || ( p[4] != 0 ) ) {
			XLOG_WARNING( "Invalid PMT, parser can only handle single sections and we got %u out of %u", p[3], p[4] );
		} else {
			p+=5;
			len-=5;

			XLOG_INFO( "PMT as follows" );

			unsigned int PCR = ( ((p[0]&0x1F)<<8)|p[1] );
			XLOG_INFO("PCR %d", PCR );

			unsigned int program_descriptor_len = ( ( p[2] & 0x03 ) << 8 ) | p[3];
			XLOG_INFO("Program Descriptors:-");
			XLOG_HEXDUMP_INFO( p+4, program_descriptor_len );

			p = p + 4 + program_descriptor_len;
			len = len - ( 4 + program_descriptor_len );
			while ( len >= 5 ) {
				unsigned int type = p[0];
				unsigned int pid = ((p[1]&0x1F)<<8)|p[2];

				XLOG_INFO("Elementary stream, type %d, pid %d", type, pid );
				ElementaryStream* es = new ElementaryStream( type, pid );
				m_streams.push_back( es );

				unsigned int es_descriptor_len = ((p[3]&0x3)<<8)|p[4];
				XLOG_HEXDUMP_INFO( p+5, es_descriptor_len );

				p = p + 5 + es_descriptor_len;
				len = len - ( 5 + es_descriptor_len );				
			}		
	
			XLOG_HEXDUMP_INFO( p, len );

			m_valid = true;
		}
	}
}

bool PMT::isvalid() {
	return m_valid;
}

void PMT::dump() {
	XLOG_HEXDUMP_INFO( m_data, m_len );
}

unsigned int PMT::streamsOfType( int type ) {
	unsigned int rc = 0;
	if ( m_valid ) {
		for ( std::vector<ElementaryStream*>::const_iterator it = m_streams.begin(); it != m_streams.end(); ++it ) {
			ElementaryStream* es = *it;

			if ( es->m_type == type ) rc++;
		}
	}
	return rc;
}

unsigned int PMT::getPidFirstOfType( int type ) {
	unsigned int rc = 0x1FFF;
	if ( m_valid ) {
		for ( std::vector<ElementaryStream*>::const_iterator it = m_streams.begin(); it != m_streams.end(); ++it ) {
			ElementaryStream* es = *it;

			if ( es->m_type == type ) {
				rc = es->m_pid;
				break;
			}
		}
	}
	return rc;
}

