#include "global.h"
#include "MusicList.h"
#include "ThemeManager.h"

/* If this actor is used anywhere other than SelectGroup, we
 * can add a setting that changes which metric group we pull
 * settings out of, so it can be configured separately. */
#define TITLES_START_X		THEME->GetMetricF("ScreenSelectGroup","TitlesStartX")
#define TITLES_WIDTH		THEME->GetMetricF("ScreenSelectGroup","TitlesWidth")
#define TITLES_SPACING_X	THEME->GetMetricF("ScreenSelectGroup","TitlesSpacingX")
#define TITLES_START_Y		THEME->GetMetricF("ScreenSelectGroup","TitlesStartY")
#define TITLES_COLUMNS		THEME->GetMetricI("ScreenSelectGroup","TitlesColumns")
#define TITLES_ROWS			THEME->GetMetricI("ScreenSelectGroup","TitlesRows")
#define TITLES_ZOOM			THEME->GetMetricF("ScreenSelectGroup","TitlesZoom")

MusicList::MusicList()
{
	CurGroup = 0;

	for( int i=0; i<TITLES_COLUMNS; i++ )
	{
		m_textTitles[i].LoadFromFont( THEME->GetPathTo("Fonts","small titles") );
		m_textTitles[i].SetXY( TITLES_START_X + i*TITLES_SPACING_X, TITLES_START_Y );
		m_textTitles[i].SetHorizAlign( Actor::align_left );
		m_textTitles[i].SetVertAlign( Actor::align_top );
		m_textTitles[i].SetShadowLength( 2 );
		m_textTitles[i].SetZoom( TITLES_ZOOM );
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

	for( int c=0; c<TITLES_COLUMNS; c++ )	// foreach col
	{
		CString sText;
		for( int r=0; r<TITLES_ROWS; r++ )	// foreach row
		{
			unsigned iIndex = c*TITLES_ROWS + r;
			if( iIndex >= Songs.size() )
				continue;

			if( c == TITLES_COLUMNS-1 && r == TITLES_ROWS-1 && Songs.size() != unsigned(TITLES_COLUMNS*TITLES_ROWS) )
			{
				sText += ssprintf( "%d more.....", Songs.size() - TITLES_COLUMNS * TITLES_ROWS + 1 );
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

	for( int c=0; c<TITLES_COLUMNS; c++ )
	{
		m_textTitles[c].SetText( m_ContentsText[CurGroup].ContentsText[c] );
		m_textTitles[c].CropToWidth( int(TITLES_WIDTH/m_textTitles[c].GetZoom()) );
	}
}

void MusicList::TweenOnScreen()
{
	for( int i=0; i<TITLES_COLUMNS; i++ )
	{
		m_textTitles[i].SetDiffuse( RageColor(1,1,1,0) );
		m_textTitles[i].BeginTweening( 0.5f );
		m_textTitles[i].BeginTweening( 0.5f );
		m_textTitles[i].SetTweenDiffuse( RageColor(1,1,1,1) );
	}
}

void MusicList::TweenOffScreen()
{
	for( int i=0; i<TITLES_COLUMNS; i++ )
	{
		m_textTitles[i].BeginTweening( 0.7f );
		m_textTitles[i].BeginTweening( 0.5f );
		m_textTitles[i].SetTweenDiffuse( RageColor(1,1,1,0) );
	}
}
