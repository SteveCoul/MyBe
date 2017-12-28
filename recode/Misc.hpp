
#ifndef __Misc_hpp__
#define __Misc_hpp__

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "PES.hpp"
#include "TS.hpp"

/// Miscellaneous utilities
class Misc {
public:
	/// Callback for pesScan() which receives each PES packet.
	class PESCallback {
	public:
		/// Invoked for each PES packet in the scan.
		/// \param[in]	opaque		User pointer passed in to pesScan()
		/// \param[in]	pes			Incoming PES packet.
		virtual void pesCallback( void* opaque, PES* pes ) = 0;
	};
private:
	Misc() { }
public:
	/// Scan transport stream invoking callback for each PES packet.
	/// \param[in]	stream		Stream of packets to decode.
	/// \param[in]	cb			Callback to invoke for each PES packet.
	/// \param[in]	opaque		User pointer to pass back into each callback invokation.
	static void pesScan( TS::Stream* stream, Misc::PESCallback* cb, void* opaque = NULL );

	/// Decode mpeg timestamp field from the given bytes.
	/// \param[in]	ptr			Pointer to bytes to decode.
	/// \return decoded value.
	static unsigned long long mpegTimestampFromBytes( const uint8_t* ptr );

	/// Make a PES representation of some raw data.
	/// \param[in]	ptr			Pointer to elementary data.
	/// \param[in]	len			Length of elementary data.
	/// \param[in]	pts			PTS value.
	/// \param[in]	dts			DTS value ( or 0xFFFFFFFFFFFFFFFF for none ).
	/// \param[in]	m_stream_id	Stream ID for PES.
	/// \return vector containing new PES packet.
	static std::vector<uint8_t>* makeUnboundPESSegment( const uint8_t* ptr, size_t len, unsigned long long pts, unsigned long long dts, unsigned int m_stream_id );
};

#endif

