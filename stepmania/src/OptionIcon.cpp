#include "stdafx.h"
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

#define TEXT_X			THEME->GetMetricF("OptionIcon","TextX")
#define TEXT_Y			THEME->GetMetricF("OptionIcon","TextY")
#define TEXT_H_ALIGN	THEME->GetMetricI("OptionIcon","TextHAlign")
#define TEXT_V_ALIGN	THEME->GetMetricI("OptionIcon","TextVAlign")
#define TEXT_WIDTH		THEME->GetMetricI("OptionIcon","TextWidth")


OptionIcon::OptionIcon()
{
	m_spr.Load( THEME->GetPathTo("Graphics","select music option icons 3x2") );
	this->AddChild( &m_spr );

	m_text.LoadFromFont( THEME->GetPathTo("Fonts","option icons") );
	this->AddChild( &m_text );
}

void OptionIcon::Load( PlayerNumber pn, CString sText, bool bHeader )
{
	bool bVacant = (sText=="");
	m_spr.SetState( pn*3 + bVacant?1:2 );

	m_text.SetText( sText );
	m_text.CropToWidth( TEXT_WIDTH );
	m_text.SetXY( TEXT_X, TEXT_Y );
	m_text.SetHorizAlign( (Actor::HorizAlign)TEXT_H_ALIGN );
	m_text.SetVertAlign( (Actor::VertAlign)TEXT_V_ALIGN );
}
