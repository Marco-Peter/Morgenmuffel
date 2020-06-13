/*
 * display.c
 *
 *  Created on: 08.01.2017
 *      Author: marco
 */

#include "display.h"
#include <string.h>
#include <stdlib.h>
#include <drivers/spi.h>

// Fundamental display commands
#define DISP_SET_CONTRAST 0x81 // second byte defines contrast
#define DISP_FULL_ON                                                           \
    0xA4 // LSB defines on/off (0xA4: normal display, 0xA5: full on)
#define DISP_INVERSE                                                           \
    0xA6 // LSB defines inversion state (0xA6: normal, 0xA7: inverse)
#define DISP_ON   0xAE // LSB defines ON-state (0xAE: off, 0xAF: on)
#define DISP_NOP  0xE3 // No operation (for whatever...)
#define DISP_LOCK 0xFD // second byte = 0x12: Unlock, second byte = 0x16: lock

// Scrolling display commands not implemented at the moment
#define DISP_SCRL_HORIZ                                                        \
    0x26 // LSB defines right/left (0x26: right, 0x27: left)                   \
            // second byte: 0x00 (dummy byte)                                  \
            // third byte: start page address (0..7)                           \
            // fourth byte: scroll frequency (in frames)
#define DISP_SCRL_VERT                                                         \
    0x29 // LSB defines right/left (0x29: right, 0x2A: left)                   \
            // second byte: 0x00: No horizontal scroll, 0x01 horizontal scroll \
            // third byte: start page address (0..7)                           \
            // fourth byte: scroll frequency (in frames)                       \
            // fifth byte: end page address (0..7)                             \
            // sixth byte: vertical scrolling offset (0.63 rows)               \
            // seventh byte: start column (0..256)                             \
            // eighth byte: end column (0..256)
#define DISP_SCRL_ACTIV                                                        \
    0x2E // Activate scrolling: 2F, stop scrolling: 2E                         \
            // (RAM needs to be rewritten after scrolling)

#define DISP_SCRL_SPD_1FRM   0x7 // scrolling at every frame
#define DISP_SCRL_SPD_4FRM   0x6 // scrolling at every 4th frame
#define DISP_SCRL_SPD_3FRM   0x5 // scrolling at every 3rd frame
#define DISP_SCRL_SPD_2FRM   0x4 // scrolling at every 2nd frame
#define DISP_SCRL_SPD_256FRM 0x3 // scrolling at every 256th frame
#define DISP_SCRL_SPD_128FRM 0x2 // scrolling at every 128th frame
#define DISP_SCRL_SPD_64FRM  0x1 // scrolling at every 64th frame
#define DISP_SCRL_SPD_5FRM   0x0 // scrolling at every 5th frame

#define DISP_SET_VERT_SCRL_AREA                                                \
    0xA3 // Set vertical scroll area                                           \
            // 2nd byte: Number of rows in top fixed area (0..63)              \
            // 3rd byte: Number of rows used for scrolling (0..64)
#define DISP_SCRL_CONT                                                         \
    0x2C // 0x2C: right, 0x2D: left                                            \
            // 2nd byte: 0x00 (dummy byte)                                     \
            // 3rd byte: start page address (0..7)                             \
            // 4th byte: 0x00 (dummy byte)                                     \
            // 5th byte: end page address (0..7)                               \
            // 6th byte: 0x00 (dummy byte)                                     \
            // 7th byte: start column (0..128)                                 \
            // 8th byte: end column (0..128)

// Addressing commands
#define DISP_ADDR_LOW_COL  0x00 // Set lower column start address (0x00..0x0F)
#define DISP_ADDR_HIGH_COL 0x10 // Set lower column start address (0x10..0x1F)
#define DISP_ADDR_MODE     0x20 // Set memory addressing mode

// second byte values
#define DISP_ADDR_HORIZ 0x00 // horizontal addressing mode
#define DISP_ADDR_VERT  0x01 // vertical addressing mode
#define DISP_ADDR_PAGE  0x10 // page addressing mode

#define DISP_COL_ADDR                                                          \
    0x21 // Set column address                                                 \
            // 2nd byte: column address (0..127)

#define DISP_PAGE_ADDR 0xB0 // Set page address (0..7, B0 = 0, B7 = 7)

#define DISP_PAGE_STRT_ADDR 0xB0 // Set page start address using three LSB

// Hardware configuration commands
#define DISP_START_LINE 0x40 // Set display start line register from 0..63
#define DISP_MIRROR_VERT                                                       \
    0xA0 // Set segment remap (0xA0: normal, 0xA1: vertical mirror)
#define DISP_MUX_RATIO                                                         \
    0xA8 // Set multiplex ratio                                                \
            // 2nd byte: Multiplex ration(16..64, Mux ratio = N + 1)

#define DISP_MIRROR_HORIZ                                                      \
    0xC0 // Mirror horizontally (0xC0: normal, 0xC8: mirrored)
#define DISP_OFFSET                                                            \
    0xD3 // Set display offset                                                 \
            // 2nd byte: vertical shift (0..63)

#define DISP_T_PRCHRG                                                          \
    0xD9 // Set precharge period                                               \
            // 2nd byte: Low nibble: Phase 1 period, high nibble: Phase 2 period

#define DISP_LVL_DESELECT                                                      \
    0xDB // Set Vcomh deselect level                                           \
            // 2nd byte: Deselect level

#define CMD_PIN_STATE_CMD  0x00 /** D/C pin marks command transmission */
#define CMD_PIN_STATE_DATA 0x01 /** D/C pin marks data transmission */

static struct spi_cs_control spi_cs_control = {
    .gpio_dev = 
};

static struct spi_config spi_config = {
    .frequency = 10'000,
    .operation = SPI_OP_MODE_SLAVE | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE | SPI_LOCK_ON,
    .slave = 0,
    .cs = 
};

/**************************************************************************/ /**
 * Internal function to write a command to the display controller
 *
 * @param pInst Display instance
 * @param cmd Command to transmit
 * @param len Number of bytes to transmit
 * @return DISPLAY_OK if successful, otherwise an error code
 *
 *****************************************************************************/
static int writeCmd(struct Display *pInst, uint8_t *cmd, uint16_t len)
{
    int res;

    // Pull D/C line to 0 to indicate a command
    gpio_pin_set(pInst->cmdPort, pInst->cmdPin, CMD_PIN_STATE_CMD);

    // Write the data to the SPI interface
    res = spi_write()
    res = spiWrite(&spiPeri, &pInst->spi, cmd, len, 1);
    if (res != SPICOMM_OK) {
        return DISPLAY_ERR_SPI;
    }

    // Pull D/C line back to 1
    gpio_pin_set(pInst->cmdPort, pInst->cmdPin, CMD_PIN_STATE_DATA);

    return 0;
}

/**************************************************************************/ /**
 * Initialises the display
 *
 * @param pInst Display instance
 * @return DISPLAY_OK if successful, otherwise an error code
 *
 *****************************************************************************/
int displayInit(struct Display *pInst)
{
    uint32_t res;
    uint8_t cmd;

    // Put the display controller to reset and disable the 13V power supply
    HAL_GPIO_WritePin(pInst->pwrPort, pInst->pwrPin, GPIO_PIN_RESET);
    dispDelay(1);
    HAL_GPIO_WritePin(pInst->rstPort, pInst->rstPin, GPIO_PIN_RESET);
    dispDelay(50);
    // Release the reset line and wait another millisecond
    HAL_GPIO_WritePin(pInst->rstPort, pInst->rstPin, GPIO_PIN_SET);
    dispDelay(1);

    // Set up the display configuration
    cmd = DISP_MIRROR_HORIZ | (pInst->mirror_horiz ? 0x08 : 0x00);
    res = writeCmd(pInst, &cmd, 1);
    if (res != DISPLAY_OK)
        return res;

    cmd = DISP_MIRROR_VERT | (pInst->mirror_vert ? 0x01 : 0x00);
    res = writeCmd(pInst, &cmd, 1);
    if (res != DISPLAY_OK)
        return res;

    // Clear the display memory
    res = displayClear(pInst);
    if (res != DISPLAY_OK)
        return res;

    // The display is now ready to be powered on
    return DISPLAY_OK;
}

/**************************************************************************/ /**
 * Switches on the display
 *
 * @param pInst Display instance
 * @return DISPLAY_OK if successful, otherwise an error code
 *
 *****************************************************************************/
int displayOn(struct Display *pInst)
{
    uint32_t res;
    uint8_t cmd;

    // switch on the high voltage power supply and wait a short moment
    if (pInst->pwrPort) {
        HAL_GPIO_WritePin(pInst->pwrPort, pInst->pwrPin, GPIO_PIN_SET);
    }
    dispDelay(10);

    // switch on the display drivers
    cmd = DISP_ON | 0x01;
    res = writeCmd(pInst, &cmd, 1);
    if (res != DISPLAY_OK)
        return res;

    return DISPLAY_OK;
}

/**************************************************************************/ /**
 * Switches off the display
 *
 * @param pInst Display instance
 * @return DISPLAY_OK if successful, otherwise an error code
 *
 *****************************************************************************/
int displayOff(struct Display *pInst)
{
    uint32_t res;
    uint8_t cmd;

    // switch off the display drivers and wait a short moment
    cmd = DISP_ON | 0x00;
    res = writeCmd(pInst, &cmd, 1);
    if (res != DISPLAY_OK)
        return res;

    // switch off the high voltage power supply
    if (pInst->pwrPort) {
        HAL_GPIO_WritePin(pInst->pwrPort, pInst->pwrPin, GPIO_PIN_RESET);
    }
    return DISPLAY_OK;
}

/**************************************************************************/ /**
 * Set a contrast level of the display
 *
 * @param pInst		Display instance
 * @param contrast	Contrast to set
 * @return DISPLAY_OK if successful, otherwise an error code
 *
 *****************************************************************************/
int displaySetContrast(struct Display *pInst, uint8_t contrast)
{
    uint32_t res;
    uint8_t cmd[2] = { DISP_SET_CONTRAST, contrast };

    if (contrast == pInst->contrast) {
        return DISPLAY_OK;
    }

    res = writeCmd(pInst, cmd, sizeof(cmd));
    if (res == DISPLAY_OK) {
        pInst->contrast = contrast;
    }

    return res;
}

/**************************************************************************/ /**
 * Set a number of display memory bytes on the current page to a certain value.
 *
 * @param pInst		Display instance
 * @param value		Value to be preset
 * @param len		Number of columns to fill
 * @param height	Number of rows to fill. Must be a multiple of DISPLAY_PAGE_SIZE
 * @return			DISPLAY_OK if successful, otherwise an error code.
 *
 *****************************************************************************/
int displayMemset(struct Display *pInst, uint8_t value, size_t len)
{
    int res;

    if (len == 0) {
        return DISPLAY_OK;
    }
    if (len > 128) {
        return DISPLAY_TOO_MUCH_ROWS;
    }
    uint8_t *data = (uint8_t *)malloc(len);
    if (data == 0) {
        return DISPLAY_ERR_MEMORY;
    }
    memset(data, value, len);
    res = spiWrite(&spiPeri, &pInst->spi, data, len, 1);
    free(data);
    if (res != SPICOMM_OK) {
        return DISPLAY_ERR_SPI;
    }
    return DISPLAY_OK;
}

/**************************************************************************/ /**
 * Set a position to draw on the display.
 *
 * @param pInst Display instance.
 * @param posX	Horizontal position.
 * @param posY	Vertical position.
 * @return The actual Y position rounded to a page or an error code.
 *
 *****************************************************************************/
int displaySetPos(struct Display *pInst, uint8_t posX, uint8_t posY)
{
    int res;
    uint8_t cmd;

    uint8_t page = posY / DISPLAY_PAGESIZE;
    cmd = DISP_PAGE_ADDR | page;
    res = writeCmd(pInst, &cmd, sizeof(cmd));
    if (res != DISPLAY_OK) {
        return res;
    }
    cmd = DISP_ADDR_LOW_COL | (posX % 16);
    res = writeCmd(pInst, &cmd, sizeof(cmd));
    if (res != DISPLAY_OK) {
        return res;
    }
    cmd = DISP_ADDR_HIGH_COL | (posX / 16);
    res = writeCmd(pInst, &cmd, sizeof(cmd));
    if (res != DISPLAY_OK) {
        return res;
    }
    return page * DISPLAY_PAGESIZE;
}

/**************************************************************************/ /**
 * Clears the complete display
 *
 * @param pInst Display instance
 * @return DISPLAY_OK if successful, otherwise an error code
 *
 *****************************************************************************/
int displayClear(struct Display *pInst)
{
    int i, res;
    uint8_t cmd;

    for (i = 0; i < 8; ++i) {
        cmd = DISP_PAGE_ADDR | i;
        res = writeCmd(pInst, &cmd, sizeof(cmd));
        if (res != DISPLAY_OK) {
            return res;
        }
        res = displayMemset(pInst, 0x00, 128);
        if (res != DISPLAY_OK) {
            return res;
        }
    }
    return DISPLAY_OK;
}
