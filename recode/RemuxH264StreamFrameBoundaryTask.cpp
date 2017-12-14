
#include "xlog.hpp"

#include "RemuxH264StreamFrameBoundaryTask.hpp"

RemuxH264StreamFrameBoundaryTask::RemuxH264StreamFrameBoundaryTask() {
}

RemuxH264StreamFrameBoundaryTask::~RemuxH264StreamFrameBoundaryTask() {
}

int RemuxH264StreamFrameBoundaryTask::run( std::vector<TSPacket*>* ret, TS::Stream* source ) {
	// Temporary for now, clone */
	XLOG_WARNING("Not implemented - currently just pushing the original video stream back");
	for ( unsigned i = 0; i < source->numPackets(); i++ )
		ret->push_back( source->packet(i) );
	return 0;
}

