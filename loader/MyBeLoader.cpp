#include <assert.h>

#include "MyBeLoader.hpp"

MyBeLoader::MyBeLoader() {
}

MyBeLoader::~MyBeLoader() {
}

MyBeLoader::session_t MyBeLoader::createSession( const char* url, Callback* callback, unsigned int timeout ) {
	assert(0);
}

bool MyBeLoader::commitSession( session_t session ) {
	assert(0);
}

bool MyBeLoader::abortSession( session_t session ) {
	assert(0);
}

void MyBeLoader::forceSessionEarly( session_t session ) {
	assert(0);
}

void MyBeLoader::forceSessionBetter( session_t session ) {
	assert(0);
}

void MyBeLoader::forceSessionBest( session_t session ) {
	assert(0);
}

