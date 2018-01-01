#ifndef __TS_hpp__
#define __TS_hpp__

#include <stddef.h>

#include <vector>

#include "TSPacket.hpp"

/// Represents an mpeg transport stream.
class TS {
public:
	/// A Stream of packets in the TS based on pid.
	class Stream {
	/// Streams are owned by TS
	friend class TS;
	public:
		/// \brief		Query how many packets are present for this PID stream.
		/// \return count.
		unsigned int numPackets();

		/// \brief		Get a given TS frame from the PID list.
		/// \param[in]	offset	Zero based offset.
		/// \return TSPacket. ( TS keeps ownership ).
		TSPacket* packet( unsigned int offset );	/* returned packet is owned by TS */

		/// \brief		Query PID for this stream.	
		/// \return value.
		int pid() { return m_pid; }
	protected:
		/// \brief		Create Stream Object.
		/// \param[in]	pid		PID
		/// \param[in]	p		List of TS Packets.
		Stream( int pid, std::vector<TSPacket*>* p ) { m_pid = pid; m_packets = p; }
	private:
		std::vector<TSPacket*>*	m_packets;	///< List of packets that form this stream.
		int m_pid;							///< PID for this stream.
	};
public:
	/// \brief		Create a TS representation from the given data.
	/// \param[in]	data	Source data.
	/// \param[in]	length	Length of data.
	TS( const void* data, size_t length );

	~TS();

	/// \brief		Return a stream object for a given PID
	/// \param[in]	pid		PID
	/// \return stream.
	Stream* stream( int pid );

	/// \brief		Find an unused PID in the transport stream.	
	/// \param[in]	lowest	Lowest PID that can be returned.
	/// \return an unused PID.
	unsigned int getUnusedPID( int lowest = 0);

	/// \brief		Write PIDs for a stream to a file.
	/// \param[in]	fd		File descriptor to output file to.
	/// \param[in]	pid		PID to write.
	/// \param[in]	skip	Number of TS packets to skip before starting to write.
	/// \param[in]	count	Number of TS packts to write ( -1 means any remaining )
	/// \return 0 on success.
	int writePIDStream( int fd, unsigned pid, int skip = -1, int count = -1 );

	/// \brief		Find out how many bytes would be needed to store all the TS packets for a PID.
	/// \param[in]	pid		PID
	/// \return length.
	size_t sizePIDStream( unsigned pid );

	/// \brief		Add a new TS Packet to the TS representation and append to the stream object.
	/// \param[in]	pkt		TSPacket to store reference to.
	void add( TSPacket* pkt );

	/// \brief		Remove a PID and it's TSPackets
	/// \param[in]	pid		PID to remove
	void removeStream( unsigned pid );

	/// \brief		Replace all the TS packets for a given PID with a new set.
	/// Existing TSPackets are deleted.
	/// \param[in]	pid		PID to operate on.
	/// \param[in]	source	List of new TS packets for given PID.
	/// \return 0 on success.
	int replaceStream( unsigned pid, std::vector<TSPacket*>*source );

	/// \brief		Cut a number of packets from the start of a PID stream
	/// \param[in]	pid		PID to operate on.
	/// \param[in]	count	Number of ts packets to trim
	void trimPIDStream( unsigned int pid, unsigned int count );
private:
	std::vector<TSPacket*>	m_packets;					///< List of packets in the stream.
	std::vector<TSPacket*>	m_packets_by_pid[ 8192 ];	///< Pointers to store TS packets per pid.
	Stream*					m_stream[ 8192 ];			///< Stream objects per PID.
};

#endif

