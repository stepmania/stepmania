#include "global.h"
#include "Combo.h"
#include "ThemeManager.h"
#include "StageStats.h"
#include "GameState.h"
#include "song.h"


CachedThemeMetricF	LABEL_X				("Combo","LabelX");
CachedThemeMetricF	LABEL_Y				("Combo","LabelY");
CachedThemeMetricI	LABEL_HORIZ_ALIGN	("Combo","LabelHorizAlign");
CachedThemeMetricI	LABEL_VERT_ALIGN	("Combo","LabelVertAlign");
CachedThemeMetricF	NUMBER_X			("Combo","NumberX");
CachedThemeMetricF	NUMBER_Y			("Combo","NumberY");
CachedThemeMetricI	NUMBER_HORIZ_ALIGN	("Combo","NumberHorizAlign");
CachedThemeMetricI	NUMBER_VERT_ALIGN	("Combo","NumberVertAlign");
CachedThemeMetricI	SHOW_COMBO_AT		("Combo","ShowComboAt");
CachedThemeMetricF	NUMBER_MIN_ZOOM		("Combo","NumberMinZoom");
CachedThemeMetricF	NUMBER_MAX_ZOOM		("Combo","NumberMaxZoom");
CachedThemeMetricF	NUMBER_MAX_ZOOM_AT	("Combo","NumberMaxZoomAt");
CachedThemeMetricF	PULSE_ZOOM			("Combo","PulseZoom");
CachedThemeMetricF	C_TWEEN_SECONDS		("Combo","TweenSeconds");
CachedThemeMetricF	FULL_COMBO_GREATS_COMMAND		("Combo","FullComboGreatsCommand");
CachedThemeMetricF	FULL_COMBO_PERFECTS_COMMAND		("Combo","FullComboPerfectsCommand");
CachedThemeMetricF	FULL_COMBO_MARVELOUSES_COMMAND	("Combo","FullComboMarvelousesCommand");
CachedThemeMetricF	FULL_COMBO_BROKEN_COMMAND		("Combo","FullComboBrokenCommand");
CachedThemeMetricB	SHOW_MISS_COMBO		("Combo","ShowMissCombo");


Combo::Combo()
{
	LABEL_X.Refresh();
	LABEL_Y.Refresh();
	LABEL_HORIZ_ALIGN.Refresh();
	LABEL_VERT_ALIGN.Refresh();
	NUMBER_X.Refresh();
	NUMBER_Y.Refresh();
	NUMBER_HORIZ_ALIGN.Refresh();
	NUMBER_VERT_ALIGN.Refresh();
	SHOW_COMBO_AT.Refresh();
	NUMBER_MIN_ZOOM.Refresh();
	NUMBER_MAX_ZOOM.Refresh();
	NUMBER_MAX_ZOOM_AT.Refresh();
	PULSE_ZOOM.Refresh();
	C_TWEEN_SECONDS.Refresh();
	FULL_COMBO_GREATS_COMMAND.Refresh();
	FULL_COMBO_PERFECTS_COMMAND.Refresh();
	FULL_COMBO_MARVELOUSES_COMMAND.Refresh();
	FULL_COMBO_BROKEN_COMMAND.Refresh();
	SHOW_MISS_COMBO.Refresh();

	m_sprComboLabel.Load( THEME->GetPathToG( "Combo label") );
	m_sprComboLabel.SetShadowLength( 4 );
	m_sprComboLabel.StopAnimating();
	m_sprComboLabel.SetXY( LABEL_X, LABEL_Y );
	m_sprComboLabel.SetHorizAlign( (Actor::HorizAlign)(int)LABEL_HORIZ_ALIGN );
	m_sprComboLabel.SetVertAlign( (Actor::VertAlign)(int)LABEL_VERT_ALIGN );
	m_sprComboLabel.SetHidden( true );
	this->AddChild( &m_sprComboLabel );

	m_sprMissesLabel.Load( THEME->GetPathToG( "Combo misses") );
	m_sprMissesLabel.SetShadowLength( 4 );
	m_sprMissesLabel.StopAnimating();
	m_sprMissesLabel.SetXY( LABEL_X, LABEL_Y );
	m_sprMissesLabel.SetHorizAlign( (Actor::HorizAlign)(int)LABEL_HORIZ_ALIGN );
	m_sprMissesLabel.SetVertAlign( (Actor::VertAlign)(int)LABEL_VERT_ALIGN );
	m_sprMissesLabel.SetHidden( true );
	this->AddChild( &m_sprMissesLabel );

	m_textNumber.LoadFromFont( THEME->GetPathToF("Combo") );
	m_textNumber.SetShadowLength( 4 );
	m_textNumber.SetXY( NUMBER_X, NUMBER_Y );
	m_textNumber.SetHorizAlign( (Actor::HorizAlign)(int)NUMBER_HORIZ_ALIGN );
	m_textNumber.SetVertAlign( (Actor::VertAlign)(int)NUMBER_VERT_ALIGN );
	m_textNumber.SetHidden( true );
	this->AddChild( &m_textNumber );
}

void Combo::SetCombo( int iCombo, int iMisses )
{
	bool bMisses = iMisses > 0;
	int iNum = bMisses ? iMisses : iCombo;

	if( (iNum < (int)SHOW_COMBO_AT)  || 
		(bMisses && !(bool)SHOW_MISS_COMBO) )
	{
		m_sprComboLabel.SetHidden( true );
		m_sprMissesLabel.SetHidden( true );
		m_textNumber.SetHidden( true );
		return;
	}

	m_sprComboLabel.SetHidden( bMisses );
	m_sprMissesLabel.SetHidden( !bMisses );
	m_textNumber.SetHidden( false );

	CString txt = ssprintf("%d", iNum);
	/* Don't do anything if it's not changing. */
	if(m_textNumber.GetText() == txt) return;

	m_textNumber.SetText( txt );
	float fNumberZoom = SCALE(iNum,0.f,(float)NUMBER_MAX_ZOOM_AT,(float)NUMBER_MIN_ZOOM,(float)NUMBER_MAX_ZOOM);
	CLAMP( fNumberZoom, (float)NUMBER_MIN_ZOOM, (float)NUMBER_MAX_ZOOM );
	m_textNumber.StopTweening();
	m_textNumber.SetZoom( fNumberZoom * (float)PULSE_ZOOM ); 
	m_textNumber.BeginTweening( C_TWEEN_SECONDS );
	m_textNumber.SetZoom( fNumberZoom );

	Sprite &sprLabel = bMisses ? m_sprMissesLabel : m_sprComboLabel;

	sprLabel.StopTweening();
	sprLabel.SetZoom( PULSE_ZOOM ); 
	sprLabel.BeginTweening( C_TWEEN_SECONDS );
	sprLabel.SetZoom( 1 );

	// don't show a colored combo until 1/4 of the way through the song
	bool bPastMidpoint = GAMESTATE->GetCourseSongIndex()>0 ||
		GAMESTATE->m_fMusicSeconds > GAMESTATE->m_pCurSong->m_fMusicLengthSeconds/4;

	if( bPastMidpoint )
	{
		if( g_CurStageStats.FullComboOfScore(m_PlayerNumber,TNS_MARVELOUS) )
		{
			sprLabel.Command( FULL_COMBO_MARVELOUSES_COMMAND );
			m_textNumber.Command( FULL_COMBO_MARVELOUSES_COMMAND );
		}
		else if( bPastMidpoint && g_CurStageStats.FullComboOfScore(m_PlayerNumber,TNS_PERFECT) )
		{
			sprLabel.Command( FULL_COMBO_PERFECTS_COMMAND );
			m_textNumber.Command( FULL_COMBO_PERFECTS_COMMAND );
		}
		else if( bPastMidpoint && g_CurStageStats.FullComboOfScore(m_PlayerNumber,TNS_GREAT) )
		{
			sprLabel.Command( FULL_COMBO_GREATS_COMMAND );
			m_textNumber.Command( FULL_COMBO_GREATS_COMMAND );
		}
		else
		{
			sprLabel.Command( FULL_COMBO_BROKEN_COMMAND );
			m_textNumber.Command( FULL_COMBO_BROKEN_COMMAND );
		}
	}
	else
	{
		sprLabel.Command( FULL_COMBO_BROKEN_COMMAND );
		m_textNumber.Command( FULL_COMBO_BROKEN_COMMAND );
	}
}

/*
 * (c) 2001-2004 Chris Danford
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
