#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenLoading

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenLoading.h"
#include "GameState.h"

ScreenLoading::ScreenLoading()
{
	m_sprLoading.Load( THEME->GetPathTo(GRAPHIC_LOADING) );
	m_sprLoading.SetXY( CENTER_X, CENTER_Y ); 
	this->AddSubActor( &m_sprLoading );

	m_textMessage.Load( THEME->GetPathTo(FONT_NORMAL) ); 
	m_textMessage.SetXY( CENTER_X, CENTER_Y ); 
	this->AddSubActor( &m_textMessage );
}

void ScreenLoading::DrawPrimitives()
{
	m_textMessage.SetText( GAMESTATE->m_sLoadingMessage );

	Screen::DrawPrimitives();
}
