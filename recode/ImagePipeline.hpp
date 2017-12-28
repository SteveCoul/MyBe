
#ifndef __ImagePipeline_hpp__
#define __ImagePipeline_hpp__

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
};

/// \brief		Image Frame Processor used by the VideoEncoder.
///				When generating the alternate video segment, each video frame from the original decoder is passed into an ImagePipeline,
///				and the image returned from the pipeline is the one written to the encoder.
///
///				The pipeline will duplicate images in batches such that the encoder will write very small non IDR frames (as nothing changed)
///				which gives the effect of a lower frame rate alternate video without actually changing the framerate and potentially 
///				confusing players/decoders further down the line. The result will be a smaller video representation.
///
/// \bug	Global variables need to be class members in implementation.
class ImagePipeline {
public:
	ImagePipeline();
	~ImagePipeline();
	/// \brief		Reset the image pipeline and initialize the size of the bitmap.
	/// \param[in]	width		Pixel width of images.
	/// \param[in]	height		Pixel height of images.
	/// \param[in]	depth		Depth of the pipeline, the pipeline returns the same image each process for 'depth' invokations.
	void reset( unsigned int width, unsigned int height, unsigned int depth );

	/// \brief		Pass in the current frame, and modify the AVFrame to have either a previous duplicate or the keep the image and change the next one in the pipeline.
	/// \param[in]	frame		The AVFrame that we may modify and/or store the image for future calls.
	/// \return 0 on success, -ve on error
	int process( AVFrame* frame );

	/// \brief		Reverse the effect of process() on a frame that may of been modified.
	///
	///				This is required because process() may modify the iframe for the calling video encoder, but ffmpeg requires that the 
	///				frame data be unmodified when continuing the decode process.
	/// \param[in]	frame		The AVFrame to return to previous state.
	void restore( AVFrame* frame );
private:
};

#endif

