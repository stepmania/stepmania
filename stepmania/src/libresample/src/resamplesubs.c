/**********************************************************************

  resamplesubs.c

  Real-time library interface by Dominic Mazzoni

  Based on resample-1.7:
    http://www-ccrma.stanford.edu/~jos/resample/

  License: LGPL - see the file LICENSE.txt for more information

  This file provides the routines that do sample-rate conversion
  on small arrays, calling routines from filterkit.

**********************************************************************/

/* Definitions */
#include "resample_defs.h"

#include "filterkit.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/* Sampling rate up-conversion only subroutine;
 * Slightly faster than down-conversion;
 */
int SrcUp(float X[],
          float Y[],
          double factor,
          double *Time,
          UWORD Nx,
          UWORD Nwing,
          float LpScl,
          float Imp[],
          float ImpD[],
          BOOL Interp)
{
    float *Xp, *Ystart;
    float v;
    
    double dt;                 /* Step through input signal */ 
    double endTime;            /* When Time reaches EndTime, return to user */
    
    dt = 1.0/factor;           /* Output sampling period */
    
    Ystart = Y;
    endTime = *Time + Nx;
    while (*Time < endTime)
    {
        Xp = &X[(int)(*Time)]; /* Ptr to current input sample */
        /* Perform left-wing inner product */
        v = FilterUp(Imp, ImpD, Nwing, Interp, Xp,
                            (*Time)-floor(*Time), -1);
        /* Perform right-wing inner product */
        v += FilterUp(Imp, ImpD, Nwing, Interp, Xp+1, 
                      ((-(*Time))-floor(-(*Time))), 1);

        v *= LpScl;   /* Normalize for unity filter gain */

        *Y++ = v;               /* Deposit output */
        *Time += dt;            /* Move to next sample by time increment */
    }
    return (Y - Ystart);        /* Return the number of output samples */
}

/* Sampling rate conversion subroutine */

int SrcUD(float X[],
          float Y[],
          double factor,
          double *Time,
          UWORD Nx,
          UWORD Nwing,
          float LpScl,
          float Imp[],
          float ImpD[],
          BOOL Interp)
{
    float *Xp, *Ystart;
    float v;
    
    double dh;                 /* Step through filter impulse response */
    double dt;                 /* Step through input signal */
    double endTime;            /* When Time reaches EndTime, return to user */
    
    dt = 1.0/factor;            /* Output sampling period */
    
    dh = MIN(Npc, factor*Npc);  /* Filter sampling period */
    
    Ystart = Y;
    endTime = *Time + Nx;
    while (*Time < endTime)
    {
        Xp = &X[(int)(*Time)];     /* Ptr to current input sample */
        v = FilterUD(Imp, ImpD, Nwing, Interp, Xp,
                     (*Time)-floor(*Time), -1, dh);
                     /* Perform left-wing inner product */
        v += FilterUD(Imp, ImpD, Nwing, Interp, Xp+1, 
                            ((-(*Time))-floor(-(*Time))), 1, dh);

        v *= LpScl;   /* Normalize for unity filter gain */
        *Y++ = v;               /* Deposit output */
        
        *Time += dt;            /* Move to next sample by time increment */
    }
    return (Y - Ystart);        /* Return the number of output samples */
}
