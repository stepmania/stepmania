#include "global.h"
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
#include "ThemeManager.h"


CachedThemeMetric	LABEL_X				("Combo","LabelX");
CachedThemeMetric	LABEL_Y				("Combo","LabelY");
CachedThemeMetric	LABEL_HORIZ_ALIGN	("Combo","LabelHorizAlign");
CachedThemeMetric	LABEL_VERT_ALIGN	("Combo","LabelVertAlign");
CachedThemeMetric	NUMBER_X			("Combo","NumberX");
CachedThemeMetric	NUMBER_Y			("Combo","NumberY");
CachedThemeMetric	NUMBER_HORIZ_ALIGN	("Combo","NumberHorizAlign");
CachedThemeMetric	NUMBER_VERT_ALIGN	("Combo","NumberVertAlign");
CachedThemeMetric	SHOW_COMBO_AT		("Combo","ShowComboAt");
CachedThemeMetric	NUMBER_MIN_ZOOM		("Combo","NumberMinZoom");
CachedThemeMetric	NUMBER_MAX_ZOOM		("Combo","NumberMaxZoom");
CachedThemeMetric	NUMBER_MAX_ZOOM_AT	("Combo","NumberMaxZoomAt");
CachedThemeMetric	PULSE_ZOOM			("Combo","PulseZoom");
CachedThemeMetric	C_TWEEN_SECONDS		("Combo","TweenSeconds");


Combo::Combo()
{
	LABEL_X.Refresh();
	LABEL_Y.Refresh();
	LABEL_HORIZ_ALIGN.Refresh();
	LABEL_VERT_ALIGN.Refresh();
	NUMBER_X.Refresh();
	NUMBER_Y.Refresh();
	NUMBER_HORIZ_ALIGN.Refresh();
	NUMBER_VERT_ALIGN.Refresh();
	SHOW_COMBO_AT.Refresh();
	NUMBER_MIN_ZOOM.Refresh();
	NUMBER_MAX_ZOOM.Refresh();
	NUMBER_MAX_ZOOM_AT.Refresh();
	PULSE_ZOOM.Refresh();
	C_TWEEN_SECONDS.Refresh();

	Reset();

	m_sprCombo.Load( THEME->GetPathTo("Graphics", "gameplay combo label") );
	m_sprCombo.EnableShadow( true );
	m_sprCombo.StopAnimating();
	m_sprCombo.SetXY( LABEL_X, LABEL_Y );
	m_sprCombo.SetHorizAlign( (Actor::HorizAlign)(int)LABEL_HORIZ_ALIGN );
	m_sprCombo.SetVertAlign( (Actor::VertAlign)(int)LABEL_VERT_ALIGN );
	this->AddChild( &m_sprCombo );

	m_textComboNumber.LoadFromNumbers( THEME->GetPathTo("Numbers","gameplay combo numbers") );
	m_textComboNumber.EnableShadow( true );
	m_textComboNumber.SetXY( NUMBER_X, NUMBER_Y );
	m_textComboNumber.SetHorizAlign( (Actor::HorizAlign)(int)NUMBER_HORIZ_ALIGN );
	m_textComboNumber.SetVertAlign( (Actor::VertAlign)(int)NUMBER_VERT_ALIGN );
	this->AddChild( &m_textComboNumber );
}

void Combo::Reset()
{
	m_iCurCombo = m_iMaxCombo = m_iCurComboOfPerfects = 0; 

	m_textComboNumber.SetDiffuse( RageColor(1,1,1,0) );	// invisible
	m_sprCombo.SetDiffuse( RageColor(1,1,1,0) );	// invisible
}

void Combo::SetScore( TapNoteScore score, int iNumNotesInThisRow, Inventory* pInventory )
{
#ifndef DEBUG
	if( PREFSMAN->m_bAutoPlay && !GAMESTATE->m_bDemonstration )	// cheaters never prosper
	{
		m_iCurCombo = 0;
		m_iCurComboOfPerfects = 0;
		return;
	}
#endif //DEBUG

	// combo of marvelous/perfect
	switch( score )
	{
	case TNS_MARVELOUS:
	case TNS_PERFECT:
		m_iCurComboOfPerfects += iNumNotesInThisRow;

		if( rand() < 0.1f && m_iCurComboOfPerfects>=150 && (m_iCurComboOfPerfects%150)==0 && !GAMESTATE->m_bDemonstration )
			SCREENMAN->SendMessageToTopScreen( SM_BeginToasty, 0 );
		break;
	default:
		m_iCurComboOfPerfects = 0;
		break;
	}

	// combo of marvelous/perfect/great
	switch( score )
	{
	case TNS_MARVELOUS:
	case TNS_PERFECT:
	case TNS_GREAT:
		{
			int iOldCombo = m_iCurCombo;

			m_iCurCombo += iNumNotesInThisRow;			// continue combo

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


			if( m_iCurCombo >= (int)SHOW_COMBO_AT )
			{
				m_textComboNumber.SetDiffuse( RageColor(1,1,1,1) );	// visible
				m_sprCombo.SetDiffuse( RageColor(1,1,1,1) );	// visible

				m_textComboNumber.SetText( ssprintf("%d", m_iCurCombo) );
				float fNumberZoom = SCALE(m_iCurCombo,0.f,(float)NUMBER_MAX_ZOOM_AT,(float)NUMBER_MIN_ZOOM,(float)NUMBER_MAX_ZOOM);
				CLAMP( fNumberZoom, (float)NUMBER_MIN_ZOOM, (float)NUMBER_MAX_ZOOM );
				m_textComboNumber.SetZoom( fNumberZoom * (float)PULSE_ZOOM ); 
				m_textComboNumber.BeginTweening( C_TWEEN_SECONDS );
				m_textComboNumber.SetTweenZoom( fNumberZoom );

				m_sprCombo.SetZoom( PULSE_ZOOM ); 
				m_sprCombo.BeginTweening( C_TWEEN_SECONDS );
				m_sprCombo.SetTweenZoom( 1 );
			}
			else
			{
				m_textComboNumber.SetDiffuse( RageColor(1,1,1,0) );	// invisible
				m_sprCombo.SetDiffuse( RageColor(1,1,1,0) );	// invisible
			}

		}
		break;
	case TNS_GOOD:
	case TNS_BOO:
	case TNS_MISS:
		{
			// don't play "combo stopped" in battle
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_BATTLE:
				if( pInventory )
					pInventory->OnComboBroken( m_PlayerNumber, m_iCurCombo );
			default:
				if( m_iCurCombo>50 )
					SCREENMAN->SendMessageToTopScreen( SM_ComboStopped, 0 );
			}

			m_iCurCombo = 0;

			m_textComboNumber.SetDiffuse( RageColor(1,1,1,0) );	// invisible
			m_sprCombo.SetDiffuse( RageColor(1,1,1,0) );	// invisible
		}
		break;
	default:
		ASSERT(0);
	}
}
