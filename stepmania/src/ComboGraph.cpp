#include "global.h"
#include "ComboGraph.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "StageStats.h"
#include "ActorUtil.h"
#include "BitmapText.h"

const int MinComboSizeToShow = 5;

void ComboGraph::Load( CString Path, const StageStats &s, PlayerNumber pn )
{
	ASSERT( m_SubActors.size() == 0 );

	/* Find the largest combo. */
	int MaxComboSize = 0;
	unsigned i;
	for( i = 0; i < s.ComboList[pn].size(); ++i )
		MaxComboSize = max( MaxComboSize, s.ComboList[pn][i].GetStageCnt() );

	float width = -1;
	for( i = 0; i < s.ComboList[pn].size(); ++i )
	{
		const StageStats::Combo_t &combo = s.ComboList[pn][i];
		if( combo.GetStageCnt() < MinComboSizeToShow )
			continue; /* too small */

		const bool IsMax = (combo.GetStageCnt() == MaxComboSize);

		LOG->Trace("combo %i is %f+%f", i, combo.fStartSecond, combo.fSizeSeconds);
		Sprite *sprite = new Sprite;
		sprite->SetName( "ComboBar" );
		const CString path = ssprintf( "%s %s", Path.c_str(), IsMax? "max":"normal" );
		sprite->Load( THEME->GetPathToG(path) );

		const float start = SCALE( combo.fStartSecond, s.fFirstSecond[pn], s.fLastSecond[pn], 0.0f, 1.0f );
		const float size = SCALE( combo.fSizeSeconds, 0, s.fLastSecond[pn]-s.fFirstSecond[pn], 0.0f, 1.0f );
		sprite->SetCropLeft ( SCALE( size, 0.0f, 1.0f, 0.5f, 0.0f ) );
		sprite->SetCropRight( SCALE( size, 0.0f, 1.0f, 0.5f, 0.0f ) );

		sprite->BeginTweening( .5f );
		sprite->SetCropLeft( start );
		sprite->SetCropRight( 1 - (size + start) );

		if( width < 0 )
			width = sprite->GetUnzoomedWidth();

		m_Sprites.push_back( sprite );
		this->AddChild( sprite );
	}

	for( i = 0; i < s.ComboList[pn].size(); ++i )
	{
		const StageStats::Combo_t &combo = s.ComboList[pn][i];
		if( combo.GetStageCnt() < MinComboSizeToShow )
			continue; /* too small */
	
		if( !MaxComboSize )
			continue;

		const bool IsMax = (combo.GetStageCnt() == MaxComboSize);
		if( !IsMax )
			continue;

		BitmapText *text = new BitmapText;
		text->SetName( "ComboMaxNumber" );
		text->LoadFromFont( THEME->GetPathToF(Path) );

		const float start = SCALE( combo.fStartSecond, s.fFirstSecond[pn], s.fLastSecond[pn], 0.0f, 1.0f );
		const float size = SCALE( combo.fSizeSeconds, 0, s.fLastSecond[pn]-s.fFirstSecond[pn], 0.0f, 1.0f );

		const float CenterPercent = start + size/2;
		const float CenterXPos = SCALE( CenterPercent, 0.0f, 1.0f, -width/2.0f, width/2.0f );
		text->SetX( CenterXPos );

		text->SetText( ssprintf("%i",combo.GetStageCnt()) );
		ON_COMMAND( text );

		m_Numbers.push_back( text );
		this->AddChild( text );
	}
}

void ComboGraph::TweenOffScreen()
{
	for( unsigned i = 0; i < m_SubActors.size(); ++i )
		OFF_COMMAND( m_SubActors[i] );
}

void ComboGraph::Unload()
{
	for( unsigned i = 0; i < m_SubActors.size(); ++i )
		delete m_SubActors[i];

	m_Sprites.clear();
	m_Numbers.clear();
	m_SubActors.clear();
}

/*
 * (c) 2003 Glenn Maynard
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
