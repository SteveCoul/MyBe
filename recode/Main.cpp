
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "AlternateVideoTask.hpp"
#include "Options.hpp"
#include "PAT.hpp"
#include "PMT.hpp"
#include "TS.hpp"
#include "VideoDebugTask.hpp"
#include "VideoDecoder.hpp"
#include "VideoEncoder.hpp"
#include "xlog.hpp"

#include "Main.hpp"

int Main::main( int argc, char** argv ) {
	Main app;
	return app.run( argc, argv );
}

Main::Main() 
	: m_ts( NULL )
	, m_raw_ptr( NULL )
	, m_pmt_stream( NULL )
	{ }

Main::~Main() {
	if ( m_ts ) delete m_ts;
	if ( m_raw_ptr ) munmap( m_raw_ptr, m_raw_size );
}

void* Main::openAndMap( std::string path, size_t* p_length ) {
	void* rc;
	int fd = open( path.c_str(), O_RDONLY );
	if ( fd < 0 ) {
		XLOG_ERROR( "Failed to open %s", path.c_str() );
		rc = NULL;
	} else {
		size_t len = lseek( fd, 0, SEEK_END );
		(void)lseek( fd, 0, SEEK_SET );

		rc = mmap( NULL, len, PROT_READ, MAP_SHARED, fd, 0 );
		if ( rc == (void*)MAP_FAILED ) {
			XLOG_ERROR( "Failed to mmap source file" );
			rc = NULL;
		} else {
			if ( p_length )
				p_length[0] = len;
			(void)close( fd );
		}
	}
	return rc;
}

void Main::pidToFile( std::string path, unsigned int pid ) {
	int ofd;
	ofd = open( path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666 );
	m_ts->writePIDStream( ofd, pid );
	(void)close( ofd );
}

void Main::decodeVideoStreamsAsRequired() {
	if ( m_opts.decodeVideos() ) {
		(void)VideoDebugTask::decode( m_ts, m_video_pid, "original_video" );
		(void)VideoDebugTask::decode( m_ts, m_alternate_pid, "new_video" );
	}
}

void Main::saveVideoStreamsAsRequired() {
	if ( m_opts.saveVideos() ) {
		pidToFile( "tmp_original.ts", m_video_pid );
		pidToFile( "tmp_new.ts", m_alternate_pid );
	}
}

bool Main::findAndDecodePAT() {
	bool rc = false;
	TS::Stream* pat_stream = m_ts->stream( 0 );
	if ( ( pat_stream == NULL ) || ( pat_stream->numPackets() == 0 ) ) {
		XLOG_ERROR( "No PAT found" );
	} else {
		/* FIXME - we need an API for TS::Stream to get the next complete section,
		   for now, I'm only worrying about PATs that fit in a single frame */
		TSPacket* pat_packet = pat_stream->packet(0);
		if ( pat_packet->pusi() == 0 ) {
			XLOG_ERROR( "First PAT TS Frame does not have PUSI set" );
		} else {
			size_t pat_len;
			const uint8_t* pat_ptr = pat_packet->getPayload( &pat_len );
			PAT pat( pat_ptr, pat_len );
			if ( pat.isvalid() == false ) {
				XLOG_ERROR( "Invalid PAT" );
			} else if ( pat.numPrograms() != 1 ) {
				XLOG_ERROR( "PAT should of had 1 single program at instead it returned %u", pat.numPrograms() );
			} else {	
				m_pmt_pid = pat.pmtPID(0); 
				m_pmt_stream = m_ts->stream( m_pmt_pid );
				if ( ( m_pmt_stream == NULL )  || ( m_pmt_stream->numPackets() == 0 ) ) {
					XLOG_ERROR( "PMT not found" );
				} else {
					rc = true;
				}
			}
		}
	}
	return rc;
}

bool Main::findAndDecodePMT() {
	bool rc = false;
	TSPacket* pmt_packet = m_pmt_stream->packet(0);
	if ( pmt_packet->pusi() == 0 ) {
		XLOG_ERROR( "First PMT TS Frame does not have PUSI set" );
	} else {
		size_t pmt_len;
		const uint8_t* pmt_ptr = pmt_packet->getPayload( &pmt_len );
		PMT pmt( pmt_ptr, pmt_len );
		if ( pmt.isvalid() == false ) {
			XLOG_ERROR( "Invalid PMT" );
		} else if ( pmt.streamsOfType( 27 ) != 1 ) {
			XLOG_ERROR( "Expected one video stream and didn't find it" );
		} else {
			m_video_pid = pmt.getPidFirstOfType( 27 );
			XLOG_INFO( "Video Stream is on Pid %d", m_video_pid );
			rc = true;
		}
	}
	return rc;
}

int Main::writeOutputFile() {
	int rc;
	/* Now we write our new output stream,
		PAT
		PMT
		Anything other than magic or video
		Enough video for first IFrame
		magic
		video */
	int ofd = open( m_opts.dest().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666 );
	if ( ofd < 0 ) {
		XLOG_ERROR( "Failed to open dest %s [ %s ]", m_opts.dest().c_str(), strerror(errno) );
		rc = 12;
	} else {
		m_ts->writePIDStream( ofd, 0 );
		m_ts->writePIDStream( ofd, m_pmt_pid );
		for ( unsigned int i = 0; i < 8192; i++ ) 
			if ( ( i != 0 ) && ( i != m_pmt_pid ) && ( i != m_video_pid ) && ( i != m_alternate_pid ) )
				m_ts->writePIDStream( ofd, i );

		/* The FIXMEs below are to do with sharing the first Iframe instead of duplicating it */

		// FIXME need to write the first IFrames worth of video 
		m_ts->writePIDStream( ofd, m_alternate_pid );	// FIXME don't write 1st Iframe
		m_ts->writePIDStream( ofd, m_video_pid );		// FIXME don't write 1st Iframe 
		(void)close( ofd );
		rc = 0;
	}
	return rc;
}

int Main::run( int argc, char** argv ) {
	xlog::init( argv[0] );

	int ret = 0;

	if ( m_opts.process( argc, argv ) < 0 ) {
		ret = 1;
	} else {
		m_raw_ptr = openAndMap( m_opts.source(), &m_raw_size );
		if ( m_raw_ptr == NULL ) {
			XLOG_ERROR( "Failed to map source into memory [%s]", strerror(errno) );
			ret = 3;
		} else {
			m_ts = new TS( m_raw_ptr, m_raw_size );

			if ( findAndDecodePAT() ) {
				if ( findAndDecodePMT() ) {
					m_alternate_pid = m_ts->getUnusedPID( 256 );
					XLOG_INFO( "Well put the magic alternate stream on PID %d", m_alternate_pid );

					/* FIXME really it needs to be an unused pid that is NOT in the PMT just to guard
					   against PMT referencing PIDS not in the transport */

					if ( AlternateVideoTask::make( m_ts, m_video_pid, m_alternate_pid ) != 0 ) {
						XLOG_ERROR( "Failed to make alternate video track" );
						ret = 11;
					} else {
						ret = writeOutputFile();
						if ( ret == 0 ) {
							saveVideoStreamsAsRequired( );
							decodeVideoStreamsAsRequired();
						}
					}
				}
			}
		}
	}
	return ret;
}


