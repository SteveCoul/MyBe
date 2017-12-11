#ifndef __MyBeLoader_hpp__
#define __MyBeLoader_hpp__

#include <stddef.h>

class MyBeLoader {
public:
	typedef void* session_t;

	class Callback {
	public:
		virtual void onError( session_t session, int error_code ) = 0;
		virtual void complete( session_t session, const void* payload, size_t length ) = 0;
	};

public:
	MyBeLoader();
	~MyBeLoader();

	/**
	 * Create a download object session.
	 * @param	url			URL to download
	 * @param	timeout		Download timeout. If this time is reached before delivering the full quality segment one of the following 
	 *						segment types will be returned to callback.
	 *						- an error if there was not enough data downloaded to even provide metadata and audio only.
	 *						- A TS segment without video.
	 *						- A TS segment with the initial IFrame as video.
	 *						- A TS segment with the alternate quality video.
	 *						- A TS segment with the full quality video.
	 * @return  unique identifying tag for this session.
	 */
	session_t createSession( const char* url, Callback* callback, unsigned int timeout );
	
	/**
	 * Start a download session.
	 * @param	session		Session to start.
	 * @return	true if session started, false if it failed to start or was already started. Failing to start a session will delete the object.
	 */
	bool commitSession( session_t session );

	/**
     * Abort a session.
	 * @param	session		Session to stop.
	 * If session is in progress an error will be returned via tha callback. In all circumstances the session object is destroyed.
	 */
	bool abortSession( session_t session );

	/**
	 * Configure a download session to always timeout for earliest possible segment. (Iframe only)
	 * @param	session		Session to configure.
	 */
	void forceSessionEarly( session_t session );

	/**
	 * Configure a download session to always timeout for better segment. (Iframe and 'alternate' video track)
	 * @param	session		Session to configure.
	 */
	void forceSessionBetter( session_t session );

	/**
	 * Configure a download session to always provide best segment regardless of timeout.
	 * @param	session		Session to configure.
	 */
	void forceSessionBest( session_t session );
private:
	void*	m_impl;
};

#endif

