#ifndef __ReferenceDecoder_hpp__
#define __ReferenceDecoder_hpp__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <vector>

/// Simple in memory decoder for MyBe files.
class ReferenceDecoder {
public:
	ReferenceDecoder() 
		: m_decoded_header( false )
		, m_misc_length(0)
		, m_first_iframe_keep(0)
		, m_alternate_total(0)
		, m_alternate_iframe_keep(0)
		, m_video_no(NULL)
		, m_size_video_no(0)
		, m_video_single(NULL)
		, m_size_video_single(0)
		, m_video_alternate(NULL)
		, m_size_video_alternate(0)
		, m_video_full( NULL )
		, m_size_video_full(0)
		{
		fprintf( stderr, "Created ReferenceDecoder\n" );
	}

	~ReferenceDecoder() {
		if ( m_video_no ) delete[] m_video_no;
		if ( m_video_single ) delete[] m_video_single;
		if ( m_video_alternate ) delete[] m_video_alternate;
		if ( m_video_full ) delete[] m_video_full;
		fprintf( stderr, "Destroyed ReferenceDecoder\n" );
	}

	/// \brief		Add more source data to the incoming sample.
	/// \param[in]	pointer		Pointer to data.
	/// \param[in]	length		Length of data.
	/// \return number of bytes consumed or -1 on error.
	int add( const void* pointer, size_t length ) {
		uint8_t* ptr = (uint8_t*)pointer;
		m_source.insert( m_source.end(), ptr, ptr+length );
		fprintf( stderr, "Adding source data, length now %u\n", (unsigned) m_source.size() );
		return length;
	}

	/// \brief		Given the current download state - return if possible a segment that has no video.
	/// \param[out]	p_length	Size of returned video.
	/// \return pointer to transport segment if possible to create or NULL if insufficient data received.
	void* generateNoVideo( size_t* p_length ) {
		if ( m_video_no ) {
			p_length[0] = m_size_video_no;
			return m_video_no;
		}

		if ( !decodeHeader() ) return NULL;

		size_t out_len = m_misc_length + m_header_size;

		if ( m_source.size() < (out_len+188) ) {
			fprintf( stderr, "not enough data to generate the NO VIDEO stream\n" );
			return NULL;
		}

		m_size_video_no = out_len;
		m_video_no = new uint8_t[ out_len ];

		uint8_t* src = (uint8_t*)m_source.data();
		uint8_t* dst = m_video_no;

		memmove( dst, src, m_header_size );
		dst+=m_header_size;
		src+=m_header_size;
		src+=188;
		memmove( dst, src, m_misc_length );

		p_length[0] = m_size_video_no;

		sortPackets( m_video_no, m_size_video_no );
		return m_video_no;
	}
	
	/// \brief		Given the current download state - return if possible a segment that has a single Iframe.
	/// \param[out]	p_length	Size of returned video.
	/// \return pointer to transport segment if possible to create or NULL if insufficient data received.
	void* generateSingleFrameVideo( size_t* p_length ) {
		if ( m_video_single ) {
			p_length[0] = m_size_video_single;
			return m_video_single;
		}

		if ( !decodeHeader() ) return NULL;

		size_t out_len = m_misc_length + m_header_size + (((m_first_iframe_keep+187)/188)*188);

		if ( m_source.size() < (out_len+188) ) {
			fprintf( stderr, "not enough data to generate the SINGLE stream\n" );
			return NULL;
		}

		m_size_video_single = out_len;
		m_video_single = new uint8_t[ out_len ];

		uint8_t* src = (uint8_t*)m_source.data();
		uint8_t* dst = m_video_single;

		memmove( dst, src, m_header_size );
		dst+=m_header_size;
		src+=m_header_size;
		src+=188;
		memmove( dst, src, m_misc_length );
		dst+=m_misc_length;
		src+=m_misc_length;
		memmove( dst, src, (((m_first_iframe_keep+187)/188)*188) );

		p_length[0] = m_size_video_single;
		sortPackets( m_video_single, m_size_video_single );
		return m_video_single;
	}

	/// \brief		Given the current download state - return if possible a segment that has the alternate video stream.
	/// \param[out]	p_length	Size of returned video.
	/// \return pointer to transport segment if possible to create or NULL if insufficient data received.
	void* generateAlternateVideo( size_t* p_length ) {

		/* For now, just output the alternate stream, later I want to stitch the original iframe on the bulk of the alternate */

		if ( m_video_alternate ) {
			p_length[0] = m_size_video_alternate;
			return m_video_alternate;
		}

		if ( !decodeHeader() ) return NULL;

		size_t out_len = m_misc_length + m_header_size + (((m_first_iframe_keep+187)/188)*188) + m_alternate_total;

		if ( m_source.size() < (out_len+188) ) {
			fprintf( stderr, "not enough data to generate ALTERNATE stream\n" );
			return NULL;
		}

		out_len -= (((m_first_iframe_keep+187)/188)*188);

		m_size_video_alternate = out_len;
		m_video_alternate = new uint8_t[ out_len ];

		uint8_t* src = (uint8_t*)m_source.data();
		uint8_t* dst = m_video_alternate;

		memmove( dst, src, m_header_size );
		dst+=m_header_size;
		src+=m_header_size;
		src+=188;
		memmove( dst, src, m_misc_length );
		dst+=m_misc_length;
		src+=m_misc_length;

		src+=(((m_first_iframe_keep+187)/188)*188);

		unsigned offset = (unsigned)(src-(uint8_t*)m_source.data());

		fprintf( stderr, "Alternate video found in source, offset %u [%x], size %u [%x]\n", offset, offset, m_alternate_total, m_alternate_total );
		memmove( dst, src, m_alternate_total );
		
		for ( unsigned int i = 0; i < m_alternate_total; i+=188 ) {
			/// \bug HACK!
			dst[i+1] = ( dst[i+1] & 0xE0 ) | ( ( 257 >> 8 ) & 0x1F );
			dst[i+2] = 257 & 0xFF;
		}

		p_length[0] = m_size_video_alternate;
		sortPackets( m_video_alternate, m_size_video_alternate );
		return m_video_alternate;
	}

	/// \brief		Given the current download state - return if possible a segment that has the original full video stream.
	/// \param[out]	p_length	Size of returned video.
	/// \return pointer to transport segment if possible to create or NULL if insufficient data received.
	void* generateFullVideo( size_t* p_length ) {

		if ( m_video_full ) {
			p_length[0] = m_size_video_full;
			return m_video_full;
		}

		if ( !decodeHeader() ) return NULL;

		size_t out_len = m_source.size() - 188 - m_alternate_total;

		if ( m_source.size() < (out_len+188) ) {
			fprintf( stderr, "not enough data to generate FULL stream\n" );
			return NULL;
		}

		m_size_video_full = out_len;
		m_video_full = new uint8_t[ out_len ];

		uint8_t* src = (uint8_t*)m_source.data();
		uint8_t* dst = m_video_full;

		memmove( dst, src, m_header_size );
		dst+=m_header_size;
		src+=m_header_size;
		src+=188;
		memmove( dst, src, m_misc_length );
		dst+=m_misc_length;
		src+=m_misc_length;

		unsigned iframe = (((m_first_iframe_keep+187)/188)*188);

		memmove( dst, src, iframe );
		dst+=iframe;
		src+=iframe;
		
		src+=m_alternate_total;

		memmove( dst, src, m_source.size() - m_header_size - 188 - m_misc_length - iframe - m_alternate_total );

		p_length[0] = m_size_video_full;
		sortPackets( m_video_full, m_size_video_full );
		return m_video_full;
	}

	/// \brief		Given the current download state - return the best video we can.
	/// \param[out]	p_length	Size of returned video.
	/// \return pointer to transport segment if possible to create or NULL if insufficient data received.
	void* generateBestVideo( size_t* p_length ) {
		void* rc;
		rc = generateFullVideo( p_length );
		if ( rc == NULL )
		rc = generateAlternateVideo( p_length );
		if ( rc == NULL )
		rc = generateSingleFrameVideo( p_length );
		if ( rc == NULL )
		rc = generateNoVideo( p_length );
		return rc;
	}
private:
	/// \brief		Scan source for metadata and decode it.
	/// \return true of the data was decoded.
	bool decodeHeader() {

		if ( m_decoded_header ) return true;

		char* header = NULL;
		unsigned int len = m_source.size();
		uint8_t* ptr = (uint8_t*)m_source.data();
		m_header_size = 0;

		for ( unsigned int i = 0; i < len; i+=188 ) {
			if ( strncmp( (const char*)ptr+i+4, "# MakeYourBestEffort\n", 21 ) == 0 ) {
				header = strdup( (const char*)ptr+i+4 );
				break;
			}
			m_header_size+=188;
		}

		if ( header == NULL ) {
			fprintf( stderr, "Still no header\n" );
			return false;
		} else {
			unsigned int l_misc_length = 0;
			unsigned int l_first_iframe_keep = 0;
			unsigned int l_alternate_total = 0;
			unsigned int l_alternate_iframe_keep = 0;
			char* end = header+strlen(header);
			char* p = header;
			for ( unsigned int i = 0; p[i] != '\0'; i++ )
				if ( p[i] == '\n' )
					p[i] = '\0';

			p = p + strlen(p) + 1;
			while ( p != end ) {
				if ( strncmp( p, "MiscLength=", 11 ) == 0 ) l_misc_length += atoi( p+11 );
				else if ( strncmp( p, "FirstIFrameKeep=", 16 ) == 0 ) l_first_iframe_keep += atoi( p+16 );
				else if ( strncmp( p, "AlternateTotal=", 15 ) == 0 ) l_alternate_total += atoi( p+15 );
				else if ( strncmp( p, "AlternateIFrameLength=", 22 ) == 0 ) l_alternate_iframe_keep += atoi( p+22 );
				else fprintf( stderr, "Skipping unknown header field '%s'\n", p );
				p = p + strlen(p) + 1;
			}
			free( header );

			if ( l_misc_length == 0 ) fprintf( stderr, "MiscLength field not found\n" );
			if ( l_first_iframe_keep == 0 ) fprintf( stderr, "FirstIFrameKeep field not found\n" );
			if ( l_alternate_total == 0 ) fprintf( stderr, "AlternateTotal field not found\n" );
			if ( l_alternate_iframe_keep == 0 ) fprintf( stderr, "AlternateIFrameLength field not found\n" );

			if ( ( l_misc_length == 0 ) 
				 || ( l_first_iframe_keep == 0 )
				 || ( l_alternate_total == 0 )
				 || ( l_alternate_iframe_keep == 0 ) ) {
				return false;
			}

			m_misc_length = l_misc_length;
			m_first_iframe_keep = l_first_iframe_keep;
			m_alternate_total = l_alternate_total;
			m_alternate_iframe_keep = l_alternate_iframe_keep;

			fprintf( stderr, "Headers %u [%u.%u], Misc Length %u [%u.%u], First IFrame Keep %u [%u.%u], Alternate Total %u [%u.%u], Alternate IFrame Keep %u [%u.%u]\n", 
						m_header_size,
						m_header_size/188,
						m_header_size%188,
						m_misc_length,
						m_misc_length/188,
						m_misc_length%188,
						m_first_iframe_keep,
						m_first_iframe_keep/188,
						m_first_iframe_keep%188,
						m_alternate_total,
						m_alternate_total/188,
						m_alternate_total%188,
						m_alternate_iframe_keep,
						m_alternate_iframe_keep/188,
						m_alternate_iframe_keep%188 );

			m_decoded_header = true;
			return true;
		}
	}

	void sortPackets( uint8_t* data, size_t length ) {
		fprintf( stderr, "TODO - sort TS packets into order %p : %u\n", data, length );
	}

private:
	std::vector<uint8_t>		m_source;				///< All stored source data.
	bool						m_decoded_header;		///< Flag set when metadata found and decoded.
	unsigned int				m_misc_length;			///< bytes after magic section before iframe
	unsigned int				m_first_iframe_keep;	///< bytes of data for the first IFrame. Note that this is not necessarily TS Frame aligned ( will be )
	unsigned int				m_alternate_total;		///< bytes of alternate video stream
	unsigned int				m_alternate_iframe_keep;///< bytes of alternate video first iframe, not TS frame aligned ( will be )
	unsigned int				m_header_size;			///< bytes of PAT, PMT before magic section
	uint8_t*					m_video_no;				///< Output TS pointer for segment with no video.
	size_t						m_size_video_no;		///< Video size.
	uint8_t*					m_video_single;			///< Output TS pointer for segment with single iframe.
	size_t						m_size_video_single;	///< Video size.
	uint8_t*					m_video_alternate;		///< Output TS pointer for segment with alternate video.
	size_t						m_size_video_alternate;	///< Video size.
	uint8_t*					m_video_full;			///< Output TS pointer for segment with full video.
	size_t						m_size_video_full;		///< Video size.
};

#endif


