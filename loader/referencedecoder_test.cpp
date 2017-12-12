#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#include "ReferenceDecoder.hpp"

int main( int argc, char** argv ) {
	void* mem;
	int fd = open( argv[1], O_RDONLY );
	if ( fd < 0 ) {
		fprintf( stderr, "Failed to open %s\n", argv[1] );
	} else {
		size_t len = lseek( fd, 0, SEEK_END );
		(void)lseek( fd, 0, SEEK_SET );

		mem = mmap( NULL, len, PROT_READ, MAP_SHARED, fd, 0 );
		if ( mem == (void*)MAP_FAILED ) {
			fprintf( stderr, "Failed to mmap source file\n" );
			(void)close( fd );
		} else {
			ReferenceDecoder d;
			(void)d.add( mem, len );

			(void)close( fd );

			char tmpname[4096];
			void* out;
			size_t out_size;

			out = d.generateNoVideo( &out_size );
			if ( out ) {
				sprintf( tmpname, "%s.novideo.ts", argv[1] );
				fd = open( tmpname, O_WRONLY | O_CREAT, 0666 );
				(void)write( fd, out, out_size );
				(void)close( fd );
			}

			out = d.generateSingleFrameVideo( &out_size );
			if ( out ) {
				sprintf( tmpname, "%s.single.ts", argv[1] );
				fd = open( tmpname, O_WRONLY | O_CREAT, 0666 );
				(void)write( fd, out, out_size );
				(void)close( fd );
			}

			out = d.generateAlternateVideo( &out_size );
			if ( out ) {
				sprintf( tmpname, "%s.alternate.ts", argv[1] );
				fd = open( tmpname, O_WRONLY | O_CREAT, 0666 );
				(void)write( fd, out, out_size );
				(void)close( fd );
			}

			out = d.generateFullVideo( &out_size );
			if ( out ) {
				sprintf( tmpname, "%s.full.ts", argv[1] );
				fd = open( tmpname, O_WRONLY | O_CREAT, 0666 );
				(void)write( fd, out, out_size );
				(void)close( fd );
			}

			munmap( mem, len );
		}
	}
	return 0;
}

