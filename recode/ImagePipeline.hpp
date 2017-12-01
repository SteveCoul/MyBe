
#ifndef __ImagePipeline_hpp__
#define __ImagePipeline_hpp__

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
};

class ImagePipeline {
public:
	ImagePipeline();
	~ImagePipeline();
	void reset( unsigned int width, unsigned int height, unsigned int depth );
	int process( AVFrame* frame );
	void restore( AVFrame* frame );
private:
};

#endif

