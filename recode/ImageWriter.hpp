#ifndef __ImageWriter_hpp__
#define __ImageWriter_hpp__

/// Utility class for writing image files.
class ImageWriter {
private:
	ImageWriter();
public:
	/// Write the given bitmap data for a YUV 4:2:2 image to a file.
	/// \param[in]	filename	Output filename.
	/// \param[in]	Y			Pointer to Y data.
	/// \param[in]	U			Pointer to U data.
	/// \param[in]	V			Pointer to V data.
	/// \param[in]	Ylen		Samples per line for Y data.
	/// \param[in]	Ulen		Samples per line for U data.
	/// \param[in]	Vlen		Samples per line for V data.
	/// \param[in]	width		Pixel size of image.
	/// \param[in]	height		Pixel size of image.
	/// \return 0 on success, -ve on error.
	static int writeYUVImage( const char* filename, unsigned char* Y, unsigned char* U, unsigned char* V, 
							  unsigned int Ylen, unsigned int Ulen, unsigned int Vlen,
							  unsigned int width, unsigned int height );
};

#endif

