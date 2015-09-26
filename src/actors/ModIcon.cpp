#include "global.h"
#include "ModIcon.h"
#include "ThemeManager.h"
#include "PlayerOptions.h"
#include "RageUtil.h"
#include "ActorUtil.h"

ModIcon::ModIcon()
{
}

ModIcon::ModIcon( const ModIcon &cpy ):
	ActorFrame(cpy),
	m_text(cpy.m_text),
	m_sprFilled(cpy.m_sprFilled),
	m_sprEmpty(cpy.m_sprEmpty),
	CROP_TEXT_TO_WIDTH(cpy.CROP_TEXT_TO_WIDTH),
	STOP_WORDS(cpy.STOP_WORDS),
	m_vStopWords(cpy.m_vStopWords)
{
	this->RemoveAllChildren();
	this->AddChild( m_sprFilled );
	this->AddChild( m_sprEmpty );
	this->AddChild( &m_text );
}

void ModIcon::Load( RString sMetricsGroup )
{
	m_sprFilled.Load( THEME->GetPathG(sMetricsGroup,"Filled") );
	m_sprFilled->SetName("Filled");
	ActorUtil::LoadAllCommands( m_sprFilled, sMetricsGroup );
	this->AddChild( m_sprFilled );

	m_sprEmpty.Load( THEME->GetPathG(sMetricsGroup,"Empty") );
	m_sprEmpty->SetName("Empty");
	ActorUtil::LoadAllCommands( m_sprEmpty, sMetricsGroup );
	this->AddChild( m_sprEmpty );

	m_text.LoadFromFont( THEME->GetPathF(sMetricsGroup,"Text") );
	m_text.SetName( "Text" );
	ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_text, sMetricsGroup );
	this->AddChild( &m_text );

	CROP_TEXT_TO_WIDTH.Load( sMetricsGroup, "CropTextToWidth" );

	// stop words
	STOP_WORDS.Load( sMetricsGroup, "StopWords" );
	m_vStopWords.empty();
	split(STOP_WORDS, ",", m_vStopWords);

	Set("");
}

void ModIcon::Set( const RString &_sText )
{
	RString sText = _sText;

	for( unsigned i = 0; i < m_vStopWords.size(); i++ )
		if( sText.EqualsNoCase(m_vStopWords[i]) )
			sText = "";

	sText.Replace( " ", "\n" );

	bool bVacant = (sText=="");
	m_sprFilled->SetVisible( !bVacant );
	m_sprEmpty->SetVisible( bVacant );

	m_text.SetText( sText );
	m_text.CropToWidth( CROP_TEXT_TO_WIDTH );
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
