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

private:
	int parse( int argc, char** argv );
	void erase();
	void setDefault();

private:
	std::string		m_source_filename;
};

#endif

