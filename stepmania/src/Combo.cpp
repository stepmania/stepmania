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

	m_sprCombo.Load( THEME->GetPathTo("Graphics", "Combo label") );
	m_sprCombo.EnableShadow( true );
	m_sprCombo.StopAnimating();
	m_sprCombo.SetXY( LABEL_X, LABEL_Y );
	m_sprCombo.SetHorizAlign( (Actor::HorizAlign)(int)LABEL_HORIZ_ALIGN );
	m_sprCombo.SetVertAlign( (Actor::VertAlign)(int)LABEL_VERT_ALIGN );
	m_sprCombo.SetDiffuse( RageColor(1,1,1,0) );	// invisible
	this->AddChild( &m_sprCombo );

	m_textComboNumber.LoadFromNumbers( THEME->GetPathTo("Numbers","Combo numbers") );
	m_textComboNumber.EnableShadow( true );
	m_textComboNumber.SetXY( NUMBER_X, NUMBER_Y );
	m_textComboNumber.SetHorizAlign( (Actor::HorizAlign)(int)NUMBER_HORIZ_ALIGN );
	m_textComboNumber.SetVertAlign( (Actor::VertAlign)(int)NUMBER_VERT_ALIGN );
	m_textComboNumber.SetDiffuse( RageColor(1,1,1,0) );	// invisible
	this->AddChild( &m_textComboNumber );
}

void Combo::SetCombo( int iCombo )
{
	if( iCombo >= (int)SHOW_COMBO_AT )
	{
		m_textComboNumber.SetDiffuse( RageColor(1,1,1,1) );	// visible
		m_sprCombo.SetDiffuse( RageColor(1,1,1,1) );	// visible

		m_textComboNumber.SetText( ssprintf("%d", iCombo) );
		float fNumberZoom = SCALE(iCombo,0.f,(float)NUMBER_MAX_ZOOM_AT,(float)NUMBER_MIN_ZOOM,(float)NUMBER_MAX_ZOOM);
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
