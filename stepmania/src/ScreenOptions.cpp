#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "InputMapper.h"
#include "ActorUtil.h"
#include "ProfileManager.h"
#include "song.h"

const float ITEM_X[NUM_PLAYERS] = { 260, 420 };

#define ICONS_X( p )					THEME->GetMetricF("ScreenOptions",ssprintf("IconsP%dX",p+1))
#define ARROWS_X						THEME->GetMetricF("ScreenOptions","ArrowsX")
#define LABELS_X						THEME->GetMetricF("ScreenOptions","LabelsX")
#define LABELS_ZOOM						THEME->GetMetricF("ScreenOptions","LabelsZoom")
#define LABELS_H_ALIGN					THEME->GetMetricI("ScreenOptions","LabelsHAlign")
#define ITEMS_ZOOM						THEME->GetMetricF("ScreenOptions","ItemsZoom")
#define ITEMS_START_X					THEME->GetMetricF("ScreenOptions","ItemsStartX")
#define ITEMS_GAP_X						THEME->GetMetricF("ScreenOptions","ItemsGapX")
#define ITEMS_START_Y					THEME->GetMetricF("ScreenOptions","ItemsStartY")
#define ITEMS_SPACING_Y					THEME->GetMetricF("ScreenOptions","ItemsSpacingY")
#define EXPLANATION_X(p)				THEME->GetMetricF("ScreenOptions",ssprintf("ExplanationP%dX",p+1))
#define EXPLANATION_Y(p)				THEME->GetMetricF("ScreenOptions",ssprintf("ExplanationP%dY",p+1))
#define EXPLANATION_ON_COMMAND(p)		THEME->GetMetric ("ScreenOptions",ssprintf("ExplanationP%dOnCommand",p+1))
#define EXPLANATION_TOGETHER_X			THEME->GetMetricF("ScreenOptions","ExplanationTogetherX")
#define EXPLANATION_TOGETHER_Y			THEME->GetMetricF("ScreenOptions","ExplanationTogetherY")
#define EXPLANATION_TOGETHER_ON_COMMAND	THEME->GetMetric ("ScreenOptions","ExplanationTogetherOnCommand")
#define SHOW_SCROLL_BAR					THEME->GetMetricB("ScreenOptions","ShowScrollBar")
/* Extra parens needed to work around stupid VC6 compiler crash: */
#define SCROLL_BAR_HEIGHT				(THEME->GetMetricF("ScreenOptions","ScrollBarHeight"))
#define SCROLL_BAR_TIME					(THEME->GetMetricF("ScreenOptions","ScrollBarTime"))
#define ITEMS_SPACING_Y					THEME->GetMetricF("ScreenOptions","ItemsSpacingY")
#define EXPLANATION_ZOOM				THEME->GetMetricF("ScreenOptions","ExplanationZoom")
#define COLOR_SELECTED					THEME->GetMetricC("ScreenOptions","ColorSelected")
#define COLOR_NOT_SELECTED				THEME->GetMetricC("ScreenOptions","ColorNotSelected")
#define NUM_SHOWN_ITEMS					THEME->GetMetricI("ScreenOptions","NumShownItems")
#define SHOW_BPM_IN_SPEED_TITLE			THEME->GetMetricB("ScreenOptions","ShowBpmInSpeedTitle")
#define FRAME_ON_COMMAND				THEME->GetMetric ("ScreenOptions","FrameOnCommand")
#define FRAME_OFF_COMMAND				THEME->GetMetric ("ScreenOptions","FrameOffCommand")
#define SEPARATE_EXIT_ROW				THEME->GetMetricB("ScreenOptions","SeparateExitRow")
#define SEPARATE_EXIT_ROW_Y				THEME->GetMetricF("ScreenOptions","SeparateExitRowY")

/*
 * Three navigation types are provided:
 *
 * NAV_THREE_KEY:
 *  left, right -> change option
 *  up, down -> don't matter (change row)
 *  start -> move to next row
 *  left+right+start -> move to prev row
 *  (next screen via "exit" entry)
 * This is the minimal navigation, for using menus with only three buttons.
 *
 * NAV_FIVE_KEY:
 *  left, right -> change option
 *  up, down -> change row
 *  start -> next screen
 * This is a much more convenient navigation, requiring five keys.
 *
 * NAV_THREE_KEY_MENU:
 *  left, right -> change row
 *  up, down -> change row
 *  start -> next screen
 * This is a specialized navigation for ScreenOptionsMenu.  It must be enabled to
 * allow screens that use rows to select other screens to work with only three
 * buttons.  (It's also used when in five-key mode.)
 *
 * NAV_FIRST_CHOICE_GOES_DOWN:
 *  left, right -> change row
 *  up, down -> change row
 *  start -> next screen
 * This is a specialized navigation for ScreenOptionsMenu.  It must be enabled to
 * allow screens that use rows to select other screens to work with only three
 * buttons.  (It's also used when in five-key mode.)
 */

ScreenOptions::ScreenOptions( CString sClassName ) : Screen(sClassName)
{
	LOG->Trace( "ScreenOptions::ScreenOptions()" );

	m_OptionsNavigation = PREFSMAN->m_bArcadeOptionsNavigation? NAV_THREE_KEY:NAV_FIVE_KEY;

	m_SoundChangeCol.Load( THEME->GetPathToS("ScreenOptions change"), true );
	m_SoundNextRow.Load( THEME->GetPathToS("ScreenOptions next"), true );
	m_SoundPrevRow.Load( THEME->GetPathToS("ScreenOptions prev"), true );
	m_SoundStart.Load( THEME->GetPathToS("Common start") );
	m_SoundToggleOn.Load( THEME->GetPathToS("ScreenOptions toggle on") );
	m_SoundToggleOff.Load( THEME->GetPathToS("ScreenOptions toggle off") );

	m_Menu.Load( sClassName );
	this->AddChild( &m_Menu );

	// add everything to m_framePage so we can animate everything at once
	this->AddChild( &m_framePage );

	m_bMoreShown = false;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iCurrentRow[p] = 0;
		m_bWasOnExit[p] = false;
	}

	m_framePage.Command( FRAME_ON_COMMAND );
}

void ScreenOptions::LoadOptionIcon( PlayerNumber pn, int iRow, CString sText )
{
	m_Rows[iRow]->m_OptionIcons[pn].Load( pn, sText, false );
}

void ScreenOptions::Init( InputMode im, OptionRowData OptionRows[], int iNumOptionLines )
{
	LOG->Trace( "ScreenOptions::Set()" );

	m_InputMode = im;

	{
		for( int r=0; r<iNumOptionLines; r++ )		// foreach row
		{
			m_Rows.push_back( new Row() );
			Row &Row = *m_Rows[r];
			Row.m_RowDef = OptionRows[r];

			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				vector<bool> &vbSelected = Row.m_vbSelected[p];
				vbSelected.resize( Row.m_RowDef.choices.size() );
				for( unsigned j=0; j<vbSelected.size(); j++ )
					vbSelected[j] = false;

				// set select the first item if not a multiselect row
				if( !Row.m_RowDef.bMultiSelect )
					vbSelected[0] = true;
			}
		}
	}

	this->ImportOptions();

	// Make all selections the same if bOneChoiceForAllPlayers
	{
		for( int r=0; r<iNumOptionLines; r++ )		// foreach row
		{
			Row &Row = *m_Rows[r];

			if( Row.m_RowDef.bOneChoiceForAllPlayers )
				for( int p=1; p<NUM_PLAYERS; p++ )
					Row.m_vbSelected[p] = m_Rows[r]->m_vbSelected[0];

			for( int p=0; p<NUM_PLAYERS; p++ )
				if( Row.m_RowDef.bMultiSelect )
					Row.m_iChoiceWithFocus[p] = 0;	// focus on the first row, which is "go down"
				else
					Row.m_iChoiceWithFocus[p] = Row.GetOneSelection( (PlayerNumber)p);	// focus on the only selected choice
		}
	}

	m_sprPage.Load( THEME->GetPathToG(m_sName+" page") );
	m_sprPage->SetName( "Page" );
	UtilSetXYAndOnCommand( m_sprPage, "ScreenOptions" );
	m_framePage.AddChild( m_sprPage );

	// init highlights
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsHumanPlayer(p) )
				continue;	// skip

			m_sprLineHighlight[p].Load( THEME->GetPathToG("ScreenOptions line highlight") );
			m_sprLineHighlight[p].SetName( "LineHighlight" );
			m_sprLineHighlight[p].SetX( CENTER_X );
			m_framePage.AddChild( &m_sprLineHighlight[p] );
			UtilOnCommand( m_sprLineHighlight[p], "ScreenOptions" );

			m_Highlight[p].Load( (PlayerNumber)p, false );
			m_framePage.AddChild( &m_Highlight[p] );
		}
	}
	
	// init row icons
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsHumanPlayer(p) )
				continue;	// skip

			for( unsigned l=0; l<m_Rows.size(); l++ )
			{	
				Row &row = *m_Rows[l];

				LoadOptionIcon( (PlayerNumber)p, l, "" );
				m_framePage.AddChild( &row.m_OptionIcons[p] );
			}
		}
	}

	// init m_textItems from optionLines
	{
		for( unsigned  r=0; r<m_Rows.size(); r++ )		// foreach row
		{
			Row &row = *m_Rows[r];
			row.Type = Row::ROW_NORMAL;

			vector<BitmapText *> & textItems = row.m_textItems;
			const OptionRowData &optline = m_Rows[r]->m_RowDef;

			unsigned c;

			m_framePage.AddChild( &row.m_sprBullet );
			m_framePage.AddChild( &row.m_textTitle );		

			float fX = ITEMS_START_X;	// indent 70 pixels
			for( c=0; c<optline.choices.size(); c++ )
			{
				// init text
				BitmapText *bt = new BitmapText;
				textItems.push_back( bt );
				bt->LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
				bt->SetText( optline.choices[c] );
				bt->SetZoom( ITEMS_ZOOM );
				bt->EnableShadow( false );

				// set the X position of each item in the line
				float fItemWidth = bt->GetZoomedWidth();
				fX += fItemWidth/2;
				bt->SetX( fX );

				// init underlines
				for( int p=0; p<NUM_PLAYERS; p++ )
				{
					if( !GAMESTATE->IsHumanPlayer(p) )
						continue;

					OptionsCursor *ul = new OptionsCursor;
					row.m_Underline[p].push_back( ul );
					ul->Load( (PlayerNumber)p, true );
					ul->SetX( fX );
					ul->SetWidth( truncf(fItemWidth) );
				}

				fX += fItemWidth/2 + ITEMS_GAP_X;

				// It goes off the edge of the screen.  Re-init with the "long row" style.
				if( fX > SCREEN_RIGHT-40 ) 
				{
					row.m_bRowIsLong = true;
					for( unsigned j=0; j<textItems.size(); j++ )	// for each option on this row
						delete textItems[j];
					textItems.clear();
					for( int p=0; p<NUM_PLAYERS; p++ )
					{
						for( unsigned j=0; j<row.m_Underline[p].size(); j++ )	// for each option on this row
							delete row.m_Underline[p][j];
						row.m_Underline[p].clear();
					}
					break;
				}
			}

			if( row.m_bRowIsLong )
			{
				// init text
				for( unsigned p=0; p<NUM_PLAYERS; p++ )
				{
					if( !GAMESTATE->IsHumanPlayer(p) )
						continue;	// skip

					BitmapText *bt = new BitmapText;
					textItems.push_back( bt );

					const int iChoiceWithFocus = row.m_iChoiceWithFocus[p];

					bt->LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
					bt->SetText( optline.choices[iChoiceWithFocus] );
					bt->SetZoom( ITEMS_ZOOM );
					bt->EnableShadow( false );

					if( optline.bOneChoiceForAllPlayers )
					{
						bt->SetX( truncf((ITEM_X[0]+ITEM_X[1])/2) );	// center the item
						break;	// only initialize one item since it's shared
					}
					else
					{
						bt->SetX( ITEM_X[p] );
					}
				}

				// init underlines
				{
					for( unsigned p=0; p<NUM_PLAYERS; p++ )
					{
						if( !GAMESTATE->IsHumanPlayer(p) )
							continue;	// skip

						OptionsCursor *ul = new OptionsCursor;
						row.m_Underline[p].push_back( ul );
						ul->Load( (PlayerNumber)p, true );
						float fX = optline.bOneChoiceForAllPlayers ? textItems[0]->GetX() : textItems[p]->GetX();
						float fWidth = optline.bOneChoiceForAllPlayers ? textItems[0]->GetZoomedWidth() : textItems[p]->GetZoomedWidth();
						ul->SetX( truncf(fX) );
						ul->SetWidth( truncf(fWidth) );
					}
				}
			}

			// Add children here and not above because of the logic that starts
			// over if we run off the right edge of the screen.
			{
				for( unsigned c=0; c<textItems.size(); c++ )
					m_framePage.AddChild( textItems[c] );
				for( int p=0; p<NUM_PLAYERS; p++ )
					for( unsigned c=0; c<row.m_Underline[p].size(); c++ )
						m_framePage.AddChild( row.m_Underline[p][c] );
			}
		}
	}

	// TRICKY:  Add one more item.  This will be "EXIT"
	{
		m_Rows.push_back( new Row() );
		Row &row = *m_Rows.back();
		row.Type = Row::ROW_EXIT;

		BitmapText *bt = new BitmapText;
		row.m_textItems.push_back( bt );

		bt->LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
		bt->SetText( THEME->GetMetric("OptionNames","Exit") );
		bt->SetZoom( ITEMS_ZOOM );
		bt->EnableShadow( false );
		bt->SetX( CENTER_X );

		m_framePage.AddChild( bt );
	}

	InitOptionsText();

	// add explanation here so it appears on top
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			m_textExplanation[p].LoadFromFont( THEME->GetPathToF("ScreenOptions explanation") );
			m_textExplanation[p].SetZoom( EXPLANATION_ZOOM );
			m_textExplanation[p].SetShadowLength( 0 );
			m_framePage.AddChild( &m_textExplanation[p] );
		}
	}

	/* Hack: if m_CurStyle is set, we're probably in the player or song options menu, so
	 * the player name is meaningful.  Otherwise, we're probably in the system menu. */
	if( GAMESTATE->m_CurStyle != STYLE_INVALID )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			m_textPlayerName[p].LoadFromFont( THEME->GetPathToF( "ScreenOptions player") );
			m_textPlayerName[p].SetName( ssprintf("PlayerNameP%i",p+1) );
			m_textPlayerName[p].SetText( PROFILEMAN->GetPlayerName((PlayerNumber)p) );
			UtilSetXYAndOnCommand( m_textPlayerName[p], "ScreenOptions" );
			m_framePage.AddChild( &m_textPlayerName[p] );
		}
	}

	if( SHOW_SCROLL_BAR )
	{
		m_ScrollBar.SetName( "DualScrollBar", "ScrollBar" );
		m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT );
		m_ScrollBar.SetBarTime( SCROLL_BAR_TIME );
		for( int p=0; p<NUM_PLAYERS; p++ )
			m_ScrollBar.EnablePlayer( (PlayerNumber)p, GAMESTATE->IsHumanPlayer(p) );
		m_ScrollBar.Load();
		UtilSetXY( m_ScrollBar, "ScreenOptions" );
		m_framePage.AddChild( &m_ScrollBar );
	}

	m_sprMore.Load( THEME->GetPathToG( "ScreenOptions more") );
	m_sprMore->SetName( "ScreenOptions", "More" );
	UtilSetXYAndOnCommand( m_sprMore, "ScreenOptions" );
	UtilCommand( m_sprMore, "ScreenOptions", m_bMoreShown? "ShowMore":"HideMore" );
	m_framePage.AddChild( m_sprMore );

	switch( m_InputMode )
	{
	case INPUTMODE_INDIVIDUAL:
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_textExplanation[p].SetXY( EXPLANATION_X(p), EXPLANATION_Y(p) );
		}
		break;
	case INPUTMODE_TOGETHER:
		m_textExplanation[0].SetXY( EXPLANATION_TOGETHER_X, EXPLANATION_TOGETHER_Y );
		break;
	default:
		ASSERT(0);
	}

	m_sprFrame.Load( THEME->GetPathToG( "ScreenOptions frame") );
	m_sprFrame->SetXY( CENTER_X, CENTER_Y );
	m_framePage.AddChild( m_sprFrame );

	// poke once at all the explanation metrics so that we catch missing ones early
	{
		for( int r=0; r<(int)m_Rows.size(); r++ )		// foreach row
		{
			GetExplanationText( r );
			GetExplanationTitle( r );
		}
	}

	CHECKPOINT;

	PositionItems();
	PositionUnderlines();
	PositionIcons();
	RefreshIcons();
	PositionCursors();
	UpdateEnabledDisabled();
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
			OnChange( (PlayerNumber)p );
	}

	CHECKPOINT;

	/* It's tweening into position, but on the initial tween-in we only want to
	 * tween in the whole page at once.  Since the tweens are nontrivial, it's
	 * easiest to queue the tweens and then force them to finish. */
	for( int r=0; r<(int) m_Rows.size(); r++ )	// foreach options line
	{
		Row &row = *m_Rows[r];
		row.m_sprBullet.FinishTweening();
		row.m_textTitle.FinishTweening();

		for( unsigned c=0; c<row.m_textItems.size(); c++ )
			row.m_textItems[c]->FinishTweening();
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			for( unsigned c=0; c<row.m_Underline[p].size(); c++ )
				row.m_Underline[p][c]->FinishTweening();
			row.m_OptionIcons[p].FinishTweening();
		}
	}

	m_sprMore->FinishTweening();
}

ScreenOptions::~ScreenOptions()
{
	LOG->Trace( "ScreenOptions::~ScreenOptions()" );
	for( unsigned i=0; i<m_Rows.size(); i++ )
		delete m_Rows[i];
}

CString ScreenOptions::GetExplanationText( int iRow ) const
{
	if( m_Rows[iRow]->Type == Row::ROW_EXIT )
		return "";

	CString sLineName = m_Rows[iRow]->m_RowDef.name;
	sLineName.Replace("\n-","");
	sLineName.Replace("\n","");
	sLineName.Replace(" ","");
	return THEME->GetMetric( "OptionExplanations", sLineName+"Help" );
}

CString ScreenOptions::GetExplanationTitle( int iRow ) const
{
	if( m_Rows[iRow]->Type == Row::ROW_EXIT )
		return "";
	
	CString sLineName = m_Rows[iRow]->m_RowDef.name;
	sLineName.Replace("\n-","");
	sLineName.Replace("\n","");
	sLineName.Replace(" ","");
	CString sTitle = THEME->GetMetric( "OptionTitles", sLineName+"Title" );

	// HACK: tack the BPM onto the name of the speed line
	if( sLineName.CompareNoCase("speed")==0 )
	{
		if( SHOW_BPM_IN_SPEED_TITLE && GAMESTATE->m_pCurSong )
		{
			float fMinBpm, fMaxBpm;
			GAMESTATE->m_pCurSong->GetDisplayBPM( fMinBpm, fMaxBpm );
			if( fMinBpm == fMaxBpm )
				sTitle += ssprintf( " (%.0f)", fMinBpm );
			else
				sTitle += ssprintf( " (%.0f-%.0f)", fMinBpm, fMaxBpm );
		}
	}

	return sTitle;
}

BitmapText &ScreenOptions::GetTextItemForRow( PlayerNumber pn, int iRow, int iChoiceOnRow )
{
	RAGE_ASSERT_M( iRow < (int)m_Rows.size(), ssprintf("%i < %i", iRow, m_Rows.size() ) );
	Row &row = *m_Rows[iRow];
	if( row.Type == Row::ROW_EXIT )
		return *row.m_textItems[0];

	bool bOneChoice = row.m_RowDef.bOneChoiceForAllPlayers;
	int index = -1;
	if( row.m_bRowIsLong )
		index = bOneChoice ? 0 : pn;
	else
		index = iChoiceOnRow;

	RAGE_ASSERT_M( index < (int)row.m_textItems.size(), ssprintf("%i < %i", index, row.m_textItems.size() ) );
	return *row.m_textItems[index];
}

void ScreenOptions::GetWidthXY( PlayerNumber pn, int iRow, int iChoiceOnRow, int &iWidthOut, int &iXOut, int &iYOut )
{
	BitmapText &text = GetTextItemForRow( pn, iRow, iChoiceOnRow );

	iWidthOut = int(roundf( text.GetZoomedWidth() ));
	iXOut = int(roundf( text.GetDestX() ));
	/* We update m_fY, change colors and tween items, and then tween rows to
	 * their final positions.  (This is so we don't tween colors, too.)  m_fY
	 * is the actual destination position, even though we may not have set up the
	 * tween yet. */
	iYOut = int(roundf( m_Rows[iRow]->m_fY ));
}

void ScreenOptions::InitOptionsText()
{
	for( unsigned i=0; i<m_Rows.size(); i++ )	// foreach options line
	{
		Row &row = *m_Rows[i];
		if( row.Type == Row::ROW_EXIT )
			continue;

		const float fY = ITEMS_START_Y + ITEMS_SPACING_Y*i;

		BitmapText &title = row.m_textTitle;

		title.LoadFromFont( THEME->GetPathToF("ScreenOptions title") );

		const CString sText = GetExplanationTitle( i );
		title.SetText( sText );
		title.SetXY( LABELS_X, fY );
		title.SetZoom( LABELS_ZOOM );
		title.SetHorizAlign( (Actor::HorizAlign)LABELS_H_ALIGN );
		title.SetVertAlign( Actor::align_middle );		
		title.EnableShadow( false );		

		Sprite &bullet = row.m_sprBullet;
		bullet.Load( THEME->GetPathToG("ScreenOptions bullet") );
		bullet.SetXY( ARROWS_X, fY );
		
		// set the Y position of each item in the line
		for( unsigned c=0; c<row.m_textItems.size(); c++ )
			row.m_textItems[c]->SetY( fY );
	}
}

void ScreenOptions::PositionUnderlines()
{
	// OPTIMIZATION OPPORTUNITY: There's no reason to the underlines for 
	// all rows when something changes.  Just recalulate for the row that 
	// changed.

	// Set the position of the underscores showing the current choice for each option line.
	for( unsigned r=0; r<m_Rows.size(); r++ )	// foreach options line
	{
		Row &row = *m_Rows[r];
		if( row.Type == Row::ROW_EXIT )
			continue;

		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsHumanPlayer(p) )
				continue;	// skip

			vector<OptionsCursor*> &vpUnderlines = row.m_Underline[p];

			const int iNumUnderlines = row.m_bRowIsLong ? 1 : vpUnderlines.size();
			
			for( int i=0; i<iNumUnderlines; i++ )
			{
				OptionsCursor& ul = *vpUnderlines[i];
	
				int iChoiceWithFocus = row.m_bRowIsLong ? row.m_iChoiceWithFocus[p] : i;

				/* Don't tween X movement and color changes. */
				int iWidth, iX, iY;
				GetWidthXY( (PlayerNumber)p, r, iChoiceWithFocus, iWidth, iX, iY );
				ul.SetGlobalX( (float)iX );
				ul.SetGlobalDiffuseColor( RageColor(1,1,1, 1.0f) );

				// Don't show underlines on the ScreenOptionsMenu.  We know we're on this
				// screen if the row title is empty.
				bool bEmptyTitle = GetExplanationTitle(r).empty();
				bool bSelected = row.m_vbSelected[p][ iChoiceWithFocus ];
				bool bHidden = bEmptyTitle || !bSelected || row.m_bHidden;

				if( ul.GetDestY() != row.m_fY )
				{
					ul.StopTweening();
					ul.BeginTweening( 0.3f );
				}

				/* XXX: diffuse doesn't work since underline is an ActorFrame */
				ul.SetDiffuse( RageColor(1,1,1,bHidden? 0.0f:1.0f) );
				ul.SetBarWidth( iWidth );
				ul.SetY( (float)iY );
			}
		}
	}
}

void ScreenOptions::PositionIcons()
{
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

		for( unsigned i=0; i<m_Rows.size(); i++ )	// foreach options line
		{
			Row &row = *m_Rows[i];
			if( row.Type == Row::ROW_EXIT )
				continue;

			OptionIcon &icon = row.m_OptionIcons[p];

			int iChoiceWithFocus = row.m_iChoiceWithFocus[p];

			int iWidth, iX, iY;			// We only use iY
			GetWidthXY( (PlayerNumber)p, i, iChoiceWithFocus, iWidth, iX, iY );
			icon.SetX( ICONS_X(p) );

			if( icon.GetDestY() != row.m_fY )
			{
				icon.StopTweening();
				icon.BeginTweening( 0.3f );
			}

			icon.SetY( (float)iY );
			/* XXX: this doesn't work since icon is an ActorFrame */
			icon.SetDiffuse( RageColor(1,1,1, row.m_bHidden? 0.0f:1.0f) );
		}
	}
}

void ScreenOptions::RefreshIcons()
{
	// handled by ScreenOptionsMaster
}


void ScreenOptions::PositionCursors()
{
	// Set the position of the highlight showing the current option the user is changing.
	// Set the position of the underscores showing the current choice for each option line.
	for( int pn=0; pn<NUM_PLAYERS; pn++ )	// foreach player
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;

		const int iRow = m_iCurrentRow[pn];
		RAGE_ASSERT_M( iRow < (int)m_Rows.size(), ssprintf("%i < %i", iRow, m_Rows.size() ) );
		Row &Row = *m_Rows[iRow];

		OptionsCursor &highlight = m_Highlight[pn];

		const int iChoiceWithFocus = Row.m_iChoiceWithFocus[pn];

		int iWidth, iX, iY;
		GetWidthXY( (PlayerNumber)pn, iRow, iChoiceWithFocus, iWidth, iX, iY );
		highlight.SetBarWidth( iWidth );
		highlight.SetXY( (float)iX, (float)iY );
	}
}

void ScreenOptions::TweenCursor( PlayerNumber pn )
{
	// Set the position of the highlight showing the current option the user is changing.
	const int iRow = m_iCurrentRow[pn];
	RAGE_ASSERT_M( iRow < (int)m_Rows.size(), ssprintf("%i < %i", iRow, m_Rows.size() ) );

	const Row &Row = *m_Rows[iRow];
	const int iChoiceWithFocus = Row.m_iChoiceWithFocus[pn];

	int iWidth, iX, iY;
	GetWidthXY( pn, iRow, iChoiceWithFocus, iWidth, iX, iY );

	OptionsCursor &highlight = m_Highlight[pn];
	highlight.StopTweening();
	highlight.BeginTweening( 0.2f );
	highlight.TweenBarWidth( iWidth );
	highlight.SetXY( (float)iX, (float)iY );

	if( GAMESTATE->IsHumanPlayer(pn) )  
	{
		UtilCommand( m_sprLineHighlight[pn], "ScreenOptions", "Change" );
		if( m_Rows[iRow]->Type == Row::ROW_EXIT )
			UtilCommand( m_sprLineHighlight[pn], "ScreenOptions", "ChangeToExit" );
		m_sprLineHighlight[pn].SetY( (float)iY );
	}
}

void ScreenOptions::UpdateText( PlayerNumber pn, int iRow )
{
	Row &row = *m_Rows[iRow];
	const OptionRowData &data = row.m_RowDef;

	if( !row.m_bRowIsLong )
		return;

	int iChoiceWithFocus = row.m_iChoiceWithFocus[pn];

	unsigned item_no = data.bOneChoiceForAllPlayers ? 0 : pn;

	/* If player_no is 2 and there is no player 1: */
	item_no = min( item_no, row.m_textItems.size()-1 );

	row.m_textItems[item_no]->SetText( data.choices[iChoiceWithFocus] );
}

void ScreenOptions::UpdateEnabledDisabled()
{
	const RageColor colorSelected = COLOR_SELECTED, colorNotSelected = COLOR_NOT_SELECTED;

	// init text
	for( unsigned i=0; i<m_Rows.size(); i++ )		// foreach line
	{
		Row &row = *m_Rows[i];

		bool bThisRowIsSelected = false;
		for( int p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsHumanPlayer(p) && m_iCurrentRow[p] == (int) i )
				bThisRowIsSelected = true;

		/* Don't tween selection colors at all. */
		const RageColor color = bThisRowIsSelected? colorSelected:colorNotSelected;
		row.m_sprBullet.SetGlobalDiffuseColor( color );
		row.m_textTitle.SetGlobalDiffuseColor( color );

		{
			for( unsigned j=0; j<row.m_textItems.size(); j++ )
				row.m_textItems[j]->SetGlobalDiffuseColor( color );
		}

		{
			for( unsigned j=0; j<row.m_textItems.size(); j++ )
			{
				const float DiffuseAlpha = row.m_bHidden? 0.0f:1.0f;
				if( row.m_textItems[j]->GetDestY() == row.m_fY &&
					row.m_textItems[j]->DestTweenState().diffuse[0][3] == DiffuseAlpha )
					continue;

				row.m_textItems[j]->StopTweening();
				row.m_textItems[j]->BeginTweening( 0.3f );
				row.m_textItems[j]->SetDiffuseAlpha( DiffuseAlpha );
				row.m_textItems[j]->SetY( row.m_fY );
			}
		}

		if( row.Type == Row::ROW_EXIT )
		{
			bool bExitRowIsSelectedByBoth = true;
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p)  &&  m_iCurrentRow[p] != (int) i )
					bExitRowIsSelectedByBoth = false;

			if( bExitRowIsSelectedByBoth )
				row.m_textItems[0]->SetEffectDiffuseShift( 1.0f, colorSelected, colorNotSelected );
			else
				row.m_textItems[0]->SetEffectNone();
		}

		if( row.m_sprBullet.GetDestY() != row.m_fY )
		{
			row.m_sprBullet.StopTweening();
			row.m_textTitle.StopTweening();
			row.m_sprBullet.BeginTweening( 0.3f );
			row.m_textTitle.BeginTweening( 0.3f );

			row.m_sprBullet.SetDiffuseAlpha( row.m_bHidden? 0.0f:1.0f );
			row.m_textTitle.SetDiffuseAlpha( row.m_bHidden? 0.0f:1.0f );

			row.m_sprBullet.SetY( row.m_fY );
			row.m_textTitle.SetY( row.m_fY );
		}
	}
}

void ScreenOptions::Update( float fDeltaTime )
{
	//LOG->Trace( "ScreenOptions::Update(%f)", fDeltaTime );

	Screen::Update( fDeltaTime );
}

void ScreenOptions::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	/* Allow input when transitioning in (m_In.IsTransitioning()), but ignore it
	 * when we're transitioning out. */
	if( m_Menu.m_Back.IsTransitioning() || m_Menu.m_Out.IsTransitioning() )
		return;

	if( type == IET_RELEASE )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_START:
		case MENU_BUTTON_RIGHT:
		case MENU_BUTTON_LEFT:
			INPUTMAPPER->ResetKeyRepeat( MenuInput(MenuI.player, MENU_BUTTON_START) );
			INPUTMAPPER->ResetKeyRepeat( MenuInput(MenuI.player, MENU_BUTTON_RIGHT) );
			INPUTMAPPER->ResetKeyRepeat( MenuInput(MenuI.player, MENU_BUTTON_LEFT) );
		}
	}

	// if we are in dedicated menubutton input and arcade navigation
	// check to see if MENU_BUTTON_LEFT and MENU_BUTTON_RIGHT are being held
	/* This was left or right, instead of left and right.  Require both.  When
	 * running through a menu quickly in three key mode with lots of right and
	 * start taps, it's very easy to tap start before actually releasing the right
	 * tap, causing the menu to move up when you wanted it to go down. */
	const bool bHoldingLeftAndRight = MenuI.IsValid() && MenuI.button == MENU_BUTTON_START &&
		m_OptionsNavigation == NAV_THREE_KEY &&
		INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_RIGHT) ) &&
		INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_LEFT) );

	if( type != IET_RELEASE && bHoldingLeftAndRight )
	{
		// If moving up from a bFirstChoiceGoesDown row, put focus back on 
		// the first choice before moving up.
		int iCurrentRow = m_iCurrentRow[MenuI.player];
		Row &row = *m_Rows[iCurrentRow];
		if( NAV_FIRST_CHOICE_GOES_DOWN )
			row.m_iChoiceWithFocus[MenuI.player] = 0;

		MoveRow( MenuI.player, -1, type != IET_FIRST_PRESS );
		
		return;
	}

	// default input handler
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenOptions::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		StartGoToNextState();
		break;
	case SM_GoToPrevScreen:
//		this->ExportOptions();	// Don't save options if we're going back!
		this->GoToPrevState();
		break;
	case SM_GoToNextScreen:
		this->ExportOptions();
		this->GoToNextState();
		break;
	case SM_BeginFadingOut:
		if(m_Menu.IsTransitioning())
			return; /* already transitioning */
		m_Menu.StartTransitioning( SM_GoToNextScreen );

		m_SoundStart.Play();

		m_framePage.Command( FRAME_OFF_COMMAND );
		break;
	case SM_GainFocus:
		INPUTFILTER->SetRepeatRate( 0.25f, 12, 0.25f, 12 );
		break;
	case SM_LoseFocus:
		INPUTFILTER->ResetRepeatRate();
		break;
	}
}


void ScreenOptions::PositionItems()
{
	const int total = NUM_SHOWN_ITEMS;
	const int halfsize = total / 2;

	int first_start, first_end, second_start, second_end;

	/* Choices for each player.  If only one player is active, it's the same for both. */
	int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? m_iCurrentRow[PLAYER_1]: m_iCurrentRow[PLAYER_2];
	int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? m_iCurrentRow[PLAYER_2]: m_iCurrentRow[PLAYER_1];

	vector<Row*> Rows( m_Rows );
	Row *ExitRow = NULL;

	if( SEPARATE_EXIT_ROW && Rows.back()->Type == Row::ROW_EXIT )
	{
		ExitRow = &*Rows.back();

		/* Remove the exit row for purposes of positioning everything else. */
		if( P1Choice == (int) Rows.size()-1 )
			--P1Choice;
		if( P2Choice == (int) Rows.size()-1 )
			--P2Choice;

		Rows.erase( Rows.begin()+Rows.size()-1, Rows.end() );
	}

	const bool BothPlayersActivated = GAMESTATE->IsHumanPlayer(PLAYER_1) && GAMESTATE->IsHumanPlayer(PLAYER_2);
	if( m_InputMode == INPUTMODE_TOGETHER || !BothPlayersActivated )
	{
		/* Simply center the cursor. */
		first_start = max( P1Choice - halfsize, 0 );
		first_end = first_start + total;
		second_start = second_end = first_end;
	} else {
		/* First half: */
		const int earliest = min( P1Choice, P2Choice );
		first_start = max( earliest - halfsize/2, 0 );
		first_end = first_start + halfsize;

		/* Second half: */
		const int latest = max( P1Choice, P2Choice );

		second_start = max( latest - halfsize/2, 0 );

		/* Don't overlap. */
		second_start = max( second_start, first_end );

		second_end = second_start + halfsize;
	}

	first_end = min( first_end, (int) Rows.size() );
	second_end = min( second_end, (int) Rows.size() );

	/* If less than total (and Rows.size()) are displayed, fill in the empty
	 * space intelligently. */
	while(1)
	{
		const int sum = (first_end - first_start) + (second_end - second_start);
		if( sum >= (int) Rows.size() || sum >= total)
			break; /* nothing more to display, or no room */

		/* First priority: expand the top of the second half until it meets
		 * the first half. */
		if( second_start > first_end )
			second_start--;
		/* Otherwise, expand either end. */
		else if( first_start > 0 )
			first_start--;
		else if( second_end < (int) Rows.size() )
			second_end++;
		else
			ASSERT(0); /* do we have room to grow or don't we? */
	}

	int pos = 0;
	for( int i=0; i<(int) Rows.size(); i++ )		// foreach row
	{
		float ItemPosition;
		if( i < first_start )
			ItemPosition = -0.5f;
		else if( i < first_end )
			ItemPosition = (float) pos++;
		else if( i < second_start )
			ItemPosition = halfsize - 0.5f;
		else if( i < second_end )
			ItemPosition = (float) pos++;
		else
			ItemPosition = (float) total - 0.5f;
			
		Row &row = *Rows[i];

		float fY = ITEMS_START_Y + ITEMS_SPACING_Y*ItemPosition;
		row.m_fY = fY;
		row.m_bHidden = i < first_start ||
							(i >= first_end && i < second_start) ||
							i >= second_end;
	}

	if( ExitRow )
	{
		ExitRow->m_fY = SEPARATE_EXIT_ROW_Y;
		ExitRow->m_bHidden = ( second_end != (int) Rows.size() );
	}
}


void ScreenOptions::OnChange( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	/* Update m_fY and m_bHidden[]. */
	PositionItems();

	/* Do positioning. */
	PositionUnderlines();
	RefreshIcons();
	PositionIcons();
	UpdateEnabledDisabled();

	if( SHOW_SCROLL_BAR )
	{
		float fPercent = 0;
		if( m_Rows.size() > 1 )
			fPercent = m_iCurrentRow[pn] / float(m_Rows.size()-1);
		m_ScrollBar.SetPercentage( pn, fPercent );
	}

	/* Update all players, since changing one player can move both cursors. */
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		TweenCursor( (PlayerNumber) p );

		/* If the last row is EXIT, and is hidden, then show MORE. */
		const bool ShowMore = m_Rows.back()->Type == Row::ROW_EXIT && m_Rows.back()->m_bHidden;
		if( m_bMoreShown != ShowMore )
		{
			m_bMoreShown = ShowMore;
			UtilCommand( m_sprMore, "ScreenOptions", m_bMoreShown? "ShowMore":"HideMore" );
		}

		const bool ExitSelected = m_Rows[m_iCurrentRow[pn]]->Type == Row::ROW_EXIT;
		if( p == pn || GAMESTATE->GetNumSidesJoined() == 1 )
		{
			if( m_bWasOnExit[p] != ExitSelected )
			{
				m_bWasOnExit[p] = ExitSelected;
				UtilCommand( m_sprMore, "ScreenOptions", ssprintf("Exit%sP%i", ExitSelected? "Selected":"Unselected", p+1) );
			}
		}
	}

	const int iCurRow = m_iCurrentRow[pn];
	const CString text = GetExplanationText( iCurRow );

	BitmapText *pText = NULL;
	switch( m_InputMode )
	{
	case INPUTMODE_INDIVIDUAL:
		pText = &m_textExplanation[pn];
		if( pText->GetText() != text )
		{
			pText->FinishTweening();
			pText->Command( EXPLANATION_ON_COMMAND(pn) );
			pText->SetText( text );
		}
		break;
	case INPUTMODE_TOGETHER:
		pText = &m_textExplanation[0];
		if( pText->GetText() != text )
		{
			pText->FinishTweening();
			pText->Command( EXPLANATION_TOGETHER_ON_COMMAND );
			pText->SetText( text );
		}
		break;
	}
}


void ScreenOptions::MenuBack( PlayerNumber pn )
{
	Screen::MenuBack( pn );

	m_Menu.Back( SM_GoToPrevScreen );
}

void ScreenOptions::StartGoToNextState()
{
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
}

void ScreenOptions::MenuStart( PlayerNumber pn, const InputEventType type )
{
	if( type == IET_RELEASE )
		return;
	
	Row &row = *m_Rows[m_iCurrentRow[pn]];
	OptionRowData &data = row.m_RowDef;
	
	// If this is a bFirstChoiceGoesDown, then  if this is a multiselect row.
	// Is this the right thing to do for five key navigation?
	if( data.bMultiSelect )
	{
		int iChoiceInRow = row.m_iChoiceWithFocus[pn];
		row.m_vbSelected[pn][iChoiceInRow] = !row.m_vbSelected[pn][iChoiceInRow];
		if( row.m_vbSelected[pn][iChoiceInRow] )
			m_SoundToggleOn.Play();
		else
			m_SoundToggleOff.Play();
		PositionUnderlines();
		RefreshIcons();

		if( m_OptionsNavigation == NAV_FIRST_CHOICE_GOES_DOWN )
			ChangeValueInRow( pn, -row.m_iChoiceWithFocus[pn], type != IET_FIRST_PRESS );	// move to the first choice
	}
	else
	{
		switch( m_OptionsNavigation )
		{
		case NAV_THREE_KEY:
		case NAV_FIRST_CHOICE_GOES_DOWN:
			{
				bool bAllOnExit = true;
				for( int p=0; p<NUM_PLAYERS; p++ )
					if( GAMESTATE->IsHumanPlayer(p)  &&  m_Rows[m_iCurrentRow[p]]->Type != Row::ROW_EXIT )
						bAllOnExit = false;

				if( row.Type == Row::ROW_EXIT  &&  bAllOnExit  &&  type == IET_FIRST_PRESS )
					StartGoToNextState();
				else if( m_OptionsNavigation == NAV_FIRST_CHOICE_GOES_DOWN )
				{
					ChangeValueInRow( pn, -row.m_iChoiceWithFocus[pn], type != IET_FIRST_PRESS );	// move to the first choice
					int iChoiceInRow = row.m_iChoiceWithFocus[pn];
					if( iChoiceInRow == 0 )
					{
						MenuDown( pn, type );
						return;
					}
				}
				else
					MenuDown( pn, type );
			}
			break;
		case NAV_THREE_KEY_MENU:
		case NAV_FIVE_KEY:
			if( type == IET_FIRST_PRESS )	// m_SMOptionsNavigation
				StartGoToNextState();
			break;
		}
	}
}

/* Left/right */
void ScreenOptions::ChangeValueInRow( PlayerNumber pn, int iDelta, bool Repeat )
{
	const int iCurRow = m_iCurrentRow[pn];
	Row &row = *m_Rows[iCurRow];
	OptionRowData &optrow = m_Rows[iCurRow]->m_RowDef;

	const int iNumOptions = (row.Type == Row::ROW_EXIT)? 1: optrow.choices.size();
	if( m_OptionsNavigation == NAV_THREE_KEY_MENU && iNumOptions <= 1 )	// 1 or 0
	{
		/* There are no other options on the row; move up or down instead of left and right.
		 * This allows navigating the options menu with left/right/start. 
		 *
		 * XXX: Only allow repeats if the opposite key isn't pressed; otherwise, holding both
		 * directions will repeat in place continuously, which is weird. */
		MoveRow( pn, iDelta, Repeat );
		return;
	}

	if( Repeat )
		return;

	if( row.Type == Row::ROW_EXIT	)	// EXIT is selected
		return;		// don't allow a move

	bool bOneChanged = false;


	int iCurrentChoiceWithFocus = row.m_iChoiceWithFocus[pn];
	int iNewChoiceWithFocus = iCurrentChoiceWithFocus + iDelta;
	wrap( iNewChoiceWithFocus, iNumOptions );
	
	if( iCurrentChoiceWithFocus != iNewChoiceWithFocus )
		bOneChanged = true;

	if( optrow.bOneChoiceForAllPlayers )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			row.m_iChoiceWithFocus[p] = iNewChoiceWithFocus;

			if( m_OptionsNavigation==NAV_FIRST_CHOICE_GOES_DOWN && iNewChoiceWithFocus==0 )
			{
				;	// do nothing
			}
			else
			{
				if( optrow.bMultiSelect )
					;	// do nothing.  User must press Start to toggle the selection.
				else
					row.SetOneSelection( (PlayerNumber)p, iNewChoiceWithFocus );			
			}

			UpdateText( (PlayerNumber)p, iCurRow );
		}
	}
	else
	{
		row.m_iChoiceWithFocus[pn] = iNewChoiceWithFocus;

		if( m_OptionsNavigation==NAV_FIRST_CHOICE_GOES_DOWN && iNewChoiceWithFocus==0 )
		{
			;	// do nothing
		}
		else
		{
			if( optrow.bMultiSelect )
				;	// do nothing.  User must press Start to toggle the selection.
			else
				row.SetOneSelection( pn, iNewChoiceWithFocus );
		}

		UpdateText( pn, iCurRow );
	}

	OnChange( pn );

	if( m_OptionsNavigation != NAV_THREE_KEY_MENU )
		m_SoundChangeCol.Play();
}


/* Up/down */
void ScreenOptions::MoveRow( PlayerNumber pn, int dir, bool Repeat ) 
{
	LOG->Trace("move pn %i, dir %i, rep %i", pn, dir, Repeat);
	bool changed = false;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_INDIVIDUAL && p != pn )
			continue;	// skip

		int row = m_iCurrentRow[p] + dir;
		if( Repeat && ( row == -1 || row == (int) m_Rows.size() ) )
			continue; // don't wrap while repeating

		wrap( row, m_Rows.size() );
		m_iCurrentRow[p] = row;

		OnChange( (PlayerNumber)p );
		changed = true;
	}
	if( changed )
		m_SoundNextRow.Play();
}

int ScreenOptions::GetCurrentRow( PlayerNumber pn ) const
{
	const int l = m_iCurrentRow[pn];
	if( m_Rows[l]->Type != Row::ROW_NORMAL )
		return -1;
	return l;
}

ScreenOptions::Row::Row()
{
	m_bRowIsLong = false;
}

ScreenOptions::Row::~Row()
{
	for( unsigned i = 0; i < m_textItems.size(); ++i )
		delete m_textItems[i];
	for( int p=0; p<NUM_PLAYERS; p++ )
		for( unsigned i = 0; i < m_Underline[p].size(); ++i )
			delete m_Underline[p][i];
}
