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

void ssgStateSelector::copy_from ( ssgStateSelector *src, int clone_flags )
{
  ssgSimpleState::copy_from ( src, clone_flags ) ;

  nstates   = src -> getNumSteps   () ;
  selection = src -> getSelectStep () ;
  statelist = new ssgSimpleState * [ nstates ] ;

  for ( int i = 0 ; i < nstates ; i++ )
  {
    ssgSimpleState *s = src -> getStep ( i ) ;

    if ( s != NULL && (clone_flags & SSG_CLONE_STATE_RECURSIVE) )
      statelist [ i ] = (ssgSimpleState *)( s -> clone ( clone_flags )) ;
    else
      statelist [ i ] = s ;

	//~~ T.G. needs ref count incremented
	if (statelist [ i ] != NULL )      
	   statelist [ i ] -> ref();   

  }
}


ssgBase *ssgStateSelector::clone ( int clone_flags )
{
  ssgStateSelector *b = new ssgStateSelector ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgStateSelector::ssgStateSelector () 
{ 
  type = ssgTypeStateSelector () ;
  nstates = 0 ;
  selection = -1 ; 
  statelist = NULL ;
}

ssgStateSelector::ssgStateSelector ( int ns ) 
{ 
  type = ssgTypeStateSelector () ;
  nstates   = ns ;
  selection = -1 ; 
  statelist = new ssgSimpleState * [ nstates ] ;

  for ( int i = 0 ; i < nstates ; i++ )
    statelist [ i ] = NULL ;
}

ssgStateSelector::~ssgStateSelector (void)
{
  //~~ T.G. deref states before deleting list
  for ( int i = 0 ; i < nstates ; i++ )    
	  ssgDeRefDelete( statelist [ i ] );  
  delete [] statelist ;
}

void ssgStateSelector::selectStep ( unsigned int s )
{
  selection = s ;
}

unsigned int ssgStateSelector::getSelectStep (void)
{
  return selection ;
}

ssgSimpleState *ssgStateSelector::getCurrentStep  (void)
{
  return ( selection < 0 ||
           selection >= nstates ||
           statelist [ selection ] == NULL ) ? this : statelist[selection] ;
}

ssgSimpleState *ssgStateSelector::getStep ( int i )
{
  return ( i < 0 ||
           i >= nstates ||
           statelist [ i ] == NULL ) ? this : statelist[i] ;
}




void ssgStateSelector::setStep  (int i, ssgSimpleState *step)
{
  if ( i < 0 || i >= nstates ) return ;

  //~~ T.G. removed null test -- not necessary
  ssgDeRefDelete ( statelist[i] ) ;

  statelist [ i ] = step ;

  if ( step != NULL )
    step -> ref () ;
}


int ssgStateSelector::isTranslucent (void)
{
  return getCurrentStep()->isTranslucent() ;
}


void ssgStateSelector::setTranslucent (void)
{
  getCurrentStep()->setTranslucent() ;
}



void ssgStateSelector::setOpaque (void)
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setOpaque() ;
  else
    s -> setOpaque() ;
}


void ssgStateSelector::force (void)
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::force() ;
  else
    s -> force() ;
}

void ssgStateSelector::apply (void)
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::apply() ;
  else
    s -> apply() ;
}


void ssgStateSelector:: care_about ( int mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::care_about (mode) ;
  else
    s -> care_about (mode);
}


void ssgStateSelector::dont_care_about ( int mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::dont_care_about(mode) ;
  else
    s -> dont_care_about(mode);
}


int  ssgStateSelector::isEnabled ( GLenum mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::isEnabled(mode) ;
  else
    return s -> isEnabled(mode) ;
}

void ssgStateSelector::disable ( GLenum mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::disable(mode) ;
  else
    s  -> disable(mode) ;
}

void ssgStateSelector::enable ( GLenum mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::enable(mode) ;
  else
    s -> enable(mode) ;
}


void ssgStateSelector::setTexture ( char *fname, int _wrapu, int _wrapv,
				    int _mipmap )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setTexture ( fname, _wrapu, _wrapv, _mipmap ) ;
  else
    s  -> setTexture ( fname, _wrapu, _wrapv, _mipmap ) ;
}

char *ssgStateSelector::getTextureFilename (void)  
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::getTextureFilename() ;
  else
    return s -> getTextureFilename();
}

GLuint ssgStateSelector::getTextureHandle (void)  
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::getTextureHandle() ;
  else
    return s -> getTextureHandle() ;
}

void ssgStateSelector::setTexture ( ssgTexture *tex )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setTexture(tex) ;
  else
    s -> setTexture(tex) ;
}

void ssgStateSelector::setTextureFilename ( char *fname )  
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  ssgTexture* texture ;
  if ( s == this )
    texture = ssgSimpleState::getTexture () ;
  else
    texture = s -> getTexture () ;

  if ( texture != NULL )
    texture -> setFilename ( fname ) ;
}

void ssgStateSelector::setTexture ( GLuint tex )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  ssgTexture* texture ;
  if ( s == this )
    texture = ssgSimpleState::getTexture () ;
  else
    texture = s -> getTexture () ;
  
  if ( texture != NULL )
    texture -> setHandle (tex) ;
}

void ssgStateSelector::setColourMaterial(GLenum which)
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setColourMaterial(which) ;
  else
    s -> setColourMaterial(which) ;
}

void ssgStateSelector::setMaterial ( GLenum which, float r, float g,
                                                   float b, float a )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setMaterial(which,r,g,b,a) ;
  else
    s -> setMaterial(which,r,g,b,a) ;
}


void ssgStateSelector::setMaterial ( GLenum which, sgVec4 rgba )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setMaterial(which,rgba) ;
  else
    s -> setMaterial(which,rgba) ;
}

float *ssgStateSelector::getMaterial ( GLenum which )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::getMaterial(which) ;
  else
    return s -> getMaterial(which) ;
}

float ssgStateSelector::getShininess (void)
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::getShininess() ;
  else
    return s -> getShininess() ;
}

void ssgStateSelector::setShininess ( float sh )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setShininess(sh) ;
  else
    s -> setShininess(sh) ;
}

void ssgStateSelector::setShadeModel ( GLenum model )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setShadeModel(model) ;
  else
    s -> setShadeModel(model) ;
}

void ssgStateSelector::setAlphaClamp ( float clamp )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setAlphaClamp(clamp) ;
  else
    s -> setAlphaClamp(clamp) ;
}

void ssgStateSelector::print ( FILE *fd, char *indent, int how_much )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::print(fd, indent, how_much) ;
  else
    s -> print(fd, indent, how_much) ;
}


int ssgStateSelector::load ( FILE *fd )
{
  int i ;

  _ssgReadInt ( fd, & nstates   ) ;
  _ssgReadInt ( fd, & selection ) ;

  if (statelist != NULL)
  {
    for ( i = 0 ; i < nstates ; i++ )
      ssgDeRefDelete( statelist [ i ] );
    delete [] statelist ;
  }

  statelist = new ssgSimpleState * [ nstates ] ;
  for ( i = 0 ; i < nstates ; i++ )
  {
    if ( ! _ssgLoadObject ( fd, (ssgBase **) &statelist[i], ssgTypeState () ) )
      return FALSE ;
  }

  return ssgSimpleState::load(fd) ;
}


int ssgStateSelector::save ( FILE *fd )
{

  _ssgWriteInt ( fd, nstates   ) ;
  _ssgWriteInt ( fd, selection ) ;
  for ( int i = 0 ; i < nstates ; i++ )
  {
    if ( ! _ssgSaveObject ( fd, statelist[i] ) )
      return FALSE ;
  }

  return ssgSimpleState::save(fd) ;
}

