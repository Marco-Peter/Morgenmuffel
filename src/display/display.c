/*
 * display.c
 *
 *  Created on: 08.01.2017
 *      Author: marco
 */

#include <zephyr.h>
#include <errno.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include "parameters.h"
#include "hardwareconfig.h"
#include "display.h"

// Fundamental display commands
#define DISP_CMD_SET_CONTRAST 0x81 // second byte defines contrast
#define DISP_FULL_ON 0xA4 // LSB defines on/off (0xA4: normal display, 0xA5: full on)
#define DISP_INVERSE 0xA6 // LSB defines inversion state (0xA6: normal, 0xA7: inverse)
#define DISP_CMD_ON ((u8_t)0xAF) // Switch on display driver
#define DISP_CMD_OFF ((u8_t)0xAE) // Switch off display driver
#define DISP_NOP 0xE3 // No operation (for whatever...)
#define DISP_LOCK 0xFD // second byte = 0x12: Unlock, second byte = 0x16: lock

// Scrolling display commands not implemented at the moment
#define DISP_SCRL_HORIZ 0x26 // LSB defines right/left (0x26: right, 0x27: left)
// second byte: 0x00 (dummy byte)
// third byte: start page address (0..7)
// fourth byte: scroll frequency (in frames)
#define DISP_SCRL_VERT 0x29 // LSB defines right/left (0x29: right, 0x2A: left)
// second byte: 0x00: No horizontal scroll, 0x01 horizontal scroll
// third byte: start page address (0..7)
// fourth byte: scroll frequency (in frames)
// fifth byte: end page address (0..7)
// sixth byte: vertical scrolling offset (0.63 rows)
// seventh byte: start column (0..256)
// eighth byte: end column (0..256)
#define DISP_SCRL_ACTIV 0x2E // Activate scrolling: 2F, stop scrolling: 2E
// (RAM needs to be rewritten after scrolling)

#define DISP_SCRL_SPD_1FRM 0x7 // scrolling at every frame
#define DISP_SCRL_SPD_4FRM 0x6 // scrolling at every 4th frame
#define DISP_SCRL_SPD_3FRM 0x5 // scrolling at every 3rd frame
#define DISP_SCRL_SPD_2FRM 0x4 // scrolling at every 2nd frame
#define DISP_SCRL_SPD_256FRM 0x3 // scrolling at every 256th frame
#define DISP_SCRL_SPD_128FRM 0x2 // scrolling at every 128th frame
#define DISP_SCRL_SPD_64FRM 0x1 // scrolling at every 64th frame
#define DISP_SCRL_SPD_5FRM 0x0 // scrolling at every 5th frame

#define DISP_SET_VERT_SCRL_AREA 0xA3 // Set vertical scroll area
// 2nd byte: Number of rows in top fixed area (0..63)
// 3rd byte: Number of rows used for scrolling (0..64)
#define DISP_SCRL_CONT 0x2C // 0x2C: right, 0x2D: left
// 2nd byte: 0x00 (dummy byte)
// 3rd byte: start page address (0..7)
// 4th byte: 0x00 (dummy byte)
// 5th byte: end page address (0..7)
// 6th byte: 0x00 (dummy byte)
// 7th byte: start column (0..128)
// 8th byte: end column (0..128)

// Addressing commands
#define DISP_ADDR_LOW_COL 0x00 // Set lower column start address (0x00..0x0F)
#define DISP_ADDR_HIGH_COL 0x10 // Set lower column start address (0x10..0x1F)
#define DISP_ADDR_MODE 0x20 // Set memory addressing mode

// second byte values
#define DISP_ADDR_HORIZ 0x00 // horizontal addressing mode
#define DISP_ADDR_VERT 0x01 // vertical addressing mode
#define DISP_ADDR_PAGE 0x10 // page addressing mode

#define DISP_COL_ADDR 0x21 // Set column address
// 2nd byte: column address (0..127)

#define DISP_PAGE_ADDR 0xB0 // Set page address (0..7, B0 = 0, B7 = 7)

#define DISP_PAGE_STRT_ADDR 0xB0 // Set page start address using three LSB

// Hardware configuration commands
#define DISP_START_LINE 0x40 // Set display start line register from 0..63
#define DISP_CMD_MIRROR_VERT ((u8_t)0xA1) // Mirror vertically
#define DISP_CMD_NO_MIRROR_VERT ((u8_t)0xA0) // Do not mirror vertically
#define DISP_MUX_RATIO 0xA8 // Set multiplex ratio
// 2nd byte: Multiplex ration(16..64, Mux ratio = N + 1)

#define DISP_CMD_MIRROR_HORIZ ((u8_t)0xC8) // Mirror horizontally
#define DISP_CMD_NO_MIRROR_HORIZ ((u8_t)0xC0) // Do not mirror horizontally
#define DISP_OFFSET 0xD3 // Set display offset
// 2nd byte: vertical shift (0..63)

#define DISP_T_PRCHRG 0xD9 // Set precharge period
// 2nd byte: Low nibble: Phase 1 period, high nibble: Phase 2 period

#define DISP_LVL_DESELECT 0xDB // Set Vcomh deselect level
// 2nd byte: Deselect level

static int writeCmd(uint8_t* cmd, size_t len);

/**************************************************************************/ /**
 * Check the return code variable rc and return it if it is not zero
 *
 *****************************************************************************/
#define checkRc() \
    if (rc != 0)  \
    return rc

static struct device* spiDevice = NULL;
static struct spi_config displayConfig = {
    .frequency = 10000000,
    .operation = SPI_WORD_SIZE_GET(8) | SPI_LINES_SINGLE,
    .slave = 0,
    NULL
};

/**************************************************************************/ /**
 * Initialises the display
 *
 * @return 0 if successful, otherwise an error code
 *
 *****************************************************************************/
int displayInit(void)
{
    struct device* rstPort;
    struct device* dcPort;
    struct device* powerPort;
    uint32_t rc;
    uint8_t cmd;

    spiDevice = device_get_binding(SPI_PERIPH);

    /* Initialise the used GPIO pins */
    rstPort = device_get_binding(DISPLAY_RST_GPIO_Port);
    rc = gpio_pin_configure(rstPort, DISPLAY_RST_Pin, GPIO_DIR_OUT | GPIO_POL_INV);
    checkRc();

    dcPort = device_get_binding(DISPLAY_DC_GPIO_Port);
    rc = gpio_pin_configure(dcPort, DISPLAY_DC_Pin, GPIO_DIR_OUT);
    checkRc();

    powerPort = device_get_binding(POWER_13V_GPIO_Port);
    rc = gpio_pin_configure(powerPort, POWER_13V_Pin, GPIO_DIR_OUT);
    checkRc();

    /* Take the display controller into reset for 50 ms*/
    rc = gpio_pin_write(rstPort, DISPLAY_RST_Pin, 1);
    k_sleep(50);
    rc = gpio_pin_write(rstPort, DISPLAY_RST_Pin, 0);
    checkRc();

    /* Set up the display configuration */
    cmd = DISPLAY_MIRROR_HORIZONTAL ? DISP_CMD_MIRROR_HORIZ : DISP_CMD_NO_MIRROR_HORIZ;
    rc = writeCmd(&cmd, sizeof(cmd));
    checkRc();

    cmd = DISPLAY_MIRROR_VERTICAL ? DISP_CMD_MIRROR_VERT : DISP_CMD_NO_MIRROR_VERT;
    rc = writeCmd(&cmd, sizeof(cmd));
    checkRc();

    // Clear the display memory
    rc = displayClear();
    checkRc();

    // The display is now ready to be powered on
    return 0;
}

/**************************************************************************/ /**
 * Switches on the display
 *
 * @return 0 if successful, otherwise an error code
 *
 *****************************************************************************/
int displayOn(void)
{
    struct device* powerPort;
    int rc;
    uint8_t cmd;

    // switch on the high voltage power supply and wait a short moment
    powerPort = device_get_binding(POWER_13V_GPIO_Port);
    rc = gpio_pin_write(powerPort, POWER_13V_Pin, 1);
    checkRc();

    k_sleep(10);

    /* Switch on the display driver */
    cmd = DISP_CMD_ON;
    rc = writeCmd(&cmd, sizeof(cmd));

    return rc;
}

/**************************************************************************/ /**
 * Switches off the display
 *
 * @return 0 if successful, otherwise an error code
 *
 *****************************************************************************/
int displayOff(void)
{
    struct device* powerPort;
    uint32_t rc;
    uint8_t cmd;

    // switch off the display drivers and wait a short moment
    cmd = DISP_CMD_OFF;
    rc = writeCmd(&cmd, sizeof(cmd));
    checkRc();

    // switch off the high voltage power supply
    powerPort = device_get_binding(POWER_13V_GPIO_Port);
    rc = gpio_pin_write(powerPort, POWER_13V_Pin, 0);

    return rc;
}

/**************************************************************************/ /**
 * Set a contrast level of the display
 *
 * @param contrast	Contrast to set
 * @return 0 if successful, otherwise an error code
 *
 *****************************************************************************/
int displaySetContrast(uint8_t contrast)
{
    u32_t rc;
    u8_t cmd[2] = {
        DISP_CMD_SET_CONTRAST,
        contrast
    };

    rc = writeCmd(cmd, sizeof(cmd));

    return rc;
}

/**************************************************************************/ /**
 * Set a number of display memory bytes on the current page to a certain value.
 *
 * @param value		Value to be preset
 * @param len		Number of columns to fill
 * @return			An error code
 *
 *****************************************************************************/
int displayMemset(uint8_t value, size_t len)
{
    int rc;

    if (len == 0) {
        return 0;
    }

    if (len > 128) {
        return -EINVAL;
    }

    struct spi_buf spiBuf = {
        .buf = NULL,
        .len = len
    };
    struct spi_buf_set spiBufSet = {
        .buffers = &spiBuf,
        .count = 1
    };

    rc = spi_write(spiDevice, &displayConfig, &spiBufSet);

    return rc;
}

/**************************************************************************/ /**
 * Set a position to draw on the display.
 *
  * @param posX	Horizontal position.
 * @param posY	Vertical position.
 * @return The actual Y position rounded to a page or an error code.
 *
 *****************************************************************************/
int displaySetPos(uint8_t posX, uint8_t posY)
{
    int rc;
    uint8_t page = posY / DISPLAY_PAGESIZE;
    uint8_t cmd = DISP_PAGE_ADDR | page;

    rc = writeCmd(&cmd, sizeof(cmd));
    checkRc();

    cmd = DISP_ADDR_LOW_COL | (posX % 16);
    rc = writeCmd(&cmd, sizeof(cmd));
    checkRc();

    cmd = DISP_ADDR_HIGH_COL | (posX / 16);
    rc = writeCmd(&cmd, sizeof(cmd));
    checkRc();

    return page * DISPLAY_PAGESIZE;
}

/**************************************************************************/ /**
 * Clears the complete display
 *
 * @return DISPLAY_OK if successful, otherwise an error code
 *
 *****************************************************************************/
int displayClear(void)
{
    int i, rc;
    uint8_t cmd;

    for (i = 0; i < 8; ++i) {
        cmd = DISP_PAGE_ADDR | i;
        rc = writeCmd(&cmd, sizeof(cmd));
        checkRc();

        rc = displayMemset(0x00, 128);
        checkRc();
    }
    return 0;
}

/**************************************************************************/ /**
 * Internal function to write a command to the display controller
 *
 * @param pInst Display instance
 * @param cmd Command to transmit
 * @param len Number of bytes to transmit
 * @return DISPLAY_OK if successful, otherwise an error code
 *
 *****************************************************************************/
static int writeCmd(uint8_t* cmd, size_t len)
{
    int rc;
    struct device* dcPort = device_get_binding(DISPLAY_DC_GPIO_Port);

    // Pull D/C line to 0 to indicate a command
    rc = gpio_pin_write(dcPort, DISPLAY_DC_Pin, 0);
    checkRc();

    // Write the data to the SPI interface
    struct spi_buf spiBuf = {
        .buf = cmd,
        .len = len
    };
    struct spi_buf_set spiBufSet = {
        .buffers = &spiBuf,
        .count = 1
    };

    rc = spi_write(spiDevice, &displayConfig, &spiBufSet);
    checkRc();

    // Pull D/C line back to 1
    rc = gpio_pin_write(dcPort, DISPLAY_DC_Pin, 1);
    checkRc();

    return 0;
}
