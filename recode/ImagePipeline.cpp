
/* This maybe a factory later for alternate implementations.

   Let's start with an initial one, where we duplicate frames to get the P/B frames as 
   tiny as possible, but retain the timing of the original video.

	FIXME

		Lots of assumptions here, its YUV 422 format, never changes alignment/linesize etc.
*/

#include "xlog.hpp"

#include "ImagePipeline.hpp"

unsigned char*	m_y;
unsigned char*	m_u;
unsigned char*	m_v;

unsigned char*	m_save_y;
unsigned char*	m_save_u;
unsigned char*	m_save_v;

unsigned int    m_width;
unsigned int    m_height;
unsigned int    m_counter;
unsigned int	m_depth;

#define MIN(a,b) (a)<(b)?(a):(b)

ImagePipeline::ImagePipeline()
	{
	m_y = NULL;
	m_u = NULL;
	m_v = NULL;
}

ImagePipeline::~ImagePipeline() {
	delete[] m_y;
	delete[] m_u;
	delete[] m_v;
	delete[] m_save_y;
	delete[] m_save_u;
	delete[] m_save_v;
}

void ImagePipeline::reset( unsigned int width, unsigned int height, unsigned int depth ) {
	// FIXME Errors
	m_y = new unsigned char[width*height];
	m_u = new unsigned char[(width/2)*(height/2)];
	m_v = new unsigned char[(width/2)*(height/2)];

	m_save_y = new unsigned char[width*height];
	m_save_u = new unsigned char[(width/2)*(height/2)];
	m_save_v = new unsigned char[(width/2)*(height/2)];

	m_counter = 0;
	m_depth = depth;
	m_width = width;
	m_height = height;
}

static
void planeCopy( uint8_t* dest, unsigned int dest_len, uint8_t* src, unsigned int src_len, unsigned int num_lines ) {
	unsigned int copy = MIN( dest_len, src_len );
	while ( num_lines-- ) {
		memmove( dest, src, copy );
		dest += dest_len;
		src += src_len;
	}
}

int ImagePipeline::process( AVFrame* frame ) {

	planeCopy( m_save_y, m_width,   frame->data[0], frame->linesize[0], m_height );
	planeCopy( m_save_u, m_width/2, frame->data[1], frame->linesize[1], m_height/2 );
	planeCopy( m_save_v, m_width/2, frame->data[2], frame->linesize[2], m_height/2 );

	if ( m_counter == 0 ) {
		/* we want to keep this frame, and use it as image for future */
		planeCopy( m_y, m_width,   frame->data[0], frame->linesize[0], m_height );
		planeCopy( m_u, m_width/2, frame->data[1], frame->linesize[1], m_height/2 );
		planeCopy( m_v, m_width/2, frame->data[2], frame->linesize[2], m_height/2 );
	}
	
	m_counter++;
	if ( m_counter == m_depth ) m_counter = 0;

	planeCopy( frame->data[0], frame->linesize[0], m_y, m_width, m_height );
	planeCopy( frame->data[1], frame->linesize[1], m_u, m_width/2, m_height/2 );
	planeCopy( frame->data[2], frame->linesize[2], m_v, m_width/2, m_height/2 );

	return 0;
}

void ImagePipeline::restore( AVFrame* frame ) {
	planeCopy( frame->data[0], frame->linesize[0], m_save_y, m_width, m_height );
	planeCopy( frame->data[1], frame->linesize[1], m_save_u, m_width/2, m_height/2 );
	planeCopy( frame->data[2], frame->linesize[2], m_save_v, m_width/2, m_height/2 );
}

