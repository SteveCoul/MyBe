
#ifndef __FindIDRTask_hpp__
#define __FindIDRTask_hpp__

#include <stddef.h>
#include <stdint.h>

#include "PES.hpp"

class FindIDRTask {
private:
	int				m_scan_state;
	int				m_pes_state;
	unsigned int	m_pointer;
	unsigned int	m_pointer2;
	unsigned int	m_first_nalu_after_iframe;

public:
	FindIDRTask() 
		: m_scan_state(0)
		, m_pes_state(0)
		, m_pointer(0)
		, m_pointer2(0)
		, m_first_nalu_after_iframe(0)
		{}

	void pes( PES* pes ) {
		size_t ps;
		const uint8_t* payload = pes->payload( &ps );

		if ( m_first_nalu_after_iframe != 0 ) return;	/* we've already found what we wanted */

		for ( size_t i = 0; i < ps; i++ ) {
			if ( m_pes_state == 0 ) {				/* looking for first 0 */
				if ( payload[i] == 0 ) {	
					m_pes_state = 1;
					m_pointer = pes->map(i);
				}
			} else if ( m_pes_state == 1 ) {		/* looking for second 0 */
				if ( payload[i] == 0 ) {	
					m_pes_state = 2;
					m_pointer2 = pes->map(i);
				} else {
					m_pes_state = 0;				
				}
			} else if ( m_pes_state == 2 ) {		/* looking for 1 */
				if ( payload[i] == 0 ) {
					/* still 0 */
					m_pes_state = 2;
					m_pointer = m_pointer2;
					m_pointer2 = pes->map(i);
				} else if ( payload[i] == 1 ) {
					m_pes_state = 3;
				} else {
					m_pes_state = 0;
				}
			} else if ( m_pes_state == 3 ) {		/* starting NALU */
				unsigned int type = payload[i] & 0x1F;
				m_pes_state = 0;

				if ( m_scan_state == 0 ) {		/* looking for IDR */
					if ( type == 5 ) {
						m_scan_state = 1;
					} else {
						m_scan_state = 0;
					}
				} else if ( m_scan_state == 1 ) {	/* Got IDR, looking for other */
					if ( type == 5 ) {
						m_scan_state = 1;
					} else {
						m_scan_state = 2;
						m_first_nalu_after_iframe = m_pointer;
					}
				} else if ( m_scan_state == 2 ) {	/* Got 1st NALU after IDR */
					m_scan_state = 3;
				} else {
					/* state 3 - not interested in anything else */
				}
			} else {
				assert(0);
			}
			if ( m_first_nalu_after_iframe != 0 ) break;
		}
	}

	unsigned int result() { return m_first_nalu_after_iframe; }
};

#endif

