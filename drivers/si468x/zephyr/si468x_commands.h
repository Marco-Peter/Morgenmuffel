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

#include <device.h>
#include <drivers/spi.h>

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

// Test commands
#define SI468X_CMD_TEST_GET_RSSI 0xE5
#define SI468X_LEN_TEST_GET_RSSI 2
#define SI468X_RPL_TEST_GET_RSSI 2

#define SI468X_CMD_DAB_TEST_GET_BER_INFO 0xE8
#define SI468X_LEN_DAB_TEST_GET_BER_INFO 2
#define SI468X_RPL_DAB_TEST_GET_BER_INFO 8

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

enum si468x_image {
	si468x_IMG_BOOTLOADER = 0,
	si468x_IMG_FMHD = 1,
	si468x_IMG_DAB = 2,
	si468x_IMG_TDMB = 3,
	si468x_IMG_FMHD_DEMOD = 4,
	si468x_IMG_AMHD = 5,
	si468x_IMG_AMHD_DEMOD = 6,
	si468x_IMG_DAB_DEMOD = 7
};

struct si468x_events {
	bool dacqint;
	bool dsrvint;
	bool stcint;
	bool devntint;
};
/* old structs */
struct si468x_dabDigradStatus {
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
};

struct si468x_dabEventStatus {
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
};

struct si468x_serviceComponent {
	uint16_t id; // Component identifier
	// Component info byte (bitmasks from LSB to MSB)
	uint8_t isCa : 1, // access control applies on component
		isSecondary : 1, // 0 is primary component; 1: is secondary component
		serviceType : 6; // Audio/Data service component type
	// Valid flags (bitmasks from LSB to MSB)
	uint8_t uaInfoValid : 1, // User application information is valid
		rfu1 : 7; // Bits reserved for future use
};

struct si468x_digitalService {
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
};

struct si468x_digitalServiceList {
	uint16_t size; // Size of the complete downloaded service list in bytes
	uint16_t version; // list version
	uint8_t nServices; // number of provided services
	uint8_t rfu1; // Empty fill byte for data alignment
	uint16_t rfu2; // Empty fill word (2 bytes) for data alignment
};

struct si468x_serviceData {
	uint32_t serviceId;
	uint32_t compId;
	uint16_t byteCount;
	uint16_t segNum;
	uint16_t numSegs;
	uint8_t srvState;
	union {
		struct {
			uint8_t dscType : 6, dataSrc : 2;
		} comps;
		uint8_t byte;
	} type;
	uint8_t *payload;
};

struct si468x_dateTime {
	uint16_t year; // Year (eg. 2000)
	uint8_t month; // Month (1..12)
	uint8_t day; // Day (1..31)
	uint8_t hour; // Hour (0..24)
	uint8_t minute; // Minute (0..60)
	uint8_t second; // Second (0..61)
};

/* common commands implemented in si468x_commands.c */
int si468x_send_command(const struct device *dev,
			const struct spi_buf_set *spi_buf_set);
int si468x_cmd_rd_reply(const struct device *dev,
			const struct spi_buf_set *spi_buf_set,
			struct si468x_events *events);
int si468x_cmd_powerup(const struct device *dev);
int si468x_cmd_load_init(const struct device *dev);
int si468x_cmd_host_load(const struct device *dev, const uint8_t *buffer,
			 uint16_t len);
int si468x_cmd_flash_load(const struct device *dev, uint32_t start_addr);
int si468x_cmd_boot(const struct device *dev);
int si468x_cmd_get_sys_state(const struct device *dev,
			     enum si468x_image *image);
int si468x_cmd_set_property(const struct device *dev, uint16_t id,
			    uint16_t val);

/* DAB specific commands implemented in si468x_commands_dab.c */
int si468x_cmd_dab_tune(const struct device *dev, uint8_t channel,
			uint16_t ant_cap);
int si468x_cmd_dab_start_service(const struct device *dev, uint16_t service_id,
				 uint8_t component_id);
int si468x_cmd_dab_get_freq_list(const struct device *dev, uint8_t *num_freqs);

/* FMHD specific commands implemented in si468x_commands_fmhd.c */

/* AM specific commands implemented in si468x_commands_am.c */

/* old commands TODO: To be removed when finished */
int si468xReadReply(const struct device *dev, uint8_t *buffer);
int si468xWaitForCts(const struct device *dev, uint16_t nBytes,
		     uint8_t *buffer);
int si468xReadOffset(const struct device *dev, uint16_t offset, uint16_t nBytes,
		     uint8_t *buffer);
int si468xGetProperty(const struct device *dev, uint16_t propId,
		      uint16_t *propVal);

int si468xDabGetFreqList(const struct device *dev, uint8_t *nFreqs,
			 uint32_t *buffer, uint8_t maxFreqs);
int si468xDabDigradStatus(const struct device *dev, uint8_t digradAck,
			  uint8_t stcAck, uint8_t atTune,
			  struct si468x_dabDigradStatus *status);
int si468xDabGetEventStatus(const struct device *dev, uint8_t digradAck,
			    struct si468x_dabEventStatus *evtStatus);
int si468xDabGetDigitalServiceList(const struct device *dev,
				   struct si468x_digitalServiceList *serviceList,
				   uint16_t maxSize);
int si468xDabStartDigitalService(const struct device *dev, uint8_t startStop,
				 uint32_t serviceId, uint16_t compId);
int si468xGetDigitalServiceData(const struct device *dev, uint8_t statusOnly,
				struct si468x_serviceData *serviceData);
int si468xDabGetTime(const struct device *dev, struct si468x_dateTime *dateTime,
		     uint8_t local);

// Helper functions for service list access
struct si468x_digitalService *
si468xDabGetService(struct si468x_digitalServiceList *svcList, uint8_t svcNum);
struct si468x_serviceComponent *
si468xDabGetServiceComp(struct si468x_digitalService *service, uint8_t compNum);

int si468xFlashSetPropList(const struct device *dev, uint8_t *propList,
			   uint16_t size);
int si468xFlashEraseChip(const struct device *dev);
int si468xFlashProgramImage(const struct device *dev, uint32_t addr,
			    uint8_t *buffer, uint16_t size);

#define si468xAnyInt(_pInst_)                                                  \
	((_pInst_)->status.stcInt || (_pInst_)->status.acfInt ||               \
	 (_pInst_)->status.rdsInt || (_pInst_)->status.rsqInt ||               \
	 (_pInst_)->status.dacqInt || (_pInst_)->status.devntInt)

#endif // __SI468X_COMMANDS_H__
