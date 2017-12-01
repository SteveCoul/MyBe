#ifndef __VideoDecoder_hpp__
#define __VideoDecoder_hpp__

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
};

#include "TS.hpp"

class VideoDecoder {
public:
    class Callback {
    public:
        virtual void videoIncoming( AVFrame* frame ) = 0;
        virtual void videoComplete() = 0;
    };
public:
    VideoDecoder( TS* ts, unsigned int pid, VideoDecoder::Callback* callback );
    ~VideoDecoder();
    int init();
    void run();
	static int avioread_wrapper(void *opaque, uint8_t *buf, int buf_size );
	int avioread( uint8_t* buf, int buf_size );
	enum AVPixelFormat format();
	int width();
	int height();
	AVRational timebase();
	int64_t bitrate();
	int approxFPS();
private:
    Callback*               m_callback;
    AVFormatContext*        m_format_context;
    AVCodecContext*         m_codec_context;
    int                     m_width;
    int                     m_height;
    enum AVPixelFormat      m_pixel_format;
    AVStream*               m_stream;
    AVFrame*                m_frame;
    AVPacket                m_packet;
    int                     m_got_frame;
	TS*						m_ts;
	unsigned int			m_pid;
    AVIOContext*			m_io_context;
	unsigned char*			m_io_context_buffer;
	unsigned int			m_next_packet;
	TS::Stream*				m_ts_stream;
	AVRational				m_time_base;
	int64_t					m_bit_rate;
	int						m_approx_fps;
};

#endif

