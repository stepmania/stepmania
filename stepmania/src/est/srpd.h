/*************************************************************************/
/*                                                                       */
/*                Centre for Speech Technology Research                  */
/*                     University of Edinburgh, UK                       */
/*                      Copyright (c) 1995,1996                          */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  THE UNIVERSITY OF EDINBURGH AND THE CONTRIBUTORS TO THIS WORK        */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL THE UNIVERSITY OF EDINBURGH NOR THE CONTRIBUTORS BE LIABLE     */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*                   Author :  Paul Taylor                               */
/*                   Date   :  April 1994                                */
/*************************************************************************/
#ifndef __SRPD_H__
#define __SRPD_H__

#include <stdio.h>

/********************
 * define constants *
 ********************/

#define MINARG                5
#define BREAK_NUMBER          0.0

#define DEFAULT_DECIMATION    4     /* samples */
#define DEFAULT_MIN_PITCH     40.0  /* Hz */
#define DEFAULT_MAX_PITCH     400.0 /* Hz */

#define DEFAULT_SF            20000 /* Hz. Sampling Frequency */
#define DEFAULT_SHIFT         5.0   /* ms */
#define DEFAULT_LENGTH        10.0   /* ms */
#define DEFAULT_TSILENT       120   /* max. abs sample amplitude of noise */
#define DEFAULT_TMIN          0.75
#define DEFAULT_TMAX_RATIO    0.85
#define DEFAULT_THIGH         0.88
#define DEFAULT_TDH           0.77

#define UNVOICED              0     /* segment classifications */
#define VOICED                1
#define SILENT                2

#define HOLD                  1
#define HELD                  1
#define SEND                  2
#define SENT                  2

/******************************
 * define abstract data types *
 ******************************/

struct CROSS_CORR_
{
  int size;
  double *coeff;
} ;


struct SEGMENT_
{                    /* segment of speech data */
  int size, shift, length;          /* in samples */
  short *data;
};


struct Srpd_Op {
  int sample_freq;      /* Hz */
  int Nmax, Nmin;
  double shift, length; /* ms */
  double min_pitch;     /* Hz */
  double max_pitch;     /* Hz */
  int L;                /* Decimation factor (samples) */
  double Tmin, Tmax_ratio, Thigh, Tdh;
  int Tsilent;
  int make_ascii;
  int peak_tracking;
};

struct STATUS_ {
  double pitch_freq;
  char v_uv, s_h;
  double cc_max, threshold;
};

typedef struct list {
  int N0, score;
  struct list *next_item;
} LIST_;

typedef enum {
  CANT_WRITE, DECI_FCTR, INSUF_MEM, FILE_ERR, FILE_SEEK, LEN_OOR, MAX_FREQ,
  MIN_FREQ, MISUSE, NOISE_FLOOR, SAMPLE_FREQ, SFT_OOR, THR_DH, THR_HIGH,
  THR_MAX_RTO, THR_MIN
  } error_flags;


void add_to_list (LIST_ **p_list_hd, LIST_ **p_list_tl, int N_val, 
		  int score_val);

void super_resolution_pda (struct Srpd_Op *paras, SEGMENT_ seg,
			   CROSS_CORR_ *p_cc, STATUS_ *p_status);
void write_track(STATUS_ status, struct Srpd_Op paras, FILE *outfile);


int read_next_segment (FILE *voxfile, struct Srpd_Op *paras, SEGMENT_ *p_seg);
void end_structure_use(SEGMENT_ *p_seg, CROSS_CORR_ *p_cc);
void initialise_status (struct Srpd_Op *p, STATUS_ *p_status);
void initialise_structures (struct Srpd_Op *p, SEGMENT_ *p_seg,
     CROSS_CORR_ *p_cc);

void initialise_parameters (struct Srpd_Op *p_par);
void error (error_flags err_type);

void free_list (LIST_ **p_list_hd);

#endif // __SRPD_H__
