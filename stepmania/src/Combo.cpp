#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Combo

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Combo.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "ScreenGameplay.h"
#include "GameState.h"


void Combo::Reset()
{
	m_iCurCombo = m_iMaxCombo = m_iCurComboOfPerfects = 0; 

	m_textComboNumber.SetDiffuse( RageColor(1,1,1,0) );	// invisible
	m_sprCombo.SetDiffuse( RageColor(1,1,1,0) );	// invisible
}

Combo::Combo()
{
	Reset();

	m_sprCombo.Load( THEME->GetPathTo("Graphics", "gameplay combo label") );
	m_sprCombo.TurnShadowOn();
	m_sprCombo.StopAnimating();
	m_sprCombo.SetX( 40 );
	m_sprCombo.SetZoom( 1.0f );

	m_textComboNumber.LoadFromNumbers( THEME->GetPathTo("Numbers","gameplay combo numbers") );
	m_textComboNumber.TurnShadowOn();
	m_textComboNumber.SetHorizAlign( Actor::align_right );
	m_textComboNumber.SetX( 0 );

	this->AddChild( &m_textComboNumber );
	this->AddChild( &m_sprCombo );
}


void Combo::UpdateScore( TapNoteScore score, int iNumNotesInThisRow )
{
	if( PREFSMAN->m_bAutoPlay && !GAMESTATE->m_bDemonstration )	// cheaters never prosper
	{
		m_iCurCombo = 0;
		m_iCurComboOfPerfects = 0;
		return;
	}

	switch( score )
	{
	case TNS_PERFECT:
	case TNS_GREAT:
		{
			int iOldCombo = m_iCurCombo;

			m_iCurCombo += iNumNotesInThisRow;			// continue combo

			if( score == TNS_PERFECT )	m_iCurComboOfPerfects += iNumNotesInThisRow;
			else						m_iCurComboOfPerfects = 0;

			if( m_iCurComboOfPerfects>=150  &&  (m_iCurComboOfPerfects%150)==0  &&  RandomFloat(0,1) > 0.5  &&  !GAMESTATE->m_bDemonstration )
				SCREENMAN->SendMessageToTopScreen( SM_BeginToasty, 0 );


	#define CROSSED( i ) (iOldCombo<i && i<=m_iCurCombo)

			if     ( CROSSED(100) )	SCREENMAN->SendMessageToTopScreen( SM_100Combo, 0 );
			else if( CROSSED(200) )	SCREENMAN->SendMessageToTopScreen( SM_200Combo, 0 );
			else if( CROSSED(300) )	SCREENMAN->SendMessageToTopScreen( SM_300Combo, 0 );
			else if( CROSSED(400) )	SCREENMAN->SendMessageToTopScreen( SM_400Combo, 0 );
			else if( CROSSED(500) )	SCREENMAN->SendMessageToTopScreen( SM_500Combo, 0 );
			else if( CROSSED(600) )	SCREENMAN->SendMessageToTopScreen( SM_600Combo, 0 );
			else if( CROSSED(700) )	SCREENMAN->SendMessageToTopScreen( SM_700Combo, 0 );
			else if( CROSSED(800) )	SCREENMAN->SendMessageToTopScreen( SM_800Combo, 0 );
			else if( CROSSED(900) )	SCREENMAN->SendMessageToTopScreen( SM_900Combo, 0 );
			else if( CROSSED(1000))	SCREENMAN->SendMessageToTopScreen( SM_1000Combo, 0 );

			// new max combo
			m_iMaxCombo = max(m_iMaxCombo, m_iCurCombo);


			if( m_iCurCombo <= 4 )
			{
				m_textComboNumber.SetDiffuse( RageColor(1,1,1,0) );	// invisible
				m_sprCombo.SetDiffuse( RageColor(1,1,1,0) );	// invisible
			}
			else
			{
				m_textComboNumber.SetDiffuse( RageColor(1,1,1,1) );	// visible
				m_sprCombo.SetDiffuse( RageColor(1,1,1,1) );	// visible

				m_textComboNumber.SetText( ssprintf("%d", m_iCurCombo) );
				float fNewZoom = min( 0.5f + m_iCurCombo/800.0f, 1.0f );
				m_textComboNumber.SetZoom( fNewZoom ); 
				
				//this->SetZoom( 1.2f );
				//this->BeginTweening( 0.3f );
				//this->SetTweenZoom( 1 );
			}
		}
		break;
	case TNS_GOOD:
	case TNS_BOO:
	case TNS_MISS:
		// end combo
		if( m_iCurCombo > 50 )
			SCREENMAN->SendMessageToTopScreen( SM_ComboStopped, 0 );

		m_iCurCombo = 0;

		m_textComboNumber.SetDiffuse( RageColor(1,1,1,0) );	// invisible
		m_sprCombo.SetDiffuse( RageColor(1,1,1,0) );	// invisible
		break;
	default:
		ASSERT(0);
	}
}
