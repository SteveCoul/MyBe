
#include "xlog.hpp"

#include "PES.hpp"

PES::PES( const uint8_t* ptr, size_t len )
	: m_ptr( ptr )
	, m_len( len )
	{
}

PES::~PES() {
}

void PES::dump() {
	XLOG_HEXDUMP_INFO( m_ptr, m_len );
}

