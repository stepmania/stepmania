#ifndef BGANIMATION_H
#define BGANIMATION_H
/*
-----------------------------------------------------------------------------
 Class: BGAnimation

 Desc: An ActorFrame that loads itself

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorScroller.h"

class BGAnimation : public ActorScroller
{
public:
	BGAnimation( bool Generic=false );
	virtual ~BGAnimation();

	void Unload();

	void LoadFromStaticGraphic( CString sPath );
	void LoadFromAniDir( CString sAniDir );
	void LoadFromMovie( CString sMoviePath );
	void LoadFromVisualization( CString sMoviePath );

	float GetLengthSeconds() const { return m_fLengthSeconds; }

protected:
	float	m_fLengthSeconds;
	bool	m_bGeneric;
};


#endif
