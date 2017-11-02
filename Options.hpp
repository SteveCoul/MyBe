#ifndef __Options_hpp__
#define __Options_hpp__

#include <string>

class Options {
public:

	Options() {
		erase();
		setDefault();
	}

	~Options() {
		erase();
	}

	int process( int argc, char** argv ) {
		erase();
		setDefault();
		return parse( argc, argv );
	}

	std::string source() { return m_source_filename; }
	std::string dest() { return m_dest_filename; }
	bool saveVideos() { return m_save_videos; }

private:
	int parse( int argc, char** argv );
	void erase();
	void setDefault();

private:
	std::string		m_source_filename;
	std::string		m_dest_filename;
	bool			m_save_videos;
};

#endif

