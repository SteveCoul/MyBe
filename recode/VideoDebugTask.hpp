#ifndef __VideoDebugTask_hpp__
#define __VideoDebugTask_hpp__

#include <string>

#include "TS.hpp"
#include "VideoDecoder.hpp"
#include "VideoEncoder.hpp"

/// A debug task for decoding and dumping a H264 video stream from a transport.
class VideoDebugTask : public VideoDecoder::Callback {
public:
	VideoDebugTask();
	void videoIncoming( AVFrame* frame );
	void videoComplete();

	/// Run debug task.
	/// \param[in]	ts		Transport stream.
	/// \param[in]	pid		Video PID
	/// \param[in]	prefix	Filename prefix for stored decoded images.
	/// \return 0 on success, -ve on error.
	int run( TS* ts, unsigned int pid, std::string* prefix );
public:
	/// Construct and run a debug task.
	/// \param[in]	ts		Transport stream.
	/// \param[in]	pid		Video PID
	/// \param[in]	prefix	Filename prefix for stored decoded images.
	/// \return 0 on success, -ve on error.
	static int decode( TS* ts, unsigned int pid, std::string* prefix );

	/// Construct and run a debug task.
	/// \param[in]	ts		Transport stream.
	/// \param[in]	pid		Video PID
	/// \param[in]	prefix	Filename prefix for stored decoded images.
	/// \return 0 on success, -ve on error.
	static int decode( TS* ts, unsigned int pid, std::string prefix ) {
		return decode( ts, pid, &prefix );
	}
private:
	TS*				m_ts;		///< Transport stream
	unsigned int	m_pid;		///< PID
	std::string*	m_prefix;	///< Filename prefix
	VideoDecoder*	m_decoder;	///< Video Decoder instance in use.
	unsigned int	m_count;	///< Current frame counter.
};

#endif

