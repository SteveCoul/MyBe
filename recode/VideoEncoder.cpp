
extern "C" {
#include <libavutil/opt.h>
};

#include "Options.hpp"
#include "VideoEncoder.hpp"

#include "xlog.hpp"

int VideoEncoder::aviowrite_wrapper(void *opaque, uint8_t *buf, int buf_size ) {
	return ((VideoEncoder*)opaque)->write( buf, buf_size );
}

int VideoEncoder::write( uint8_t* buf, int buf_size ) {
	int ret = buf_size;
	while ( buf_size >= 188 ) {
		TSPacket* pkt = new TSPacket( buf, true /* with copy so we can change pid */ );
		if ( pkt->pid() == 256 ) {	/// \todo Stop assuming we only want to output video pid 256
			pkt->changePID( m_pid );
			m_ts->add( pkt );	/* TS takes ownership */
		} else {
			delete pkt;
		}
		buf+=188;
		buf_size-=188;
	}
	return ret;
}

VideoEncoder::VideoEncoder( TS* ts, unsigned int pid, enum AVPixelFormat format, int width, int height, AVRational time_base, int quality, int opt_frames )
	: m_ts( ts )
	, m_pid( pid )
	, m_pixel_format( format )
	, m_width( width )
	, m_height( height )
	, m_time_base( time_base )
	, m_quality( quality )
	, m_io_context( NULL )
	{
	XLOG_INFO( "Encoder configured for %dx%d, qp %d, frame control %d\n", width, height, quality, opt_frames );
	m_pipeline.reset( m_width, m_height, opt_frames );
}

VideoEncoder::~VideoEncoder() {
	av_write_trailer(m_format_context);

	avcodec_free_context(&m_codec_context);

	if ( m_io_context ) {
		av_freep( &(m_io_context->buffer) );
		av_freep( &m_io_context );
	}
	avformat_free_context(m_format_context);
}

int VideoEncoder::init() {
	int ret;
	AVDictionary *opts = NULL;

	av_register_all();

	m_io_context_buffer = (uint8_t*)av_malloc( ENCODER_BUFFER_SIZE );
	if ( m_io_context_buffer == NULL ) {
		XLOG_ERROR( "Failed to alloc context buffer" );
		ret = -1;
	} else {

		m_io_context = avio_alloc_context( m_io_context_buffer, ENCODER_BUFFER_SIZE, 1, this, NULL, &aviowrite_wrapper, NULL );
		if ( !m_io_context ) {
			XLOG_ERROR( "Failed to allocate IO context" );
			ret = -1;
		} else {

			avformat_alloc_output_context2(&m_format_context, NULL, "mpegts", NULL );
			if (!m_format_context) {
				XLOG_ERROR( "Failed to allocate output context");
				ret = -1;
			} else {

				m_format_context->pb = m_io_context;
				m_output_format = m_format_context->oformat;
				m_output_format->video_codec = AV_CODEC_ID_H264;
				//m_output_format->video_codec = AV_CODEC_ID_MPEG4;

				add_stream( m_output_format->video_codec);

				ret = avcodec_open2(m_codec_context, m_codec, &opts);
				if (ret < 0) {
					XLOG_ERROR( "Could not open video codec" );
					ret = -1;
				} else {

					ret = avcodec_parameters_from_context(m_stream->codecpar, m_codec_context);
					if (ret < 0) {
						XLOG_ERROR( "Could not copy the stream parameters");
						ret = -1;
					} else {
						ret = avformat_write_header(m_format_context, &opts);
						if (ret < 0) {
							XLOG_ERROR( "Error occurred when opening output file" );
							ret = -1;
						} else {
							ret = 0;
						}
					}
				}
			}
		}
	}
	return ret;
}

void VideoEncoder::endOfVideo() {
	newFrame( NULL );
}

void VideoEncoder::newFrame( AVFrame* frame ) {
	int got_packet = 0;
	AVPacket pkt = { 0 };
	int ret;

	if ( frame ) {
		if ( m_pipeline.process( frame ) < 0 ) {
			XLOG_ERROR( "Failed to process frame via image pipeline, continue with original data" );
		}
	}
	
	do {
		av_init_packet(&pkt);

		ret = avcodec_encode_video2(m_codec_context, &pkt, frame, &got_packet);
		if (ret < 0) {
			XLOG_ERROR( "Error encoding video frame" );
			return;
		}

		if (got_packet) {
			pkt.stream_index = m_stream->index;
			ret = av_interleaved_write_frame(m_format_context, &pkt);
			// ERROR CHECK
		}
	} while ( got_packet && !frame );

	if ( frame )
		m_pipeline.restore( frame );
}

void VideoEncoder::add_stream( enum AVCodecID codec_id) {
	int i;

	/* find the encoder */
	m_codec = avcodec_find_encoder(codec_id);
	if (!m_codec) {
		XLOG_ERROR( "Could not find encoder for '%s'", avcodec_get_name(codec_id));
		return;
	}

	m_stream = avformat_new_stream(m_format_context, NULL);
	if (!m_stream) {
		XLOG_ERROR( "Could not allocate stream");
		return;
	}
	m_stream->id = m_format_context->nb_streams-1;
	m_codec_context = avcodec_alloc_context3(m_codec);
	if (!m_codec_context) {
		XLOG_ERROR( "Could not alloc an encoding context");
		return;
	}

	m_codec_context->codec_id = codec_id;
	m_codec_context->width    = m_width;
	m_codec_context->height   = m_height;
	m_stream->time_base = m_time_base;
	m_codec_context->time_base       = (AVRational){1001, 30000};	/// \todo get the proper framerate/timing from source video and pass it into encoder
	m_codec_context->gop_size      = 99999;
	m_codec_context->pix_fmt       = m_pixel_format;

	char tmp[8];	
	(void)snprintf( tmp, sizeof(tmp), "%d", m_quality );
    av_opt_set(m_codec_context->priv_data, "qp", tmp, 0);

	/* Some formats want stream headers to be separate. */
	if (m_format_context->oformat->flags & AVFMT_GLOBALHEADER)
		m_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}


