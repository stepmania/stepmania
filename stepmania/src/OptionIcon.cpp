#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: OptionIcon

 Desc: A graphic displayed in the OptionIcon during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "OptionIcon.h"
#include "ThemeManager.h"
#include "PlayerOptions.h"

#define TEXT_OFFSET_X	THEME->GetMetricF("OptionIcon","TextOffsetX")
#define TEXT_OFFSET_Y	THEME->GetMetricF("OptionIcon","TextOffsetY")
#define TEXT_H_ALIGN	THEME->GetMetricI("OptionIcon","TextHAlign")
#define TEXT_V_ALIGN	THEME->GetMetricI("OptionIcon","TextVAlign")
#define TEXT_WIDTH		THEME->GetMetricI("OptionIcon","TextWidth")
#define TEXT_ZOOM		THEME->GetMetricF("OptionIcon","TextZoom")
#define UPPERCASE		THEME->GetMetricB("OptionIcon","Uppercase")


OptionIcon::OptionIcon()
{
	m_spr.Load( THEME->GetPathToG("OptionIcon frame 3x2") );
	m_spr.StopAnimating();
	this->AddChild( &m_spr );

	m_text.LoadFromFont( THEME->GetPathToF("OptionIcon") );
	m_text.EnableShadow( false );
	m_text.SetZoom( TEXT_ZOOM );
	m_text.SetXY( TEXT_OFFSET_X, TEXT_OFFSET_Y );
	m_text.SetHorizAlign( (Actor::HorizAlign)TEXT_H_ALIGN );
	m_text.SetVertAlign( (Actor::VertAlign)TEXT_V_ALIGN );
	this->AddChild( &m_text );
}

void OptionIcon::Load( PlayerNumber pn, CString sText, bool bHeader )
{
	static CString sStopWords[] = { "OFF", "VISIBLE", "VIVID", "STANDARD", "X1", "HOLDS", "DEFAULT", "OVERHEAD" };
	const int iNumStopWords = sizeof(sStopWords)/sizeof(sStopWords[0]);
	
	for( int i=0; i<iNumStopWords; i++ )
		if( 0==stricmp(sText,sStopWords[i]) )
			sText = "";

	if( UPPERCASE )
		sText.MakeUpper();

	sText.Replace( " ", "\n" );

	bool bVacant = (sText=="");
	m_spr.SetState( pn*3 + (bHeader?0:(bVacant?1:2)) );

	m_text.SetText( bHeader ? "" : sText );
	m_text.SetZoom( TEXT_ZOOM );
	m_text.CropToWidth( TEXT_WIDTH );
}

void OptionIcon::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}

