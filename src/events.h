/**************************************************************************/ /**
 * events.h
 *
 * System events
 *
 * 2020 by M. Peter
 *
 *****************************************************************************/
#ifndef EVENTS_C_
#define EVENTS_C_

#define EVT_BTN_ALARM1_SHORT 0x00000001 /// Alarm 1 button event
#define EVT_BTN_ALARM1_LONG  0x00000002 /// Alarm 1 button event
#define EVT_BTN_ALARM2_SHORT 0x00000004 /// Alarm 2 button event
#define EVT_BTN_ALARM2_LONG  0x00000008 /// Alarm 2 button event
#define EVT_BTN_ALARM3_SHORT 0x00000010 /// Alarm 3 button event
#define EVT_BTN_ALARM3_LONG  0x00000020 /// Alarm 3 button event
#define EVT_BTN_POWER_SHORT  0x00000040 /// Power button event
#define EVT_BTN_POWER_LONG   0x00000080 /// Power button event
#define EVT_BTN_SELECT_SHORT 0x00000100 /// Select button event
#define EVT_BTN_SELECT_LONG  0x00000200 /// Select button event
#define EVT_BTN_PLAY_SHORT   0x00000400 /// Play button event
#define EVT_BTN_PLAY_LONG    0x00000800 /// Play button event
#define EVT_BTN_FWD_SHORT    0x00001000 /// Forward button event
#define EVT_BTN_FWD_LONG     0x00002000 /// Forward button event
#define EVT_BTN_BACK_SHORT   0x00004000 /// Back button event
#define EVT_BTN_BACK_LONG    0x00008000 /// Back button event
#define EVT_BTN_MENU_SHORT   0x00010000 /// Menu button event
#define EVT_BTN_MENU_LONG    0x00020000 /// Menu button event
#define EVT_ENC_VOLUME_UP    0x00040000 /// Volume encoder event
#define EVT_ENC_VOLUME_DN    0x00080000 /// Volume encoder event
#define EVT_ENC_SELECT_UP    0x00100000 /// Select up encoder event
#define EVT_ENC_SELECT_DN    0x00200000 /// Select up encoder event

/// All user input events
#define EVT_ALL_USERINPUT 0x003FFFFF

extern struct k_poll_signal buttonEvents;

#endif // EVENTS_C_
