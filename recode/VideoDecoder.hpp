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

/// Class decodes a video stream from a transport using ffmpeg.
class VideoDecoder {
public:
	/// Callback for video decoding progress.
    class Callback {
    public:
		/// Callback invoked for incoming decoded video frames.
		/// \param[in]	frame		Video frame from ffmpeg.
        virtual void videoIncoming( AVFrame* frame ) = 0;

		/// Callback invoked when video decode complete.
        virtual void videoComplete() = 0;
    };
public:
	/// Construct video decoder instance.
	/// \param[in]	ts			Transport stream.
	/// \param[in]	pid			PID to decode.
	/// \param[in]	callback	Callback instance to deliver decoded video frames to.
    VideoDecoder( TS* ts, unsigned int pid, VideoDecoder::Callback* callback );

    ~VideoDecoder();

	/// Initialize.
	/// \return 0 on success, -ve on error.
    int init();

	/// Run decode task.
    void run();

	/// Internal wrapper for AVIO to read from memory.
	/// \param[in]	opaque		See ffmpeg docs.
	/// \param[in]	buf			See ffmpeg docs.
	/// \param[in]	buf_size	See ffmpeg docs.
	/// \return	See ffmpeg docs.
	static int avioread_wrapper(void *opaque, uint8_t *buf, int buf_size );

	/// Read data from memory to AVIO buffer.
	/// \param[in]	buf			Where to read data to.
	/// \param[in]	buf_size	Maximum amount of data to read.
	/// \return number of bytes read,
	int avioread( uint8_t* buf, int buf_size );

	/// Get Pixel format of decoded video frames.
	/// \return value.
	enum AVPixelFormat format();

	/// Get pixel width of video frames.
	/// \return value.
	int width();

	/// Get pixel height of video frames.
	/// \return value.
	int height();

	/// Query timebase of video.
	/// \return value.
	AVRational timebase();

	/// Query bitrate of video.
	/// \return value.
	int64_t bitrate();

	/// Query best guess at video frame-rate.
	/// \return value.
	int approxFPS();
private:
    Callback*               m_callback;			///< Callback to deliver decoded frames to.
    AVFormatContext*        m_format_context;	///< libavformat instance for source file.
    AVCodecContext*         m_codec_context;	///< codec for video decode.
    int                     m_width;			///< pixel width of video.
    int                     m_height;			///< Pixel height of video.
    enum AVPixelFormat      m_pixel_format;		///< Stream format of video.
    AVStream*               m_stream;			///< Stream of packets for video source.
    AVFrame*                m_frame;			///< Current video frame.
    AVPacket                m_packet;			///< Current video packet.
    int                     m_got_frame;		///< Flag. if current decode operation has a complete AVFrame yet.
	TS*						m_ts;				///< Transport stream to source data.
	unsigned int			m_pid;				///< PID of video.
    AVIOContext*			m_io_context;		///< AVIO internal for reading.
	unsigned char*			m_io_context_buffer;///< AVIO internal for reading.
	unsigned int			m_next_packet;		///< Offset of next TS packet to return on read.
	TS::Stream*				m_ts_stream;		///< Stream of packets for video.
	AVRational				m_time_base;		///< Timebase of decoded video.
	int64_t					m_bit_rate;			///< Bitrate of decoded video.
	int						m_approx_fps;		///< Best guesse at decoded video frame rate.
};

#endif

