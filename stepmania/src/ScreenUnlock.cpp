/*
-----------------------------------------------------------------------------
 Class: ScreenUnlock

 Desc: See header.

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	curewater
-----------------------------------------------------------------------------
*/

#include "global.h"
#include "PrefsManager.h"
#include "ScreenUnlock.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "RageLog.h"
#include "UnlockSystem.h"
#include "SongManager.h"

ScreenUnlock::ScreenUnlock() : ScreenAttract("ScreenUnlock")
{
	LOG->Trace("ScreenUnlock::ScreenUnlock()");
	PointsUntilNextUnlock.LoadFromFont( THEME->GetPathToF("Common normal") );
	PointsUntilNextUnlock.SetHorizAlign( Actor::align_left );

	CString sDP = ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->NumPointsUntilNextUnlock() );

	for(int i=1; i <= THEME->GetMetricI("ScreenUnlock", "NumUnlocks"); i++)
	{
		// new unlock graphic
		Unlocks[i].Load( THEME->GetPathToG(ssprintf("ScreenUnlock icon %d", i)) );

		Unlocks[i].SetName( ssprintf("Unlock%d",i) );
		SET_XY( Unlocks[i] );

		Song *pSong = SONGMAN->FindSong("", THEME->GetMetric("ScreenUnlock", ssprintf("Unlock%dSong", i)) );
		if( pSong == NULL )
			continue;

		const bool SongIsLocked = GAMESTATE->m_pUnlockingSys->SongIsLocked( pSong );
		if ( !SongIsLocked )
			this->AddChild(&Unlocks[i]);
	}

	// No negative numbers
	if( sDP.Left(1) == "-" ) 
		sDP = "*";

	PointsUntilNextUnlock.SetName( "DancePointsDisplay" );
	PointsUntilNextUnlock.SetText( sDP );
	PointsUntilNextUnlock.SetZoom( THEME->GetMetricF("ScreenUnlock","DancePointsZoom") );
	SET_XY( PointsUntilNextUnlock );
	this->AddChild( &PointsUntilNextUnlock );
}
