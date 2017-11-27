#ifndef __AlternateVideoTask_hpp__
#define __AlternateVideoTask_hpp__

#include "TS.hpp"
#include "VideoDecoder.hpp"
#include "VideoEncoder.hpp"

class AlternateVideoTask : public VideoDecoder::Callback {
public:
	void videoIncoming( AVFrame* frame );
	void videoComplete();
	int run( TS* ts, unsigned int video_pid, unsigned int alternate_pid );
public:
	static int make( TS* ts, unsigned int video_pid, unsigned int alternate_pid );
private:
	TS*				m_ts;
	unsigned int	m_video_pid;
	unsigned int	m_alternate_pid;
	VideoDecoder*	m_decoder;
	VideoEncoder*	m_encoder;
};

#endif

