
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "Options.hpp"
#include "PAT.hpp"
#include "PMT.hpp"
#include "TS.hpp"
#include "xlog.hpp"

int main( int argc, char** argv ) {
	xlog::init( argv[0] );

	int ret = 0;
	Options opts;

	if ( opts.process( argc, argv ) < 0 ) {
		ret = 1;
	} else {
		int fd = open( opts.source().c_str(), O_RDONLY );
		if ( fd < 0 ) {
			XLOG_ERROR( "Failed to open source %s [ %s ]", opts.source().c_str(), strerror(errno) );
			ret = 2;
		} else {
			ssize_t len = lseek( fd, 0, SEEK_END );
			(void)lseek( fd, 0, SEEK_SET );

			void* ptr = mmap( NULL, len, PROT_READ, MAP_SHARED, fd, 0 );
			if ( ptr == (void*)MAP_FAILED ) {
				XLOG_ERROR( "Failed to map source into memory [%s]", strerror(errno) );
				ret = 3;
			} else {
				XLOG_INFO( "Source is %ld bytes", len );

				TS ts( ptr, len );
				TS::Stream* pat_stream = ts.stream( 0 );
				if ( ( pat_stream == NULL ) || ( pat_stream->numPackets() == 0 ) ) {
					XLOG_ERROR( "No PAT found" );
					ret = 4;
				} else {
					/* FIXME - we need an API for TS::Stream to get the next complete section,
					   for now, I'm only worrying about PATs that fit in a single frame */
					TSPacket* pat_packet = pat_stream->packet(0);
					if ( pat_packet->pusi() == 0 ) {
						XLOG_ERROR( "First PAT TS Frame does not have PUSI set" );
						ret = 5;
					} else {
						size_t pat_len;
						const uint8_t* pat_ptr = pat_packet->getPayload( &pat_len );
						PAT pat( pat_ptr, pat_len );
						if ( pat.isvalid() == false ) {
							XLOG_ERROR( "Invalid PAT" );
							ret = 6;
						} else if ( pat.numPrograms() != 1 ) {
							XLOG_ERROR( "PAT should of had 1 single program at instead it returned %u", pat.numPrograms() );
						} else {
							TS::Stream* pmt_stream = ts.stream( pat.pmtPID(0) );
							if ( ( pmt_stream == NULL )  || ( pmt_stream->numPackets() == 0 ) ) {
								XLOG_ERROR( "PMT not found" );
								ret = 7;
							} else {
								/* FIXME as with PAT above, assuming a single frame for a PMT */
								TSPacket* pmt_packet = pmt_stream->packet(0);
								if ( pmt_packet->pusi() == 0 ) {
									XLOG_ERROR( "First PMT TS Frame does not have PUSI set" );
									ret = 8;
								} else {
									size_t pmt_len;
									const uint8_t* pmt_ptr = pmt_packet->getPayload( &pmt_len );
									PMT pmt( pmt_ptr, pmt_len );
									if ( pmt.isvalid() == false ) {
										XLOG_ERROR( "Invalid PMT" );
										ret = 9;
									} else if ( pmt.streamsOfType( 27 ) != 1 ) {
										XLOG_ERROR( "Expected one video stream and didn't find it" );
										ret = 10;
									} else {
										unsigned int video_pid = pmt.getPidFirstOfType( 27 );
										XLOG_INFO( "Video Stream is on Pid %d", video_pid );

										unsigned int alternate_pid = ts.getUnusedPID( 256 );
										XLOG_INFO( "Well put the magic alternate stream on PID %d", alternate_pid );
										/* FIXME really it needs to be an unused pid that is NOT in the PMT just to guard
										   against PMT referencing PIDS not in the transport */
									}
								}
							}
						}
					}
				}

				(void)munmap( ptr, len );
			}

			(void)close( fd );
		}
	}

	return ret;
}

