#include "priority.h"
#include <sched.h>

/*
 * Set the maximum available priority and the correct scheduler for this
 * process.
 */
int setMaxPriority() {
    int maxPriority;
    int status;
    struct sched_param sched;

    // get max available priority
    maxPriority = sched_get_priority_max(SCHED_RR);
    if (maxPriority == -1) {
        // not able to determine max available priority
        return 1;
    }

    // set scheduler to SCHED_FIFO and priority to max priority
    sched.sched_priority = maxPriority;
    status = sched_setscheduler(0, SCHED_RR, &sched);

    if (status) {
        return 2;
    }

    return 0;
}
