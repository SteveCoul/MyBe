
#ifndef __PES_hpp__
#define __PES_hpp__

#include <stdint.h>

/// Represents MPEG PES section.
class PES { 
public:
	/// Construct PES object from raw data and map.
	/// \param[in]	ptr		Source data pointer.
	/// \param[in]	len		Length of source data.
	/// \param[in]	map		1:1 map of byte positions in raw data  mapped to original position in file.
	PES( const uint8_t* ptr, size_t len, const uint32_t* map = NULL );

	~PES();

	/// Dump PES packet to console.
	void dump();

	/// Check the PES header start code prefix.
	/// \return true if valid.
	bool checkStartCode() { return ( ( m_len > 3 ) && ( m_ptr[0] == 0x00 ) && ( m_ptr[1] == 0x00 ) && ( m_ptr[2] == 0x01 ) ); }

	/// Return data from PES header.
	/// \return value.
	unsigned int streamId() { return m_streamId; }

	/// Return data from PES header.
	/// \return value.
	unsigned int packetLength() { return m_length; };

	/// Return data from PES header.
	/// \return value.
	unsigned int scramblingControl() { return m_scrambling_control; }

	/// Return data from PES header.
	/// \return value.
	bool priority() { return m_priority; }

	/// Return data from PES header.
	/// \return value.
	bool dataAligned() { return m_data_alignment; }

	/// Return data from PES header.
	/// \return value.
	bool copyright(){ return m_copyright; }

	/// Return data from PES header.
	/// \return value.
	bool original() { return m_original; }

	/// Return data from PES header.
	/// \return value.
	bool hasPTS() { return m_hasPTS; }

	/// Return data from PES header.
	/// \return value.
	bool hasDTS() { return m_hasDTS; }

	/// Return payload.
	/// \param[out]	p_size		When non-null, where to return payload size if known.
	/// \return Pointer to any payload data.
	const uint8_t* payload( size_t* p_size = NULL ) { if ( p_size ) p_size[0] = m_payload_length; return m_payload; }

	/// Return data from PES header.
	/// \return value.
	unsigned long long PTS() { return m_PTS; }

	/// Return data from PES header.
	/// \return value.
	unsigned long long DTS() { return m_DTS; }

	/// Get original file position of a byte in the PES data.
	/// \param[in]	offset	Byte offset
	/// \return original file position.
	unsigned long long map( unsigned long offset ) { return m_map[offset] + m_payload_offset; }
private:
	const uint8_t*	m_ptr;					///< original data pointer.
	size_t			m_len;					///< Length of original data.
	const uint32_t*	m_map;					///< A 1:1 map for each byte in the original data and it's position in the original file.
	unsigned int	m_streamId;				///< Decoded information from PES header.
	unsigned int	m_length;				///< Decoded information from PES header.
	unsigned int	m_scrambling_control;	///< Decoded information from PES header.
	bool			m_priority;				///< Decoded information from PES header.
	bool			m_data_alignment;		///< Decoded information from PES header.
	bool			m_copyright;			///< Decoded information from PES header.
	bool			m_original;				///< Decoded information from PES header.
	bool			m_hasPTS;				///< Decoded information from PES header.
	bool			m_hasDTS;				///< Decoded information from PES header.
	const uint8_t*	m_payload;				///< Pointer to payload data.
	size_t			m_payload_length;		///< Length of payload data if known.
	unsigned long long	m_PTS;				///< Decoded information from PES header.
	unsigned long long	m_DTS;				///< Decoded information from PES header.
	unsigned int m_payload_offset;			///< Offset from start of header to payload data.
};

#endif

