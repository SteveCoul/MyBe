#ifndef __VideoEncoder_hpp__
#define __VideoEncoder_hpp__

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
};

#include "ImagePipeline.hpp"
#include "TS.hpp"

/// Class used to encode a video from images using ffmpeg
class VideoEncoder {
public:
    VideoEncoder( TS* ts, unsigned int pid, enum AVPixelFormat format, int width, int height, AVRational time_base, int quality, int opt_frames );
    ~VideoEncoder();
    int init();
    void newFrame( AVFrame* frame );
	void endOfVideo();
private:
	static int aviowrite_wrapper(void *opaque, uint8_t *buf, int buf_size );
    void add_stream( enum AVCodecID codec_id);
	int write( uint8_t* buf, int buf_size );
private:
	static const size_t		ENCODER_BUFFER_SIZE = 4096*188;
private:
	ImagePipeline			m_pipeline;
    AVOutputFormat*			m_output_format;
    AVFormatContext*		m_format_context;
    AVCodec*				m_codec;
    AVCodecContext*			m_codec_context;
    AVStream*				m_stream;
	TS*						m_ts;
	unsigned int			m_pid;
	AVPixelFormat			m_pixel_format;
	int						m_width;
	int						m_height;
	AVRational				m_time_base;
	int						m_quality;
    AVIOContext*			m_io_context;
	unsigned char*			m_io_context_buffer;
	int						m_opt_frames;
};

#endif

