#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GrooveRadar

 Desc: See header.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "GrooveRadar.h"
#include "ThemeManager.h"
#include "RageBitmapTexture.h"


GrooveRadar::GrooveRadar()
{
	m_sprRadar.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_RADAR) );
	this->AddActor( &m_sprRadar );
}

void GrooveRadar::LoadFromSong( Song* pSong )		// NULL means no song
{

}
