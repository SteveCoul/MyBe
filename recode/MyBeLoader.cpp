#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include <curl/curl.h>

#include <vector>

#include "ReferenceDecoder.hpp"
#include "MyBeLoader.hpp"

/// Implementation of the MyBeLoader class
class Impl {
public:
	/// Return the current system time in milliseconds.
	/// \return time.
	static unsigned long long NOW() {
		struct timeval tv;
		(void)gettimeofday( &tv, NULL );
		unsigned long long rc = (unsigned long long)tv.tv_usec;
		rc/=1000;
		rc = rc + ( tv.tv_sec * 1000 );
		return rc;
	};

	/// A download session.
	class Session {
	public:
		/// Create a download session.
		/// \param[in]	url			URL of resource to download.
		/// \param[in]	callback	Callback instance to invoke on success or error.
		/// \param[in]	timeout		Millisecond time offset after which loader will return the best segment it can.
		/// \param[in]	manager		The parent of this download session.
		Session( const char* url, MyBeLoader::Callback* callback, unsigned int timeout, Impl* manager )
			: m_timeout( timeout )
			, m_callback( callback )
			, m_curl( NULL )
			, m_manager( manager )
			{

			m_curl = curl_easy_init();
			(void)curl_easy_setopt( m_curl, CURLOPT_URL, url );
			fprintf( stderr, "Session %p created for url %s as curl handle %p\n", this, url, m_curl );
		}

		~Session() {
			fprintf( stderr, "Clean session %p\n", this );
			(void)curl_easy_cleanup( m_curl );
		}

		/// Start download.
		/// \return true on success.
		bool start() {
			m_start_time = NOW();
			/* do I need this to be locked? */
			fprintf( stderr, "Starting %p\n", this );
			curl_multi_add_handle( m_manager->m_curl_multi, m_curl );
			return true;
		}

		/// Force download to return the video segment at the earliest possiblity. (Single IFrame)
		void forceEarly() {
			assert(0);
		}

		/// Force download to return the video segment with the alternate video. (Lower framerate/quality)
		void forceBetter() {
			assert(0);
		}

		/// Force download to return the video segment with the original video.
		void forceBest() {
			assert(0);
		}

		CURL*					m_curl;			///< The CURL handle for this download.
		unsigned int			m_timeout;		///< The millisecond timeout for this download at which it would return the best video it can.
		unsigned int			m_start_time;	///< Time download started.
		MyBeLoader::Callback*	m_callback;		///< Callback for download error or success.
		Impl*					m_manager;		///< Parent.
	};
public:
	Impl() 
		: m_curl_multi( NULL )
		, m_shutdown(false)
		{
		m_curl_multi = curl_multi_init();
		fprintf( stderr, "Curl Multi Handle %p\n", m_curl_multi );
		(void)pthread_create( &m_thread, NULL, thread_wrapper, this );
	}

	~Impl() {
		fprintf( stderr, "Waiting on all sessions\n" );
		m_shutdown = true;
		void* res;
		(void)pthread_join( m_thread, &res );
		fprintf( stderr, "shutdown finished\n" );
		curl_multi_cleanup( m_curl_multi );
	}

	/// Create a download session.
	/// \param[in]	url			URL of resource to download.
	/// \param[in]	callback	Callback instance to invoke on success or error.
	/// \param[in]	timeout		Millisecond time offset after which loader will return the best segment it can.
	/// \return Session pointer.
	Impl::Session* createSession( const char* url, MyBeLoader::Callback* callback, unsigned int timeout ) {
		Session* s = new Session( url, callback, timeout, this );
		//lock();
		m_sessions.push_back( s );
		//unlock();
		return s;
	}
private:

	void thread() {
		fprintf( stderr, "loader thread running\n" );
		while ( !m_shutdown ) {
			sleep(1);
		}
		fprintf( stderr, "loader thread done\n" );
	}

	static void* thread_wrapper( void* param ) { ((Impl*)param)->thread(); return param; }

private:
	std::vector<Session*>			m_sessions;
	CURLM*							m_curl_multi;
	bool							m_shutdown;
	pthread_t						m_thread;
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

