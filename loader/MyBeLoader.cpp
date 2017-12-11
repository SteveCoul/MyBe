#include <assert.h>
#include <stdio.h>

#include "MyBeLoader.hpp"

class Impl {
public:
	class Session {
	public:
		Session() {
			assert(0);
		}

		~Session() {
			assert(0);
		}

		bool start() {
			assert(0);
			return false;
		}

		void forceEarly() {
			assert(0);
		}

		void forceBetter() {
			assert(0);
		}

		void forceBest() {
			assert(0);
		}
	};
public:
	Impl() {
	}

	~Impl() {
	}

	Impl::Session* createSession( const char* url, MyBeLoader::Callback* callback, unsigned int timeout ) {
		assert(0);
		return NULL;
	}
private:

};

/* ************************************************************************************************************** *
 *
 * ************************************************************************************************************** */

MyBeLoader::MyBeLoader() {
	m_impl = (void*) new Impl();
}

MyBeLoader::~MyBeLoader() {
	delete (Impl*) m_impl;
}

MyBeLoader::session_t MyBeLoader::createSession( const char* url, Callback* callback, unsigned int timeout ) {
	Impl::Session* s = ((Impl*)m_impl)->createSession( url, callback, timeout );
	return (session_t)s;
}

bool MyBeLoader::commitSession( session_t session ) {
	Impl::Session* s = (Impl::Session*)session;
	if ( s->start() == false ) {
		delete s;
		return false;
	}
	return true;
}

bool MyBeLoader::abortSession( session_t session ) {
	delete (Impl::Session*)session;
	return true;
}

void MyBeLoader::forceSessionEarly( session_t session ) {
	((Impl::Session*)session)->forceEarly();
}

void MyBeLoader::forceSessionBetter( session_t session ) {
	((Impl::Session*)session)->forceBetter();
}

void MyBeLoader::forceSessionBest( session_t session ) {
	((Impl::Session*)session)->forceBest();
}

