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

static ssgStateCallback  _ssgPendingPostDrawCB = NULL ;
static ssgState         *_ssgPreviousState = NULL ;

void ssgState::preApply ()
{
  if ( _ssgPendingPostDrawCB )
    (*_ssgPendingPostDrawCB)(_ssgPreviousState) ;

  if ( preApplyCB ) (*preApplyCB)(this) ;
}


void ssgState::preDraw ()
{
  if ( preDrawCB )
  {
    (*preDrawCB)(this) ;
    _ssgPendingPostDrawCB = postDrawCB ;
    _ssgPreviousState = this ;
  }
}

void _ssgStartOfFrameInit ()
{
  _ssgPendingPostDrawCB = NULL ;
  _ssgPreviousState     = NULL ;
}

void _ssgEndOfFrameCleanup ()
{
  if ( _ssgPendingPostDrawCB ) (*_ssgPendingPostDrawCB)(_ssgPreviousState) ;

  _ssgPendingPostDrawCB = NULL ;
  _ssgPreviousState     = NULL ;
}

void ssgState::copy_from ( ssgState *src, int clone_flags )
{
  ssgBase::copy_from ( src, clone_flags ) ;
  setExternalPropertyIndex ( src -> getExternalPropertyIndex () ) ;

  if ( src -> isTranslucent () )
    setTranslucent () ;
  else
    setOpaque () ;
}



ssgState::ssgState (void)
{
  type = ssgTypeState () ;

  preApplyCB = NULL ;
  preDrawCB  = NULL ;
  postDrawCB = NULL ;

  setOpaque () ;
  setExternalPropertyIndex ( 0 ) ;
}

ssgState::~ssgState (void)
{
  if ( _ssgPreviousState == this )
  {
    _ssgPendingPostDrawCB = NULL ;
    _ssgPreviousState     = NULL ;
  }
}


void ssgState::print ( FILE *fd, char *indent, int how_much )
{
  ssgBase::print ( fd, indent, how_much ) ;

  if ( how_much < 2 )
    return;

  fprintf ( fd, "%s  Translucent  = %s\n", indent, translucent?"True":"False");
  fprintf ( fd, "%s  ExternalProp = %d\n", indent, external_property_index ) ;
}

 
int ssgState::load ( FILE *fd )
{
  _ssgReadInt ( fd, & translucent ) ;
  _ssgReadInt ( fd, & external_property_index ) ;

  preApplyCB = NULL ;
  preDrawCB  = NULL ;
  postDrawCB = NULL ;

  return ssgBase::load ( fd ) ;
}


int ssgState::save ( FILE *fd )
{
  _ssgWriteInt ( fd, translucent ) ;
  _ssgWriteInt ( fd, external_property_index ) ;
  return ssgBase::save ( fd ) ;
}




