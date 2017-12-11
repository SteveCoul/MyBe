
#ifndef __PES_hpp__
#define __PES_hpp__

#include <stdint.h>

class PES { 
public:
	PES( const uint8_t* ptr, size_t len, const uint32_t* map = NULL );
	~PES();
	void dump();
	bool checkStartCode() { return ( ( m_len > 3 ) && ( m_ptr[0] == 0x00 ) && ( m_ptr[1] == 0x00 ) && ( m_ptr[2] == 0x01 ) ); }
	unsigned int streamId() { return m_streamId; }
	unsigned int packetLength() { return m_length; };
	unsigned int scramblingControl() { return m_scrambling_control; }
	bool priority() { return m_priority; }
	bool dataAligned() { return m_data_alignment; }
	bool copyright(){ return m_copyright; }
	bool original() { return m_original; }
	bool hasPTS() { return m_hasPTS; }
	bool hasDTS() { return m_hasDTS; }
	const uint8_t* payload( size_t* p_size = NULL ) { if ( p_size ) p_size[0] = m_payload_length; return m_payload; }
	unsigned long long PTS() { return m_PTS; }
	unsigned long long DTS() { return m_DTS; }
	unsigned long long map( unsigned long offset ) { return m_map[offset] + m_payload_offset; }
private:
	const uint8_t*	m_ptr;
	size_t			m_len;
	const uint32_t*	m_map;
	unsigned int	m_streamId;
	unsigned int	m_length;
	unsigned int	m_scrambling_control;
	bool			m_priority;
	bool			m_data_alignment;
	bool			m_copyright;	
	bool			m_original;
	bool			m_hasPTS;
	bool			m_hasDTS;
	const uint8_t*	m_payload;
	size_t			m_payload_length;
	unsigned long long	m_PTS;
	unsigned long long	m_DTS;
	unsigned int m_payload_offset;
};

#endif

