#include "blackout_recovery.h"
#include <string.h>

bool blackoutBuildPlan(int savedPhase, int savedRemaining, const int totals[4],
                       time_t restartTime, time_t targetTime, blackoutPlan_t *plan)
{
    if (!totals || !plan || savedPhase < 0 || savedPhase > 3 ||
        savedRemaining < 0 || restartTime == (time_t)-1 || targetTime == (time_t)-1)
    {
        return false;
    }
    for (int i = 0; i < 4; i++)
    {
        if (totals[i] < 0 || totals[i] > 255 * 60 + 59)
        {
            return false;
        }
    }

    memset(plan, 0, sizeof(*plan));
    plan->phase = savedPhase;
    plan->completeTime = targetTime;
    for (int i = savedPhase; i < 4; i++)
    {
        plan->remaining[i] = totals[i];
    }

    if (savedPhase <= 1)
    {
        /* Reserve the full, not-yet-started fermentation stages first. */
        double available = difftime(targetTime, restartTime) / 60.0
                         - totals[2] - totals[3];
        if (available > 255 * 60 + 59)
        {
            return false;
        }
        int coldMinutes = available > 0 ? (int)available : 0;
        if (savedPhase == 0)
        {
            plan->remaining[1] = coldMinutes < totals[1] ? coldMinutes : totals[1];
            plan->remaining[0] = coldMinutes - plan->remaining[1];
        }
        else
        {
            plan->remaining[1] = coldMinutes;
        }
        /* When the deadline is already too close, only the deadline moves. */
        if (available < 0)
        {
            plan->completeTime = restartTime + (time_t)(totals[2] + totals[3]) * 60;
        }
    }
    else
    {
        if (savedRemaining > totals[savedPhase])
        {
            savedRemaining = totals[savedPhase];
        }
        plan->remaining[savedPhase] = savedRemaining;
        int remaining = savedRemaining;
        for (int i = savedPhase + 1; i < 4; i++)
        {
            remaining += totals[i];
        }
        plan->completeTime = restartTime + (time_t)remaining * 60;
    }

    /* Never restart a finished/zero-duration stage with an X0 command. */
    while (plan->phase < 4 && plan->remaining[plan->phase] == 0)
    {
        plan->phase++;
    }
    plan->complete = plan->phase == 4;
    return true;
}
