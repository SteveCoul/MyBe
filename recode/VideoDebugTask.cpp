
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "ImageWriter.hpp"
#include "TS.hpp"
#include "VideoDebugTask.hpp"
#include "VideoDecoder.hpp"
#include "VideoEncoder.hpp"
#include "xlog.hpp"

VideoDebugTask::VideoDebugTask()
	: m_count(0)
	{}

void VideoDebugTask::videoIncoming( AVFrame* frame ) {
	m_count++;

	// FIXME read up on C++ strings

	char filename[4096];

	(void)snprintf( filename, sizeof(filename), "%s_%05d_%lld_%d.tga", m_prefix->c_str(), m_count, frame->pts, frame->pict_type );	

	// FIXME Making assumption about picture format here

	ImageWriter::writeYUVImage( filename, frame->data[0], frame->data[1], frame->data[2], 
								frame->linesize[0], frame->linesize[1], frame->linesize[2],
								frame->width, frame->height );
}

void VideoDebugTask::videoComplete() {
}

int VideoDebugTask::run( TS* ts, unsigned int pid, std::string* prefix ) {

	m_prefix = prefix;

	m_decoder = new VideoDecoder( ts, pid, this );
	if ( m_decoder == NULL ) {
		XLOG_ERROR("Failed to create video decoder" );
		return -1;
	}
	if ( m_decoder->init() != 0 ) {
		XLOG_ERROR("Failed to init video decoder" );
		return -2;
	}
	m_decoder->run();
	delete m_decoder;
	return 0;
}

int VideoDebugTask::decode( TS* ts, unsigned int pid, std::string* prefix ) {
	VideoDebugTask task;
	return task.run( ts, pid, prefix );
}

