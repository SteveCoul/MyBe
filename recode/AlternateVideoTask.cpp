
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "AlternateVideoTask.hpp"
#include "TS.hpp"
#include "VideoDecoder.hpp"
#include "VideoEncoder.hpp"
#include "xlog.hpp"

void AlternateVideoTask::videoIncoming( AVFrame* frame ) {
	m_encoder->newFrame( frame );
}

void AlternateVideoTask::videoComplete() {
	m_encoder->endOfVideo();
}

int AlternateVideoTask::run( TS* ts, unsigned int video_pid, unsigned int alternate_pid, int opt_frames, int opt_rate ) {
	m_decoder = new VideoDecoder( ts, video_pid, this );
	if ( m_decoder == NULL ) {
		XLOG_ERROR("Failed to create video decoder" );
		return -1;
	}
	if ( m_decoder->init() != 0 ) {
		XLOG_ERROR("Failed to init video decoder" );
		return -2;
	}
	m_encoder = new VideoEncoder( ts, alternate_pid, m_decoder->format(), m_decoder->width(), m_decoder->height(), m_decoder->timebase(), opt_rate < 0 ? m_decoder->bitrate() : opt_rate, opt_frames < 0 ? m_decoder->approxFPS() : opt_frames );
	if ( m_encoder == NULL ) {
		XLOG_ERROR("Failed to create video encoder" );
		return -1;
	}
	if ( m_encoder->init() != 0 ) {
		XLOG_ERROR("Failed to init video encoder" );
		return -2;
	}
	m_decoder->run();
	delete m_decoder;
	delete m_encoder;
	return 0;
}

int AlternateVideoTask::make( TS* ts, unsigned int video_pid, unsigned int alternate_pid, int opt_frames, int opt_rate ) {
	AlternateVideoTask task;
	return task.run( ts, video_pid, alternate_pid, opt_frames, opt_rate );
}

