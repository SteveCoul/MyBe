
#include "Options.hpp"

#include "xlog.hpp"

int Options::parse( int argc, char** argv ) {
	XLOG_WARNING( "Not implemented, just default options will be used" );
	return 0;
}

void Options::erase() {
}

void Options::setDefault() {
	m_source_filename = "input.ts";
	m_dest_filename = "output.ts";
	m_save_videos = true;
	m_decode_videos = true;
}

