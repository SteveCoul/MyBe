#ifndef __VideoEncoder_hpp__
#define __VideoEncoder_hpp__

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
};

#include "TS.hpp"

class VideoEncoder {
public:
    VideoEncoder( TS* ts, unsigned int pid, enum AVPixelFormat format, int width, int height, AVRational time_base, int64_t bit_rate );
    ~VideoEncoder();
    int init();
    void newFrame( AVFrame* frame );
private:
	static int aviowrite_wrapper(void *opaque, uint8_t *buf, int buf_size );
    void add_stream( enum AVCodecID codec_id);
	int write( uint8_t* buf, int buf_size );
private:
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
	int64_t					m_bit_rate;
    AVIOContext*			m_io_context;
	unsigned char*			m_io_context_buffer;
};

#endif

