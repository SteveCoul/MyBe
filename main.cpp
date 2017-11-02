
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "Options.hpp"
#include "PAT.hpp"
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

