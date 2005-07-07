#include "global.h"
#include "OptionIcon.h"
#include "ThemeManager.h"
#include "PlayerOptions.h"
#include "RageUtil.h"

#define TEXT_OFFSET_X	THEME->GetMetricF("OptionIcon","TextOffsetX")
#define TEXT_OFFSET_Y	THEME->GetMetricF("OptionIcon","TextOffsetY")
#define TEXT_H_ALIGN	THEME->GetMetricI("OptionIcon","TextHAlign")
#define TEXT_V_ALIGN	THEME->GetMetricI("OptionIcon","TextVAlign")
#define TEXT_WIDTH		THEME->GetMetricI("OptionIcon","TextWidth")
#define TEXT_ZOOM		THEME->GetMetricF("OptionIcon","TextZoom")
#define UPPERCASE		THEME->GetMetricB("OptionIcon","Uppercase")


OptionIcon::OptionIcon()
{
}

void OptionIcon::Load( CString sType )
{
	ASSERT( m_SubActors.empty() );	// don't load twice

	m_spr.Load( THEME->GetPathG(sType,"icon 3x2") );
	m_spr.StopAnimating();
	this->AddChild( &m_spr );

	m_text.LoadFromFont( THEME->GetPathF(sType,"icon") );
	m_text.SetShadowLength( 0 );
	m_text.SetZoom( TEXT_ZOOM );
	m_text.SetXY( TEXT_OFFSET_X, TEXT_OFFSET_Y );
	m_text.SetHorizAlign( (Actor::HorizAlign)TEXT_H_ALIGN );
	m_text.SetVertAlign( (Actor::VertAlign)TEXT_V_ALIGN );
	this->AddChild( &m_text );
}

void OptionIcon::Set( PlayerNumber pn, const CString &_sText, bool bHeader )
{
	CString sText = _sText;

	static const CString sStopWords[] = 
	{
		"1X",
		"DEFAULT",
		"OVERHEAD",
		"OFF",
	};
	
	for( unsigned i=0; i<ARRAYSIZE(sStopWords); i++ )
		if( 0==stricmp(sText,sStopWords[i]) )
			sText = "";

	if( UPPERCASE )
		sText.MakeUpper();

	sText.Replace( " ", "\n" );

	bool bVacant = (sText=="");
	int iState = pn*3 + (bHeader?0:(bVacant?1:2));
	m_spr.SetState( iState );

	m_text.SetText( bHeader ? CString("") : sText );
	m_text.SetZoom( TEXT_ZOOM );
	m_text.CropToWidth( TEXT_WIDTH );
}

/*
 * (c) 2002-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
