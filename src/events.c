/**************************************************************************/ /**
 * events.c
 *
 * System events
 *
 * 2020 by M. Peter
 *
 *****************************************************************************/
#include "events.h"
#include <zephyr.h>

struct k_poll_signal buttonEvents = K_POLL_SIGNAL_INITIALIZER(buttonEvents);
struct k_poll_event events[] = { K_POLL_EVENT_STATIC_INITIALIZER(
        K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &buttonEvents, 0) };
