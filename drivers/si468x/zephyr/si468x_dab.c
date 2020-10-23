/*
 * Copyright (c) 2020 Marco Peter
 */

#include "si468x_private.h"
#include "si468x_commands.h"
#include <logging/log.h>

LOG_MODULE_REGISTER(si468x_commands_dab, LOG_LEVEL_DBG);

// Digital service interrupt source
#define SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE 0x8100
// Data package available --> GET DIGITAL_SERVICE_DATA
#define SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE_DSRVPKTINT 0x0001
// Input buffer overflow --> reading digital service data too slow
#define SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE_DSRVOVRFLINT 0x0002

#define SI468X_PROP_DIGITAL_SERVICE_RESTART_DELAY 0x8101
#define SI468X_PROP_DAB_DIGRAD_INTERRUPT_SOURCE 0xB000

#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_HARDMUTEINT 0x0010
#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_FICERRINT 0x0008
#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_ACQINT 0x0004
#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_RSSIHINT 0x0002
#define SI468X_PROP_DAB_DIGRAD_INT_SOURCE_RSSILINT 0x0001

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
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_SRVLIST_INTEN 0x0001
// Frequency information interrupt
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_FREQINFO_INTEN 0x0002
// Reconfiguration warning interrupt
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_RECFGWRN_INTEN 0x0040
// Reconfiguration interrupt
#define SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_RECFG_INTEN 0x0080

#define SI468X_PROP_DAB_EVENT_MIN_SVRLIST_PERIOD 0xB301
#define SI468X_PROP_DAB_EVENT_MIN_SVRLIST_PERIOD_RECONFIG 0xB302
#define SI468X_PROP_DAB_EVENT_MIN_FREQINFO_PERIOD 0xB303
// Select which PAD data application will be forwarded to the host
#define SI468X_PROP_DAB_XPAD_ENABLE 0xB400
// Enables PAD delivered DLS packets
#define SI468X_PROP_DAB_XPAD_ENABLE_DLS_ENABLE 0x0001
// Enables PAD delivered MOT packets
#define SI468X_PROP_DAB_XPAD_ENABLE_MOT_ENABLE 0x0002
// Enables PAD delivered TDC packets
#define SI468X_PROP_DAB_XPAD_ENABLE_TDC_ENABLE 0x0004

// Dynamic range control setting
#define SI468X_PROP_DAB_DRC_OPTION 0xB401
// Disable DRC function (no gain)
#define SI468X_PROP_DAB_DRC_OPTION_NONE 0x0000
// Apply 1/2 gain from specified value on PAD data
#define SI468X_PROP_DAB_DRC_OPTION_HALF 0x0001
// Apply full gain from specified value on PAD data
#define SI468X_PROP_DAB_DRC_OPTION_FULL 0x0010

#define SI468X_PROP_DAB_CTRL_DAB_MUTE_ENABLE 0xB500
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGNAL_LEVEL_THRESHOLD 0xB501
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_WIN_THRESHOLD 0xB502
#define SI468X_PROP_DAB_CTRL_DAB_UNMUTE_WIN_THRESHOLD 0xB503
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGLOSS_THRESHOLD 0xB504
#define SI468X_PROP_DAB_CTRL_DAB_MUTE_SIGLOW_THRESHOLD 0xB505
#define SI468X_PROP_DAB_TEST_BER_CONFIG 0xE800

int si468x_dab_startup(const struct device *dev)
{
	int rc;

	rc = si468x_cmd_set_property(dev, SI468X_PROP_TUNE_FRONTEND_VARM,
				     IS_ENABLED(SI468X_VHFSW_DAB));
	if (rc != 0) {
		LOG_ERR("%s: failed to set dab varactor slope with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev, SI468X_PROP_TUNE_FRONTEND_VARM,
				     CONFIG_SI468X_VARACTOR_SLOPE_DAB);
	if (rc != 0) {
		LOG_ERR("%s: failed to set dab varactor slope with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev, SI468X_PROP_TUNE_FRONTEND_VARM,
				     CONFIG_SI468X_VARACTOR_INTCP_DAB);
	if (rc != 0) {
		LOG_ERR("%s: failed to set dab varactor intercept with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev, SI468X_PROP_DAB_VALID_RSSI_THRESHOLD,
				     CONFIG_SI468X_DAB_VALID_RSSI_THRESHOLD);
	if (rc != 0) {
		LOG_ERR("%s: failed to set dab valid RSSI threshold with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_set_property(
		dev, SI468X_PROP_INT_CTL_ENABLE,
		SI468X_PROP_INT_CTL_ENABLE_STCIEN |
			SI468X_PROP_INT_CTL_ENABLE_DSRVIEN |
			SI468X_PROP_INT_CTL_ENABLE_DACQIEN |
			SI468X_PROP_INT_CTL_ENABLE_DEVNTIEN);
	if (rc != 0) {
		LOG_ERR("%s: failed to set dab interrupts with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_set_property(
		dev, SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE,
		SI468X_PROP_INT_CTL_ENABLE_STCIEN |
			SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE_DSRVOVRFLINT |
			SI468X_PROP_DIGITAL_SERVICE_INT_SOURCE_DSRVPKTINT);
	if (rc != 0) {
		LOG_ERR("%s: failed to set dab digital service interrupts with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev,
				     SI468X_PROP_DAB_DIGRAD_INTERRUPT_SOURCE,
				     SI468X_PROP_DAB_DIGRAD_INT_SOURCE_ACQINT);
	if (rc != 0) {
		LOG_ERR("%s: failed to set dab digrad interrupt sources with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_set_property(
		dev, SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE,
		SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_SRVLIST_INTEN |
			SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_FREQINFO_INTEN |
			SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_RECFGWRN_INTEN |
			SI468X_PROP_DAB_EVENT_INTERRUPT_SOURCE_RECFG_INTEN);
	if (rc != 0) {
		LOG_ERR("%s: failed to set dab event interrupt sources with rc %d",
			dev->name, rc);
		return rc;
	}
	rc = si468x_cmd_set_property(dev, SI468X_PROP_DAB_DRC_OPTION,
				     SI468X_PROP_DAB_DRC_OPTION_FULL);
	if (rc != 0) {
		LOG_ERR("%s: failed to set dab drc with rc %d", dev->name, rc);
		return rc;
	}
	return 0;
}
