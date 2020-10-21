/*
 * si468x_commands.h
 *
 * Declares the commands which are sent to the SI468x radio receiver
 *
 *  Created on: 20.04.2017
 *      Author: marco
 */

#ifndef __SI468X_COMMANDS_H__
#define __SI468X_COMMANDS_H__

#include <stdint.h>

// ---- STATUS0 ----
// General bit masks of any reply
// Clear to send next command
#define SI468X_MSK_CTS 0x80
// Command error
#define SI468X_MSK_ERR_CMD 0x40
// Digital radio link change interrupt -> Change in digital radio ensemble acquisition
// Service using DAB_DIGRAD_STATUS command
#define SI468X_MSK_DACQINT 0x20
// An enabled component of a digital radio service requires attention
// Service using GET_DIGITAL_SERVICE_DATA command
#define SI468X_MSK_DSRVINT 0x10
#define SI468X_MSK_RSQINT 0x08
#define SI468X_MSK_RDSINT 0x04
#define SI468X_MSK_ACFINT 0x02
// Seek/Tune command is complete
#define SI468X_MSK_STCINT 0x01

// ---- STATUS1 ----
// Digital radio event change interrupt indicator. New event related to digital radio occured
// Service using DAB_DIGRAD_STATUS command
#define SI468X_MSK_DEVNTINT 0x20

// ---- STATUS2 ----
// Is empty

// ---- STATUS3 ----
// Power up state mask and offset
#define SI468X_MSK_PUPSTATE 0xC0
#define SI468X_OFS_PUPSTATE 6
// DSP frame overrun
#define SI468X_MSK_DSPERR 0x10
// Reply data lost --> SPI interface too fast
#define SI468X_MSK_REPOFERR 0x08
// Command data lost --> SPI interface too fast
#define SI468X_MSK_CMDOFERR 0x04
// Arbiter error occured
#define SI468X_MSK_ARBERR 0x02
// Non recoverable error occured. Keep alive timer expired
#define SI468X_MSK_ERRNR 0x01

// Command errors
#define SI468X_ERR_CMD_UNSPEC 0x01
#define SI468X_ERR_CMD_REPLYOVL 0x02
#define SI468X_ERR_CMD_NOTAVAIL 0x03
#define SI468X_ERR_CMD_NOSUPPORT 0x04
#define SI468X_ERR_CMD_BADFREQ 0x05
#define SI468X_ERR_CMD_NOTFOUND 0x10
#define SI468X_ERR_CMD_BADARG1 0x11
#define SI468X_ERR_CMD_BADARG2 0x12
#define SI468X_ERR_CMD_BADARG3 0x13
#define SI468X_ERR_CMD_BADARG4 0x14
#define SI468X_ERR_CMD_BADARG5 0x15
#define SI468X_ERR_CMD_BADARG6 0x16
#define SI468X_ERR_CMD_BADARG7 0x17
#define SI468X_ERR_CMD_BUSY 0x18
#define SI468X_ERR_CMD_BANDLIM 0x19
#define SI468X_ERR_CMD_BADNVM 0x20
#define SI468X_ERR_CMD_BADPATCH 0x30
#define SI468X_ERR_CMD_BADBOOT 0x31
#define SI468X_ERR_CMD_BADPROP 0x40
#define SI468X_ERR_CMD_NOTACQ 0x50
#define SI468X_ERR_CMD_APPSUPP 0xFF

// Common commands
#define SI468X_CMD_RD_REPLY 0x00
#define SI468X_LEN_RD_REPLY 1
#define SI468X_RPL_RD_REPLY 4

#define SI468X_CMD_POWER_UP 0x01
#define SI468X_LEN_POWER_UP 16

#define SI468X_CMD_HOST_LOAD 0x04
#define SI468X_LEN_HOST_LOAD_HDR 4

#define SI468X_CMD_FLASH_LOAD 0x05
#define SI468X_LEN_FLASH_LOAD 12

#define SI468X_CMD_LOAD_INIT 0x06
#define SI468X_LEN_LOAD_INIT 2

#define SI468X_CMD_BOOT 0x07
#define SI468X_LEN_BOOT 2

#define SI468X_CMD_GET_PART_INFO 0x08
#define SI468X_LEN_GET_PART_INFO 2
#define SI468X_RPL_GET_PART_INFO 19

#define SI468X_CMD_GET_SYS_STATE 0x09
#define SI468X_LEN_GET_SYS_STATE 2
#define SI468X_RPL_GET_SYS_STATE 2

#define SI468X_CMD_GET_POWER_UP_ARGS 0x0A
#define SI468X_LEN_GET_POWER_UP_ARGS 2
#define SI468X_RPL_GET_POWER_UP_ARGS 14

#define SI468X_CMD_READ_OFFSET 0x10
#define SI468X_LEN_READ_OFFSET 4
#define SI468X_RPL_READ_OFFSET 1

#define SI468X_CMD_GET_FUNC_INFO 0x12
#define SI468X_LEN_GET_FUNC_INFO 2
#define SI468X_RPL_GET_FUNC_INFO 8

#define SI468X_CMD_SET_PROPERTY 0x13
#define SI468X_LEN_SET_PROPERTY 6

#define SI468X_CMD_GET_PROPERTY 0x14
#define SI468X_LEN_GET_PROPERTY 4
#define SI468X_RPL_GET_PROPERTY 2

#define SI468X_CMD_WRITE_STORAGE 0x15
#define SI468X_LEN_WRITE_STORAGE 9

#define SI468X_CMD_READ_STORAGE 0x16
#define SI468X_LEN_READ_STORAGE 4
#define SI468X_RPL_READ_STORAGE 1

// FM Mode commands
#define SI468X_CMD_FM_TUNE_FREQ 0x30
#define SI468X_LEN_FM_TUNE_FREQ 7

#define SI468X_CMD_FM_SEEK_START 0x31
#define SI468X_LEN_FM_SEEK_START 6

#define SI468X_CMD_FM_RSQ_STATUS 0x32
#define SI468X_LEN_FM_RSQ_STATUS 2
#define SI468X_RPL_FM_RSQ_STATUS 18

#define SI468X_CMD_FM_ACF_STATUS 0x33
#define SI468X_LEN_FM_ACF_STATUS 2
#define SI468X_RPL_FM_ACF_STATUS 7

#define SI468X_CMD_FM_RDS_STATUS 0x34
#define SI468X_LEN_FM_RDS_STATUS 2
#define SI468X_RPL_FM_RDS_STATUS 16

#define SI468X_CMD_FM_RDS_BLOCKCOUNT 0x35
#define SI468X_LEN_FM_RDS_BLOCKCOUNT 2
#define SI468X_RPL_FM_RDS_BLOCKCOUNT 6

// AM Mode commands
#define SI468X_CMD_AM_TUNE_FREQ 0x40
#define SI468X_LEN_AM_TUNE_FREQ 6

#define SI468X_CMD_AM_SEEK_START 0x41
#define SI468X_LEN_AM_SEEK_START 6

#define SI468X_CMD_AM_RSQ_STATUS 0x42
#define SI468X_LEN_AM_RSQ_STATUS 2
#define SI468X_RPL_AM_RSQ_STATUS 12

#define SI468X_CMD_AM_ACF_STATUS 0x43
#define SI468X_LEN_AM_ACF_STATUS 2
#define SI468X_RPL_AM_ACF_STATUS 5

// General digital mode commands
#define SI468X_CMD_GET_DIGITAL_SERVICE_LIST 0x80
#define SI468X_LEN_GET_DIGITAL_SERVICE_LIST 2
// contains just the number of bytes
#define SI468X_RPL_GET_DIGITAL_SERVICE_LIST_HDR 2

#define SI468X_CMD_START_DIGITAL_SERVICE 0x81
#define SI468X_LEN_START_DIGITAL_SERVICE 12

#define SI468X_CMD_STOP_DIGITAL_SERVICE 0x82
#define SI468X_LEN_STOP_DIGITAL_SERVICE 12

#define SI468X_CMD_GET_DIGITAL_SERVICE_DATA 0x84
#define SI468X_LEN_GET_DIGITAL_SERVICE_DATA 2
#define SI468X_RPL_GET_DIGITAL_SERVICE_DATA 20

// Digital service data types
// Standard data service, see DSCTy for details
#define SI468X_DIGITAL_SERVICE_TYPE_STD 0x00
// Non-DLS PAD, see DSCTy for details
#define SI468X_DIGITAL_SERVICE_TYPE_PAD 0x01
// DLS PAD, DSCTy will be zero
#define SI468X_DIGITAL_SERVICE_TYPE_DLS 0x02

// DAB mode commands
#define SI468X_CMD_DAB_TUNE_FREQ 0xB0
#define SI468X_LEN_DAB_TUNE_FREQ 6

#define SI468X_CMD_DAB_DIGRAD_STATUS 0xB2
#define SI468X_LEN_DAB_DIGRAD_STATUS 2
#define SI468X_RPL_DAB_DIGRAD_STATUS 19

#define SI468X_CMD_DAB_GET_EVENT_STATUS 0xB3
#define SI468X_LEN_DAB_GET_EVENT_STATUS 2
#define SI468X_RPL_DAB_GET_EVENT_STATUS 4

#define SI468X_CMD_DAB_GET_ENSEMBLE_INFO 0xB4
#define SI468X_LEN_DAB_GET_ENSEMBLE_INFO 2
#define SI468X_RPL_DAB_GET_ENSEMBLE_INFO 22

#define SI468X_CMD_DAB_GET_ANNOUNCEMENT_SUPPORT_INFO 0xB5
#define SI468X_LEN_DAB_GET_ANNOUNCEMENT_SUPPORT_INFO 8
#define SI468X_RPL_DAB_GET_ANNOUNCEMENT_SUPPORT_INFO 8

#define SI468X_CMD_DAB_GET_ANNOUNCEMENT_INFO 0xB6
#define SI468X_LEN_DAB_GET_ANNOUNCEMENT_INFO 1
#define SI468X_RPL_DAB_GET_ANNOUNCEMENT_INFO 12

#define SI468X_CMD_DAB_GET_SERVICE_LINKING_INFO 0xB7
#define SI468X_LEN_DAB_GET_SERVICE_LINKING_INFO 8
#define SI468X_RPL_DAB_GET_SERVICE_LINKING_INFO 12

#define SI468X_CMD_DAB_SET_FREQ_LIST 0xB8
#define SI468X_LEN_DAB_SET_FREQ_LIST_HDR 4

#define SI468X_CMD_DAB_GET_FREQ_LIST 0xB9
#define SI468X_LEN_DAB_GET_FREQ_LIST 2
#define SI468X_RPL_DAB_GET_FREQ_LIST_HDR 4

#define SI468X_CMD_DAB_GET_COMPONENT_INFO 0xBB
#define SI468X_LEN_DAB_GET_COMPONENT_INFO 12
#define SI468X_RPL_DAB_GET_COMPONENT_INFO 29

#define SI468X_CMD_DAB_GET_TIME 0xBC
#define SI468X_LEN_DAB_GET_TIME 2
#define SI468X_RPL_DAB_GET_TIME 7

#define SI468X_CMD_DAB_GET_AUDIO_INFO 0xBD
#define SI468X_LEN_DAB_GET_AUDIO_INFO 2
#define SI468X_RPL_DAB_GET_AUDIO_INFO 6

#define SI468X_CMD_DAB_GET_SUBCHAN_INFO 0xBE
#define SI468X_LEN_DAB_GET_SUBCHAN_INFO 12
#define SI468X_RPL_DAB_GET_SUBCHAN_INFO 8

#define SI468X_CMD_DAB_GET_FREQ_INFO 0xBF
#define SI468X_LEN_DAB_GET_FREQ_INFO 2
#define SI468X_RPL_DAB_GET_FREQ_INFO 16

#define SI468X_CMD_DAB_GET_SERVICE_INFO 0xC0
#define SI468X_LEN_DAB_GET_SERVICE_INFO 8
#define SI468X_RPL_DAB_GET_SERVICE_INFO 22

#define SI468X_CMD_DAB_GET_OE_SERVICES_INFO 0xC1
#define SI468X_LEN_DAB_GET_OE_SERVICES_INFO 8
#define SI468X_RPL_DAB_GET_OE_SERVICES_INFO 6

#define SI468X_CMD_DAB_ACF_STATUS 0xC2
#define SI468X_LEN_DAB_ACF_STATUS 2
#define SI468X_RPL_DAB_ADF_STATUS 6

// Test commands
#define SI468X_CMD_TEST_GET_RSSI 0xE5
#define SI468X_LEN_TEST_GET_RSSI 2
#define SI468X_RPL_TEST_GET_RSSI 2

#define SI468X_CMD_DAB_TEST_GET_BER_INFO 0xE8
#define SI468X_LEN_DAB_TEST_GET_BER_INFO 2
#define SI468X_RPL_DAB_TEST_GET_BER_INFO 8

// General properties for all modes

//Enable top level interrupt sources
#define SI468X_PROP_INT_CTL_ENABLE 0x0000
// seek / tune complete --> Safe for next Seek/Tune command
#define SI468X_MASK_INT_CTL_ENABLE_STCIEN 0x0001
// ACF information changed (FM mode)
#define SI468X_MASK_INT_CTL_ENABLE_ACFIEN 0x0002
// RDS information updated (FM mode)
#define SI468X_MASK_INT_CTL_ENABLE_RDSIEN 0x0004
// Received signal quality interrupt
#define SI468X_MASK_INT_CTL_ENABLE_RSQIEN 0x0008
// enabled data component needs attention --> GET_DIGITAL_SERVICE_DATA
#define SI468X_MASK_INT_CTL_ENABLE_DSRVIEN 0x0010
// digital radio link change interrupt --> DAB_DIGRAD_STATUS
#define SI468X_MASK_INT_CTL_ENABLE_DACQIEN 0x0020
// Command error
#define SI468X_MASK_INT_CTL_ENABLE_ERR_CMDIEN 0x0040
// Clear to send next command
#define SI468X_MASK_INT_CTL_ENABLE_CTSIEN 0x0080
// digital radio event change interrupt
#define SI468X_MASK_INT_CTL_ENABLE_DEVNTIEN 0x2000

#define SI468X_PROP_INT_CTL_REPEAT 0x0001
#define SI468X_PROP_DIGITAL_IO_OUTPUT_SELECT 0x0200
#define SI468X_PROP_DIGITAL_IO_OUTPUT_SAMPLE_RATE 0x0201
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT 0x0202
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVRDS1 0x0203
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVRDS2 0x0204
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVRDS3 0x0205
#define SI468X_PROP_DIGITAL_IO_OUTPUT_FORMAT_OVRDS4 0x0206
#define SI468X_PROP_AUDIO_ANALOG_VOLUME 0x0300
#define SI468X_PROP_AUDIO_MUTE 0x0301
#define SI468X_PROP_AUDIO_OUTPUT_CONFIG 0x0302
#define SI468X_PROP_PIN_CONFIG_ENABLE 0x0800
#define SI468X_PROP_WAKE_TONE_ENABLE 0x0900
#define SI468X_PROP_WAKE_TONE_PERIOD 0x0901
#define SI468X_PROP_WAKE_TONE_FREQ 0x0902
#define SI468X_PROP_WAKE_TONE_AMPLITUDE 0x0903
#define SI468X_PROP_TUNE_FRONTEND_VARM 0x1710 // slope
#define SI468X_PROP_TUNE_FRONTEND_VARB 0x1711 // intercept
#define SI468X_PROP_TUNE_FRONTEND_CFG 0x1712

// Properties for FM mode

#define SI468X_PROP_FM_SEEK_BAND_BOTTOM 0x3100
#define SI468X_PROP_FM_SEEK_BAND_TOP 0x3101
#define SI468X_PROP_FM_SEEK_FREQUENCY_SPACING 0x3102
#define SI468X_PROP_FM_VALID_MAX_TUNE_ERROR 0x3200
#define SI468X_PROP_FM_VALID_RSSI_TIME 0x3201
#define SI468X_PROP_FM_VALID_RSSI_THRESHOLD 0x3202
#define SI468X_PROP_FM_VALID_SNR_TIME 0x3203
#define SI468X_PROP_FM_VALID_SNR_THRESHOLD 0x3204
#define SI468X_PROP_FM_VALID_HDLEVEL_THRESHOLD 0x3206
#define SI468X_PROP_FM_RSQ_INTERRUPT_SOURCE 0x3300
#define SI468X_PROP_FM_RSQ_SNR_HIGH_THRESHOLD 0x3301
#define SI468X_PROP_FM_RSQ_SNR_LOW_THRESHOLD 0x3302
#define SI468X_PROP_FM_RSQ_RSSI_HIGH_THRESHOLD 0x3303
#define SI468X_PROP_FM_RSQ_RSSI_LOW_THRESHOLD 0x3304
#define SI468X_PROP_FM_RSQ_HD_DETECTION 0x3307
#define SI468X_PROP_FM_RSQ_HD_LEVEL_TIME_CONST 0x3308
#define SI468X_PROP_FM_RSQ_HDDETECTED_THD 0x3309
#define SI468X_PROP_FM_ACF_INTERRUPT_SOURCE 0x3400
#define SI468X_PROP_FM_ACF_SOFTMUTE_THRESHOLD 0x3401
#define SI468X_PROP_FM_ACF_HIGHCUT_THRESHOLD 0x3402
#define SI468X_PROP_FM_ACF_BLEND_THRESHOLD 0x3403
#define SI468X_PROP_FM_ACF_SOFTMUTE_TOLERANCE 0x3404
#define SI468X_PROP_FM_ACF_HIGHCUT_TOLERANCE 0x3405
#define SI468X_PROP_FM_ACF_BLEND_TOLERANCE 0x3406

// Properties for DAB mode

// Digital service interrupt source
#define SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE 0x8100
// Data package available --> GET DIGITAL_SERVICE_DATA
#define SI468X_MASK_DIGITAL_SERVICE_INT_SOURCE_DSRVPKTINT 0x0001
// Input buffer overflow --> reading digital service data too slow
#define SI468X_MASK_DIGITAL_SERVICE_INT_SOURCE_DSRVOVRFLINT 0x0002

#define SI468X_PROP_DIGITAL_SERVICE_RESTART_DELAY 0x8101
#define SI468X_PROP_DAB_DIGRAD_INTERRUPT_SOURCE 0xB000

#define SI468X_MASK_DAB_DIGRAD_INT_SOURCE_HARDMUTEINT 0x0010
#define SI468X_MASK_DAB_DIGRAD_INT_SOURCE_FICERRINT 0x0008
#define SI468X_MASK_DAB_DIGRAD_INT_SOURCE_ACQINT 0x0004
#define SI468X_MASK_DAB_DIGRAD_INT_SOURCE_RSSIHINT 0x0002
#define SI468X_MASK_DAB_DIGRAD_INT_SOURCE_RSSILINT 0x0001

#define SI468X_PROP_DAB_DIGRAD_RSSI_HIGH_THRESHOLD 0xB001
#define SI468X_PROP_DAB_DIGRAD_RSSI_LOW_THRESHOLD 0xB002
#define SI468X_PROP_DAB_VALID_RSSI_TIME 0xB200
#define SI468X_PROP_DAB_VALID_RSSI_THRESHOLD 0xB201
#define SI468X_PROP_DAB_VALID_ACQ_TIME 0xB202
#define SI468X_PROP_DAB_VALID_SYNC_TIME 0xB203
#define SI468X_PROP_DAB_VALID_DETECT_TIME 0xB204

// Event status change interrupt sources --> DAB_GET_EVENT_STATUS
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE 0xB300
// Digital service list interrupt
#define SI468X_MASK_DAB_EVENT_INTERRUPT_SOURCE_SRVLIST_INTEN 0x0001
// Frequency information interrupt
#define SI468X_MASK_DAB_EVENT_INTERRUPT_SOURCE_FREQINFO_INTEN 0x0002
// Reconfiguration warning interrupt
#define SI468X_MASK_DAB_EVENT_INTERRUPT_SOURCE_RECFGWRN_INTEN 0x0040
// Reconfiguration interrupt
#define SI468X_MASK_DAB_EVENT_INTERRUPT_SOURCE_RECFG_INTEN 0x0080

#define SI468X_PROP_DAB_EVENT_MIN_SVRLIST_PERIOD 0xB301
#define SI468X_PROP_DAB_EVENT_MIN_SVRLIST_PERIOD_RECONFIG 0xB302
#define SI468X_PROP_DAB_EVENT_MIN_FREQINFO_PERIOD 0xB303
// Select which PAD data application will be forwarded to the host
#define SI468X_PROP_DAB_XPAD_ENABLE 0xB400
// Enables PAD delivered DLS packets
#define SI468X_MASK_DAB_XPAD_ENABLE_DLS_ENABLE 0x0001
// Enables PAD delivered MOT packets
#define SI468X_MASK_DAB_XPAD_ENABLE_MOT_ENABLE 0x0002
// Enables PAD delivered TDC packets
#define SI468X_MASK_DAB_XPAD_ENABLE_TDC_ENABLE 0x0004

// Dynamic range control setting
#define SI468X_PROP_DAB_DRC_OPTION 0xB401
// Disable DRC function (no gain)
#define SI468X_MASK_DAB_DRC_OPTION_NONE 0x0000
// Apply 1/2 gain from specified value on PAD data
#define SI468X_MASK_DAB_DRC_OPTION_HALF 0x0001
// Apply full gain from specified value on PAD data
#define SI468X_MASK_DAB_DRC_OPTION_FULL 0x0010

#define SI468X_PROP_DAB_CTRL_DAB_MUTE_ENABLE 0xB500
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGNAL_LEVEL_THRESHOLD 0xB501
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_WIN_THRESHOLD 0xB502
#define SI468X_PROP_DAB_CTRL_DAB_UNMUTE_WIN_THRESHOLD 0xB503
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGLOSS_THRESHOLD 0xB504
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGLOW_THRESHOLD 0xB505
#define SI468X_PROP_DAB_TEST_BER_CONFIG 0xE800

typedef struct SI468xDabDigradStatus {
	// Answer bte 0 (from LSB to MSB)
	uint8_t rssiLInt : 1, // RSSI below DAB_DIGRAD_RSSI_LOW_THRESHOLD
		rssiHInt : 1, // RSSI above DAB_DIGRAD_RSSI_HIGH_THRESHOLD
		acquiredInt : 1, // Ensemble is acquired interrupt
		ficErrInt : 1, // Fast Information Channel error interrupt
		hardMuteInt : 1, // Hard muting interrupt
		rfu1 : 3; // reserved for future use - fill bits
	// Answer byte 0 (from LSB to MSB)
	uint8_t valid : 1, // Ensemble is valid
		rfu2 : 1, // reserved for future use - fill bit
		acquired : 1, // Ensemble is acquired
		ficErr : 1, // Fast Information Channel error
		hardMute : 1, // Hard muting active
		rfu3 : 3; // reserved for future use - fill bits

	uint8_t rssi; // Received signal strength indicator
	uint8_t snr; // Signal to noise ratio
	uint8_t ficQuality; // Fast Information Channel quality
	uint8_t cnr; // Ratio of OFDM signal level
	uint16_t fibErrors; // Fast Information Block errors
	uint32_t tunedFreq; // Tuned frequency
	uint8_t tuneIndex; // Frequency index
	uint8_t fftOffset; // FFT offset
	uint16_t antCap; // Antenna tuning capacitor value
	uint16_t cuLevel; // Capacity units level
	uint8_t fastDect; // Fast detect
} SI468xDabDigradStatus;

typedef struct SI468xDabEventStatus {
	// Answer byte 0 (from LSB to MSB)
	uint8_t svrListInt : 1, // New service list version available
		freqInfoInt : 1, // New frequency information --> dabGetFreqInfo()
		// --> dabGetDigitalServiceList()
		rfu1 : 1, // reserved for future use - fill bit
		annoInt : 1, // New announcement information available
		// --> dabGetAnnouncementInfo()
		rfu2 : 2, // reserved for future use - fill bits
		recfgWarnInt : 1, // Ensemble will be reconfigured within 6 seconds
		recfgInt : 1; // Ensemble has been reconfigured
	// Answer byte 1 (from LSB to MSB)
	uint8_t svrList : 1, // Service list available --> dabGetDigitalServiceList()
		freqInfo : 1, // Frequency information available --> dabGetFreqInt()
		rfu3 : 6; // reserved for future use - fill bits

	uint16_t listVersion; // Current service list version
} SI468xDabEventStatus;

typedef struct SI468xServiceComponent {
	uint16_t id; // Component identifier
	// Component info byte (bitmasks from LSB to MSB)
	uint8_t isCa : 1, // access control applies on component
		isSecondary : 1, // 0 is primary component; 1: is secondary component
		serviceType : 6; // Audio/Data service component type
	// Valid flags (bitmasks from LSB to MSB)
	uint8_t uaInfoValid : 1, // User application information is valid
		rfu1 : 7; // Bits reserved for future use
} SI468xServiceComponent;

typedef struct SI468xDigitalService {
	uint32_t id; // Service ID
	// Service Info 1 byte (bitmasks from LSB to MSB)
	uint8_t pdFlag : 1, // 0: Program (audio); 1: Data service
		pty : 5, // Program type identifier: ETSI TS 101 756 Annex A
		srvLnkInfo : 1, // Service linking information available for this service
		rfu1 : 1; // Bit reserved for future use
	// Service Info 2 byte (bitmasks from LSB to MSB)
	uint8_t nComponents : 4, // number of available service components
		caId : 3, // Type of access control used - if any
		local : 1; // Program only available in a part of the service area
	// Service Info 3 byte (bitmasks from LSB to MSB)
	uint8_t siCharset : 4, // used character set for this service --> ETSI TS 101 756, clause 5.3
		rfu2 : 4; // Bits reserved for future use

	uint8_t rfu3; // Reserved for future use / alignment byte
	char label[16]; // Service label
} SI468xDigitalService;

typedef struct SI468xDigitalServiceList {
	uint16_t size; // Size of the complete downloaded service list in bytes
	uint16_t version; // list version
	uint8_t nServices; // number of provided services
	uint8_t rfu1; // Empty fill byte for data alignment
	uint16_t rfu2; // Empty fill word (2 bytes) for data alignment
} SI468xDigitalServiceList;

typedef struct SI468xDateTime {
	uint16_t year; // Year (eg. 2000)
	uint8_t month; // Month (1..12)
	uint8_t day; // Day (1..31)
	uint8_t hour; // Hour (0..24)
	uint8_t minute; // Minute (0..60)
	uint8_t second; // Second (0..61)
} SI468xDateTime;

SI468xErr si468xReadReply(SI468x *pInst, uint16_t nBytes, uint8_t *buffer);
SI468xErr si468xWaitForCts(SI468x *pInst, uint16_t nBytes, uint8_t *buffer);
SI468xErr si468xReadOffset(SI468x *pInst, uint16_t offset, uint16_t nBytes,
			   uint8_t *buffer);
SI468xErr si468xPowerUp(SI468x *pInst);
SI468xErr si468xLoadInit(SI468x *pInst);
SI468xErr si468xHostLoad(SI468x *pInst, const uint8_t *buffer, uint16_t size);
SI468xErr si468xFlashLoad(SI468x *pInst, uint32_t startAddr);
SI468xErr si468xBoot(SI468x *pInst);
SI468xErr si468xGetSysState(SI468x *pInst, SI468xImage *image);
SI468xErr si468xSetProperty(SI468x *pInst, uint16_t propId, uint16_t propVal);
SI468xErr si468xGetProperty(SI468x *pInst, uint16_t propId, uint16_t *propVal);

SI468xErr si468xDabGetFreqList(SI468x *pInst, uint8_t *nFreqs, uint32_t *buffer,
			       uint8_t maxFreqs);
SI468xErr si468xDabTuneFreq(SI468x *pInst, uint8_t freqIndex, uint16_t antCap);
SI468xErr si468xDabDigradStatus(SI468x *pInst, uint8_t digradAck,
				uint8_t stcAck, uint8_t atTune,
				SI468xDabDigradStatus *status);
SI468xErr si468xDabGetEventStatus(SI468x *pInst, uint8_t digradAck,
				  SI468xDabEventStatus *evtStatus);
SI468xErr si468xDabGetDigitalServiceList(SI468x *pInst,
					 SI468xDigitalServiceList *serviceList,
					 uint16_t maxSize);
SI468xErr si468xDabStartDigitalService(SI468x *pInst, uint8_t startStop,
				       uint32_t serviceId, uint16_t compId);
SI468xErr si468xGetDigitalServiceData(SI468x *pInst, uint8_t statusOnly,
				      ServiceData *serviceData);
SI468xErr si468xDabGetTime(SI468x *pInst, SI468xDateTime *dateTime,
			   uint8_t local);

// Helper functions for service list access
SI468xDigitalService *si468xDabGetService(SI468xDigitalServiceList *svcList,
					  uint8_t svcNum);
SI468xServiceComponent *si468xDabGetServiceComp(SI468xDigitalService *service,
						uint8_t compNum);

SI468xErr si468xFlashSetPropList(SI468x *pInst, uint8_t *propList,
				 uint16_t size);
SI468xErr si468xFlashEraseChip(SI468x *pInst);
SI468xErr si468xFlashProgramImage(SI468x *pInst, uint32_t addr, uint8_t *buffer,
				  uint16_t size);

#define si468xAnyInt(_pInst_)                                                  \
	((_pInst_)->status.stcInt || (_pInst_)->status.acfInt ||               \
	 (_pInst_)->status.rdsInt || (_pInst_)->status.rsqInt ||               \
	 (_pInst_)->status.dacqInt || (_pInst_)->status.devntInt)

#endif // __SI468X_COMMANDS_H__
