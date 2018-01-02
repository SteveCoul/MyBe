
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main( int argc, char** argv ) {
	if ( argc < 2 ) {
		fprintf( stderr, "%s pid <source1>....<sourceN>\n", argv[0] );
		return 1;
	}

	int pid = atoi( argv[1] );

	fprintf( stderr, "Changing PID to %d\n", pid );

	for ( int i = 2; i < argc; i++ ) {
		fprintf( stderr, "  %s\n", argv[i] );
		int fd = open( argv[i], O_RDONLY );
		if ( fd < 0 ) {
			fprintf( stderr, "Failed to open %s\n", argv[i] );
			return 2;
		}
	
		for (;;) {
			unsigned char dst[188];
			int r = (int)read( fd, dst, 188 );
			if ( r < 0 ) {
				fprintf( stderr, "Error reading\n");
				(void)close( fd );
				return 3;
			}
			if ( r == 0 ) break;
			
			dst[1] = ( dst[1] & 0xE0 ) | ( ( pid >> 8 ) & 0x1F );
			dst[2] = pid & 0xFF;
		
			(void)write( STDOUT_FILENO, dst, 188 );
		}

		(void)close( fd );
	}
	return 0;
}

