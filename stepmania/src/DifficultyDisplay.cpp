#include "global.h"
#include "DifficultyDisplay.h"
#include "song.h"
#include "ThemeManager.h"

CachedThemeMetric  ICONONCOMMAND	("DifficultyDisplay","IconOnCommand");
CachedThemeMetric  ICONOFFCOMMAND	("DifficultyDisplay","IconOffCommand");

DifficultyDisplay::DifficultyDisplay()
{
	ICONONCOMMAND.Refresh();
	ICONOFFCOMMAND.Refresh();
	float fHeight = 0;
	int diff;
	for( diff = DIFFICULTY_BEGINNER; diff <= DIFFICULTY_CHALLENGE; ++diff )
	{
		m_difficulty[diff].Load( THEME->GetPathToG(ssprintf("DifficultyDisplay bar %dx1",NUM_DIFFICULTIES)) );
		m_difficulty[diff].SetState(diff);
		m_difficulty[diff].StopAnimating();
		this->AddChild( &m_difficulty[diff] );

		fHeight += m_difficulty[diff].GetUnzoomedHeight();
	}

	float fY = -fHeight/2;
	for( diff = DIFFICULTY_BEGINNER; diff <= DIFFICULTY_CHALLENGE; ++diff )
	{
		m_difficulty[diff].SetHorizAlign( align_left );
		m_difficulty[diff].SetY( fY );
		fY += m_difficulty[diff].GetUnzoomedHeight();
	}
}

void DifficultyDisplay::SetDifficulties( const Song* pSong, StepsType curType )
{
	for( int diff = DIFFICULTY_BEGINNER; diff <= DIFFICULTY_CHALLENGE; ++diff )
	{
		if( pSong->HasStepsTypeAndDifficulty( curType, Difficulty(diff) ) )
			m_difficulty[diff].Command( ICONONCOMMAND );
		else
			m_difficulty[diff].Command( ICONOFFCOMMAND );
	}
}

void DifficultyDisplay::UnsetDifficulties()
{
	for( int diff = DIFFICULTY_BEGINNER; diff <= DIFFICULTY_CHALLENGE; ++diff )
		m_difficulty[diff].Command( ICONOFFCOMMAND );
}

/*
 * (c) 2003 Steven Towle
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
