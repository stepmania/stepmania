#include "global.h"
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
#include "Course.h"
#include "Style.h"

const float ITEM_X[NUM_PLAYERS] = { 260, 420 };

#define ICONS_X( p )					THEME->GetMetricF("ScreenOptions",ssprintf("IconsP%dX",p+1))
#define ARROWS_X						THEME->GetMetricF("ScreenOptions","ArrowsX")
#define LABELS_X						THEME->GetMetricF("ScreenOptions","LabelsX")
#define LABELS_ZOOM						THEME->GetMetricF("ScreenOptions","LabelsZoom")
#define LABELS_H_ALIGN					THEME->GetMetricI("ScreenOptions","LabelsHAlign")
#define ITEMS_ZOOM						THEME->GetMetricF("ScreenOptions","ItemsZoom")
#define ITEMS_START_X					THEME->GetMetricF("ScreenOptions","ItemsStartX")
#define ITEMS_END_X						THEME->GetMetricF("ScreenOptions","ItemsEndX")
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
#define CAPITALIZE_ALL_OPTION_NAMES		THEME->GetMetricB(m_sName,"CapitalizeAllOptionNames")

/*
 * These navigation types are provided:
 *
 * All modes:
 *  left, right -> change option
 *  up, down -> change row
 *   (in toggle modes, focus on nearest item to old focus)
 *
 * NAV_THREE_KEY:
 *  start -> move to next row
 *  left+right+start -> move to prev row
 *  (next screen via "exit" entry)
 * This is the minimal navigation, for using menus with only three buttons.
 *
 * NAV_FIVE_KEY:
 *  start -> next screen
 * This is a much more convenient navigation, requiring five keys.
 *
 * NAV_TOGGLE_THREE_KEY:
 *  start -> on first choice, move to next row; otherewise toggle option and move to first choice
 *  left+right+start -> move to prev row
 *
 * NAV_TOGGLE_FIVE_KEY:
 *  start -> toggle option
 *
 * Regular modes and toggle modes must be enabled by the theme.  We could simply
 * automatically switch to toggle mode for multiselect rows, but that would be strange
 * for non-multiselect rows (eg. scroll speed).
 *
 * THREE_KEY modes are navigatable with only MenuLeft, MenuRight and MenuStart, and
 * are used when PREFSMAN->m_bArcadeOptionsNavigation is enabled.  However, they can
 * still use MenuUp and MenuDown for nonessential behavior.
 *
 * NAV_THREE_KEY_MENU:
 *  left, right -> change row
 *  up, down -> change row
 *  start -> next screen
 * This is a specialized navigation for ScreenOptionsMenu.  It must be enabled to
 * allow screens that use rows to select other screens to work with only three
 * buttons.  (It's also used when in five-key mode.)
 *
 * We don't want to simply allow left/right to move up and down on single-entry
 * rows when in NAV_THREE_KEY, becasue left and right shouldn't exit the "exit" row
 * in player options menus, but it should in the options menu.
 */

ScreenOptions::ScreenOptions( CString sClassName ) : ScreenWithMenuElements(sClassName)
{
	LOG->Trace( "ScreenOptions::ScreenOptions()" );

	m_OptionsNavigation = PREFSMAN->m_bArcadeOptionsNavigation? NAV_THREE_KEY:NAV_FIVE_KEY;

	m_SoundChangeCol.Load( THEME->GetPathToS("ScreenOptions change"), true );
	m_SoundNextRow.Load( THEME->GetPathToS("ScreenOptions next"), true );
	m_SoundPrevRow.Load( THEME->GetPathToS("ScreenOptions prev"), true );
	m_SoundToggleOn.Load( THEME->GetPathToS("ScreenOptions toggle on") );
	m_SoundToggleOff.Load( THEME->GetPathToS("ScreenOptions toggle off") );

	// add everything to m_framePage so we can animate everything at once
	this->AddChild( &m_framePage );

	m_bMoreShown = false;
	FOREACH_PlayerNumber( p )
	{
		m_iCurrentRow[p] = 0;
		m_iFocusX[p] = 0;
		m_bWasOnExit[p] = false;
		m_bGotAtLeastOneStartPressed[p] = false;
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

	for( int r=0; r<iNumOptionLines; r++ )		// foreach row
	{
		m_Rows.push_back( new Row() );
		Row &Row = *m_Rows[r];
		Row.m_RowDef = OptionRows[r];
		
		if( !OptionRows[r].choices.size() )
			RageException::Throw( "Screen %s menu entry \"%s\" has no choices",
			m_sName.c_str(), OptionRows[r].name.c_str() );
		
		FOREACH_PlayerNumber( p )
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
	
	this->ImportOptions();
	
	for( int r=0; r<iNumOptionLines; r++ )		// foreach row
	{
		Row &Row = *m_Rows[r];
		
		// Make all selections the same if bOneChoiceForAllPlayers
		if( Row.m_RowDef.bOneChoiceForAllPlayers )
		{
			for( int p=1; p<NUM_PLAYERS; p++ )
				Row.m_vbSelected[p] = m_Rows[r]->m_vbSelected[0];
		}
			
		CHECKPOINT_M( ssprintf("row %i: %s", r, Row.m_RowDef.name.c_str()) );
		FOREACH_PlayerNumber( p )
		{
			if( m_OptionsNavigation==NAV_TOGGLE_THREE_KEY || m_OptionsNavigation==NAV_TOGGLE_FIVE_KEY )
				Row.m_iChoiceWithFocus[p] = 0;	// focus on the first row, which is "go down"
			else
			{
				/* Make sure the row actually has a selection. */
				bool bHasSelection = false;
				unsigned i;
				for( i=0; i<Row.m_vbSelected[p].size(); i++ )
				{
					if( Row.m_vbSelected[p][i] )
						bHasSelection = true;
				}

				if( !bHasSelection )
				{
					LOG->Warn( "Options menu \"%s\" row %i has no selection", m_sName.c_str(), i );
					Row.m_vbSelected[p][0] = true;
				}
				
				Row.m_iChoiceWithFocus[p] = Row.GetOneSelection( (PlayerNumber)p);	// focus on the only selected choice
			}
		}
	}

	m_sprPage.Load( THEME->GetPathToG(m_sName+" page") );
	m_sprPage->SetName( "Page" );
	UtilSetXYAndOnCommand( m_sprPage, "ScreenOptions" );
	m_framePage.AddChild( m_sprPage );

	// init line highlights
	FOREACH_PlayerNumber( p )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		m_sprLineHighlight[p].Load( THEME->GetPathToG("ScreenOptions line highlight") );
		m_sprLineHighlight[p].SetName( "LineHighlight" );
		m_sprLineHighlight[p].SetX( CENTER_X );
		m_framePage.AddChild( &m_sprLineHighlight[p] );
		UtilOnCommand( m_sprLineHighlight[p], "ScreenOptions" );
	}
	
	// init highlights
	FOREACH_PlayerNumber( p )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		m_Highlight[p].Load( (PlayerNumber)p, false );
		m_framePage.AddChild( &m_Highlight[p] );
	}

	// init row icons
	FOREACH_PlayerNumber( p )
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

	// init m_textItems from optionLines
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
			CString sText = optline.choices[c];
			if( CAPITALIZE_ALL_OPTION_NAMES )
				sText.MakeUpper();
			bt->SetText( sText );
			bt->SetZoom( ITEMS_ZOOM );
			bt->SetShadowLength( 0 );

			// set the X position of each item in the line
			float fItemWidth = bt->GetZoomedWidth();
			fX += fItemWidth/2;
			bt->SetX( fX );

			// init underlines
			FOREACH_PlayerNumber( p )
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
			if( fX > ITEMS_END_X ) 
			{
				row.m_bRowIsLong = true;
				for( unsigned j=0; j<textItems.size(); j++ )	// for each option on this row
					delete textItems[j];
				textItems.clear();
				FOREACH_PlayerNumber( p )
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
			FOREACH_HumanPlayer( p )
			{
				BitmapText *bt = new BitmapText;
				textItems.push_back( bt );

				const int iChoiceWithFocus = row.m_iChoiceWithFocus[p];

				bt->LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
				bt->SetText( optline.choices[iChoiceWithFocus] );
				bt->SetZoom( ITEMS_ZOOM );
				bt->SetShadowLength( 0 );

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
			FOREACH_HumanPlayer( p )
			{
				OptionsCursor *ul = new OptionsCursor;
				row.m_Underline[p].push_back( ul );
				ul->Load( (PlayerNumber)p, true );
				int iWidth, iX, iY;
				GetWidthXY( (PlayerNumber) p, r, c, iWidth, iX, iY );
				ul->SetX( float(iX) );
				ul->SetWidth( float(iWidth) );
			}
		}

		// Add children here and not above because of the logic that starts
		// over if we run off the right edge of the screen.
		{
			for( unsigned c=0; c<textItems.size(); c++ )
				m_framePage.AddChild( textItems[c] );
			FOREACH_PlayerNumber( p )
				for( unsigned c=0; c<row.m_Underline[p].size(); c++ )
					m_framePage.AddChild( row.m_Underline[p][c] );
		}
	}

	// TRICKY:  Add one more item.  This will be "EXIT"
	m_Rows.push_back( new Row() );
	Row &row = *m_Rows.back();
	row.Type = Row::ROW_EXIT;

	BitmapText *bt = new BitmapText;
	row.m_textItems.push_back( bt );

	bt->LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
	bt->SetText( THEME->GetMetric("OptionNames","Exit") );
	bt->SetZoom( ITEMS_ZOOM );
	bt->SetShadowLength( 0 );
	bt->SetX( CENTER_X );

	m_framePage.AddChild( bt );

	InitOptionsText();

	// add explanation here so it appears on top
	{
		FOREACH_PlayerNumber( p )
		{
			m_textExplanation[p].LoadFromFont( THEME->GetPathToF("ScreenOptions explanation") );
			m_textExplanation[p].SetZoom( EXPLANATION_ZOOM );
			m_textExplanation[p].SetShadowLength( 0 );
			m_framePage.AddChild( &m_textExplanation[p] );
		}
	}

	/* Hack: if m_CurStyle is set, we're probably in the player or song options menu, so
	 * the player name is meaningful.  Otherwise, we're probably in the system menu. */
	if( GAMESTATE->m_pCurStyle != NULL )
	{
		FOREACH_HumanPlayer( p )
		{
			m_textPlayerName[p].LoadFromFont( THEME->GetPathToF( "ScreenOptions player") );
			m_textPlayerName[p].SetName( ssprintf("PlayerNameP%i",p+1) );
			m_textPlayerName[p].SetText( GAMESTATE->GetPlayerDisplayName((PlayerNumber)p) );
			UtilSetXYAndOnCommand( m_textPlayerName[p], "ScreenOptions" );
			m_framePage.AddChild( &m_textPlayerName[p] );
		}
	}

	if( SHOW_SCROLL_BAR )
	{
		m_ScrollBar.SetName( "DualScrollBar", "ScrollBar" );
		m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT );
		m_ScrollBar.SetBarTime( SCROLL_BAR_TIME );
		FOREACH_PlayerNumber( p )
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
			FOREACH_PlayerNumber( p )
				m_textExplanation[p].SetXY( EXPLANATION_X(p), EXPLANATION_Y(p) );
		}
		break;
	case INPUTMODE_SHARE_CURSOR:
		m_textExplanation[0].SetXY( EXPLANATION_TOGETHER_X, EXPLANATION_TOGETHER_Y );
		break;
	default:
		ASSERT(0);
	}

	FOREACH_PlayerNumber( p )
	{
		m_sprDisqualify[p].Load( THEME->GetPathToG( "ScreenOptions disqualify") );
		m_sprDisqualify[p]->SetName( "ScreenOptions", ssprintf("DisqualifyP%i",p+1) );
		UtilSetXYAndOnCommand( m_sprDisqualify[p], "ScreenOptions" );
		m_sprDisqualify[p]->SetHidden( true );	// unhide later if handicapping options are discovered
		m_framePage.AddChild( m_sprDisqualify[p] );
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
		FOREACH_PlayerNumber( p )
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
		FOREACH_PlayerNumber( p )
		{
			for( unsigned c=0; c<row.m_Underline[p].size(); c++ )
				row.m_Underline[p][c]->FinishTweening();
			row.m_OptionIcons[p].FinishTweening();
		}
	}

	m_sprMore->FinishTweening();

	this->SortByDrawOrder();
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
		if( SHOW_BPM_IN_SPEED_TITLE )
		{
			DisplayBpms bpms;
			if( GAMESTATE->m_pCurSong )
			{
				Song* pSong = GAMESTATE->m_pCurSong;
				pSong->GetDisplayBpms( bpms );
			}
			else if( GAMESTATE->m_pCurCourse )
			{
				Course *pCourse = GAMESTATE->m_pCurCourse;
				StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
				Trail* pTrail = pCourse->GetTrail( st );
				ASSERT( pTrail );
				pTrail->GetDisplayBpms( bpms );
			}

			if( bpms.IsMystery() )
				sTitle += ssprintf( " (??" "?)" ); /* split so gcc doesn't think this is a trigraph */
			else if( bpms.BpmIsConstant() )
				sTitle += ssprintf( " (%.0f)", bpms.GetMin() );
			else
				sTitle += ssprintf( " (%.0f-%.0f)", bpms.GetMin(), bpms.GetMax() );
		}
	}

	return sTitle;
}

BitmapText &ScreenOptions::GetTextItemForRow( PlayerNumber pn, int iRow, int iChoiceOnRow )
{
	ASSERT_M( iRow < (int)m_Rows.size(), ssprintf("%i < %i", iRow, (int)m_Rows.size() ) );
	Row &row = *m_Rows[iRow];
	if( row.Type == Row::ROW_EXIT )
		return *row.m_textItems[0];

	bool bOneChoice = row.m_RowDef.bOneChoiceForAllPlayers;
	int index = -1;
	if( row.m_bRowIsLong )
	{
		index = bOneChoice ? 0 : pn;
		/* If only P2 is enabled, his selections will be in index 0. */
		if( row.m_textItems.size() == 1 )
			index = 0;
	}
	else
		index = iChoiceOnRow;

	ASSERT_M( index < (int)row.m_textItems.size(), ssprintf("%i < %i", index, (int)row.m_textItems.size() ) );
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
		title.SetShadowLength( 0 );		

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

		FOREACH_PlayerNumber( p )
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
	FOREACH_PlayerNumber( p )	// foreach player
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
	FOREACH_PlayerNumber( pn )	// foreach player
	{
		if( !GAMESTATE->IsHumanPlayer(pn) )
			continue;

		const int iRow = m_iCurrentRow[pn];
		ASSERT_M( iRow < (int)m_Rows.size(), ssprintf("%i < %i", iRow, (int)m_Rows.size() ) );
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
	ASSERT_M( iRow < (int)m_Rows.size(), ssprintf("%i < %i", iRow, (int)m_Rows.size() ) );

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

/* For "long row-style" rows, update the text on screen to contain the currently-
 * focused options. 
 *
 * This used to update a single player, but it's not always clear what that means
 * when dealing with bOneChoiceForAllPlayers and disabled players, which was brittle.
 * Update the whole row. */
void ScreenOptions::UpdateText( int iRow )
{
	Row &row = *m_Rows[iRow];
	const OptionRowData &data = row.m_RowDef;

	if( !row.m_bRowIsLong )
		return;

	FOREACH_HumanPlayer( pn )
	{
		int iChoiceWithFocus = row.m_iChoiceWithFocus[pn];
		unsigned item_no = data.bOneChoiceForAllPlayers ? 0 : pn;

		/* If player_no is 2 and there is no player 1: */
		item_no = min( item_no, row.m_textItems.size()-1 );

		row.m_textItems[item_no]->SetText( data.choices[iChoiceWithFocus] );
	}
}

void ScreenOptions::UpdateEnabledDisabled()
{
	const RageColor colorSelected = COLOR_SELECTED, colorNotSelected = COLOR_NOT_SELECTED;

	// init text
	for( unsigned i=0; i<m_Rows.size(); i++ )		// foreach line
	{
		Row &row = *m_Rows[i];

		bool bThisRowIsSelected = false;
		FOREACH_PlayerNumber( p )
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
			FOREACH_PlayerNumber( p )
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
	Screen::DrawPrimitives();
}

void ScreenOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	/* Allow input when transitioning in (m_In.IsTransitioning()), but ignore it
	 * when we're transitioning out. */
	if( m_Back.IsTransitioning() || m_Out.IsTransitioning() )
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
		if(IsTransitioning())
			return; /* already transitioning */
		StartTransitioning( SM_GoToNextScreen );

		SCREENMAN->PlayStartSound();

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
	if( m_InputMode == INPUTMODE_SHARE_CURSOR || !BothPlayersActivated )
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


	/* If the last row is EXIT, and is hidden, then show MORE. */
	const bool ShowMore = m_Rows.back()->Type == Row::ROW_EXIT && m_Rows.back()->m_bHidden;
	if( m_bMoreShown != ShowMore )
	{
		m_bMoreShown = ShowMore;
		UtilCommand( m_sprMore, "ScreenOptions", m_bMoreShown? "ShowMore":"HideMore" );
	}

	/* Update all players, since changing one player can move both cursors. */
	FOREACH_PlayerNumber( p )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
			TweenCursor( (PlayerNumber) p );

		const bool ExitSelected = m_Rows[m_iCurrentRow[pn]]->Type == Row::ROW_EXIT;
		if( p == pn || GAMESTATE->GetNumHumanPlayers() == 1 )
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
	case INPUTMODE_SHARE_CURSOR:
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

	Back( SM_GoToPrevScreen );
}

void ScreenOptions::StartGoToNextState()
{
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
}

bool ScreenOptions::AllAreOnExit() const
{
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsHumanPlayer(p)  &&  m_Rows[m_iCurrentRow[p]]->Type != Row::ROW_EXIT )
			return false;
	return true;
}

void ScreenOptions::MenuStart( PlayerNumber pn, const InputEventType type )
{
	switch( type )
	{
	case IET_FIRST_PRESS:
		m_bGotAtLeastOneStartPressed[pn] = true;
		break;
	case IET_RELEASE:
		return;	// ignore
	default:	// repeat type
		if( !m_bGotAtLeastOneStartPressed[pn] )
			return;	// don't allow repeat
		break;
	}
	
	Row &row = *m_Rows[m_iCurrentRow[pn]];
	OptionRowData &data = row.m_RowDef;


	/* If we are in a three-button mode, check to see if MENU_BUTTON_LEFT and
	 * MENU_BUTTON_RIGHT are being held. */
	switch( m_OptionsNavigation )
	{
	case NAV_THREE_KEY:
	case NAV_TOGGLE_THREE_KEY:
		{
			bool bHoldingLeftAndRight = 
				INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_RIGHT) ) &&
				INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_LEFT) );
			if( bHoldingLeftAndRight )
			{
				MoveRow( pn, -1, type != IET_FIRST_PRESS );		
				return;
			}
		}
	}


	// If on exit, check it all players are on "Exit"
	if( row.Type == Row::ROW_EXIT )
	{
		/* Don't accept START to go to the next screen if we're still transitioning in. */
		if( AllAreOnExit()  &&  type == IET_FIRST_PRESS && !IsTransitioning() )
			StartGoToNextState();
		return;
	}


	if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY )
	{
		int iChoiceInRow = row.m_iChoiceWithFocus[pn];
		if( iChoiceInRow == 0 )
		{
			MenuDown( pn, type );
			return;
		}
	}
	
	// If this is a bFirstChoiceGoesDown, then  if this is a multiselect row.
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

		if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY )
			ChangeValueInRow( pn, -row.m_iChoiceWithFocus[pn], type != IET_FIRST_PRESS );	// move to the first choice
	}
	else
	{
		switch( m_OptionsNavigation )
		{
		case NAV_THREE_KEY:
			MenuDown( pn, type );
			break;
		case NAV_TOGGLE_THREE_KEY:
		case NAV_TOGGLE_FIVE_KEY:
			if( !data.bMultiSelect )
			{
				int iChoiceInRow = row.m_iChoiceWithFocus[pn];
				if( row.m_RowDef.bOneChoiceForAllPlayers )
					row.SetOneSharedSelection( iChoiceInRow );
				else
					row.SetOneSelection( pn, iChoiceInRow );
			}
			if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY )
				ChangeValueInRow( pn, -row.m_iChoiceWithFocus[pn], type != IET_FIRST_PRESS );	// move to the first choice
			else
				ChangeValueInRow( pn, 0, type != IET_FIRST_PRESS );
			break;
		case NAV_THREE_KEY_MENU:
			/* Don't accept START to go to the next screen if we're still transitioning in. */
			if( type == IET_FIRST_PRESS && !IsTransitioning() )
				StartGoToNextState();
			break;
		case NAV_FIVE_KEY:
			/* Jump to the exit row.  (If everyone's already on the exit row, then
			 * we'll have already gone to the next screen above.) */
			MoveRow( pn, m_Rows.size()-m_iCurrentRow[pn]-1, type != IET_FIRST_PRESS );
			break;
		}
	}
}

void ScreenOptions::StoreFocus( PlayerNumber pn )
{
	/* Long rows always put us in the center, so don't update the focus. */
	const Row &Row = *m_Rows[m_iCurrentRow[pn]];
	if( Row.m_bRowIsLong )
		return;

	int iWidth, iY;
	GetWidthXY( pn, m_iCurrentRow[pn], m_Rows[m_iCurrentRow[pn]]->m_iChoiceWithFocus[pn], iWidth, m_iFocusX[pn], iY );
	LOG->Trace("cur selection %ix%i @ %i", m_iCurrentRow[pn], m_Rows[m_iCurrentRow[pn]]->m_iChoiceWithFocus[pn],
		m_iFocusX[pn]);
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

	row.m_iChoiceWithFocus[pn] = iNewChoiceWithFocus;
	StoreFocus( pn );

	if( optrow.bOneChoiceForAllPlayers )
	{
		/* If this row is bOneChoiceForAllPlayers, then lock the cursors together
		 * for this row.  Don't do this in toggle modes, since the current selection
		 * and the current focus are detached. */
		bool bForceFocusedChoiceTogether = false;
		if( m_OptionsNavigation!=NAV_TOGGLE_THREE_KEY &&
			m_OptionsNavigation!=NAV_TOGGLE_FIVE_KEY &&
			optrow.bOneChoiceForAllPlayers )
			bForceFocusedChoiceTogether = true;

		/* Also lock focus if the screen is explicitly set to share cursors. */
		if( m_InputMode == INPUTMODE_SHARE_CURSOR )
			bForceFocusedChoiceTogether = true;

		if( bForceFocusedChoiceTogether )
		{
			// lock focus together
			FOREACH_HumanPlayer( p )
			{
				row.m_iChoiceWithFocus[p] = iNewChoiceWithFocus;
				StoreFocus( pn );
			}
		}

		FOREACH_PlayerNumber( p )
		{
			if( m_OptionsNavigation==NAV_TOGGLE_THREE_KEY || m_OptionsNavigation==NAV_TOGGLE_FIVE_KEY )
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
		}
	}
	else
	{
		if( m_OptionsNavigation==NAV_TOGGLE_THREE_KEY || m_OptionsNavigation==NAV_TOGGLE_FIVE_KEY )
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
	}

	UpdateText( iCurRow );

	OnChange( pn );

	if( m_OptionsNavigation != NAV_THREE_KEY_MENU )
		m_SoundChangeCol.Play();
}


/* Up/down */
void ScreenOptions::MoveRow( PlayerNumber pn, int dir, bool Repeat ) 
{
	if( m_OptionsNavigation==NAV_TOGGLE_THREE_KEY )
	{
		// If moving from a bFirstChoiceGoesDown row, put focus back on 
		// the first choice before moving.
		const int iCurrentRow = m_iCurrentRow[pn];
		Row &row = *m_Rows[iCurrentRow];
		row.m_iChoiceWithFocus[pn] = 0;
	}

	LOG->Trace("move pn %i, dir %i, rep %i", pn, dir, Repeat);
	bool changed = false;
	FOREACH_PlayerNumber( p )
	{
		if( m_InputMode == INPUTMODE_INDIVIDUAL && p != pn )
			continue;	// skip

		int row = m_iCurrentRow[p] + dir;
		if( Repeat && ( row == -1 || row == (int) m_Rows.size() ) )
			continue; // don't wrap while repeating

		wrap( row, m_Rows.size() );

		const unsigned iOldSelection = m_Rows[m_iCurrentRow[p]]->m_iChoiceWithFocus[p];

		m_iCurrentRow[p] = row;

		switch( m_OptionsNavigation )
		{
		case NAV_TOGGLE_THREE_KEY:
		case NAV_TOGGLE_FIVE_KEY:
			if( !m_Rows[row]->m_bRowIsLong )
			{
				int iSelectionDist = -1;
				for( unsigned i = 0; i < m_Rows[row]->m_textItems.size(); ++i )
				{
					int iWidth, iX, iY;
					GetWidthXY( p, m_iCurrentRow[p], i, iWidth, iX, iY );
					const int iDist = abs( iX-m_iFocusX[p] );
					if( iSelectionDist == -1 || iDist < iSelectionDist )
					{
						iSelectionDist = iDist;
						m_Rows[row]->m_iChoiceWithFocus[p] = i;
					}
				}
				m_Rows[row]->m_iChoiceWithFocus[p] = min( iOldSelection, m_Rows[row]->m_textItems.size()-1 );
			}
		}

		OnChange( (PlayerNumber)p );
		changed = true;
	}
	if( changed )
	{
		if( dir < 0 )
			m_SoundPrevRow.Play();
		else
			m_SoundNextRow.Play();
	}
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
	FOREACH_PlayerNumber( p )
		for( unsigned i = 0; i < m_Underline[p].size(); ++i )
			delete m_Underline[p][i];
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
