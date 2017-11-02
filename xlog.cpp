
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

};

