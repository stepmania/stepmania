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

#include "ssgLocal.h"

int stats_num_vertices    = 0 ;
int stats_isect_triangles = 0 ;
int stats_isect_test      = 0 ;
int stats_cull_test       = 0 ;
int stats_bind_textures   = 0 ;
int stats_num_leaves      = 0 ;

int stats_hot_triangles   = 0 ;
int stats_hot_test        = 0 ;
int stats_hot_no_trav     = 0 ;
int stats_hot_radius_reject=0 ;
int stats_hot_triv_accept = 0 ;
int stats_hot_straddle    = 0 ;

int stats_los_triangles   = 0 ;
int stats_los_test        = 0 ;
int stats_los_no_trav     = 0 ;
int stats_los_radius_reject=0 ;
int stats_los_triv_accept = 0 ;
int stats_los_straddle    = 0 ;

static char stats_string [ 1024 ] ;

char *ssgShowStats ()
{
  sprintf ( stats_string, "V=%4d, L=%3d H=%3d IS=%3d IT=%3d HT=%3d CT=%3d BT=%3d\n",
            stats_num_vertices   ,
            stats_num_leaves     ,
            stats_hot_triangles  ,
            stats_isect_triangles,
	    stats_isect_test     ,
	    stats_hot_test       ,
	    stats_cull_test      ,
	    stats_bind_textures  ) ;
/*
  sprintf ( stats_string, "Tri=%d, Tst=%d NoTr=%d Rej=%d Acp=%d Str=%d\n",
	    stats_hot_triangles   ,
	    stats_hot_test        ,
	    stats_hot_no_trav     ,
	    stats_hot_radius_reject,
	    stats_hot_triv_accept ,
	    stats_hot_straddle    ) ;
*/

  stats_num_vertices    = 0 ;
  stats_num_leaves      = 0 ;
  stats_isect_triangles = 0 ;
  stats_isect_test      = 0 ;
  stats_cull_test       = 0 ;
  stats_bind_textures   = 0 ;

  stats_hot_triangles   = 0 ;
  stats_hot_test        = 0 ;
  stats_hot_no_trav     = 0 ;
  stats_hot_radius_reject=0 ;
  stats_hot_triv_accept = 0 ;
  stats_hot_straddle    = 0 ;

  stats_los_triangles   = 0 ;
  stats_los_test        = 0 ;
  stats_los_no_trav     = 0 ;
  stats_los_radius_reject=0 ;
  stats_los_triv_accept = 0 ;
  stats_los_straddle    = 0 ;

  return stats_string ;
}



