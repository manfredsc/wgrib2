/** @file
 * @brief Functions to stop wgrib2 after a specified number of seconds. 
 * @author Public Domain: Wesley Ebisuzaki @date 04/2017
 */

#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:alarm:setup:1:terminate after X seconds
 */

#ifndef DISABLE_ALARM

#include <unistd.h>

/** 
 * Terminates wgrib2 after N seconds (real time). The alarm is activated in the 
 * setup phase and removed in the finalize phase.
 * 
 * @param ARG1 Arguments and context for the wgrib2 function macro. Requires a single argument which is the number of seconds to wait before terminating.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @note The code for -alarm is SVr4, BSD, POSIX-1-2001 and IEEE Std 1003.1-2001 compatible. 
 * The code is also supported by the Cygwin system for Windows systems. The -alarm will need 
 * to be disabled for non-compatible systems. (-DDISABLE_ALARM=on)
 * 
 * The current version of -alarm simply terminates the process. This may be the appropriate
 * action when wgrib2 is taking too long. However, this heavy-handed action may not be an ideal 
 * action for programs that are calling the wgrib2 subroutine. For these situations, the alarm should
 * be set up by the main program so that a more appropriate action can be taken.
 * 
 * The -alarm option is a replacement for -limit which limits the number of (sub)messages which are 
 * processed. Jobs on web servers may hang because of problems with the network connections which will 
 * never trigger the -limit option. 
 * 
 * ## Example Usage:
 *  -alarm N
 * 
 * N is an integer from 0..65536 (ISO C Standard). N = 0 will remove any pending alarm.
 * 
 * For Posix systems, the system generates a SIGALRM signal to the process after N seconds. The default 
 * action is to terminate the process (wgrib2). 
*/
int f_alarm(ARG1) {
    int i;
    if (mode == -1) {
        // set the alarm for n seconds
        i = atoi(arg1);
        alarm(i <= 0 ? 0: (unsigned int) i);
    }
    else if (mode == -2) {
        // turn off the alarm
        alarm(0);
    }
    return 0;
}

#else

int f_alarm(ARG1) {
    fatal_error("alarm: not supported");
    return 0;
}

#endif
