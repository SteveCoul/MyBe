#ifndef __Main_hpp__
#define __Main_hpp__

#include <stddef.h>
#include <string.h>

#include "Misc.hpp"
#include "Options.hpp"
#include "TS.hpp"

class Main : public Misc::PESCallback {
public:
	static int main( int argc, char** argv );
private:
	Main();
	~Main();
	static void* openAndMap( std::string path, size_t* p_length );
	void pidToFile( std::string path, unsigned int pid );
	void decodeVideoStreamsAsRequired();
	void saveVideoStreamsAsRequired();
	bool findAndDecodePAT();
	bool findAndDecodePMT();
	int writeOutputFile();
	int run( int argc, char** argv );
	void pesCallback( PES* pes );
private:
	Options			m_opts;
	unsigned int	m_video_pid;
	unsigned int	m_alternate_pid;
	TS*				m_ts;
	void*			m_raw_ptr;	
	size_t			m_raw_size;
	TS::Stream*		m_pmt_stream;
	unsigned int	m_pmt_pid;
};

#endif

