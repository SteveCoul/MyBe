#include "VideoDecoder.hpp"

#include "xlog.hpp"

static unsigned int BUFFER_SIZE		=	256 * 188;

int VideoDecoder::avioread_wrapper(void *opaque, uint8_t *buf, int buf_size ) {
	VideoDecoder* THIS = (VideoDecoder*)opaque;
	return THIS->avioread( buf, buf_size );
}

int VideoDecoder::avioread( uint8_t* buf, int buf_size ) {

	if ( m_next_packet == m_ts_stream->numPackets() ) return AVERROR_EOF;

	int rc = 0;
	while ( ( buf_size >= 188 ) && ( m_next_packet != m_ts_stream->numPackets() ) ) {
		TSPacket* pkt = m_ts_stream->packet( m_next_packet++ );
		memcpy( buf, pkt->ptr(), 188 );
		rc+=188;
		buf_size-=188;
		buf+=188;
	}
	return rc;
}

VideoDecoder::VideoDecoder( TS* ts, unsigned int pid, VideoDecoder::Callback* callback ) 
	: m_callback( callback )
	, m_format_context( NULL )
	, m_codec_context( NULL )   
	, m_width( 0 )
	, m_height( 0 )
	, m_pixel_format( AV_PIX_FMT_NONE )
	, m_stream( NULL )
	, m_frame( NULL )
	, m_got_frame( 0 )
	, m_ts( ts )
	, m_pid( pid )
	, m_io_context( NULL )
	, m_next_packet( 0 ) 
	{
}

VideoDecoder::~VideoDecoder() {
	if ( m_codec_context != NULL )
		avcodec_free_context(&m_codec_context);
	if ( m_format_context != NULL )
		avformat_close_input(&m_format_context);
	if ( m_frame != NULL )
		av_frame_free(&m_frame);
	if ( m_io_context ) {
		av_freep( &(m_io_context->buffer) );
		av_freep( &m_io_context );
	}
}

int VideoDecoder::init() {
	AVCodec* codec = NULL;
	AVDictionary *opts = NULL;

	av_register_all();

	m_ts_stream = m_ts->stream( m_pid );
	if ( m_ts_stream == NULL ) {
		XLOG_ERROR("no stream" );
		return -1;
	}

	m_io_context_buffer = (uint8_t*)av_malloc( BUFFER_SIZE );
	if ( m_io_context_buffer == NULL ) {
		XLOG_ERROR( "Failed to alloc context buffer" );
		return -1;
	}

    m_format_context = avformat_alloc_context();
	if ( m_format_context == NULL ) {
		XLOG_ERROR( "Failed to make format context" );
		return -1;
	}

	m_io_context = avio_alloc_context( m_io_context_buffer, BUFFER_SIZE, 0, this, &avioread_wrapper, NULL, NULL );
	if ( !m_io_context ) {
		XLOG_ERROR( "Failed to allocate IO context" );
		return -1;
	}

	m_format_context->pb = m_io_context;

	if (avformat_open_input(&m_format_context, NULL, NULL, NULL) < 0) {
		XLOG_ERROR( "Could not open format" );
		return -1;
	}

	if (avformat_find_stream_info(m_format_context, NULL) < 0) {
		XLOG_ERROR( "Could not find stream information");
		return -1;
	}

	m_stream = m_format_context->streams[ 0 ];    /* there is only one stream and it's our video */

	codec = avcodec_find_decoder(m_stream->codecpar->codec_id);
	if (!codec) {
		XLOG_ERROR( "Failed to find codec" );
		return -1;
	}

	m_codec_context = avcodec_alloc_context3(codec);
	if (!m_codec_context) {
		XLOG_ERROR( "Failed to allocate codec context" );
		return -1;
	}

	if ( avcodec_parameters_to_context(m_codec_context, m_stream->codecpar) < 0) {
		XLOG_ERROR( "Failed to copy codec parameters" );
		return -1;
	}

	if ( avcodec_open2(m_codec_context, codec, &opts) < 0) {
		XLOG_ERROR( "Failed to open codec" );
		return -1;
	}

	m_width = m_codec_context->width;
	m_height = m_codec_context->height;
	m_pixel_format = m_codec_context->pix_fmt;
	m_time_base = m_stream->time_base;
	m_bit_rate = m_codec_context->bit_rate;		/* seems to be 0 ? */
	m_frame = av_frame_alloc();

	XLOG_INFO( "stream avg frame rate %d:%d", m_stream->avg_frame_rate.num, m_stream->avg_frame_rate.den );

	if ( m_stream->avg_frame_rate.den == 0 ) 
		m_approx_fps = 0;
	else 
		m_approx_fps = (int)( (m_stream->avg_frame_rate.num+(m_stream->avg_frame_rate.den/2)) / m_stream->avg_frame_rate.den );

	if (!m_frame) {
		XLOG_ERROR( "Could not allocate frame");
		return -1;
	}

	return 0;
}

int VideoDecoder::approxFPS() {
	return m_approx_fps;
}

void VideoDecoder::run() {
	int ret;

	av_init_packet(&m_packet);
	m_packet.data = NULL;
	m_packet.size = 0;

	while (av_read_frame(m_format_context, &m_packet) >= 0) {
		m_got_frame = 0;
		ret = avcodec_decode_video2(m_codec_context, m_frame, &m_got_frame, &m_packet);
		if (ret < 0)
			break;
		if (m_got_frame) 
			m_callback->videoIncoming( m_frame );
		av_packet_unref(&m_packet);
	}

	m_packet.data = NULL;
	m_packet.size = 0;
	do {
		m_got_frame = 0;
		(void)avcodec_decode_video2(m_codec_context, m_frame, &m_got_frame, &m_packet);
		if (m_got_frame) 
			m_callback->videoIncoming( m_frame );
	} while (m_got_frame);

	m_callback->videoComplete();
}

enum AVPixelFormat VideoDecoder::format() { return m_pixel_format; }
int VideoDecoder::width() { return m_width; }
int VideoDecoder::height() { return m_height; }
int64_t VideoDecoder::bitrate() { return m_bit_rate; }
AVRational VideoDecoder::timebase() { return m_time_base; }

