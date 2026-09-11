#ifndef BLACKOUT_RECOVERY_H
#define BLACKOUT_RECOVERY_H

#include <stdbool.h>
#include <time.h>

/* Phases use OP_MODE_FREEZE .. OP_MODE_FERM2 (0 .. 3). */
typedef struct
{
    int phase;
    int remaining[4];
    time_t completeTime;
    bool complete;
} blackoutPlan_t;

/* Freeze/defrost absorb downtime to preserve the deadline where possible.
 * Fermentation resumes its saved remaining minutes; downtime is not processing.
 * Both timestamps must be on a minute boundary, as in the UART protocol. */
bool blackoutBuildPlan(int savedPhase, int savedRemaining, const int totals[4],
                       time_t restartTime, time_t targetTime, blackoutPlan_t *plan);

#endif
