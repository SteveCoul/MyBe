
#include "xlog.hpp"

#include <stdarg.h>

#include <string>

namespace xlog {

void init( const char* name ) {
	openlog( name, LOG_PERROR, LOG_USER );
}

void out( int level, const char* function, int line, const char* prefix, const char* suffix, const char* fmt, ... ) {
	line = line;
	std::string output;
	output.append( prefix );
	output.append( function );
	output.append( "() -- " );
	output.append( fmt );
	output.append( suffix );

	va_list arg;
	va_start( arg, fmt );
	vsyslog( level, output.c_str(), arg );
	va_end( arg );
}

void hexdump( int level, const char* function, int line, const char* prefix, const char* suffix, const void* data, size_t length ) {
	char buffer[80];
	unsigned o = 0;
	char* p;
	const uint8_t* ptr = (const uint8_t*)data;

	out( level, function, line, prefix, suffix, "dump %p:%u", data, length );
	while ( length != 0 ) {
		p = buffer;
		p+=sprintf( p, "%08X| ", o );

		for ( int i = 0; i < 16; i++ ) {
			if ( length > i )	
				p+=sprintf( p, "%02X ", ptr[i] ); 
			else 
				p+=sprintf( p, "   " );		
			if ( i == 8 ) p+=sprintf( p, " " );
		}
		p+=sprintf( p, "| " );
		for ( int i = 0; i < 16; i++ ) {
			if ( length == i )
				break;
			p+=sprintf( p, "%c", isprint( ptr[i] ) ? ptr[i] : '.' );
		}

		ptr += 16;	
		o += 16;
		if ( length >= 16 ) length-=16; else length = 0;
		out( level, function, line, prefix, suffix, buffer );
	}
}

};

