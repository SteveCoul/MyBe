
/* This maybe a factory later for alternate implementations.

   Let's start with an initial one, where we duplicate frames to get the P/B frames as 
   tiny as possible, but retain the timing of the original video.
*/

#include "xlog.hpp"

#include "ImagePipeline.hpp"

ImagePipeline::ImagePipeline()
	{
}

ImagePipeline::~ImagePipeline() {
}

void ImagePipeline::reset( unsigned int width, unsigned int height, unsigned int depth ) {
}

int ImagePipeline::process( AVFrame* frame ) {
	return 0;
}

