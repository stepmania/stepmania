#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: TextBanner.h

 Desc: The song's TextBanner displayed in SelectSong.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "TextBanner.h"

TextBanner::TextBanner()
{
	m_textTitle.LoadFromFontName( "Arial Bold" );
	m_textSubTitle.LoadFromFontName( "Arial Bold" );
	m_textArtist.LoadFromFontName( "Arial Bold" );

	this->AddActor( &m_textTitle );
	this->AddActor( &m_textSubTitle );
	this->AddActor( &m_textArtist );
	this->SetZoom( 0.5f );


}


bool TextBanner::LoadFromSong( Song &song )
{
	CString sTitle = song.GetTitle();
	CString sSubTitle;

	m_textTitle.SetText( sTitle );
	m_textTitle.SetZoom( 1.0f );
	m_textTitle.SetXY( 0, -30 );

	m_textSubTitle.SetText( sSubTitle );
	m_textTitle.SetZoom( 0.5f );
	m_textSubTitle.SetXY( 0, 0 );

	m_textArtist.SetText( song.GetArtist() );
	m_textTitle.SetZoom( 0.5f );
	m_textArtist.SetXY( 0, 30 );


	

	return true;
}
