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

ScreenUnlock::ScreenUnlock() : ScreenAttract("ScreenUnlock")
{
		LOG->Trace("ScreenUnlock::ScreenUnlock()");
		PointsUntilNextUnlock.LoadFromFont( THEME->GetPathToF("Common normal") );
		PointsUntilNextUnlock.SetHorizAlign( Actor::align_left );

		CString sDP =  ssprintf( "%d", (int)GAMESTATE->m_pUnlockingSys->NumPointsUntilNextUnlock() );

		for(int i=1; i <= THEME->GetMetricI("ScreenUnlock", "NumUnlocks"); i++)
		{
			// new unlock graphic
			char filename[30];
			sprintf(filename, "UnlockGraphic%d", i);
			Unlocks[i].Load( THEME->GetPathToG(filename) );
			sprintf(filename, "Unlock%d", i);
			PointsUntilNextUnlock.SetXY( 
				THEME->GetMetricI("ScreenUnlock",ssprintf("Unlock%dX", i)),
				THEME->GetMetricI("ScreenUnlock",ssprintf("Unlock%dX", i)) );
			Unlocks[i].SetName( ssprintf("Unlock%d",i) );
			UtilSetXY( Unlocks[i], "ScreenUnlock" );
			if (!GAMESTATE->m_pUnlockingSys->SongIsLocked
				(THEME->GetMetric
				 ("ScreenUnlock", ssprintf
				  ("Unlock%dSong", i))))
				this->AddChild(&Unlocks[i]);
		}
		/* Remove the decimal
		if( sDP.Find(".",1) > 0 )
		{
			sDP = sDP.Left(sDP.Find(".",1));
		} no longer necessary since its an int*/
		
		// No negative numbers
		if( sDP.Left(1) == "-" ) 
			sDP = "*";
		
		PointsUntilNextUnlock.SetText( sDP );
		PointsUntilNextUnlock.SetZoom( THEME->GetMetricF("ScreenUnlock","DancePointsZoom") );
		PointsUntilNextUnlock.SetXY( THEME->GetMetricI("ScreenUnlock","DancePointsDisplayX"), THEME->GetMetricI("ScreenUnlock","DancePointsDisplayY") );
		this->AddChild( &PointsUntilNextUnlock );
}
