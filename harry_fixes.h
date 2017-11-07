
#ifndef __harry_fixes_h__
#define __harry_fixes_h__

#define HIERARCHY_NONE 0
#define HIERARCHY_1 1
#define HIERARCHY_2 2
#define HIERARCHY_4 3
#define HIERARCHY_AUTO 4

#define BANDWIDTH_6_MHZ 0
#define BANDWIDTH_7_MHZ 1
#define BANDWIDTH_8_MHZ 2
#define BANDWIDTH_AUTO 3

#define FE_HAS_SIGNAL 0
#define FE_HAS_CARRIER 0
#define FE_HAS_VITERBI 0
#define FE_HAS_SYNC 0
#define FE_HAS_LOCK 0
#define FE_TIMEDOUT 0
#define FE_REINIT 0
#define FE_SPECTRUM_INV 0

#define TRANSMISSION_MODE_2K 0
#define TRANSMISSION_MODE_8K 1
#define TRANSMISSION_MODE_AUTO 2

#define GUARD_INTERVAL_1_16 0
#define GUARD_INTERVAL_1_4 1
#define GUARD_INTERVAL_1_8 2
#define GUARD_INTERVAL_AUTO 3
#define GUARD_INTERVAL_1_32 4

#define FEC_NONE 0
#define FEC_1_2 1
#define FEC_2_3 2
#define FEC_3_4 3
#define FEC_4_5 4
#define FEC_5_6 5
#define FEC_6_7 6
#define FEC_7_8 7
#define FEC_8_9 8
#define FEC_AUTO 9
#define QPSK 10
#define QAM_16 11
#define QAM_32 12
#define QAM_64 13
#define QAM_128 14
#define QAM_256 15
#define QAM_AUTO 16
#define INVERSION_OFF 0
#define INVERSION_ON 1
#define INVERSION_AUTO 2
#define FE_READ_BER 0
#define FE_READ_SNR 0
#define FE_READ_SIGNAL_STRENGTH 0
#define FE_READ_STATUS 0
#define FE_READ_UNCORRECTED_BLOCKS 0
#define FE_OFDM 0
#define FE_QAM 1
#define FE_QPSK 2
#define FE_GET_FRONTEND 0
#define FE_GET_INFO 0
#define FE_IS_STUPID 0
#define FE_CAN_INVERSION_AUTO 0
#define FE_CAN_FEC_1_2 0
#define FE_CAN_FEC_2_3 0
#define FE_CAN_FEC_3_4 0
#define FE_CAN_FEC_4_5 0
#define FE_CAN_FEC_5_6 0
#define FE_CAN_FEC_6_7 0
#define FE_CAN_FEC_7_8 0
#define FE_CAN_FEC_AUTO 0
#define FE_CAN_QPSK 0
#define FE_CAN_QAM_16 0
#define FE_CAN_QAM_32 0
#define FE_CAN_QAM_64 0
#define FE_CAN_QAM_128 0
#define FE_CAN_QAM_256 0
#define FE_CAN_QAM_AUTO 0
#define FE_CAN_TRANSMISSION_MODE_AUTO 0
#define FE_CAN_BANDWIDTH_AUTO 0
#define FE_CAN_GUARD_INTERVAL_AUTO 0
#define FE_CAN_HIERARCHY_AUTO 0
#define DMX_FILTER_SIZE 256
#define DMX_IN_FRONTEND 0
#define DMX_OUT_TAP 0
#define DMX_PES_OTHER 0
#define DMX_IMMEDIATE_START 0
#define DMX_SET_PES_FILTER 0
#define DMX_STOP 0
#define DMX_SET_BUFFER_SIZE 0
#define DMX_CHECK_CRC 0
#define DMX_SET_FILTER 0
#define DMX_OUT_TS_TAP 0

typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef int fe_code_rate_t;
typedef int fe_modulation_t;
typedef int fe_bandwidth_t;
typedef int fe_transmit_mode_t;
typedef int fe_guard_interval_t;
typedef int fe_hierarchy_t;
typedef int fe_type_t;
typedef int fe_status_t;

struct dmx_pes_filter_params {	
	int pid;
	int input;
	int output;
	int pes_type;
	int flags;
};

struct dmx_sct_filter_params {
	int pid;	
	int timeout;
	int flags;
	union {
		int  mask[256];
		int  filter[256];
	} filter;

};

struct dvb_qam_parameters {
	int symbol_rate;
	int fec_inner;
	int modulation;
};

struct dvb_qpsk_parameters {
	int symbol_rate;
	int fec_inner;
};

struct dvb_ofdm_parameters {
	int	code_rate_HP;
	int	code_rate_LP;
	int constellation;
	int bandwidth;
	int guard_interval;
	int hierarchy_information;
	int transmission_mode;
};

struct dvb_vsb_parameters {
};

struct dvb_frontend_info {
	char name[128];
	int symbol_rate_min;
	int symbol_rate_max;
	int frequency_tolerance;
	int frequency_stepsize;
	int frequency_max;
	int frequency_min;
	int caps;
	int notifier_delay;
	int type;
	int symbol_rate_tolerance;
};

struct dvb_frontend_parameters {
	int	frequency;	
	int inversion;
	union {
		struct dvb_qpsk_parameters qpsk;
		struct dvb_qam_parameters  qam;
		struct dvb_ofdm_parameters ofdm;
		struct dvb_vsb_parameters vsb;
	} u;
};

#define DEMUX_DEVICE_MASK "no"
#define DVB_STD_ADAPTER_NR 0
#define DVB_STD_DEVICE_NR 0
#define DVR_DEVICE_MASK "no"
#define DVB_STD_ADAPTER_NR 0
#define DVB_STD_DEVICE_NR 0
#define FRONTEND_DEVICE_MASK "no"
#define DVB_STD_ADAPTER_NR 0
#define DVB_STD_DEVICE_NR 0
#define DEMUX_DEVICE_MASK "no"
#define DVR_DEVICE_MASK "no"
#define FRONTEND_DEVICE_MASK "no"

#define DVB_MAX_DEV_PATH_LEN 128

#define DVB_API_VERSION 1

#endif

