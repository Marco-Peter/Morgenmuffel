/**************************************************************************/ /**
 * userinput.c
 *
 * Handling of user inputs (pushbuttons and turn knobs)
 * The user inputs are sampled each 50ms. If something happens an event will
 * be set.
 *
 * 2020 by M. Peter
 *
 *****************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include "events.h"

#define BTN_POWER_LABEL DT_GPIO_LABEL(DT_NODELABEL(btn_pwr), gpios)
#define BTN_POWER_PIN   DT_GPIO_PIN(DT_NODELABEL(btn_pwr), gpios)
#define BTN_POWER_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(btn_pwr), gpios)

#define BTN_SELECT_LABEL DT_GPIO_LABEL(DT_NODELABEL(btn_sel), gpios)
#define BTN_SELECT_PIN   DT_GPIO_PIN(DT_NODELABEL(btn_sel), gpios)
#define BTN_SELECT_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(btn_sel), gpios)

#define BTN_BACK_LABEL DT_GPIO_LABEL(DT_NODELABEL(btn_bk), gpios)
#define BTN_BACK_PIN   DT_GPIO_PIN(DT_NODELABEL(btn_bk), gpios)
#define BTN_BACK_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(btn_bk), gpios)

#define BTN_FORWARD_LABEL DT_GPIO_LABEL(DT_NODELABEL(btn_pwr), gpios)
#define BTN_FORWARD_PIN   DT_GPIO_PIN(DT_NODELABEL(btn_fwd), gpios)
#define BTN_FORWARD_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(btn_fwd), gpios)

#define BTN_MENU_LABEL DT_GPIO_LABEL(DT_NODELABEL(btn_menu), gpios)
#define BTN_MENU_PIN   DT_GPIO_PIN(DT_NODELABEL(btn_menu), gpios)
#define BTN_MENU_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(btn_menu), gpios)

#define BTN_PLAY_LABEL DT_GPIO_LABEL(DT_NODELABEL(btn_play), gpios)
#define BTN_PLAY_PIN   DT_GPIO_PIN(DT_NODELABEL(btn_play), gpios)
#define BTN_PLAY_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(btn_play), gpios)

#define BTN_ALARM1_LABEL DT_GPIO_LABEL(DT_NODELABEL(btn_alarm1), gpios)
#define BTN_ALARM1_PIN   DT_GPIO_PIN(DT_NODELABEL(btn_alarm1), gpios)
#define BTN_ALARM1_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(btn_alarm1), gpios)

#define BTN_ALARM2_LABEL DT_GPIO_LABEL(DT_NODELABEL(btn_alarm2), gpios)
#define BTN_ALARM2_PIN   DT_GPIO_PIN(DT_NODELABEL(btn_alarm2), gpios)
#define BTN_ALARM2_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(btn_alarm2), gpios)

#define BTN_ALARM3_LABEL DT_GPIO_LABEL(DT_NODELABEL(btn_alarm3), gpios)
#define BTN_ALARM3_PIN   DT_GPIO_PIN(DT_NODELABEL(btn_alarm3), gpios)
#define BTN_ALARM3_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(btn_alarm3), gpios)

#define ENC_SELECT_A_LABEL DT_GPIO_LABEL(DT_NODELABEL(enc_select_a), gpios)
#define ENC_SELECT_A_PIN   DT_GPIO_PIN(DT_NODELABEL(enc_select_a), gpios)
#define ENC_SELECT_A_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(enc_select_a), gpios)

#define ENC_SELECT_B_LABEL DT_GPIO_LABEL(DT_NODELABEL(enc_select_b), gpios)
#define ENC_SELECT_B_PIN   DT_GPIO_PIN(DT_NODELABEL(enc_select_b), gpios)
#define ENC_SELECT_B_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(enc_select_b), gpios)

#define ENC_VOLUME_A_LABEL DT_GPIO_LABEL(DT_NODELABEL(enc_volume_a), gpios)
#define ENC_VOLUME_A_PIN   DT_GPIO_PIN(DT_NODELABEL(enc_volume_a), gpios)
#define ENC_VOLUME_A_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(enc_volume_a), gpios)

#define ENC_VOLUME_B_LABEL DT_GPIO_LABEL(DT_NODELABEL(enc_volume_b), gpios)
#define ENC_VOLUME_B_PIN   DT_GPIO_PIN(DT_NODELABEL(enc_volume_b), gpios)
#define ENC_VOLUME_B_FLAGS DT_GPIO_FLAGS(DT_NODELABEL(enc_volume_b), gpios)

#define BTN_POWER   0x00000001
#define BTN_SELECT  0x00000002
#define BTN_BACK    0x00000004
#define BTN_FORWARD 0x00000008
#define BTN_MENU    0x00000010
#define BTN_PLAY    0x00000020
#define BTN_ALARM1  0x00000040
#define BTN_ALARM2  0x00000080
#define BTN_ALARM3  0x00000100

#define ENC_VOLUME_A 0x00000001
#define ENC_VOLUME_B 0x00000002
#define ENC_SELECT_A 0x00000004
#define ENC_SELECT_B 0x00000008

#define CYCLE_PERIOD_MS   50
#define LONG_PUSH_TIME_MS 1000

#define BTN(_btn_) btn_##_btn_##_port, BTN_##_btn_##_PIN
#define ENC(_enc_) enc_##_enc_##_port, ENC_##_enc_##_PIN

static void scan_inputs(void)
{
    int ret;
    bool encSelectA;
    bool encSelectB;
    bool encVolumeA;
    bool encVolumeB;

    struct device *btn_POWER_port = device_get_binding(BTN_POWER_LABEL);
    struct device *btn_SELECT_port = device_get_binding(BTN_SELECT_LABEL);
    struct device *btn_BACK_port = device_get_binding(BTN_BACK_LABEL);
    struct device *btn_FORWARD_port = device_get_binding(BTN_FORWARD_LABEL);
    struct device *btn_MENU_port = device_get_binding(BTN_MENU_LABEL);
    struct device *btn_PLAY_port = device_get_binding(BTN_PLAY_LABEL);
    struct device *btn_ALARM1_port = device_get_binding(BTN_ALARM1_LABEL);
    struct device *btn_ALARM2_port = device_get_binding(BTN_ALARM2_LABEL);
    struct device *btn_ALARM3_port = device_get_binding(BTN_ALARM3_LABEL);
    struct device *enc_VOLUME_A_port = device_get_binding(ENC_VOLUME_A_LABEL);
    struct device *enc_VOLUME_B_port = device_get_binding(ENC_VOLUME_B_LABEL);
    struct device *enc_SELECT_A_port = device_get_binding(ENC_SELECT_A_LABEL);
    struct device *enc_SELECT_B_port = device_get_binding(ENC_SELECT_B_LABEL);

    ret = gpio_pin_configure(BTN(POWER), BTN_POWER_FLAGS);
    ret = gpio_pin_configure(BTN(SELECT), BTN_SELECT_FLAGS);
    ret = gpio_pin_configure(BTN(BACK), BTN_BACK_FLAGS);
    ret = gpio_pin_configure(BTN(FORWARD), BTN_FORWARD_FLAGS);
    ret = gpio_pin_configure(BTN(MENU), BTN_MENU_FLAGS);
    ret = gpio_pin_configure(BTN(PLAY), BTN_PLAY_FLAGS);
    ret = gpio_pin_configure(BTN(ALARM1), BTN_ALARM1_FLAGS);
    ret = gpio_pin_configure(BTN(ALARM2), BTN_ALARM2_FLAGS);
    ret = gpio_pin_configure(BTN(ALARM3), BTN_ALARM3_FLAGS);
    ret = gpio_pin_configure(ENC(VOLUME_A), ENC_VOLUME_A_FLAGS);
    ret = gpio_pin_configure(ENC(VOLUME_B), ENC_VOLUME_B_FLAGS);
    ret = gpio_pin_configure(ENC(SELECT_A), ENC_SELECT_A_FLAGS);
    ret = gpio_pin_configure(ENC(SELECT_B), ENC_SELECT_B_FLAGS);

    encSelectA = gpio_pin_get(ENC(VOLUME_A));
    encSelectB = gpio_pin_get(ENC(VOLUME_B));
    encVolumeA = gpio_pin_get(ENC(SELECT_A));
    encVolumeB = gpio_pin_get(ENC(SELECT_B));

    while (true) {
        uint32_t newBtnStates;
        uint32_t btnEvents;
        uint32_t oldBtnStates = 0;
        int pressStart = k_uptime_get_32();

        k_msleep(CYCLE_PERIOD_MS);

        newBtnStates = 0;
        newBtnStates |= gpio_pin_get(BTN(POWER)) ? BTN_POWER : 0;
        newBtnStates |= gpio_pin_get(BTN(SELECT)) ? BTN_SELECT : 0;
        newBtnStates |= gpio_pin_get(BTN(BACK)) ? BTN_BACK : 0;
        newBtnStates |= gpio_pin_get(BTN(FORWARD)) ? BTN_FORWARD : 0;
        newBtnStates |= gpio_pin_get(BTN(MENU)) ? BTN_MENU : 0;
        newBtnStates |= gpio_pin_get(BTN(PLAY)) ? BTN_PLAY : 0;
        newBtnStates |= gpio_pin_get(BTN(ALARM1)) ? BTN_ALARM1 : 0;
        newBtnStates |= gpio_pin_get(BTN(ALARM2)) ? BTN_ALARM2 : 0;
        newBtnStates |= gpio_pin_get(BTN(ALARM3)) ? BTN_ALARM3 : 0;

        /* Find out if any buttons have been released before the timeout */
        btnEvents = (~newBtnStates & oldBtnStates);
        if (btnEvents) {
            if (btnEvents & BTN_POWER) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_POWER_SHORT);
            }
            if (btnEvents & BTN_SELECT) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_SELECT_SHORT);
            }
            if (btnEvents & BTN_BACK) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_BACK_SHORT);
            }
            if (btnEvents & BTN_FORWARD) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_FWD_SHORT);
            }
            if (btnEvents & BTN_MENU) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_MENU_SHORT);
            }
            if (btnEvents & BTN_PLAY) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_PLAY_SHORT);
            }
            if (btnEvents & BTN_ALARM1) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_ALARM1_SHORT);
            }
            if (btnEvents & BTN_ALARM2) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_ALARM2_SHORT);
            }
            if (btnEvents & BTN_ALARM3) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_ALARM3_SHORT);
            }
        }
        if (newBtnStates & ~oldBtnStates) {
            pressStart = k_uptime_get_32();
        }
        if ((newBtnStates & oldBtnStates) &&
            (k_uptime_get_32() - pressStart > LONG_PUSH_TIME_MS)) {
            if (btnEvents & BTN_POWER) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_POWER_LONG);
            }
            if (btnEvents & BTN_SELECT) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_SELECT_LONG);
            }
            if (btnEvents & BTN_BACK) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_BACK_LONG);
            }
            if (btnEvents & BTN_FORWARD) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_FWD_LONG);
            }
            if (btnEvents & BTN_MENU) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_MENU_LONG);
            }
            if (btnEvents & BTN_PLAY) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_PLAY_LONG);
            }
            if (btnEvents & BTN_ALARM1) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_ALARM1_LONG);
            }
            if (btnEvents & BTN_ALARM2) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_ALARM2_LONG);
            }
            if (btnEvents & BTN_ALARM3) {
                k_poll_signal_raise(&buttonEvents, EVT_BTN_ALARM3_LONG);
            }
            newBtnStates = 0;
        }
        oldBtnStates = newBtnStates;

        if (gpio_pin_get(ENC(VOLUME_A)) != encVolumeA) {
            if (gpio_pin_get(ENC(VOLUME_A)) != gpio_pin_get(ENC(VOLUME_B))) {
                k_poll_signal_raise(&buttonEvents, EVT_ENC_VOLUME_UP);
            }
            else {
                k_poll_signal_raise(&buttonEvents, EVT_ENC_VOLUME_DN);
            }
            encVolumeA = gpio_pin_get(ENC(VOLUME_A));
        }
        else if (gpio_pin_get(ENC(VOLUME_B)) != encVolumeB) {
            if (gpio_pin_get(ENC(VOLUME_B)) == gpio_pin_get(ENC(VOLUME_A))) {
                k_poll_signal_raise(&buttonEvents, EVT_ENC_VOLUME_UP);
            }
            else {
                k_poll_signal_raise(&buttonEvents, EVT_ENC_VOLUME_DN);
            }
            encVolumeB = gpio_pin_get(ENC(VOLUME_B));
        }

        if (gpio_pin_get(ENC(SELECT_A)) != encSelectA) {
            if (gpio_pin_get(ENC(SELECT_A)) != gpio_pin_get(ENC(SELECT_B))) {
                k_poll_signal_raise(&buttonEvents, EVT_ENC_SELECT_UP);
            }
            else {
                k_poll_signal_raise(&buttonEvents, EVT_ENC_SELECT_DN);
            }
            encSelectA = gpio_pin_get(ENC(SELECT_A));
        }
        else if (gpio_pin_get(ENC(SELECT_B)) != encSelectB) {
            if (gpio_pin_get(ENC(SELECT_B)) == gpio_pin_get(ENC(SELECT_A))) {
                k_poll_signal_raise(&buttonEvents, EVT_ENC_SELECT_UP);
            }
            else {
                k_poll_signal_raise(&buttonEvents, EVT_ENC_SELECT_DN);
            }
        }
    }
}

K_THREAD_DEFINE(userinput, 256, scan_inputs, NULL, NULL, NULL, 1, K_ESSENTIAL,
                1000);