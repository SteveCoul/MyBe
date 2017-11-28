#ifndef __VideoDebugTask_hpp__
#define __VideoDebugTask_hpp__

#include <string>

#include "TS.hpp"
#include "VideoDecoder.hpp"
#include "VideoEncoder.hpp"

class VideoDebugTask : public VideoDecoder::Callback {
public:
	VideoDebugTask();
	void videoIncoming( AVFrame* frame );
	void videoComplete();
	int run( TS* ts, unsigned int pid, std::string* prefix );
public:
	static int decode( TS* ts, unsigned int pid, std::string* prefix );
	static int decode( TS* ts, unsigned int pid, std::string prefix ) {
		return decode( ts, pid, &prefix );
	}
private:
	TS*				m_ts;
	unsigned int	m_pid;
	std::string*	m_prefix;
	VideoDecoder*	m_decoder;
	unsigned int	m_count;
};

#endif

