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

		CString sDP =  ssprintf( "%f", GAMESTATE->m_pUnlockingSys->NumPointsUntilNextUnlock() );
	
		// Remove the decimal
		if( sDP.Find(".",1) > 0 )
		{
			sDP = sDP.Left(sDP.Find(".",1));
		}
		
		// No negative numbers
		if( sDP.Left(1) == "-" ) 
		{ 
			sDP = "*";
		};
		
		PointsUntilNextUnlock.SetText( sDP );
		PointsUntilNextUnlock.SetZoom( 3 );
		PointsUntilNextUnlock.SetXY( 10, 370 );
		this->AddChild( &PointsUntilNextUnlock );
}