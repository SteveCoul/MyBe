#ifndef __PAT_hpp__
#define __PAT_hpp__

#include <stddef.h>
#include <stdint.h>

class PAT {
public:
	PAT( const uint8_t* data, size_t len );
	bool isvalid();
	void dump();
};

#endif

