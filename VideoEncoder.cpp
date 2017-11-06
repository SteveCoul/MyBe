
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
		TSPacket pkt( buf, true /* with copy so we can change pid */ );
		if ( pkt.pid() == 256 ) {
			pkt.changePID( m_pid );
		}
		buf+=188;
		buf_size-=188;
	}
exit(0);
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
	av_write_trailer(oc);

	avcodec_free_context(&enc);

	if ( m_io_context ) {
		av_freep( &(m_io_context->buffer) );
		av_freep( &m_io_context );
	}
	avformat_free_context(oc);
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

	avformat_alloc_output_context2(&oc, NULL, "mpegts", NULL );
	if (!oc)
		return 1;

	oc->pb = m_io_context;

	fmt = oc->oformat;

	fmt->video_codec = AV_CODEC_ID_H264;

	add_stream( fmt->video_codec);

	ret = avcodec_open2(enc, video_codec, &opts);
	if (ret < 0) {
		fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
		exit(1);
	}

	ret = avcodec_parameters_from_context(st->codecpar, enc);
	if (ret < 0) {
		fprintf(stderr, "Could not copy the stream parameters\n");
		exit(1);
	}

	ret = avformat_write_header(oc, &opts);
	if (ret < 0) {
		fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(ret));
		return 1;
	}
	return 0;
}

void VideoEncoder::newFrame( AVFrame* frame ) {
	int got_packet = 0;
	AVPacket pkt = { 0 };
	int ret;

	av_init_packet(&pkt);

	ret = avcodec_encode_video2(enc, &pkt, frame, &got_packet);
	if (ret < 0) {
		fprintf(stderr, "Error encoding video frame: %s\n", av_err2str(ret));
		return;
	}

	if (got_packet) {
		pkt.stream_index = st->index;
		ret = av_interleaved_write_frame(oc, &pkt);
		// ERROR CHECK
	}
}

void VideoEncoder::add_stream( enum AVCodecID codec_id) {
	int i;

	/* find the encoder */
	video_codec = avcodec_find_encoder(codec_id);
	if (!video_codec) {
		fprintf(stderr, "Could not find encoder for '%s'\n", avcodec_get_name(codec_id));
		exit(1);
	}

	st = avformat_new_stream(oc, NULL);
	if (!st) {
		fprintf(stderr, "Could not allocate stream\n");
		exit(1);
	}
	st->id = oc->nb_streams-1;
	enc = avcodec_alloc_context3(video_codec);
	if (!enc) {
		fprintf(stderr, "Could not alloc an encoding context\n");
		exit(1);
	}

	enc->codec_id = codec_id;
	enc->bit_rate = m_bit_rate == 0 ? 40000 : m_bit_rate;
	enc->width    = m_width;
	enc->height   = m_height;
	st->time_base = m_time_base;
	enc->time_base       = (AVRational){1001, 30000};
	enc->gop_size      = 200;
	enc->pix_fmt       = m_pixel_format;

	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}


