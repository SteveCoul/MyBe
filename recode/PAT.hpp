#ifndef __PAT_hpp__
#define __PAT_hpp__

#include <stddef.h>
#include <stdint.h>

#include <vector>

/// Class decodes and represents an mpeg PAT.
class PAT {
public:
	/// Construct PAT object from the given raw data.
	/// Object retains the pointers provided.
	/// \param[in]	data	Raw Pointer.
	/// \param[in]	len		Length of data.
	PAT( const uint8_t* data, size_t len );

	/// Check to see if the PAT decoded correctly.
	/// \return true if PAT valid.
	bool isvalid();

	/// Dump PAT data.
	void dump();

	/// Count the number of programs discovered in the PAT.
	/// \return count.
	unsigned int numPrograms();

	/// Get the PMT of a program in the PAT.
	/// \param[in]	offset	Zero based offset into list of programs.
	/// \return PID.
	unsigned int pmtPID( unsigned int offset );
private:
	const uint8_t* m_data;					///< Original data pointer.
	size_t			m_len;					///< Data length.
	bool			m_valid;				///< Validity flag.
	unsigned int	m_tsid;					///< Transport ID (not used)
	std::vector<unsigned int>m_pmt_pids;	///< PMT pids of each program.
};

#endif

