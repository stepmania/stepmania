#include "global.h"
#include "MusicList.h"
#include "ThemeManager.h"

/* If this actor is used anywhere other than SelectGroup, we
 * can add a setting that changes which metric group we pull
 * settings out of, so it can be configured separately. */
#define NUM_COLUMNS		THEME->GetMetricI("MusicList","NumColumns")
#define NUM_ROWS		THEME->GetMetricI("MusicList","NumRows")
#define START_X			THEME->GetMetricF("MusicList","StartX")
#define START_Y			THEME->GetMetricF("MusicList","StartY")
#define SPACING_X		THEME->GetMetricF("MusicList","SpacingX")
#define CROP_WIDTH		THEME->GetMetricF("MusicList","CropWidth")
#define INIT_COMMAND	THEME->GetMetric ("MusicList","InitCommand")

MusicList::MusicList()
{
	CurGroup = 0;

	for( int i=0; i<NUM_COLUMNS; i++ )
	{
		m_textTitles[i].LoadFromFont( THEME->GetPathTo("Fonts","MusicList titles") );
		m_textTitles[i].SetXY( START_X + i*SPACING_X, START_Y );
		m_textTitles[i].Command( INIT_COMMAND );
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
		CString sText;
		for( int r=0; r<NUM_ROWS; r++ )	// foreach row
		{
			unsigned iIndex = c*NUM_ROWS + r;
			if( iIndex >= Songs.size() )
				continue;

			if( c == NUM_COLUMNS-1 && r == NUM_ROWS-1 && Songs.size() != unsigned(NUM_COLUMNS*NUM_ROWS) )
			{
				sText += ssprintf( "%d more.....", Songs.size() - NUM_COLUMNS * NUM_ROWS + 1 );
				continue;
			}

			CString sTitle = Songs[iIndex]->GetFullDisplayTitle();
			// TODO:  Move this crop threshold into a theme metric or make automatic based on column width
			if( sTitle.GetLength() > 40 )
				sTitle = sTitle.Left( 37 ) + "...";

			CString sTrTitle = Songs[iIndex]->GetFullTranslitTitle();
			if( sTrTitle.GetLength() > 40 )
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
		m_textTitles[i].SetTweenDiffuse( RageColor(1,1,1,1) );
	}
}

void MusicList::TweenOffScreen()
{
	for( int i=0; i<NUM_COLUMNS; i++ )
	{
		m_textTitles[i].StopTweening();
		m_textTitles[i].BeginTweening( 0.7f );
		m_textTitles[i].BeginTweening( 0.5f );
		m_textTitles[i].SetTweenDiffuse( RageColor(1,1,1,0) );
	}
}
