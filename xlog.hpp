#ifndef __xlog_hpp__
#define __xlog_hpp__

#include <syslog.h>

namespace xlog {

void init( const char* name );
void out( int level, const char* function, int line, const char* prefix, const char* suffix, const char* fmt, ... );

};

#define XLOG_INFO( fmt,... )	xlog::out( LOG_INFO,    __PRETTY_FUNCTION__, __LINE__, "", "", fmt, ## __VA_ARGS__ )
#define XLOG_WARNING( fmt,... )	xlog::out( LOG_WARNING, __PRETTY_FUNCTION__, __LINE__, "", "", fmt, ## __VA_ARGS__ )
#define XLOG_ERROR( fmt,... )	xlog::out( LOG_ERR,     __PRETTY_FUNCTION__, __LINE__, "", "", fmt, ## __VA_ARGS__ )

#endif

