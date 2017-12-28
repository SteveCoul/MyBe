
#include <fcntl.h>
#include <unistd.h>

#include "ImageWriter.hpp"

/// \bug Currently converts to greyscale
static void YUV2RGB( unsigned char* pixel, int Y, int U, int V ) {
	// FIXME
	pixel[0] = Y;
	pixel[1] = Y;
	pixel[2] = Y;
}

/// \bug no error checking
int ImageWriter::writeYUVImage( const char* filename, unsigned char* Y, unsigned char* U, unsigned char* V, 
							    unsigned int Ylen, unsigned int Ulen, unsigned int Vlen,
                                unsigned int width, unsigned int height ) {

	unsigned char header[18];

	int fd = open( filename, O_WRONLY | O_CREAT | O_TRUNC, 0666 );

	header[0] = 0;	// id length
	header[1] = 0;	// 0=no colormap
	header[2] = 2;	// 2=uncompressed true color
	header[3] = header[4] = header[5] = header[6] = header[7] = 0;	// empty colormap specification
	header[8] = header[9] = 0; // x origin
	header[10] = header[11] = 0; // y origin
	header[12] = ( width & 255 ); header[13] = ( width >> 8 ) & 255;
	header[14] = ( height & 255 ); header[15] = ( height >> 8 ) & 255;
	header[16] = 24;	// bits per pixel
	header[17] = 32; // image descriptor

	(void)write( fd, header, 18 );

	unsigned char* output = new unsigned char[ width*height*3 ];

	for ( int y = 0; y < height; y++ ) {
		for ( int x = 0; x < width; x++ ) {
			YUV2RGB( output+(y*width*3)+(x*3),
						Y[ (y*Ylen)+x ],
						U[ (y*Ulen)+(x/2) ],
						V[ (y*Vlen)+(x/2) ] );
		}
	}

	write( fd, output,width*height*3 );
	delete[] output;
	(void)close( fd );
	return 0;
}

