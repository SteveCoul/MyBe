#ifndef __TSPacket_hpp__
#define __TSPacket_hpp__

#include <stddef.h>
#include <stdint.h>

class TSPacket {
public:
	TSPacket( const void* data, bool copy = true );
	~TSPacket();
	bool isvalid();
	// Caller is responsible for ensuring TS Packet is valid before calling any queries
	unsigned int pid();
private:
	bool			m_copied;
	uint8_t*		m_copy_data;
	const uint8_t*	m_raw_data;
};

#endif
