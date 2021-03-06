#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "AlternateVideoTask.hpp"
#include "FindIDRTask.hpp"
#include "Misc.hpp"
#include "Options.hpp"
#include "PAT.hpp"
#include "PMT.hpp"
#include "RemuxH264StreamFrameBoundaryTask.hpp"
#include "TS.hpp"
#include "VideoDebugTask.hpp"
#include "VideoDecoder.hpp"
#include "VideoEncoder.hpp"
#include "xlog.hpp"

#include "Process_Recode.hpp"

int Process_Recode::main( int argc, char** argv ) {
	Process_Recode app;
	return app.run( argc, argv );
}

Process_Recode::Process_Recode() 
	: m_ts( NULL )
	, m_raw_ptr( NULL )
	, m_pmt_stream( NULL )
	{ }

Process_Recode::~Process_Recode() {
	if ( m_ts ) delete m_ts;
	if ( m_raw_ptr ) munmap( m_raw_ptr, m_raw_size );
}

void* Process_Recode::openAndMap( std::string path, size_t* p_length ) {
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

void Process_Recode::pidToFile( std::string path, unsigned int pid, int skip = -1, int count = -1 ) {
	int ofd;
	ofd = open( path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666 );
	m_ts->writePIDStream( ofd, pid, skip, count );
	(void)close( ofd );
}

void Process_Recode::decodeVideoStreamsAsRequired() {
	if ( m_opts.decodeVideos() ) {
		(void)VideoDebugTask::decode( m_ts, m_video_pid, "original_video" );
		(void)VideoDebugTask::decode( m_ts, m_alternate_pid, "new_video" );
	}
}

void Process_Recode::saveVideoStreamsAsRequired( unsigned iframe_len, unsigned alternate_iframe_len ) {
	if ( m_opts.saveVideos() ) {
		pidToFile( "tmp_original.ts", m_video_pid );
		pidToFile( "tmp_new.ts", m_alternate_pid );
		pidToFile( "tmp_original_iframe.ts", m_video_pid, 0, iframe_len / 188 );
		pidToFile( "tmp_new_iframe.ts", m_alternate_pid, 0, alternate_iframe_len / 188 );
		pidToFile( "tmp_original_body.ts", m_video_pid, iframe_len / 188, -1 );
		pidToFile( "tmp_new_body.ts", m_alternate_pid, alternate_iframe_len / 188, -1 );
	}
}

bool Process_Recode::findAndDecodePAT() {
	bool rc = false;
	TS::Stream* pat_stream = m_ts->stream( 0 );
	if ( ( pat_stream == NULL ) || ( pat_stream->numPackets() == 0 ) ) {
		XLOG_ERROR( "No PAT found" );
	} else {
		/// \todo we need an API for TS::Stream to get the next complete section, for now, I'm only worrying about PATs that fit in a single frame 
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

bool Process_Recode::findAndDecodePMT() {
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

int Process_Recode::writeOutputFile( unsigned int iframe_original_len, unsigned int iframe_alternate_len ) {
	int rc;
	/* Now we write our new output stream,
		PAT
		PMT
		My Magic Description Header
		Anything other than magic or video
		Enough video for first IFrame
		magic
		video */

	/// \todo	we now that the incoing 'len' values are offsets to the first NALU after the IDR, and we know that each NALU is in a TS frame,
	///			therefore we know that the number of TS frames we need for the IDR are (len/188)-1 m'kay - bit messy.

	/// \todo	rework the metadata so we can specifiy, number of frames instead of keep lengths etc

	// FOR NOW
	iframe_original_len = ( ( iframe_original_len / 188 ) - 1 ) * 188;
	iframe_alternate_len = ( ( iframe_alternate_len / 188 ) - 1 ) * 188;

	/// \todo FIXME. Eventually, I'll want to remove the first iframe from the alternate video chunk,
	//		  but for now I'll just remember the byte offset into that pid stream and store it
	//		  in the header.

	unsigned int l_misc_length = 0;			/* Length of magic header, PAT, PMT and non video segments */
	unsigned int l_first_iframe_keep = 0;	/* How much of the first video IFrame data needs to be kept for alternate stream */
	unsigned int l_alternate_total = 0;		/* Total length of alternate stream */
	unsigned int l_alternate_iframe_length = 0;	/* How much of alternate stream need be discarded to jump first iframe */

	if ( iframe_original_len % 188 ) XLOG_WARNING("Original Video IFrame does not fix exactly into TS frames for now - loader will have to cope");
	if ( iframe_alternate_len % 188 ) XLOG_WARNING("Alternate Video IFrame does not fix exactly into TS frames for now - loader will have to cope");
	XLOG_WARNING("Warning - Alternate video stream currently contains the IFrame and it should not");

	for ( unsigned int i = 0; i < 8192; i++ ) 
		if ( ( i != 0 ) && ( i != m_pmt_pid ) && ( i != m_video_pid ) && ( i != m_alternate_pid ) )
			l_misc_length += m_ts->sizePIDStream( i );
	l_first_iframe_keep = iframe_original_len;
	l_alternate_total = m_ts->sizePIDStream( m_alternate_pid );
	l_alternate_iframe_length = iframe_alternate_len;

	char header[180];
	int len = snprintf( header, sizeof(header), "# MakeYourBestEffort\nMiscLength=%u\nFirstIFrameKeep=%u\nAlternateTotal=%u\nAlternateIFrameLength=%u\n", l_misc_length, l_first_iframe_keep, l_alternate_total, l_alternate_iframe_length );

	len++;

	XLOG_INFO( "iframe_original_len %u, iframe_alternate_len %u", iframe_original_len, iframe_alternate_len );

	if ( len >= sizeof(header) ) {
		XLOG_ERROR("Magic header too big" );
		rc = -1;
	} else {
		XLOG_INFO( "%s", header );
		int ofd = open( m_opts.dest().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666 );
		if ( ofd < 0 ) {
			XLOG_ERROR( "Failed to open dest %s [ %s ]", m_opts.dest().c_str(), strerror(errno) );
			rc = -2;
		} else {
			unsigned int written = 0;

			if ( m_ts->sizePIDStream( 0 ) != 188 ) {
				XLOG_WARNING("Be warned, only writing first frame of PAT stream because we only want one and we KNOW it's a single frame long atm");
			}
			if ( m_ts->sizePIDStream( m_pmt_pid ) != 188 ) {
				XLOG_WARNING("Be warned, only writing first frame of PMT stream because we only want one and we KNOW it's a single frame long atm");
			}

			XLOG_INFO( "%u written - write PAT", written );
			written+= m_ts->writePIDStream( ofd, 0, 0, 1 );	// See warning above.

			XLOG_INFO( "%u written - write PMT", written );
			written+= m_ts->writePIDStream( ofd, m_pmt_pid, 0, 1 ); // See warning above

			TSPacket* m = TSPacket::create( 8191, 0, header, len );

			XLOG_INFO( "%u written - write Magic", written );
			written+=m->write( ofd );

			delete m;

			XLOG_INFO( "%u written - write Stuff", written );
			for ( unsigned int i = 0; i < 8192; i++ ) 
				if ( ( i != 0 ) && ( i != m_pmt_pid ) && ( i != m_video_pid ) && ( i != m_alternate_pid ) ) {
					if ( m_ts->sizePIDStream( i ) != 0 ) {
						XLOG_INFO("%u written - write PID %u", written, i );
						written+= m_ts->writePIDStream( ofd, i );
					}
				}

			unsigned int iframe_packets = ( iframe_original_len + 187 ) / 188;

			XLOG_INFO( "iframe_packets %u", iframe_packets );

			XLOG_INFO( "%u written - write iframe", written );
			written+=m_ts->writePIDStream( ofd, m_video_pid, 0, iframe_packets );	
			XLOG_INFO( "%u written - write alternate", written );
			written+=m_ts->writePIDStream( ofd, m_alternate_pid );
			XLOG_INFO( "%u written - write original", written );
			written+=m_ts->writePIDStream( ofd, m_video_pid, iframe_packets, -1 );	
			XLOG_INFO( "%u written - done", written );
			(void)close( ofd );
			rc = written;
		}
	}
	return rc;
}

int Process_Recode::guessInputQuality() {
	XLOG_INFO("Original video stream is %u bytes", m_ts->sizePIDStream( m_video_pid ) );

	int tpid = m_ts->getUnusedPID( 256 );

	if ( m_ts->sizePIDStream(tpid) != 0 ) {
		XLOG_ERROR("Cannot run the temporary pid we were going to use is already in use." );
		return -1;
	}

	int lower = 0;
	int upper = 51;
	int mid;

	while ( lower < upper ) {
		mid = lower + ( ( upper - lower ) / 2 );

		if ( AlternateVideoTask::make( m_ts, m_video_pid, tpid, 1, mid ) != 0 ) {
			XLOG_ERROR("Aborting - failed to make the alternate video" );
			return -1;
		}

		XLOG_INFO( "%d.%d] Quality %d gave a file %d%% of original", lower, upper, mid, m_ts->sizePIDStream( tpid ) * 100 / m_ts->sizePIDStream(m_video_pid) );

		if ( abs( lower-upper) <= 1 ) break;

		if ( m_ts->sizePIDStream(tpid) > m_ts->sizePIDStream(m_video_pid) ) lower = mid;
		else upper = mid;
		
		m_ts->removeStream( tpid );
	}
	m_ts->removeStream( tpid );

	return mid;
}

int Process_Recode::run( int argc, char** argv ) {
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

					int quality = m_opts.quality();
					if ( quality == -1 )
						quality = guessInputQuality();
					if ( quality == -1 )
						quality = 22;

					/// \todo	FIXME really it needs to be an unused pid that is NOT in the PMT just to guard against PMT referencing PIDS not in the transport 
					if ( AlternateVideoTask::make( m_ts, m_video_pid, m_alternate_pid, m_opts.frames(), m_opts.quality() ) != 0 ) {
						XLOG_ERROR( "Failed to make alternate video track" );
						ret = 11;
					} else {
						/* Recode video stream so each NALU is in it's own PES and entirely contained in TS packets,
						   we do this primarily to ensure that the first IFrame is encoded in a unique set of TS frames
						   so it can be reused */
						std::vector<TSPacket*>new_h264;
						RemuxH264StreamFrameBoundaryTask::run( "original video", &new_h264, m_ts->stream( m_video_pid ), NULL );
						m_ts->replaceStream( m_video_pid, &new_h264 );

						FindIDRTask findVideo;
						Misc::pesScan( m_ts->stream( m_video_pid ), this, &findVideo );

						/* Recode the alt stream so we can discard the initial iframe and reuse the one from above */
						std::vector<TSPacket*>new_alt_h264;
						RemuxH264StreamFrameBoundaryTask::run( "alternate video", &new_alt_h264, m_ts->stream( m_alternate_pid ), NULL );
						m_ts->replaceStream( m_alternate_pid, &new_alt_h264 );

						FindIDRTask findAlternate;
						Misc::pesScan( m_ts->stream( m_alternate_pid ), this, &findAlternate );

						/* Strip some pids. Recurring data in some source videos that we just don't need/want and 
						   cannot reorder easily in the loader because they have no PTS etc. */
						if ( m_ts->sizePIDStream( 17 ) != 0 ) {
							XLOG_WARNING("Stripping SDT/BAT");
							m_ts->removeStream( 17 );
						}

						ret = writeOutputFile( findVideo.result(), findAlternate.result() );
						if ( ret > 0 ) {
							saveVideoStreamsAsRequired( findVideo.result(), findAlternate.result() );
							decodeVideoStreamsAsRequired();
							XLOG_INFO( "Initial Segment was %u, new segment %u", (unsigned)m_raw_size, (unsigned)ret );
							ret = 0;
						}
					}
				}
			}
		}
	}
	return ret;
}

void Process_Recode::pesCallback( void* opaque, PES* pes ) {
	FindIDRTask* f = (FindIDRTask*)opaque;
	f->pes( pes );
}

int main( int argc, char** argv ) {
	return Process_Recode::main( argc, argv );
}

