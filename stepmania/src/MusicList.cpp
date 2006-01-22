#include "global.h"
#include "MusicList.h"
#include "ThemeManager.h"
#include "ThemeMetric.h"
#include "RageUtil.h"

/* If this actor is used anywhere other than SelectGroup, we
 * can add a setting that changes which metric group we pull
 * settings out of, so it can be configured separately. */
static const ThemeMetric<int>			NUM_COLUMNS		("MusicList","NumColumns");
static const ThemeMetric<int>			NUM_ROWS		("MusicList","NumRows");
static const ThemeMetric<float>			START_X			("MusicList","StartX");
static const ThemeMetric<float>			START_Y			("MusicList","StartY");
static const ThemeMetric<float>			SPACING_X		("MusicList","SpacingX");
static const ThemeMetric<float>			CROP_WIDTH		("MusicList","CropWidth");
static const ThemeMetric<apActorCommands> INIT_COMMAND	("MusicList","InitCommand");

MusicList::MusicList()
{
	CurGroup = 0;
}

void MusicList::Load()
{
	for( int i=0; i<NUM_COLUMNS; i++ )
	{
		m_textTitles[i].LoadFromFont( THEME->GetPathF("MusicList","titles") );
		m_textTitles[i].SetXY( START_X + i*SPACING_X, START_Y );
		m_textTitles[i].RunCommands( INIT_COMMAND );
		this->AddChild( &m_textTitles[i] );
	}
}

void MusicList::AddGroup()
{
	m_ContentsText.push_back(group());
}

void MusicList::AddSongsToGroup(const vector<Song*> &Songs)
{
	// Generate what text will show in the contents for each group
	unsigned group = m_ContentsText.size()-1;

	m_ContentsText[group].m_iNumSongsInGroup = Songs.size();

	for( int c=0; c<NUM_COLUMNS; c++ )	// foreach col
	{
		RString sText;
		for( int r=0; r<NUM_ROWS; r++ )	// foreach row
		{
			unsigned iIndex = c*NUM_ROWS + r;
			if( iIndex >= Songs.size() )
				continue;

			if( c == NUM_COLUMNS-1 && r == NUM_ROWS-1 && Songs.size() != unsigned(NUM_COLUMNS*NUM_ROWS) )
			{
				sText += ssprintf( "%i more.....", int(Songs.size() - NUM_COLUMNS * NUM_ROWS + 1) );
				continue;
			}

			RString sTitle = Songs[iIndex]->GetDisplayFullTitle();
			// TODO:  Move this crop threshold into a theme metric or make automatic based on column width
			if( sTitle.size() > 40 )
				sTitle = sTitle.Left( 37 ) + "...";

			RString sTrTitle = Songs[iIndex]->GetTranslitFullTitle();
			if( sTrTitle.size() > 40 )
				sTrTitle = sTrTitle.Left( 37 ) + "...";

			/* If the main title isn't complete for this font, and we have a translit,
			 * use it. */
			if(m_textTitles[c].StringWillUseAlternate(sTitle, sTrTitle))
				sText += sTrTitle;
			else
				sText += sTitle;
			sText += "\n";
		}
		m_ContentsText[group].ContentsText[c] = sText;
	}

}

/* TODO: tween? */
void MusicList::SetGroupNo(int group)
{
	CurGroup = group;

	for( int c=0; c<NUM_COLUMNS; c++ )
	{
		m_textTitles[c].SetText( m_ContentsText[CurGroup].ContentsText[c] );
		m_textTitles[c].CropToWidth( int(CROP_WIDTH/m_textTitles[c].GetZoom()) );
	}
}

void MusicList::TweenOnScreen()
{
	for( int i=0; i<NUM_COLUMNS; i++ )
	{
		m_textTitles[i].SetDiffuse( RageColor(1,1,1,0) );
		m_textTitles[i].BeginTweening( 0.5f );
		m_textTitles[i].BeginTweening( 0.5f );
		m_textTitles[i].SetDiffuse( RageColor(1,1,1,1) );
	}
}

void MusicList::TweenOffScreen()
{
	for( int i=0; i<NUM_COLUMNS; i++ )
	{
		m_textTitles[i].StopTweening();
		m_textTitles[i].BeginTweening( 0.7f );
		m_textTitles[i].BeginTweening( 0.5f );
		m_textTitles[i].SetDiffuse( RageColor(1,1,1,0) );
	}
}

/*
 * (c) 2001-2003 Chris Danford
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
