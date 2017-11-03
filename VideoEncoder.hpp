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
    VideoEncoder( TS* ts, unsigned int pid );
    ~VideoEncoder();
    int init();
    void newFrame( AVFrame* frame );
private:
    void add_stream( enum AVCodecID codec_id);
private:
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVCodec *video_codec;
    AVCodecContext *enc;
    AVStream *st;
    AVFrame *frame;
    AVFrame *tmp_frame;
	TS* m_ts;
	unsigned int m_pid;
};

#endif

