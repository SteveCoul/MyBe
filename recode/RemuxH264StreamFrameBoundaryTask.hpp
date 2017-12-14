#ifndef __RemuxH264StreamFrameBoundaryTask_hpp__
#define __RemuxH264StreamFrameBoundaryTask_hpp__

#include <vector>

#include "TS.hpp"

class RemuxH264StreamFrameBoundaryTask {
public:
	static int run( std::vector<TSPacket*>* ret, TS::Stream* source );
private:
	RemuxH264StreamFrameBoundaryTask();
	~RemuxH264StreamFrameBoundaryTask();
};

#endif

