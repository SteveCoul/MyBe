
#if 0
look at http://ffmpeg.org/doxygen/2.8/frame_8c.html#a52ecfa0b4c5c6b3c81faf25edb0b5dea

for frame_copy_video.

Create a pipeline class we call inside newFrame() where we pass it the current frame and it returns us one to encode that might be a copy of a previous image!


#endif

#include "VideoEncoder.hpp"

#include "xlog.hpp"

int VideoEncoder::aviowrite_wrapper(void *opaque, uint8_t *buf, int buf_size ) {
	return ((VideoEncoder*)opaque)->write( buf, buf_size );
}

int VideoEncoder::write( uint8_t* buf, int buf_size ) {
	// HACKERY AHEAD - Video is always pid 256 from ffmpeg atm. Here we will only pass packets which area on that pid
	// and we will also change the pid to the one requested in the constructor!
	int ret = buf_size;
	while ( buf_size >= 188 ) {
		TSPacket* pkt = new TSPacket( buf, true /* with copy so we can change pid */ );
		if ( pkt->pid() == 256 ) {
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

VideoEncoder::VideoEncoder( TS* ts, unsigned int pid, enum AVPixelFormat format, int width, int height, AVRational time_base, int64_t bit_rate )
	: m_ts( ts )
	, m_pid( pid )
	, m_pixel_format( format )
	, m_width( width )
	, m_height( height )
	, m_time_base( time_base )
	, m_bit_rate( bit_rate )
	, m_io_context( NULL )
	{
}

VideoEncoder::~VideoEncoder() {
	av_write_trailer(m_format_context);

	// FIXME need to close codec_context first! 
	
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

	m_io_context_buffer = (uint8_t*)av_malloc( 4096*188 );
	if ( m_io_context_buffer == NULL ) {
		XLOG_ERROR( "Failed to alloc context buffer" );
		return -1;
	}

	m_io_context = avio_alloc_context( m_io_context_buffer, 4096*188, 1, this, NULL, &aviowrite_wrapper, NULL );
	if ( !m_io_context ) {
		XLOG_ERROR( "Failed to allocate IO context" );
		return -1;
	}

	avformat_alloc_output_context2(&m_format_context, NULL, "mpegts", NULL );
	if (!m_format_context)
		return 1;

	m_format_context->pb = m_io_context;

	m_output_format = m_format_context->oformat;

	m_output_format->video_codec = AV_CODEC_ID_H264;

	add_stream( m_output_format->video_codec);

	ret = avcodec_open2(m_codec_context, m_codec, &opts);
	if (ret < 0) {
		fprintf(stderr, "Could not open video codec\n" );
		exit(1);
	}

	ret = avcodec_parameters_from_context(m_stream->codecpar, m_codec_context);
	if (ret < 0) {
		fprintf(stderr, "Could not copy the stream parameters\n");
		exit(1);
	}

	ret = avformat_write_header(m_format_context, &opts);
	if (ret < 0) {
		fprintf(stderr, "Error occurred when opening output file\n" );
		return 1;
	}
	return 0;
}

static const char* pict_type( int code ) {
	const char* rc;
	switch( code ) {
	case AV_PICTURE_TYPE_I: rc = "I"; break;
	case AV_PICTURE_TYPE_P: rc = "P"; break;
	case AV_PICTURE_TYPE_B: rc = "B"; break;
	case AV_PICTURE_TYPE_S: rc = "S"; break;
	case AV_PICTURE_TYPE_SI: rc = "SI"; break;
	case AV_PICTURE_TYPE_SP: rc = "SP"; break;
	case AV_PICTURE_TYPE_BI: rc = "BI"; break;
	default: rc = "?"; break;	
	}
	return rc;
}

void VideoEncoder::newFrame( AVFrame* frame ) {
	int got_packet = 0;
	AVPacket pkt = { 0 };
	int ret;

	XLOG_INFO( "Frame type %s", pict_type( frame->pict_type ) );

	av_init_packet(&pkt);

	ret = avcodec_encode_video2(m_codec_context, &pkt, frame, &got_packet);
	if (ret < 0) {
		fprintf(stderr, "Error encoding video frame\n" );
		return;
	}

	if (got_packet) {
		pkt.stream_index = m_stream->index;
		ret = av_interleaved_write_frame(m_format_context, &pkt);
		// ERROR CHECK
	}
}

void VideoEncoder::add_stream( enum AVCodecID codec_id) {
	int i;

	/* find the encoder */
	m_codec = avcodec_find_encoder(codec_id);
	if (!m_codec) {
		fprintf(stderr, "Could not find encoder for '%s'\n", avcodec_get_name(codec_id));
		exit(1);
	}

	m_stream = avformat_new_stream(m_format_context, NULL);
	if (!m_stream) {
		fprintf(stderr, "Could not allocate stream\n");
		exit(1);
	}
	m_stream->id = m_format_context->nb_streams-1;
	m_codec_context = avcodec_alloc_context3(m_codec);
	if (!m_codec_context) {
		fprintf(stderr, "Could not alloc an encoding context\n");
		exit(1);
	}

	m_codec_context->codec_id = codec_id;
	m_codec_context->bit_rate = m_bit_rate == 0 ? 1000 : m_bit_rate;
	m_codec_context->width    = m_width;
	m_codec_context->height   = m_height;
	m_stream->time_base = m_time_base;
	m_codec_context->time_base       = (AVRational){1001, 30000};
	m_codec_context->gop_size      = 200;
	m_codec_context->pix_fmt       = m_pixel_format;


	m_format_context->bit_rate = 100;

	/* Some formats want stream headers to be separate. */
	if (m_format_context->oformat->flags & AVFMT_GLOBALHEADER)
		m_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}


