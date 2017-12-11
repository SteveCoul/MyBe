#ifndef __TSPacket_hpp__
#define __TSPacket_hpp__

#include <stddef.h>
#include <stdint.h>

class TSPacket {
public:
	static TSPacket* create( unsigned int pid, unsigned int cc, const void* payload, size_t payload_size );
	TSPacket( const void* data, bool copy = true );
	~TSPacket();
	bool isvalid();
	// Caller is responsible for ensuring TS Packet is valid before calling any queries
	unsigned int pid();
	bool pusi();
	bool hasPayload();
	bool hasAdaptation();
	unsigned int headerLen();
	const uint8_t* getPayload( size_t* p_size );
	int write( int fd );
	const uint8_t* ptr();
	void changePID(unsigned int new_pid);
private:
	bool			m_copied;
	uint8_t*		m_copy_data;
	const uint8_t*	m_raw_data;
};

#endif

