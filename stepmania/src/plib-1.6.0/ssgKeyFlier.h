/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#ifndef _SSG_KEYFLIER_H_
#define _SSG_KEYFLIER_H_

/***********************************************\
*                                               *
* SSG_KEYFLIER.H                                *
*                                               *
*  This file contains inline functions that     *
*  provide an ESIG-like keyboard-driven         *
*  flying carpet flight mode.                   *
*                                               *
*  Simply declare an instance of this class     *
*  somewhere in your program - and call the     *
*  'incoming_keystroke' routine every time a    *
*  relevent character is typed - call the       *
*  'update' function once per frame and the     *
*  'get_coord' function whenever you need new   *
*  coordinates. Other handy functions are also  *
*  available.                                   *
*                                               *
*  It's handy to use the 'Keyboard' class in    *
*  'tty_port.h' to provide the incoming keys.   *
*                                               *
\***********************************************/

typedef void (*queryFunc)(float,float,float,float,float,float) ;
typedef void (*exitFunc)(int) ;

class ssgKeyFlier
{
protected:
  float    tscale ;
  float    rscale ;
  sgCoord  curr_pos  ;
  sgCoord  reset_pos ;
  sgCoord  last_pos  ;
  sgCoord  curr_vel  ;
  sgCoord  velocity  ;

  queryFunc query_action ;
  exitFunc  exit_action  ;

public:
  ssgKeyFlier ()
  {
    query_action = NULL ;
    exit_action  = NULL ;

    tscale = 1.0 ;
    rscale = 1.0 ;
    sgZeroVec3 ( reset_pos.xyz ) ; sgZeroVec3 ( reset_pos.hpr ) ;
    sgZeroVec3 (  last_pos.xyz ) ; sgZeroVec3 (  last_pos.hpr ) ;
    sgZeroVec3 (  velocity.xyz ) ; sgZeroVec3 (  velocity.hpr ) ;
    update   () ;
  }

  void set_query_action ( queryFunc f ) { query_action = f ; }
  void set_exit_action  ( exitFunc  f ) { exit_action  = f ; }

  void stop_some ()
  {
    velocity.xyz[0]=velocity.xyz[2]=0.0 ;
    sgZeroVec3 ( velocity.hpr ) ;
    update   () ;
  }

  void stop_all ()
  {
    sgZeroVec3 (  velocity.xyz ) ; sgZeroVec3 (  velocity.hpr ) ;
    update   () ;
  }

  void reset ()
  {
    stop_all () ;
    sgCopyVec3 (  last_pos.xyz, reset_pos.xyz ) ;
    sgCopyVec3 (  last_pos.hpr, reset_pos.hpr ) ;
    update   () ;
  }

  void get_scale_factors ( float *translation_scale_factor, float *rotation_scale_factor )
  {
    if ( rotation_scale_factor != NULL )
      *rotation_scale_factor = rscale ;

    if ( translation_scale_factor != NULL )
      *translation_scale_factor = tscale ;
  }

  /* Scale factors are in meters-per-update and degrees-per-update */

  void set_scale_factors ( float translation_scale_factor, float rotation_scale_factor )
  {
    rscale =    rotation_scale_factor ;
    tscale = translation_scale_factor ;
  }


  void set_xyz_scale ( float translation_scale_factor )
  {
    tscale = translation_scale_factor ;
  }


  void set_hpr_scale ( float rotation_scale_factor )
  {
    rscale =    rotation_scale_factor ;
  }


  void set_reset ( sgCoord *pos )
  {
    sgCopyCoord ( & reset_pos, pos ) ;
  }

  sgCoord *get_velocity ()
  {
    sgCopyCoord ( & curr_vel, & velocity ) ;
    return & curr_vel ;
  }

  void set_velocity ( sgCoord *vel )
  {
    stop_all () ;
    sgCopyCoord ( & velocity, vel ) ;
    update   () ;
  }

  sgCoord *get_coord ()
  {
    sgCopyCoord ( & curr_pos, & last_pos ) ;
    return & curr_pos ;
  }

  void set_coord ( sgCoord *pos )
  {
    stop_all () ;
    sgCopyCoord ( & last_pos, pos ) ;
    update   () ;
  }

  void show_help ( FILE *fd = stderr )
  {
    fprintf ( fd, "ssgKeyFlier HELP.\n" ) ;
    fprintf ( fd, "=================\n\n" ) ;
    fprintf ( fd, "f  : Faster - Make translational controls more sensitive.\n" ) ;
    fprintf ( fd, "F  : Faster - Make rotational    controls more sensitive.\n" ) ;
    fprintf ( fd, "s  : Slower - Make translational controls less sensitive.\n" ) ;
    fprintf ( fd, "S  : Slower - Make rotational    controls less sensitive.\n" ) ;
    fprintf ( fd, "*,/: Same as 'f' and 's'.\n" ) ;
    fprintf ( fd, "r/R: Reset  - Reset eyepoint to pre-defined position.\n" ) ;
    fprintf ( fd, "h/H: Help   - Display this help message.\n" ) ;
    fprintf ( fd, "^C :        - Exit.\n" ) ;
    fprintf ( fd, "?  :        - Where am I?\n" ) ;
    fprintf ( fd, "5  : Stop rotation & slew     0 : Stop all movement.\n");
    fprintf ( fd, "2  : Pitch Up                 8 : Pitch Down    \n");
    fprintf ( fd, "4  : Yaw Left                 6 : Yaw Right\n");
    fprintf ( fd, "7  : Roll Left                9 : Roll Right    \n");
    fprintf ( fd, "1  : Translate Left.          3 : Translate Right\n");
    fprintf ( fd, "+/,: Translate Down           - : Translate Up\n");
    fprintf ( fd, " . : Translate Backwards  ENTER : Translate Forwards\n");
    fprintf ( fd, "[Note: Some of these controls may be overridden by the\n");
    fprintf ( fd, "       application program.].\n");
  }

  void incoming_keystroke ( char k )
  {
    switch ( k )
    {
      case  'h' :
      case  'H' : show_help  () ; break ;
      case  '/' :
      case  'f' : tscale *= 2.0 ; break ;
      case  'F' : rscale *= 2.0 ; break ;
      case  '*' :
      case  's' : tscale /= 2.0 ; break ;
      case  'S' : rscale /= 2.0 ; break ;

      case  0x03: if ( exit_action ) exit_action( 0 ) ; exit ( 1 ) ;

      case  '?' : if ( query_action ) 
                    query_action ( last_pos.xyz[0], last_pos.xyz[1], last_pos.xyz[2],
                                   last_pos.hpr[0], last_pos.hpr[1], last_pos.hpr[2] ) ;
                  else
                    ulSetError ( UL_DEBUG, "ssgKeyFlier: XYZ=(%g,%g,%g), HPR=(%g,%g,%g)",
                                 last_pos.xyz[0], last_pos.xyz[1], last_pos.xyz[2],
                                 last_pos.hpr[0], last_pos.hpr[1], last_pos.hpr[2] ) ;

                  break ;
      case  'r' :
      case  'R' : reset     () ; break ;
      case  '5' : stop_some () ; break ;
      case  '0' : stop_all  () ; break ;

      case  '1' : velocity.xyz[0] -= tscale ; break ;
      case  '3' : velocity.xyz[0] += tscale ; break ;
      case  '.' : velocity.xyz[1] -= tscale ; break ;
      case '\n' :
      case '\r' : velocity.xyz[1] += tscale ; break ;
      case  '+' : velocity.xyz[2] -= tscale ; break ;
      case  '-' : velocity.xyz[2] += tscale ; break ;

      case  '6' : velocity.hpr[0] -= rscale ; break ;
      case  '4' : velocity.hpr[0] += rscale ; break ;
/* Kludge to try to prevent *exact* +/-90 degree pitch terms */
      case  '8' : velocity.hpr[1] -= rscale+0.00001 ; break ;
      case  '2' : velocity.hpr[1] += rscale+0.00001 ; break ;

      case  '7' : velocity.hpr[2] -= rscale ; break ;
      case  '9' : velocity.hpr[2] += rscale ; break ;
    }
  }

  int isStationary ()
  {
    return ( velocity.hpr[0] == 0.0 &&
             velocity.hpr[1] == 0.0 &&
             velocity.hpr[2] == 0.0 &&
             velocity.xyz[0] == 0.0 &&
             velocity.xyz[1] == 0.0 &&
             velocity.xyz[2] == 0.0 ) ;
  }

  virtual void update ()
  {
    sgMat4 mat    ;
    sgMat4 result ;
    sgMat4 delta  ;

    /* Form new matrix */

    sgMakeCoordMat4 ( delta, & velocity  ) ;
    sgMakeCoordMat4 ( mat  , & last_pos  ) ;
    sgMultMat4      ( result, mat, delta ) ;
    sgSetCoord      ( &last_pos, result ) ;
  }
} ;


#endif

