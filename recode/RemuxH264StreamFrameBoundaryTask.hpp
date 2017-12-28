#ifndef __RemuxH264StreamFrameBoundaryTask_hpp__
#define __RemuxH264StreamFrameBoundaryTask_hpp__

#include <vector>
#include "Misc.hpp"
#include "TS.hpp"

/// Task implementation to recode a stream of PES encoded H264 such that the new stream has each NALU inside it's own PES stream and entirely aligned with transport-stream packets.
class RemuxH264StreamFrameBoundaryTask : public Misc::PESCallback {
public:
	/// Create task instance and run it.
	/// \param[in]	ret			Where to store transport segment packets of new video.
	/// \param[in]	source		Source video.
	/// \param[in]	dump_file	Files to dump to for debug.
	/// \return 0 on success, -ve on error.
	static int run( std::vector<TSPacket*>* ret, TS::Stream* source, const char* dump_file = NULL );
private:

	virtual void pesCallback( void* opaque, PES* pes );

	/// Create task instance.
	/// \param[in]	ret			Where to store transport segment packets of new video.
	/// \param[in]	source		Source video.
	/// \param[in]	dump_file	Files to dump to for debug.
	RemuxH264StreamFrameBoundaryTask( std::vector<TSPacket*>* ret, TS::Stream* source, const char* dump_file );

	~RemuxH264StreamFrameBoundaryTask();

	/// Deliver the first NALU if complete in the buffer as a PES
	/// \return true if a complete NALU was found.
	bool deliverComplete();

	/// Deliver the entirety of the NALU buffer as a PES.
	void deliverAll();

	/// \brief Check to see if the memory described stores a valid NALU prefix
	/// \param[in]	ptr		Pointer of memory to check.
	/// \param[in]	len		Length of memory to check.
	/// \return true if NALU header found.
	bool checkHeader( const uint8_t* ptr, int len );

	/// Deliver the raw data given as a PES.
	/// Data will be converted to a PES packet, then TS frames and output to the vector used for data output.
	/// \param[in]	ptr		Pointer to raw data to deliver.
	/// \param[in]	len		Length of data to deliver.
	void deliver( const uint8_t* ptr, size_t len );
private:
	std::vector<TSPacket*>*		m_output;		///< Where to store output TS Packets.
	TS::Stream*					m_source;		///< Original video source.
	std::vector<uint8_t>		m_buffer;		///< Storage whilst building NALUs from incoming PES.
	unsigned long long			m_pts;			///< Current PTS.
	unsigned long long			m_dts;			///< Current DTS.
	int							m_fd;			///< File descriptor of where to save debug data if non -1.
	unsigned int				m_stream_id;	///< Stream Id of video stream.
};

#endif

