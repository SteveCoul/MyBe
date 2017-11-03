
#include "VideoEncoder.hpp"

#include "xlog.hpp"

const char* hack_filename = "hack.ts";

VideoEncoder::VideoEncoder( TS* ts, unsigned int pid )
	: m_ts( ts )
	, m_pid( pid ) {
}

VideoEncoder::~VideoEncoder() {
	av_write_trailer(oc);

	avcodec_free_context(&enc);
	av_frame_free(&frame);
	av_frame_free(&tmp_frame);

	if (!(fmt->flags & AVFMT_NOFILE))
		avio_closep(&oc->pb);
	avformat_free_context(oc);
}

int VideoEncoder::init() {
	int ret;
	AVDictionary *opts = NULL;

	av_register_all();

	avformat_alloc_output_context2(&oc, NULL, "mpegts", hack_filename);
	if (!oc)
		return 1;

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

	av_dump_format(oc, 0, hack_filename, 1);

	if (!(fmt->flags & AVFMT_NOFILE)) {
		ret = avio_open(&oc->pb, hack_filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			fprintf(stderr, "Could not open '%s': %s\n", hack_filename, av_err2str(ret));
			return 1;
		}
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
		av_packet_rescale_ts(&pkt, enc->time_base, st->time_base);
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
	enc->bit_rate = 400000; // FIXME
	enc->width    = 352;    // FIXME
	enc->height   = 288;    // FIXME
	st->time_base = (AVRational){ 1, 30 };  //FIXME
	enc->time_base       = st->time_base;
	enc->gop_size      = 12; 
	enc->pix_fmt       = AV_PIX_FMT_YUV420P;    // FIXME
	//enc->max_b_frames = 2;
	//enc->mb_decision = 2;

	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}


