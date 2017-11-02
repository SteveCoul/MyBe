
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "Options.hpp"
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
			} else {
				XLOG_INFO( "Source is %ld bytes", len );

				TS ts( ptr, len );

				(void)munmap( ptr, len );
			}

			(void)close( fd );
		}
	}

	return ret;
}

