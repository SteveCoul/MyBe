#ifndef __AlternateVideoTask_hpp__
#define __AlternateVideoTask_hpp__

#include "TS.hpp"
#include "VideoDecoder.hpp"
#include "VideoEncoder.hpp"

/// A task class which builds an alternate video stream.
class AlternateVideoTask : public VideoDecoder::Callback {
public:
	/// \brief		Callback invoked for each incoming video frame from the decoder task.
	/// \param[in]	frame		Incoming video frame from ffmpeg.
	void videoIncoming( AVFrame* frame );

	/// \brief		Callback invoked when video decoder completed.
	void videoComplete();

	/// \brief		Run the alternate video generation task.
	/// \param[in]	ts				The transport stream that sources the video and will recieve the newly created video.
	/// \param[in]	video_pid		The PID of the original video.
	/// \param[in]	alternate_pid	The PID to use for the new video stream.
	/// \param[in]	opt_frames		How many frames are duplicated in the image pipeline to reduce encoded video size.
	/// \param[in]	opt_rate		Output encoder rate.
	/// \return 0 on success, -ve on error.
	int run( TS* ts, unsigned int video_pid, unsigned int alternate_pid, int opt_frames, int opt_rate );
public:
	/// \brief		Create and run the decoder task to generate an alternate video stream.
	/// \param[in]	ts				The transport stream that sources the video and will recieve the newly created video.
	/// \param[in]	video_pid		The PID of the original video.
	/// \param[in]	alternate_pid	The PID to use for the new video stream.
	/// \param[in]	opt_frames		How many frames are duplicated in the image pipeline to reduce encoded video size.
	/// \param[in]	opt_rate		Output encoder rate.
	/// \return 0 on success, -ve on error.
	static int make( TS* ts, unsigned int video_pid, unsigned int alternate_pid, int opt_frames, int opt_rate );
private:
	TS*				m_ts;				///< Transport stream to operate on.
	unsigned int	m_video_pid;		///< PID of original video.
	unsigned int	m_alternate_pid;	///< PID for newly created alternate video.
	VideoDecoder*	m_decoder;			///< Video Decoder instance.	
	VideoEncoder*	m_encoder;			///< Video Encoder instance.
};

#endif

