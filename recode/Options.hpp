#ifndef __Options_hpp__
#define __Options_hpp__

#include <string>

/// Command line option parsing.
class Options {
public:

	Options() {
		erase();
		setDefault();
	}

	~Options() {
		erase();
	}

	/// \brief		Process incoming command line arguments and configure.
	/// \param[in]	argc	Count of arguments.
	/// \param[in]	argv	Incoming arguments.
	/// \return 0 on success.
	int process( int argc, char** argv ) {
		erase();
		setDefault();
		return parse( argc, argv );
	}

	/// \brief Return source stream filename
	/// \return value.
	std::string source() { return m_source_filename; }

	/// \brief Return destination filename
	/// \return value.
	std::string dest() { return m_dest_filename; }

	/// \brief Check to see if intermediate video streams should be saved.
	/// \return value.
	bool saveVideos() { return m_save_videos; }

	/// \brief Check to see if videos should be decoded and have frame files dumped.
	/// \return value.
	bool decodeVideos() { return m_decode_videos; }

	/// \brief Return how many frames should be duplicated in alternate stream.
	/// \return value.
	int frames() { return m_frames; }

	/// \brief Return alternate video encode quality.
	/// \return value.
	int quality() { return m_quality; }

private:
	int parse( int argc, char** argv );
	void erase();
	void setDefault();

private:
	std::string		m_source_filename;
	std::string		m_dest_filename;
	bool			m_save_videos;
	bool			m_decode_videos;
	int				m_frames;
	int				m_quality;
};

#endif

