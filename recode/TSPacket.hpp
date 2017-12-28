#ifndef __TSPacket_hpp__
#define __TSPacket_hpp__

#include <stddef.h>
#include <stdint.h>

/// Represents an mpeg transport stream packet.
class TSPacket {
public:
	/// Create a TSPacket with the given payload.
	/// Payload data is copied.
	/// \param[in]	pid			PID for packet.
	/// \param[in]	cc			Continuity counter value.
	/// \param[in]	payload		Raw data for payload.
	/// \param[in]	payload_size  Size of raw data to copy.
	/// \return new packet object.
	static TSPacket* create( unsigned int pid, unsigned int cc, const void* payload, size_t payload_size );

	/// Create a TSPacket object that wraps the given raw data.
	/// \param[in]	data	Source data (188 bytes)
	/// \param[in]	copy	If true duplicate the data, otherwise copy data to local storage.
	TSPacket( const void* data, bool copy = true );

	~TSPacket();

	/// Check to see if packet header is valid.
	/// \return true if packet is valid.
	bool isvalid();

	/// Get the PID from the TS packet header.
	/// \return value.
	unsigned int pid();

	/// Get the packet start indicator flag from header.
	/// \return value.
	bool pusi();

	/// Check to see if packet has a payload.
	/// \return bool.
	bool hasPayload();

	/// Check to see if packet has an adaptation field.
	/// \return bool.
	bool hasAdaptation();

	/// Get the length of the header.
	/// \return the number of bytes in the TS header.
	unsigned int headerLen();

	/// Get the payload of the TS frame.
	/// \param[out]	p_size		Where to store payload size.
	/// \return pointer to payload data.
	const uint8_t* getPayload( size_t* p_size );

	/// Write TS Packet to a file
	/// \param[in]	fd	File descriptor.
	/// \return number of bytes written or -ve on error.
	int write( int fd );

	/// Get raw pointer to entire TS packet.
	/// \return pointer.
	const uint8_t* ptr();

	/// Change the pid of a ts packet.
	/// \param[in] new_pid	New pid to use.
	void changePID(unsigned int new_pid);
private:
	bool			m_copied;		///< Set if data is copied locally.
	uint8_t*		m_copy_data;	///< If data is copied locally, this is the buffer.
	const uint8_t*	m_raw_data;		///< Raw data pointer.
};

#endif

