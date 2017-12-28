#ifndef __PMT_hpp__
#define __PMT_hpp__

#include <stddef.h>
#include <stdint.h>

#include <vector>

/// Decodes and represents a mpeg PMT.
class PMT {
public:
	/// Information on an elementary stream within the PMT
	class ElementaryStream {
	/// PMT contains list of these streams.
	friend class PMT;
	public:
	private:
		/// Create stream descriptor object.
		/// \param[in]	type	Stream type.
		/// \param[in]	pid		Stream PID.
		ElementaryStream( int type, int pid ) : m_type(type), m_pid( pid ) { }
	private:
		int m_type;	///< Stream type
		int m_pid;	///< Stream pid
	};
public:
	/// Construct PMT object from given data.
	/// \param[in]	data	Raw data.
	/// \param[in]	len		Length of raw data.
	PMT( const uint8_t* data, size_t len );

	/// Destructor
	~PMT();

	/// Check to see if the PMT data decoded.
	/// \return true if valid.
	bool isvalid();

	/// Dump data to console
	void dump();

	/// Count how many streams are available for a given type.
	/// \param[in]	type
	/// \return count.
	unsigned int streamsOfType( int type );

	/// Get the PID of the first stream for a given type.
	/// \param[in]	type	Stream type to look for.
	/// \return PID.
	unsigned int getPidFirstOfType( int type );
private:
	const uint8_t*  m_data;						///< Original data.
	size_t			m_len;						///< Length of original data.
	bool			m_valid;					///< Parse validity.
	std::vector<ElementaryStream*>m_streams;	///< List of decoded streams.
};

#endif

