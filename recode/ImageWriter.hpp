#ifndef __ImageWriter_hpp__
#define __ImageWriter_hpp__

class ImageWriter {
private:
	ImageWriter();
public:
	static int writeYUVImage( const char* filename, unsigned char* Y, unsigned char* U, unsigned char* V, 
							  unsigned int Ylen, unsigned int Ulen, unsigned int Vlen,
							  unsigned int width, unsigned int height );
};

#endif

