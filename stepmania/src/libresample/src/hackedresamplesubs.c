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


float FilterUp(float Imp[],  /* impulse response */
               float ImpD[], /* impulse response deltas */
               UWORD Nwing, /* len of one wing of filter */
               BOOL Interp,  /* Interpolate coefs using deltas? */
               float *Xp,    /* Current sample */
               double Ph,    /* Phase */
               int Inc)      /* increment (1 for right wing or -1 for left) */
{
   float *Hp, *Hdp = NULL, *End;
   double a = 0;
   float v, t;

   Ph *= Npc; /* Npc is number of values per 1/delta in impulse response */
   
   v = 0.0; /* The output value */
   Hp = &Imp[(int)Ph];
   End = &Imp[Nwing];
   if (Interp) {
      Hdp = &ImpD[(int)Ph];
      a = Ph - floor(Ph); /* fractional part of Phase */
   }

   if (Inc == 1)		/* If doing right wing...              */
   {				      /* ...drop extra coeff, so when Ph is  */
      End--;			/*    0.5, we don't do too many mult's */
      if (Ph == 0)		/* If the phase is zero...           */
      {			         /* ...then we've already skipped the */
         Hp += Npc;		/*    first sample, so we must also  */
         Hdp += Npc;		/*    skip ahead in Imp[] and ImpD[] */
      }
   }

   if (Interp)
      while (Hp < End) {
         t = *Hp;		/* Get filter coeff */
         t += (*Hdp)*a; /* t is now interp'd filter coeff */
         Hdp += Npc;		/* Filter coeff differences step */
         t *= *Xp;		/* Mult coeff by input sample */
         v += t;			/* The filter output */
         Hp += Npc;		/* Filter coeff step */
         Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      } 
   else 
      while (Hp < End) {
         t = *Hp;		/* Get filter coeff */
         t *= *Xp;		/* Mult coeff by input sample */
         v += t;			/* The filter output */
         Hp += Npc;		/* Filter coeff step */
         Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      }
   
   return v;
}

float FilterUD(float Imp[],  /* impulse response */
               float ImpD[], /* impulse response deltas */
               UWORD Nwing,  /* len of one wing of filter */
               BOOL Interp,  /* Interpolate coefs using deltas? */
               float *Xp,    /* Current sample */
               double Ph,    /* Phase */
               int Inc,      /* increment (1 for right wing or -1 for left) */
               double dhb)   /* filter sampling period */
{
   float a;
   float *Hp, *Hdp, *End;
   float v, t;
   double Ho;
    
   v = 0.0; /* The output value */
   Ho = Ph*dhb;
   End = &Imp[Nwing];
   if (Inc == 1)		/* If doing right wing...              */
   {				      /* ...drop extra coeff, so when Ph is  */
      End--;			/*    0.5, we don't do too many mult's */
      if (Ph == 0)		/* If the phase is zero...           */
         Ho += dhb;		/* ...then we've already skipped the */
   }				         /*    first sample, so we must also  */
                        /*    skip ahead in Imp[] and ImpD[] */

   if (Interp)
      while ((Hp = &Imp[(int)Ho]) < End) {
         t = *Hp;		/* Get IR sample */
         Hdp = &ImpD[(int)Ho];  /* get interp bits from diff table*/
         a = Ho - floor(Ho);	  /* a is logically between 0 and 1 */
         t += (*Hdp)*a; /* t is now interp'd filter coeff */
         t *= *Xp;		/* Mult coeff by input sample */
         v += t;			/* The filter output */
         Ho += dhb;		/* IR step */
         Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      }
   else 
      while (1) {
	 float *Hp = &Imp[(int)Ho];
	 float t;
	 if ( Hp >= End)
	    break;
      
         t = *Hp;		/* Get IR sample */
         t *= *Xp;		/* Mult coeff by input sample */
         v += t;			/* The filter output */
         Ho += dhb;		/* IR step */
         Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      }

   return v;
}
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
    float *Ystart;
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
        float *Xp = &X[(int)(*Time)];     /* Ptr to current input sample */

	{
               double Ph=(*Time)-floor(*Time);    /* Phase */
               int Inc=-1;      /* increment (1 for right wing or -1 for left) */
               double dhb=dh;   /* filter sampling period */

		   float *End;
		   float t;
		   double Ho;
		    
		   v = 0.0; /* The output value */
		   Ho = Ph*dhb;
		   End = &Imp[Nwing];
		   if (Inc == 1)		/* If doing right wing...              */
		   {				      /* ...drop extra coeff, so when Ph is  */
		      End--;			/*    0.5, we don't do too many mult's */
		      if (Ph == 0)		/* If the phase is zero...           */
			 Ho += dhb;		/* ...then we've already skipped the */
		   }				         /*    first sample, so we must also  */
					/*    skip ahead in Imp[] and ImpD[] */

		   while (1) {
			 float *Hp = &Imp[(int)Ho];
			 float t;
			 if ( Hp >= End)
			    break;
		      
			 t = *Hp;		/* Get IR sample */
			 t *= *Xp;		/* Mult coeff by input sample */
			 v += t;			/* The filter output */
			 Ho += dhb;		/* IR step */
			 Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
		      }



	}
	




	{
               double Ph=((-(*Time))-floor(-(*Time)));    /* Phase */
               int Inc=1;      /* increment (1 for right wing or -1 for left) */
               double dhb=dh;   /* filter sampling period */

		   float *End;
		   float t;
		   double Ho;
		    
		   v = 0.0; /* The output value */
		   Ho = Ph*dhb;
		   End = &Imp[Nwing];
		   if (Inc == 1)		/* If doing right wing...              */
		   {				      /* ...drop extra coeff, so when Ph is  */
		      End--;			/*    0.5, we don't do too many mult's */
		      if (Ph == 0)		/* If the phase is zero...           */
			 Ho += dhb;		/* ...then we've already skipped the */
		   }				         /*    first sample, so we must also  */
					/*    skip ahead in Imp[] and ImpD[] */

		   while (1) {
			 float *Hp = &Imp[(int)Ho];
			 float t;
			 if ( Hp >= End)
			    break;
		      
			 t = *Hp;		/* Get IR sample */
			 t *= *Xp;		/* Mult coeff by input sample */
			 v += t;			/* The filter output */
			 Ho += dhb;		/* IR step */
			 Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
		      }



	}
	







	
       // v = FilterUD(Imp, ImpD, Nwing, Interp, Xp,
         //            (*Time)-floor(*Time), -1, dh);
                     /* Perform left-wing inner product */
       // v += FilterUD(Imp, ImpD, Nwing, Interp, Xp+1, 
         //                   ((-(*Time))-floor(-(*Time))), 1, dh);

        v *= LpScl;   /* Normalize for unity filter gain */
        *Y++ = v;               /* Deposit output */
        
        *Time += dt;            /* Move to next sample by time increment */
    }
    return (Y - Ystart);        /* Return the number of output samples */
}
