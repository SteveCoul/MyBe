#ifndef __Process_Recode_hpp__
#define __Process_Recode_hpp__

#include <stddef.h>
#include <string.h>

#include "Misc.hpp"
#include "Options.hpp"
#include "TS.hpp"

/// Main application class for the recoder.
class Process_Recode : public Misc::PESCallback {
public:
	/// \brief		Create instance of application and run it.
	/// \param[in]	argc		Incoming argument count.
	/// \param[in]	argv		Incoming arguments.
	/// \return 0 on success.
	static int main( int argc, char** argv );
private:
	Process_Recode();
	~Process_Recode();

	/// \brief		Utility function to open and mmap a file
	/// \param[in]	path		Source filename
	/// \param[out]	p_length	Where to return file length on success.
	/// \return pointer to memory copy of file, or NULL on error.
	static void* openAndMap( std::string path, size_t* p_length );

	/// \brief		Dump all the packets in the TS of a given PID to a file.
	/// \param[in]	path		Destination filename
	/// \param[in]	pid			PID to dump
	void pidToFile( std::string path, unsigned int pid );

	/// \brief		If required, decode original and alternate video streams and store on disk.
	void decodeVideoStreamsAsRequired();

	/// \brief		Save any intermediate streams as required.
	void saveVideoStreamsAsRequired();

	/// \brief		Scan the transport stream and look for the first PAT, store the PMT of the first program.
	/// \return true on success.
	bool findAndDecodePAT();

	/// \brief		Scan the transport stream and look for the PMT discovered by reading the PAT.
	/// \return true on success.
	bool findAndDecodePMT();

	/// \brief		Generate the output file
	/// \param[in]	iframe_original_len		Length of the IDR portion of the original video.
	/// \param[in]	iframe_alternate_len	Length of the IDR portion of the alternate video.
	/// \return 0 on success.
	int writeOutputFile( unsigned int iframe_original_len, unsigned int iframe_alternate_len );

	/// \brief		Run this instance.
	/// \param[in]	argc		Incoming argument count.
	/// \param[in]	argv		Incoming arguments.
	/// \return 0 on success.
	int run( int argc, char** argv );

	void pesCallback( void* opaque, PES* pes );
private:
	Options			m_opts;				///< Runtime options and configuration.
	unsigned int	m_video_pid;		///< The PID of the video in the input stream.
	unsigned int	m_alternate_pid;	///< The PID of the alternate video.
	TS*				m_ts;				///< Incoming transport stream.
	void*			m_raw_ptr;			///< Raw pointer of mmapped transport stream.	
	size_t			m_raw_size;			///< Length of transport stream.
	TS::Stream*		m_pmt_stream;		///< Stream of packets for the PMT.
	unsigned int	m_pmt_pid;			///< PID of the PMT.
};

#endif

