#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#include "MyBeLoader.hpp"

class App : public MyBeLoader::Callback {
public:

	virtual void onError( MyBeLoader::session_t session, int error_code ) {
		(void)sem_post( m_wait );
	}

	virtual void complete( MyBeLoader::session_t session, const void* payload, size_t length ) {
		(void)write( STDOUT_FILENO, payload, length );
		(void)sem_post( m_wait );
	}

	int main( int argc, char** argv ) {
		m_wait = sem_open("mybetest", O_CREAT | O_EXCL, 0666 );
		(void)sem_wait( m_wait );
		MyBeLoader::session_t session = m_loader.createSession( argv[1], this, 1000 );
		if ( m_loader.commitSession( session ) ) {
//			forceSessionEarly( session );
//			forceSessionBetter( session );
//			forceSessionBest( session );
			(void)sem_wait( m_wait );
		} else {
		}
		(void)sem_close( m_wait );
		return 0;
	}

private:
	MyBeLoader		m_loader;
	sem_t*			m_wait;
};

int main( int argc, char** argv ) {
	App app;
	return app.main( argc, argv );
}

