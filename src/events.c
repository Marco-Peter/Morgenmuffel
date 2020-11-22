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

int wait_for_user_event(void)
{
        int result;

        k_poll(events, 1, K_FOREVER);
        result = events->signal->result;
        events[0].signal->signaled = 0;
        events[0].state = K_POLL_STATE_NOT_READY;
        return result;
}
