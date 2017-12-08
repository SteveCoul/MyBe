
#ifndef __Misc_hpp__
#define __Misc_hpp__

#include <stddef.h>
#include <stdint.h>
#include "PES.hpp"
#include "TS.hpp"

class Misc {
public:
	class PESCallback {
	public:
		virtual void pesCallback( PES* pes ) = 0;
	};
private:
	Misc() { }
public:
	static void pesScan( TS::Stream* stream, Misc::PESCallback* cb );
	static unsigned long long mpegTimestampFromBytes( const uint8_t* ptr );
};

#endif

