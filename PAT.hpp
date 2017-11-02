#ifndef __PAT_hpp__
#define __PAT_hpp__

#include <stddef.h>
#include <stdint.h>

#include <vector>

class PAT {
public:
	PAT( const uint8_t* data, size_t len );
	bool isvalid();
	void dump();
	unsigned int numPrograms();
private:
	const uint8_t* m_data;
	size_t			m_len;
	bool			m_valid;
	unsigned int	m_tsid;
	std::vector<unsigned int>m_program_ids;
};

#endif

