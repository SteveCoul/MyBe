
#include "Misc.hpp"
#include "xlog.hpp"

#include "PES.hpp"

PES::PES( const uint8_t* ptr, size_t len, const uint32_t* map )
	: m_ptr( ptr )
	, m_len( len )
	, m_map( map )
	, m_streamId( 0 )
	, m_length( 0 )
	, m_scrambling_control( 0 )
	, m_priority( false )
	, m_data_alignment( false )
	, m_copyright( false )
	, m_original( false )
	, m_hasPTS( false )
	, m_hasDTS( false )
	, m_payload( NULL )
	, m_payload_length( 0 )
	, m_PTS( 0 )
	, m_DTS( 0 )
	, m_payload_offset( 0 )
	{

	// PES Header
	
	if ( len <= 6 ) {
		XLOG_WARNING("packet not long enough");
		return;
	}
	m_streamId = ptr[3];
	m_length = ((ptr[4] << 8 ) | ptr[5] );

	if ( m_streamId < 0xBD ) return;
	if ( m_streamId == 0xBE ) return;
	if ( m_streamId == 0xBF ) return;
	if ( m_streamId > 0xEF ) return;

	ptr += 6;
	len -= 6;

	// PES Extension

	if ( len < 3 ) {
		XLOG_WARNING("not enough extension data");
		return;
	}

	m_scrambling_control = (ptr[0]>>4)&3;
	m_priority = ( (ptr[0]&0x8) != 0 );
	m_data_alignment = ( (ptr[0]&0x04) != 0 );
	m_copyright = ( (ptr[0]&0x02) != 0 );
	m_original = ( (ptr[0]&0x01) != 0 );

	m_hasPTS = ( ( ptr[1] & 0x80 ) != 0 );
	m_hasDTS = ( ( ptr[1] & 0x40 ) != 0 );

//	bool has_escr = ( ( ptr[1] & 0x20 ) != 0 );
//	bool has_rate = ( ( ptr[1] & 0x10 ) != 0 );
//	bool dsm_trick_mode = ( ( ptr[1] & 0x08 ) != 0 );
//	bool has_additional = ( ( ptr[1] & 0x04 ) != 0 );
//	bool pes_crc_flag = ( ( ptr[1] & 0x02 ) != 0 );
//	bool pes_extension_flag = ( ( ptr[1] & 0x01 ) != 0 );

	unsigned int pes_header_length = ptr[2];

	ptr += 3;
	len -= 3;

	if ( len < pes_header_length ) {
		XLOG_WARNING("not enough header data" );
		return;
	}

	if ( m_hasPTS ) {
		m_PTS = Misc::mpegTimestampFromBytes( ptr );
		if ( m_hasDTS ) {
			m_DTS = Misc::mpegTimestampFromBytes( ptr+5 );
		}
	} else if ( m_hasDTS ) {
		m_DTS = Misc::mpegTimestampFromBytes( ptr );
	}

	ptr += pes_header_length;
	len -= pes_header_length;

	m_payload_offset = ptr - m_ptr;

	m_payload = ptr;
	m_payload_length = ( m_length == 0 ) ? len : m_length;
}

PES::~PES() {
}

void PES::dump() {
	XLOG_HEXDUMP_INFO( m_ptr, m_len );
}

