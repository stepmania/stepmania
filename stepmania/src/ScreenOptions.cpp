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
#include "ScreenDimensions.h"
#include "Command.h"
#include "FontManager.h"
#include "Font.h"


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

CString ROW_Y_NAME( size_t r )					{ return ssprintf("Row%dY",r+1); }
CString ITEMS_LONG_ROW_X_NAME( size_t p )		{ return ssprintf("ItemsLongRowP%dX",p+1); }
CString ICONS_X_NAME( size_t p )				{ return ssprintf("IconsP%dX",p+1); }
CString EXPLANATION_X_NAME( size_t p )			{ return ssprintf("ExplanationP%dX",p+1); }
CString EXPLANATION_Y_NAME( size_t p )			{ return ssprintf("ExplanationP%dY",p+1); }
CString EXPLANATION_ON_COMMAND_NAME( size_t p )	{ return ssprintf("ExplanationP%dOnCommand",p+1); }


//REGISTER_SCREEN_CLASS( ScreenOptions );	// can't be instantiated
ScreenOptions::ScreenOptions( CString sClassName ) : ScreenWithMenuElements(sClassName),
	ARROWS_X   						(m_sName,"ArrowsX"),
	LABELS_X						(m_sName,"LabelsX"),
	LABELS_ON_COMMAND				(m_sName,"LabelsOnCommand"),
	NUM_ROWS_SHOWN					(m_sName,"NumRowsShown"),
	ROW_Y							(m_sName,ROW_Y_NAME,NUM_ROWS_SHOWN),
	ROW_Y_OFF_SCREEN_TOP			(m_sName,"RowYOffScreenTop"),
	ROW_Y_OFF_SCREEN_CENTER			(m_sName,"RowYOffScreenCenter"),
	ROW_Y_OFF_SCREEN_BOTTOM			(m_sName,"RowYOffScreenBottom"),
	ITEMS_ZOOM						(m_sName,"ItemsZoom"),
	ITEMS_START_X					(m_sName,"ItemsStartX"),
	ITEMS_END_X						(m_sName,"ItemsEndX"),
	ITEMS_GAP_X						(m_sName,"ItemsGapX"),
	ITEMS_LONG_ROW_X				(m_sName,ITEMS_LONG_ROW_X_NAME,NUM_PLAYERS),
	ITEMS_LONG_ROW_SHARED_X			(m_sName,"ItemsLongRowSharedX"),
	ICONS_X							(m_sName,ICONS_X_NAME,NUM_PLAYERS),
	EXPLANATION_X					(m_sName,EXPLANATION_X_NAME,NUM_PLAYERS),
	EXPLANATION_Y					(m_sName,EXPLANATION_Y_NAME,NUM_PLAYERS),
	EXPLANATION_ON_COMMAND			(m_sName,EXPLANATION_ON_COMMAND_NAME,NUM_PLAYERS),
	EXPLANATION_TOGETHER_X			(m_sName,"ExplanationTogetherX"),
	EXPLANATION_TOGETHER_Y			(m_sName,"ExplanationTogetherY"),
	EXPLANATION_TOGETHER_ON_COMMAND	(m_sName,"ExplanationTogetherOnCommand"),
	SHOW_SCROLL_BAR					(m_sName,"ShowScrollBar"),
	SCROLL_BAR_HEIGHT				(m_sName,"ScrollBarHeight"),
	SCROLL_BAR_TIME					(m_sName,"ScrollBarTime"),
	EXPLANATION_ZOOM				(m_sName,"ExplanationZoom"),
	COLOR_SELECTED					(m_sName,"ColorSelected"),
	COLOR_NOT_SELECTED				(m_sName,"ColorNotSelected"),
	SHOW_BPM_IN_SPEED_TITLE			(m_sName,"ShowBpmInSpeedTitle"),
	FRAME_ON_COMMAND				(m_sName,"FrameOnCommand"),
	FRAME_OFF_COMMAND				(m_sName,"FrameOffCommand"),
	SEPARATE_EXIT_ROW				(m_sName,"SeparateExitRow"),
	SEPARATE_EXIT_ROW_Y				(m_sName,"SeparateExitRowY"),
	CAPITALIZE_ALL_OPTION_NAMES		(m_sName,"CapitalizeAllOptionNames")
{
	LOG->Trace( "ScreenOptions::ScreenOptions()" );


	m_OptionsNavigation = PREFSMAN->m_bArcadeOptionsNavigation? NAV_THREE_KEY:NAV_FIVE_KEY;

	m_SoundChangeCol.Load( THEME->GetPathS(m_sName,"change"), true );
	m_SoundNextRow.Load( THEME->GetPathS(m_sName,"next"), true );
	m_SoundPrevRow.Load( THEME->GetPathS(m_sName,"prev"), true );
	m_SoundToggleOn.Load( THEME->GetPathS(m_sName,"toggle on") );
	m_SoundToggleOff.Load( THEME->GetPathS(m_sName,"toggle off") );

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

	m_bShowUnderlines = true;

	m_framePage.RunCommands( FRAME_ON_COMMAND );
}

void ScreenOptions::LoadOptionIcon( PlayerNumber pn, int iRow, CString sText )
{
	m_Rows[iRow]->m_OptionIcons[pn].Load( pn, sText, false );
}

void ScreenOptions::InitMenu( InputMode im, OptionRowDefinition defs[], int iNumOptionLines, bool bShowUnderlines )
{
	LOG->Trace( "ScreenOptions::Set()" );

	m_InputMode = im;

	m_bShowUnderlines = bShowUnderlines;

	for( int r=0; r<iNumOptionLines; r++ )		// foreach row
	{
		m_Rows.push_back( new OptionRow() );
		OptionRow &row = *m_Rows.back();
		row.m_RowDef = defs[r];
		row.Type = OptionRow::ROW_NORMAL;
		
		if( !defs[r].choices.size() )
			RageException::Throw( "Screen %s menu entry \"%s\" has no choices",
			m_sName.c_str(), defs[r].name.c_str() );
		
		FOREACH_PlayerNumber( p )
		{
			vector<bool> &vbSelected = row.m_vbSelected[p];
			vbSelected.resize( row.m_RowDef.choices.size() );
			for( unsigned j=0; j<vbSelected.size(); j++ )
				vbSelected[j] = false;
			
			// set select the first item if a SELECT_ONE row
			if( row.m_RowDef.selectType == SELECT_ONE )
				vbSelected[0] = true;
		}
	}
	
	this->ImportOptions();
	
	for( int r=0; r<iNumOptionLines; r++ )		// foreach row
	{
		OptionRow &row = *m_Rows[r];
		
		// Make all selections the same if bOneChoiceForAllPlayers
		if( row.m_RowDef.bOneChoiceForAllPlayers )
		{
			for( int p=1; p<NUM_PLAYERS; p++ )
				row.m_vbSelected[p] = m_Rows[r]->m_vbSelected[0];
		}
			
		CHECKPOINT_M( ssprintf("row %i: %s", r, row.m_RowDef.name.c_str()) );
		FOREACH_PlayerNumber( p )
		{
			//
			// Set focus to one item in the row
			//
			if( m_OptionsNavigation==NAV_TOGGLE_THREE_KEY || m_OptionsNavigation==NAV_TOGGLE_FIVE_KEY )
			{
				row.m_iChoiceInRowWithFocus[p] = 0;	// focus on the first row, which is "go down"
			}
			else if( row.m_RowDef.selectType == SELECT_ONE )
			{
				/* Make sure the row actually has a selection. */
				bool bHasSelection = false;
				unsigned i;
				for( i=0; i<row.m_vbSelected[p].size(); i++ )
				{
					if( row.m_vbSelected[p][i] )
						bHasSelection = true;
				}

				if( !bHasSelection )
				{
					LOG->Warn( "Options menu \"%s\" row index %i has no selection", m_sName.c_str(), r );
					row.m_vbSelected[p][0] = true;
				}
				
				row.m_iChoiceInRowWithFocus[p] = row.GetOneSelection(p);	// focus on the selection we just set
			}
			else
			{
				row.m_iChoiceInRowWithFocus[p] = 0;
			}
		}
	}

	m_sprPage.Load( THEME->GetPathG(m_sName,"page") );
	m_sprPage->SetName( "Page" );
	SET_XY_AND_ON_COMMAND( m_sprPage );
	m_framePage.AddChild( m_sprPage );

	// init line highlights
	FOREACH_HumanPlayer( p )
	{
		m_sprLineHighlight[p].Load( THEME->GetPathG(m_sName,"line highlight") );
		m_sprLineHighlight[p].SetName( "LineHighlight" );
		m_sprLineHighlight[p].SetX( SCREEN_CENTER_X );
		m_framePage.AddChild( &m_sprLineHighlight[p] );
		ON_COMMAND( m_sprLineHighlight[p] );
	}
	
	// init highlights
	FOREACH_HumanPlayer( p )
	{
		m_Highlight[p].Load( p, false );
		m_framePage.AddChild( &m_Highlight[p] );
	}

	// init row icons
	FOREACH_HumanPlayer( p )
	{
		for( unsigned l=0; l<m_Rows.size(); l++ )
		{	
			OptionRow &row = *m_Rows[l];

			LoadOptionIcon( p, l, "" );
			m_framePage.AddChild( &row.m_OptionIcons[p] );
		}
	}

	
	Font* pFont = FONT->LoadFont( THEME->GetPathF(m_sName,"item") );


	// init m_textItems from optionLines
	for( unsigned  r=0; r<m_Rows.size(); r++ )		// foreach row
	{
		OptionRow &row = *m_Rows[r];

		vector<BitmapText *> & textItems = row.m_textItems;
		const OptionRowDefinition &optline = m_Rows[r]->m_RowDef;

		m_framePage.AddChild( &row.m_sprBullet );
		m_framePage.AddChild( &row.m_textTitle );		


		// If the items will go off the edge of the screen, then re-init with the "long row" style.
		{
			float fX = ITEMS_START_X;
			
			for( unsigned c=0; c<optline.choices.size(); c++ )
			{
				CString sText = optline.choices[c];
				if( CAPITALIZE_ALL_OPTION_NAMES )
					sText.MakeUpper();
				fX += ITEMS_ZOOM * pFont->GetLineWidthInSourcePixels( CStringToWstring(sText) );

				if( fX > ITEMS_END_X ) 
				{
					row.m_RowDef.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
					break;
				}
			}
		}

		switch( row.m_RowDef.layoutType )
		{
		case LAYOUT_SHOW_ONE_IN_ROW:
			// init text
			FOREACH_HumanPlayer( p )
			{
				BitmapText *bt = new BitmapText;
				textItems.push_back( bt );

				const int iChoiceInRowWithFocus = row.m_iChoiceInRowWithFocus[p];

				bt->LoadFromFont( THEME->GetPathF(m_sName,"item") );
				CString sText = optline.choices[iChoiceInRowWithFocus];
				if( CAPITALIZE_ALL_OPTION_NAMES )
					sText.MakeUpper();
				bt->SetText( sText );
				bt->SetZoom( ITEMS_ZOOM );
				bt->SetShadowLength( 0 );

				if( optline.bOneChoiceForAllPlayers )
				{
					bt->SetX( ITEMS_LONG_ROW_SHARED_X );
					break;	// only initialize one item since it's shared
				}
				else
				{
					bt->SetX( ITEMS_LONG_ROW_X.GetValue(p) );
				}
			}

			// init underlines
			FOREACH_HumanPlayer( p )
			{
				OptionsCursor *ul = new OptionsCursor;
				row.m_Underline[p].push_back( ul );
				ul->Load( p, true );
				int iWidth, iX, iY;
				GetWidthXY( p, r, 0, iWidth, iX, iY );
				ul->SetX( float(iX) );
				ul->SetWidth( float(iWidth) );
			}
			break;

		case LAYOUT_SHOW_ALL_IN_ROW:
			{
				float fX = ITEMS_START_X;
				for( unsigned c=0; c<optline.choices.size(); c++ )
				{
					// init text
					BitmapText *bt = new BitmapText;
					textItems.push_back( bt );
					bt->LoadFromFont( THEME->GetPathF(m_sName,"item") );
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
					FOREACH_HumanPlayer( p )
					{
						OptionsCursor *ul = new OptionsCursor;
						row.m_Underline[p].push_back( ul );
						ul->Load( p, true );
						ul->SetX( fX );
						ul->SetWidth( truncf(fItemWidth) );
					}

					fX += fItemWidth/2 + ITEMS_GAP_X;
				}
			}
			break;

		default:
			ASSERT(0);
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

	FONT->UnloadFont( pFont );
	pFont = NULL;


	// TRICKY:  Add one more item.  This will be "EXIT"
	m_Rows.push_back( new OptionRow() );
	OptionRow &row = *m_Rows.back();
	row.Type = OptionRow::ROW_EXIT;

	BitmapText *bt = new BitmapText;
	row.m_textItems.push_back( bt );

	bt->LoadFromFont( THEME->GetPathF(m_sName,"item") );
	bt->SetText( THEME->GetMetric("OptionNames","Exit") );
	bt->SetZoom( ITEMS_ZOOM );
	bt->SetShadowLength( 0 );
	bt->SetX( ITEMS_LONG_ROW_SHARED_X );

	m_framePage.AddChild( bt );

	InitOptionsText();

	// add explanation here so it appears on top
	FOREACH_PlayerNumber( p )
	{
		m_textExplanation[p].LoadFromFont( THEME->GetPathF(m_sName,"explanation") );
		m_textExplanation[p].SetZoom( EXPLANATION_ZOOM );
		m_textExplanation[p].SetShadowLength( 0 );
		m_framePage.AddChild( &m_textExplanation[p] );
	}

	if( SHOW_SCROLL_BAR )
	{
		m_ScrollBar.SetName( "DualScrollBar", "ScrollBar" );
		m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT );
		m_ScrollBar.SetBarTime( SCROLL_BAR_TIME );
		FOREACH_PlayerNumber( p )
			m_ScrollBar.EnablePlayer( p, GAMESTATE->IsHumanPlayer(p) );
		m_ScrollBar.Load();
		SET_XY( m_ScrollBar );
		m_framePage.AddChild( &m_ScrollBar );
	}

	m_sprMore.Load( THEME->GetPathG( m_sName,"more") );
	m_sprMore->SetName( m_sName, "More" );
	SET_XY_AND_ON_COMMAND( m_sprMore );
	COMMAND( m_sprMore, m_bMoreShown? "ShowMore":"HideMore" );
	m_framePage.AddChild( m_sprMore );

	switch( m_InputMode )
	{
	case INPUTMODE_INDIVIDUAL:
		{
			FOREACH_PlayerNumber( p )
				m_textExplanation[p].SetXY( EXPLANATION_X.GetValue(p), EXPLANATION_Y.GetValue(p) );
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
		m_sprDisqualify[p].Load( THEME->GetPathG(m_sName,"disqualify") );
		m_sprDisqualify[p]->SetName( m_sName, ssprintf("DisqualifyP%i",p+1) );
		SET_XY_AND_ON_COMMAND( m_sprDisqualify[p] );
		m_sprDisqualify[p]->SetHidden( true );	// unhide later if handicapping options are discovered
		m_framePage.AddChild( m_sprDisqualify[p] );
	}

	// poke once at all the explanation metrics so that we catch missing ones early
	for( int r=0; r<(int)m_Rows.size(); r++ )		// foreach row
	{
		GetExplanationText( r );
		GetExplanationTitle( r );
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
			OnChange( p );
	}

	CHECKPOINT;

	/* It's tweening into position, but on the initial tween-in we only want to
	 * tween in the whole page at once.  Since the tweens are nontrivial, it's
	 * easiest to queue the tweens and then force them to finish. */
	for( int r=0; r<(int) m_Rows.size(); r++ )	// foreach options line
	{
		OptionRow &row = *m_Rows[r];
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
	if( m_Rows[iRow]->Type == OptionRow::ROW_EXIT )
		return "";

	CString sLineName = m_Rows[iRow]->m_RowDef.name;
	sLineName.Replace("\n-","");
	sLineName.Replace("\n","");
	sLineName.Replace(" ","");
	return THEME->GetMetric( "OptionExplanations", sLineName+"Help" );
}

CString ScreenOptions::GetExplanationTitle( int iRow ) const
{
	if( m_Rows[iRow]->Type == OptionRow::ROW_EXIT )
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
	OptionRow &row = *m_Rows[iRow];
	if( row.Type == OptionRow::ROW_EXIT )
		return *row.m_textItems[0];

	bool bOneChoice = row.m_RowDef.bOneChoiceForAllPlayers;
	int index = -1;
	switch( row.m_RowDef.layoutType )
	{
	case LAYOUT_SHOW_ONE_IN_ROW:
		index = bOneChoice ? 0 : pn;
		/* If only P2 is enabled, his selections will be in index 0. */
		if( row.m_textItems.size() == 1 )
			index = 0;
		break;
	case LAYOUT_SHOW_ALL_IN_ROW:
		index = iChoiceOnRow;
		break;
	default:
		ASSERT(0);
	}

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
		OptionRow &row = *m_Rows[i];
		if( row.Type == OptionRow::ROW_EXIT )
			continue;

		const float fY = ROW_Y.GetValue( i );

		BitmapText &title = row.m_textTitle;

		title.LoadFromFont( THEME->GetPathF(m_sName,"title") );

		const CString sText = GetExplanationTitle( i );
		title.SetText( sText );
		title.SetXY( LABELS_X, fY );
		title.RunCommands( LABELS_ON_COMMAND );

		Sprite &bullet = row.m_sprBullet;
		bullet.Load( THEME->GetPathG(m_sName,"bullet") );
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
		OptionRow &row = *m_Rows[r];
		if( row.Type == OptionRow::ROW_EXIT )
			continue;

		FOREACH_HumanPlayer( p )
		{
			vector<OptionsCursor*> &vpUnderlines = row.m_Underline[p];

			const int iNumUnderlines = (row.m_RowDef.layoutType == LAYOUT_SHOW_ONE_IN_ROW) ? 1 : vpUnderlines.size();
			
			for( int i=0; i<iNumUnderlines; i++ )
			{
				OptionsCursor& ul = *vpUnderlines[i];
	
				int iChoiceWithFocus = (row.m_RowDef.layoutType == LAYOUT_SHOW_ONE_IN_ROW) ? row.m_iChoiceInRowWithFocus[p] : i;

				/* Don't tween X movement and color changes. */
				int iWidth, iX, iY;
				GetWidthXY( p, r, iChoiceWithFocus, iWidth, iX, iY );
				ul.SetGlobalX( (float)iX );
				ul.SetGlobalDiffuseColor( RageColor(1,1,1, 1.0f) );

				bool bSelected = row.m_vbSelected[p][ iChoiceWithFocus ];
				bool bHidden = !bSelected || row.m_bHidden;
				if( !m_bShowUnderlines )
					bHidden = true;

				if( ul.GetDestY() != row.m_fY )
				{
					ul.StopTweening();
					ul.BeginTweening( 0.3f );
				}

				ul.SetHidden( bHidden );
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
			OptionRow &row = *m_Rows[i];
			if( row.Type == OptionRow::ROW_EXIT )
				continue;

			OptionIcon &icon = row.m_OptionIcons[p];

			int iChoiceWithFocus = row.m_iChoiceInRowWithFocus[p];

			int iWidth, iX, iY;			// We only use iY
			GetWidthXY( p, i, iChoiceWithFocus, iWidth, iX, iY );
			icon.SetX( ICONS_X.GetValue(p) );

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
		OptionRow &OptionRow = *m_Rows[iRow];

		OptionsCursor &highlight = m_Highlight[pn];

		const int iChoiceWithFocus = OptionRow.m_iChoiceInRowWithFocus[pn];

		int iWidth, iX, iY;
		GetWidthXY( pn, iRow, iChoiceWithFocus, iWidth, iX, iY );
		highlight.SetBarWidth( iWidth );
		highlight.SetXY( (float)iX, (float)iY );
	}
}

void ScreenOptions::TweenCursor( PlayerNumber pn )
{
	// Set the position of the highlight showing the current option the user is changing.
	const int iRow = m_iCurrentRow[pn];
	ASSERT_M( iRow < (int)m_Rows.size(), ssprintf("%i < %i", iRow, (int)m_Rows.size() ) );

	const OptionRow &OptionRow = *m_Rows[iRow];
	const int iChoiceWithFocus = OptionRow.m_iChoiceInRowWithFocus[pn];

	int iWidth, iX, iY;
	GetWidthXY( pn, iRow, iChoiceWithFocus, iWidth, iX, iY );

	OptionsCursor &highlight = m_Highlight[pn];
	highlight.StopTweening();
	highlight.BeginTweening( 0.2f );
	highlight.TweenBarWidth( iWidth );
	highlight.SetXY( (float)iX, (float)iY );

	if( GAMESTATE->IsHumanPlayer(pn) )  
	{
		COMMAND( m_sprLineHighlight[pn], "Change" );
		if( m_Rows[iRow]->Type == OptionRow::ROW_EXIT )
			COMMAND( m_sprLineHighlight[pn], "ChangeToExit" );
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
	OptionRow &row = *m_Rows[iRow];
	const OptionRowDefinition &data = row.m_RowDef;

	if( !row.m_RowDef.layoutType == LAYOUT_SHOW_ONE_IN_ROW )
		return;

	FOREACH_HumanPlayer( pn )
	{
		int iChoiceWithFocus = row.m_iChoiceInRowWithFocus[pn];
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
		OptionRow &row = *m_Rows[i];

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

		if( row.Type == OptionRow::ROW_EXIT )
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

void ScreenOptions::Input( const DeviceInput& DeviceI, const InputEventType selectType, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	/* Allow input when transitioning in (m_In.IsTransitioning()), but ignore it
	 * when we're transitioning out. */
	if( m_Back.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	if( selectType == IET_RELEASE )
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
	Screen::Input( DeviceI, selectType, GameI, MenuI, StyleI );
}

void ScreenOptions::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		StartGoToNextScreen();
		break;
	case SM_GoToPrevScreen:
//		this->ExportOptions();	// Don't save options if we're going back!
		this->GoToPrevScreen();
		break;
	case SM_GoToNextScreen:
		this->ExportOptions();
		this->GoToNextScreen();
		break;
	case SM_BeginFadingOut:
		if(IsTransitioning())
			return; /* already transitioning */
		StartTransitioning( SM_GoToNextScreen );

		SCREENMAN->PlayStartSound();

		m_framePage.RunCommands( FRAME_OFF_COMMAND );
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
	const int total = NUM_ROWS_SHOWN;
	const int halfsize = total / 2;

	int first_start, first_end, second_start, second_end;

	/* Choices for each player.  If only one player is active, it's the same for both. */
	int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? m_iCurrentRow[PLAYER_1]: m_iCurrentRow[PLAYER_2];
	int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? m_iCurrentRow[PLAYER_2]: m_iCurrentRow[PLAYER_1];

	vector<OptionRow*> Rows( m_Rows );
	OptionRow *ExitRow = NULL;

	if( (bool)SEPARATE_EXIT_ROW && Rows.back()->Type == OptionRow::ROW_EXIT )
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
		float fY;
		if( i < first_start )
			fY = ROW_Y_OFF_SCREEN_TOP;
		else if( i < first_end )
			fY = ROW_Y.GetValue( pos++ );
		else if( i < second_start )
			fY = ROW_Y_OFF_SCREEN_CENTER;
		else if( i < second_end )
			fY = ROW_Y.GetValue( pos++ );
		else
			fY = ROW_Y_OFF_SCREEN_BOTTOM;
			
		OptionRow &row = *Rows[i];

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
	const bool ShowMore = m_Rows.back()->Type == OptionRow::ROW_EXIT && m_Rows.back()->m_bHidden;
	if( m_bMoreShown != ShowMore )
	{
		m_bMoreShown = ShowMore;
		COMMAND( m_sprMore, m_bMoreShown? "ShowMore":"HideMore" );
	}

	/* Update all players, since changing one player can move both cursors. */
	FOREACH_PlayerNumber( p )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
			TweenCursor(  p );

		const bool ExitSelected = m_Rows[m_iCurrentRow[pn]]->Type == OptionRow::ROW_EXIT;
		if( p == pn || GAMESTATE->GetNumHumanPlayers() == 1 )
		{
			if( m_bWasOnExit[p] != ExitSelected )
			{
				m_bWasOnExit[p] = ExitSelected;
				COMMAND( m_sprMore, ssprintf("Exit%sP%i", ExitSelected? "Selected":"Unselected", p+1) );
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
			pText->RunCommands( EXPLANATION_ON_COMMAND.GetValue(pn) );
			pText->SetText( text );
		}
		break;
	case INPUTMODE_SHARE_CURSOR:
		pText = &m_textExplanation[0];
		if( pText->GetText() != text )
		{
			pText->FinishTweening();
			pText->RunCommands( EXPLANATION_TOGETHER_ON_COMMAND );
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

void ScreenOptions::StartGoToNextScreen()
{
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
}

bool ScreenOptions::AllAreOnExit() const
{
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsHumanPlayer(p)  &&  m_Rows[m_iCurrentRow[p]]->Type != OptionRow::ROW_EXIT )
			return false;
	return true;
}

void ScreenOptions::MenuStart( PlayerNumber pn, const InputEventType selectType )
{
	switch( selectType )
	{
	case IET_FIRST_PRESS:
		m_bGotAtLeastOneStartPressed[pn] = true;
		break;
	case IET_RELEASE:
		return;	// ignore
	default:	// repeat selectType
		if( !m_bGotAtLeastOneStartPressed[pn] )
			return;	// don't allow repeat
		break;
	}
	
	OptionRow &row = *m_Rows[m_iCurrentRow[pn]];
	OptionRowDefinition &data = row.m_RowDef;


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
				MoveRow( pn, -1, selectType != IET_FIRST_PRESS );		
				return;
			}
		}
	}


	// If on exit, check it all players are on "Exit"
	if( row.Type == OptionRow::ROW_EXIT )
	{
		/* Don't accept START to go to the next screen if we're still transitioning in. */
		if( AllAreOnExit()  &&  selectType == IET_FIRST_PRESS && !IsTransitioning() )
			StartGoToNextScreen();
		return;
	}


	if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY )
	{
		int iChoiceInRow = row.m_iChoiceInRowWithFocus[pn];
		if( iChoiceInRow == 0 )
		{
			MenuDown( pn, selectType );
			return;
		}
	}
	
	// If this is a bFirstChoiceGoesDown, then if this is a multiselect row.
	if( data.selectType == SELECT_MULTIPLE )
	{
		int iChoiceInRow = row.m_iChoiceInRowWithFocus[pn];
		row.m_vbSelected[pn][iChoiceInRow] = !row.m_vbSelected[pn][iChoiceInRow];
		if( row.m_vbSelected[pn][iChoiceInRow] )
			m_SoundToggleOn.Play();
		else
			m_SoundToggleOff.Play();
		PositionUnderlines();
		RefreshIcons();

		if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY )
		{
			// move to the first choice in the row
			ChangeValueInRow( pn, -row.m_iChoiceInRowWithFocus[pn], selectType != IET_FIRST_PRESS );
		}
	}
	else	// data.selectType != SELECT_MULTIPLE
	{
		switch( m_OptionsNavigation )
		{
		case NAV_THREE_KEY:
			MenuDown( pn, selectType );
			break;
		case NAV_TOGGLE_THREE_KEY:
		case NAV_TOGGLE_FIVE_KEY:
			if( data.selectType != SELECT_MULTIPLE )
			{
				int iChoiceInRow = row.m_iChoiceInRowWithFocus[pn];
				if( row.m_RowDef.bOneChoiceForAllPlayers )
					row.SetOneSharedSelection( iChoiceInRow );
				else
					row.SetOneSelection( pn, iChoiceInRow );
			}
			if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY )
				ChangeValueInRow( pn, -row.m_iChoiceInRowWithFocus[pn], selectType != IET_FIRST_PRESS );	// move to the first choice
			else
				ChangeValueInRow( pn, 0, selectType != IET_FIRST_PRESS );
			break;
		case NAV_THREE_KEY_MENU:
			/* Don't accept START to go to the next screen if we're still transitioning in. */
			if( selectType == IET_FIRST_PRESS && !IsTransitioning() )
				StartGoToNextScreen();
			break;
		case NAV_FIVE_KEY:
			/* Jump to the exit row.  (If everyone's already on the exit row, then
			 * we'll have already gone to the next screen above.) */
			MoveRow( pn, m_Rows.size()-m_iCurrentRow[pn]-1, selectType != IET_FIRST_PRESS );
			break;
		}
	}
}

void ScreenOptions::StoreFocus( PlayerNumber pn )
{
	/* Long rows always put us in the center, so don't update the focus. */
	const OptionRow &OptionRow = *m_Rows[m_iCurrentRow[pn]];
	if( OptionRow.m_RowDef.layoutType == LAYOUT_SHOW_ONE_IN_ROW )
		return;

	int iWidth, iY;
	GetWidthXY( pn, m_iCurrentRow[pn], m_Rows[m_iCurrentRow[pn]]->m_iChoiceInRowWithFocus[pn], iWidth, m_iFocusX[pn], iY );
	LOG->Trace("cur selection %ix%i @ %i", m_iCurrentRow[pn], m_Rows[m_iCurrentRow[pn]]->m_iChoiceInRowWithFocus[pn],
		m_iFocusX[pn]);
}

/* Left/right */
void ScreenOptions::ChangeValueInRow( PlayerNumber pn, int iDelta, bool Repeat )
{
	const int iCurRow = m_iCurrentRow[pn];
	OptionRow &row = *m_Rows[iCurRow];
	OptionRowDefinition &optrow = m_Rows[iCurRow]->m_RowDef;

	const int iNumOptions = (row.Type == OptionRow::ROW_EXIT)? 1: optrow.choices.size();
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

	if( row.Type == OptionRow::ROW_EXIT	)	// EXIT is selected
		return;		// don't allow a move

	bool bOneChanged = false;


	int iCurrentChoiceWithFocus = row.m_iChoiceInRowWithFocus[pn];
	int iNewChoiceWithFocus = iCurrentChoiceWithFocus + iDelta;
	wrap( iNewChoiceWithFocus, iNumOptions );
	
	if( iCurrentChoiceWithFocus != iNewChoiceWithFocus )
		bOneChanged = true;

	row.m_iChoiceInRowWithFocus[pn] = iNewChoiceWithFocus;
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
				row.m_iChoiceInRowWithFocus[p] = iNewChoiceWithFocus;
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
				if( optrow.selectType == SELECT_MULTIPLE )
					;	// do nothing.  User must press Start to toggle the selection.
				else
					row.SetOneSelection( p, iNewChoiceWithFocus );			
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
			if( optrow.selectType == SELECT_MULTIPLE )
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
		OptionRow &row = *m_Rows[iCurrentRow];
		row.m_iChoiceInRowWithFocus[pn] = 0;
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

		const unsigned iOldSelection = m_Rows[m_iCurrentRow[p]]->m_iChoiceInRowWithFocus[p];

		m_iCurrentRow[p] = row;

		switch( m_OptionsNavigation )
		{
		case NAV_TOGGLE_THREE_KEY:
		case NAV_TOGGLE_FIVE_KEY:
			if( m_Rows[row]->m_RowDef.layoutType != LAYOUT_SHOW_ONE_IN_ROW )
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
						m_Rows[row]->m_iChoiceInRowWithFocus[p] = i;
					}
				}
				m_Rows[row]->m_iChoiceInRowWithFocus[p] = min( iOldSelection, m_Rows[row]->m_textItems.size()-1 );
			}
		}

		OnChange( p );
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
	if( m_Rows[l]->Type != OptionRow::ROW_NORMAL )
		return -1;
	return l;
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
