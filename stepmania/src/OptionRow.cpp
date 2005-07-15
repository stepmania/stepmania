#include "global.h"
#include "OptionRow.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "Foreach.h"
#include "OptionRowHandler.h"
#include "CommonMetrics.h"
#include "GameState.h"
#include "song.h"
#include "Course.h"
#include "Style.h"

static const CString SelectTypeNames[] = {
	"SelectOne",
	"SelectMultiple",
	"SelectNone",
};
XToString( SelectType, NUM_SELECT_TYPES );
StringToX( SelectType );

static const CString LayoutTypeNames[] = {
	"ShowAllInRow",
	"ShowOneInRow",
};
XToString( LayoutType, NUM_LAYOUT_TYPES );
StringToX( LayoutType );

const CString NEXT_ROW_NAME = "NextRow";
const CString EXIT_NAME = "Exit";

void OptionRow::PrepareItemText( CString &s ) const
{
	if( s == "" )
		return;
	bool bTheme = false;
	
	// HACK: Always theme the NEXT_ROW and EXIT items, even if metrics says not to theme.
	if( s == NEXT_ROW_NAME )							bTheme = true;
	if( s == EXIT_NAME )								bTheme = true;
	if( THEME_ITEMS && m_RowDef.m_bAllowThemeItems )	bTheme = true;

	if( bTheme ) 
		s = THEME_OPTION_ITEM( s, false ); 
	if( CAPITALIZE_ALL_OPTION_NAMES )
		s.MakeUpper(); 
}

CString OptionRow::OptionTitle( CString s ) const
{
	bool bTheme = false;
	
	// HACK: Always theme the NEXT_ROW and EXIT items, even if metrics says not to theme.
	if( THEME_TITLES && m_RowDef.m_bAllowThemeTitles )	bTheme = true;

	return bTheme ? THEME->GetMetric("OptionTitles",s) : s;
}

CString ITEMS_LONG_ROW_X_NAME( size_t p )		{ return ssprintf("ItemsLongRowP%dX",int(p+1)); }
CString ICONS_X_NAME( size_t p )				{ return ssprintf("IconsP%dX",int(p+1)); }

OptionRow::OptionRow()
{
	m_pHand = NULL;

	Clear();

	FOREACH_PlayerNumber( p )
		m_Frame.AddChild( &m_OptionIcons[p] );
	m_Frame.AddChild( &m_sprBullet );
	m_Frame.AddChild( &m_textTitle );
	m_Frame.AddChild( &m_ItemFrame );

	this->AddChild( &m_Frame );
}

OptionRow::~OptionRow()
{
	Clear();
}

void OptionRow::Clear()
{
	ActorFrame::RemoveAllChildren();

	FOREACH_PlayerNumber( p )
		m_vbSelected[p].clear();

	m_ItemFrame.DeleteAllChildren();
	m_textItems.clear();
	FOREACH_PlayerNumber( p )
		m_Underline[p].clear();

	ASSERT( m_pHand == NULL );

	m_bFirstItemGoesDown = false;
	ZERO( m_bRowHasFocus );
	ZERO( m_iChoiceInRowWithFocus );
}

void OptionRow::DetachHandler()
{
	if( m_pHand )
	{
		FOREACH_CONST( CString, m_pHand->m_vsReloadRowMessages, m )
			MESSAGEMAN->Unsubscribe( this, *m );
	}
	m_pHand = NULL;
}

void OptionRow::LoadMetrics( const CString &sType )
{
	m_sType = sType;
	ARROWS_X   						.Load(m_sType,"ArrowsX");
	LABELS_X						.Load(m_sType,"LabelsX");
	LABELS_ON_COMMAND				.Load(m_sType,"LabelsOnCommand");
	LABEL_GAIN_FOCUS_COMMAND		.Load(m_sType,"LabelGainFocusCommand");
	LABEL_LOSE_FOCUS_COMMAND		.Load(m_sType,"LabelLoseFocusCommand");
	ITEMS_START_X					.Load(m_sType,"ItemsStartX");
	ITEMS_END_X						.Load(m_sType,"ItemsEndX");
	ITEMS_GAP_X						.Load(m_sType,"ItemsGapX");
	ITEMS_LONG_ROW_X				.Load(m_sType,ITEMS_LONG_ROW_X_NAME,NUM_PLAYERS);
	ITEMS_LONG_ROW_SHARED_X			.Load(m_sType,"ItemsLongRowSharedX");
	ITEMS_ON_COMMAND				.Load(m_sType,"ItemsOnCommand");
	ITEM_GAIN_FOCUS_COMMAND			.Load(m_sType,"ItemGainFocusCommand");
	ITEM_LOSE_FOCUS_COMMAND			.Load(m_sType,"ItemLoseFocusCommand");
	ICONS_X							.Load(m_sType,ICONS_X_NAME,NUM_PLAYERS);
	ICONS_ON_COMMAND				.Load(m_sType,"IconsOnCommand");
	COLOR_SELECTED					.Load(m_sType,"ColorSelected");
	COLOR_NOT_SELECTED				.Load(m_sType,"ColorNotSelected");
	COLOR_DISABLED					.Load(m_sType,"ColorDisabled");
	CAPITALIZE_ALL_OPTION_NAMES		.Load(m_sType,"CapitalizeAllOptionNames");
	TWEEN_SECONDS					.Load(m_sType,"TweenSeconds");
	THEME_ITEMS						.Load(m_sType,"ThemeItems");
	THEME_TITLES					.Load(m_sType,"ThemeTitles");
	SHOW_BPM_IN_SPEED_TITLE			.Load(m_sType,"ShowBpmInSpeedTitle");

	FOREACH_PlayerNumber( p )
	{
		m_OptionIcons[p].Load( m_sType );
		m_OptionIcons[p].RunCommands( ICONS_ON_COMMAND );
	}

	m_textItemParent.LoadFromFont( THEME->GetPathF(m_sType,"item") );
	m_UnderlineParent.Load( m_sType, OptionsCursor::underline );
	m_textTitle.LoadFromFont( THEME->GetPathF(m_sType,"title") );
	m_sprBullet.Load( THEME->GetPathG(m_sType,"bullet") );
}

void OptionRow::LoadNormal( const OptionRowDefinition &def, OptionRowHandler *pHand, bool bFirstItemGoesDown )
{
	m_RowDef = def;
	m_RowType = OptionRow::ROW_NORMAL;
	m_pHand = pHand;
	m_bFirstItemGoesDown = bFirstItemGoesDown;

	if( m_pHand )
	{
		FOREACH_CONST( CString, m_pHand->m_vsReloadRowMessages, m )
			MESSAGEMAN->Subscribe( this, *m );
	}

	FOREACH_PlayerNumber( p )
	{
		vector<bool> &vbSelected = m_vbSelected[p];
		vbSelected.resize( 0 );
		vbSelected.resize( m_RowDef.m_vsChoices.size(), false );
		
		// set select the first item if a SELECT_ONE row
		if( vbSelected.size() && m_RowDef.m_selectType == SELECT_ONE )
			vbSelected[0] = true;
	}

	// TRICKY:  Insert a down arrow as the first choice in the row.
	if( m_bFirstItemGoesDown )
	{
		m_RowDef.m_vsChoices.insert( m_RowDef.m_vsChoices.begin(), NEXT_ROW_NAME );
		FOREACH_PlayerNumber( p )
			m_vbSelected[p].insert( m_vbSelected[p].begin(), false );
	}
}

CString OptionRow::GetRowTitle() const
{
	CString sLineName = m_RowDef.m_sName;
	CString sTitle = OptionTitle(sLineName);

	// HACK: tack the BPM onto the name of the speed line
	if( sLineName.CompareNoCase("speed")==0 )
	{
		bool bShowBpmInSpeedTitle = SHOW_BPM_IN_SPEED_TITLE;

		if( GAMESTATE->m_pCurCourse )
		{
			Trail* pTrail = GAMESTATE->m_pCurTrail[GAMESTATE->m_MasterPlayerNumber];
			ASSERT( pTrail != NULL );
			const int iNumCourseEntries = pTrail->m_vEntries.size();
			if( iNumCourseEntries > MAX_COURSE_ENTRIES_BEFORE_VARIOUS )
				bShowBpmInSpeedTitle = false;
		}

		if( bShowBpmInSpeedTitle )
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

			if( bpms.IsSecret() )
				sTitle += ssprintf( " (??" "?)" ); /* split so gcc doesn't think this is a trigraph */
			else if( bpms.BpmIsConstant() )
				sTitle += ssprintf( " (%.0f)", bpms.GetMin() );
			else
				sTitle += ssprintf( " (%.0f-%.0f)", bpms.GetMin(), bpms.GetMax() );
		}
	}

	return sTitle;
}

/* Set up text, underlines and titles for options.  This can be called
 * as soon as m_RowDef is available. */
void OptionRow::InitText()
{
	/* If we have elements already, we're being updated from a new set of options.
	 * Delete the old ones. */
	m_ItemFrame.DeleteAllChildren();
	m_textItems.clear();
	FOREACH_PlayerNumber( p )
		m_Underline[p].clear();

	// If the items will go off the edge of the screen, then re-init with the "long row" style.
	{
		BitmapText bt( m_textItemParent );
		bt.RunCommands( ITEMS_ON_COMMAND );

		float fX = ITEMS_START_X;
		
		for( unsigned c=0; c<m_RowDef.m_vsChoices.size(); c++ )
		{
			CString sText = m_RowDef.m_vsChoices[c];
			PrepareItemText( sText );
			bt.SetText( sText );
			
			fX += bt.GetZoomedWidth();
			
			if( c != m_RowDef.m_vsChoices.size()-1 )
				fX += ITEMS_GAP_X;

			if( fX > ITEMS_END_X ) 
			{
				m_RowDef.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
				break;
			}
		}
	}

	//
	// load m_textItems
	//
	switch( m_RowDef.m_layoutType )
	{
	case LAYOUT_SHOW_ONE_IN_ROW:
		// init text
		FOREACH_HumanPlayer( p )
		{
			BitmapText *bt = new BitmapText( m_textItemParent );
			m_textItems.push_back( bt );

			bt->RunCommands( ITEMS_ON_COMMAND );
			bt->SetShadowLength( 0 );

			if( m_RowDef.m_bOneChoiceForAllPlayers )
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
			OptionsCursor *ul = new OptionsCursor( m_UnderlineParent );
			m_Underline[p].push_back( ul );

			ul->Set( p );
			int iWidth, iX, iY;
			GetWidthXY( p, 0, iWidth, iX, iY );
			ul->SetX( float(iX) );
			ul->SetWidth( float(iWidth) );
		}
		break;

	case LAYOUT_SHOW_ALL_IN_ROW:
		{
			float fX = ITEMS_START_X;
			for( unsigned c=0; c<m_RowDef.m_vsChoices.size(); c++ )
			{
				// init text
				BitmapText *bt = new BitmapText( m_textItemParent );
				m_textItems.push_back( bt );
				CString sText = m_RowDef.m_vsChoices[c];
				PrepareItemText( sText );
				bt->SetText( sText );
				bt->RunCommands( ITEMS_ON_COMMAND );
				bt->SetShadowLength( 0 );

				// set the X position of each item in the line
				float fItemWidth = bt->GetZoomedWidth();
				fX += fItemWidth/2;
				bt->SetX( fX );

				// init underlines
				FOREACH_HumanPlayer( p )
				{
					OptionsCursor *ul = new OptionsCursor( m_UnderlineParent );
					m_Underline[p].push_back( ul );
					ul->Set( p );
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

	for( unsigned c=0; c<m_textItems.size(); c++ )
		m_ItemFrame.AddChild( m_textItems[c] );
	FOREACH_PlayerNumber( p )
		for( unsigned c=0; c<m_Underline[p].size(); c++ )
			m_ItemFrame.AddChild( m_Underline[p][c] );

	CString sTitle = GetRowTitle();
	m_textTitle.SetText( sTitle );
	m_textTitle.SetX( LABELS_X );
	m_textTitle.RunCommands( LABELS_ON_COMMAND );

	m_sprBullet.SetX( ARROWS_X );
}

/* After importing options, choose which item is focused. */
void OptionRow::AfterImportOptions()
{
	// Make all selections the same if bOneChoiceForAllPlayers
	// Hack: we only import active players, so if only player 2 is imported,
	// we need to copy p2 to p1, not p1 to p2.
	if( m_RowDef.m_bOneChoiceForAllPlayers )
	{
		PlayerNumber pnCopyFrom = GAMESTATE->m_MasterPlayerNumber;
		if( GAMESTATE->m_MasterPlayerNumber == PLAYER_INVALID )
			pnCopyFrom = PLAYER_1;
		FOREACH_PlayerNumber( p )
			m_vbSelected[p] = m_vbSelected[pnCopyFrom];
	}

	FOREACH_PlayerNumber( p )
	{
		switch( m_RowDef.m_selectType )
		{
		case SELECT_ONE:
			{
				/* Make sure the row actually has a selection. */
				bool bHasASelection = false;
				for( unsigned i=0; i<m_vbSelected[p].size(); i++ )
				{
					if( m_vbSelected[p][i] )
						bHasASelection = true;
				}

				if( !bHasASelection && !m_vbSelected[p].empty() )
					m_vbSelected[p][0] = true;
				
				m_iChoiceInRowWithFocus[p] = GetOneSelection(p, true);	// focus on the selection we just set
			}
			break;
		case SELECT_MULTIPLE:
		case SELECT_NONE:
			m_iChoiceInRowWithFocus[p] = 0;
			break;
		default:
			ASSERT(0);
		}
	}

	// init row icons
	FOREACH_HumanPlayer( p )
		SetOptionIcon( p, "" );

	//
	// HACK: Set focus to one item in the row, which is "go down"
	//
	if( m_bFirstItemGoesDown )
		FOREACH_PlayerNumber( p )
			m_iChoiceInRowWithFocus[p] = 0;	

	UpdateText();
}

void OptionRow::LoadExit()
{
	m_RowType = OptionRow::ROW_EXIT;
	m_RowDef.m_sName = EXIT_NAME;
	m_RowDef.m_vsChoices.push_back( "" );

	BitmapText *bt = new BitmapText;
	m_textItems.push_back( bt );

	bt->LoadFromFont( THEME->GetPathF(m_sType,"item") );
	CString sText = "Exit";
	PrepareItemText( sText );
	bt->SetText( sText );
	bt->RunCommands( ITEMS_ON_COMMAND );
	bt->SetShadowLength( 0 );
	bt->SetX( ITEMS_LONG_ROW_SHARED_X );
	m_Frame.AddChild( bt );

	FOREACH_PlayerNumber( p )
		m_OptionIcons[p].SetHidden( true );
	m_sprBullet.SetHidden( true );
	m_textTitle.SetHidden( true );
}

void OptionRow::PositionUnderlines( PlayerNumber pn )
{
	if( m_RowType == ROW_EXIT )
		return;

	vector<OptionsCursor*> &vpUnderlines = m_Underline[pn];

	PlayerNumber pnTakeSelectedFrom = m_RowDef.m_bOneChoiceForAllPlayers ? PLAYER_1 : pn;

	const int iNumUnderlines = (m_RowDef.m_layoutType == LAYOUT_SHOW_ONE_IN_ROW) ? 1 : vpUnderlines.size();
	
	for( int i=0; i<iNumUnderlines; i++ )
	{
		OptionsCursor& ul = *vpUnderlines[i];

		int iChoiceWithFocus = (m_RowDef.m_layoutType == LAYOUT_SHOW_ONE_IN_ROW) ? m_iChoiceInRowWithFocus[pnTakeSelectedFrom] : i;

		/* Don't tween X movement and color changes. */
		int iWidth, iX, iY;
		GetWidthXY( pn, iChoiceWithFocus, iWidth, iX, iY );
		ul.SetGlobalX( (float)iX );
		ul.SetGlobalDiffuseColor( RageColor(1,1,1, 1.0f) );

		ASSERT( m_vbSelected[pnTakeSelectedFrom].size() == m_RowDef.m_vsChoices.size() );

		bool bSelected = (iChoiceWithFocus==-1) ? false : m_vbSelected[pnTakeSelectedFrom][ iChoiceWithFocus ];
		bool bHidden = !bSelected || m_bHidden;

		ul.StopTweening();
		ul.BeginTweening( TWEEN_SECONDS );
		ul.SetHidden( bHidden );
		ul.SetBarWidth( iWidth );
	}
}

void OptionRow::PositionIcons()
{
	if( m_RowType == OptionRow::ROW_EXIT )
		return;

	FOREACH_HumanPlayer( p )	// foreach player
	{
		OptionIcon &icon = m_OptionIcons[p];

		icon.SetX( ICONS_X.GetValue(p) );

		/* XXX: this doesn't work since icon is an ActorFrame */
		icon.SetDiffuse( RageColor(1,1,1, m_bHidden? 0.0f:1.0f) );
	}
}

void OptionRow::UpdateText()
{
	switch( m_RowDef.m_layoutType )
	{
	case LAYOUT_SHOW_ONE_IN_ROW:
		FOREACH_HumanPlayer( p )
		{
			unsigned pn = m_RowDef.m_bOneChoiceForAllPlayers ? 0 : p;
			int iChoiceWithFocus = m_iChoiceInRowWithFocus[pn];
			if( iChoiceWithFocus == -1 )
				continue;

			CString sText = m_RowDef.m_vsChoices[iChoiceWithFocus];
			PrepareItemText( sText );

			// If player_no is 2 and there is no player 1:
			int index = min( pn, m_textItems.size()-1 );

			// TODO: Always have one textItem for each player

			m_textItems[index]->SetText( sText );
		}
		break;
	}
}

void OptionRow::SetRowFocus( bool bRowHasFocus[NUM_PLAYERS] )
{
	FOREACH_PlayerNumber( p )
		m_bRowHasFocus[p] = bRowHasFocus[p];
}

void OptionRow::UpdateEnabledDisabled()
{
	bool bThisRowHasFocusByAny = false;
	FOREACH_HumanPlayer( p )
		bThisRowHasFocusByAny |= m_bRowHasFocus[p];

	bool bThisRowHasFocusByAll = true;
	FOREACH_HumanPlayer( p )
		bThisRowHasFocusByAll &= m_bRowHasFocus[p];
	
	bool bRowEnabled = !m_RowDef.m_vEnabledForPlayers.empty();
	if( m_Frame.GetDestY() != m_fY )
	{
		m_Frame.StopTweening();
		m_Frame.BeginTweening( TWEEN_SECONDS );
		m_Frame.SetY( m_fY );
	}

	if( bThisRowHasFocusByAny )
		m_textTitle.RunCommands( LABEL_GAIN_FOCUS_COMMAND );
	else
		m_textTitle.RunCommands( LABEL_LOSE_FOCUS_COMMAND );

	/* Don't tween selection colors at all. */
	RageColor color;
	if( bThisRowHasFocusByAny )	color = COLOR_SELECTED;
	else if( bRowEnabled )		color = COLOR_NOT_SELECTED;
	else						color = COLOR_DISABLED;

	if( m_bHidden )
		color.a = 0;

	m_sprBullet.SetGlobalDiffuseColor( color );
	m_textTitle.SetGlobalDiffuseColor( color );


	for( unsigned j=0; j<m_textItems.size(); j++ )
	{
		bool bThisItemHasFocusByAny = false;
		FOREACH_HumanPlayer( p )
		{
			if( m_bRowHasFocus[p] )
			{
				if( (int)j == GetChoiceInRowWithFocus(p) )
				{
					bThisItemHasFocusByAny = true;
					break;
				}
			}
		}

		if( bThisItemHasFocusByAny )
			m_textItems[j]->RunCommands( ITEM_GAIN_FOCUS_COMMAND );
		else
			m_textItems[j]->RunCommands( ITEM_LOSE_FOCUS_COMMAND );
	}

	switch( m_RowDef.m_layoutType )
	{
	case LAYOUT_SHOW_ALL_IN_ROW:
		for( unsigned j=0; j<m_textItems.size(); j++ )
		{
			if( m_textItems[j]->DestTweenState().diffuse[0] == color ) 	 
				continue;

			m_textItems[j]->StopTweening();
			m_textItems[j]->BeginTweening( TWEEN_SECONDS );
			m_textItems[j]->SetDiffuse( color );
		}
		break;
	case LAYOUT_SHOW_ONE_IN_ROW:
		FOREACH_HumanPlayer( pn )
		{
			bool bRowEnabled = m_RowDef.m_vEnabledForPlayers.find(pn) != m_RowDef.m_vEnabledForPlayers.end();
			
			if( m_bRowHasFocus[pn] )	color = COLOR_SELECTED;
			else if( bRowEnabled )		color = COLOR_NOT_SELECTED;
			else						color = COLOR_DISABLED;

			if( m_bHidden )
				color.a = 0;

			unsigned item_no = m_RowDef.m_bOneChoiceForAllPlayers ? 0 : pn;

			// If player_no is 2 and there is no player 1:
			item_no = min( item_no, m_textItems.size()-1 );

			BitmapText &bt = *m_textItems[item_no];

			if( bt.DestTweenState().diffuse[0] != color )
			{
				bt.StopTweening();
				bt.BeginTweening( TWEEN_SECONDS );
				bt.SetDiffuse( color );

				OptionsCursor &ul = *m_Underline[pn][0];
				ul.StopTweening();
				ul.BeginTweening( TWEEN_SECONDS );
				ul.SetDiffuseAlpha( color.a );
			}
		}
		break;
	default:
		ASSERT(0);
	}

	if( m_RowType == OptionRow::ROW_EXIT )
	{
		if( bThisRowHasFocusByAll )
			m_textItems[0]->SetEffectDiffuseShift( 1.0f, COLOR_SELECTED, COLOR_NOT_SELECTED );
		else
			m_textItems[0]->SetEffectNone();
	}

	if( m_sprBullet.DestTweenState().diffuse[0] != color )
	{
		m_sprBullet.StopTweening();
		m_textTitle.StopTweening();
		m_sprBullet.BeginTweening( TWEEN_SECONDS );
		m_textTitle.BeginTweening( TWEEN_SECONDS );
		m_sprBullet.SetDiffuseAlpha( color.a );
		m_textTitle.SetDiffuseAlpha( color.a );
	}
}

void OptionRow::SetOptionIcon( PlayerNumber pn, const CString &sText )
{
	m_OptionIcons[pn].Set( pn, sText, false );
}

BitmapText &OptionRow::GetTextItemForRow( PlayerNumber pn, int iChoiceOnRow )
{
	if( m_RowType == OptionRow::ROW_EXIT )
		return *m_textItems[0];

	bool bOneChoice = m_RowDef.m_bOneChoiceForAllPlayers;
	int index = -1;
	switch( m_RowDef.m_layoutType )
	{
	case LAYOUT_SHOW_ONE_IN_ROW:
		index = bOneChoice ? 0 : pn;
		/* If only P2 is enabled, his selections will be in index 0. */
		if( m_textItems.size() == 1 )
			index = 0;
		break;
	case LAYOUT_SHOW_ALL_IN_ROW:
		index = iChoiceOnRow;
		break;
	default:
		ASSERT(0);
	}

	ASSERT_M( index < (int)m_textItems.size(), ssprintf("%i < %i", index, (int)m_textItems.size() ) );
	return *m_textItems[index];
}

void OptionRow::GetWidthXY( PlayerNumber pn, int iChoiceOnRow, int &iWidthOut, int &iXOut, int &iYOut )
{
	BitmapText &text = GetTextItemForRow( pn, iChoiceOnRow );

	iWidthOut = int(roundf( text.GetZoomedWidth() ));
	iXOut = int(roundf( text.GetDestX() ));
	/* We update m_fY, change colors and tween items, and then tween rows to
	* their final positions.  (This is so we don't tween colors, too.)  m_fY
	* is the actual destination position, even though we may not have set up the
	* tween yet. */
	iYOut = int(roundf( m_fY ));
}


int OptionRow::GetOneSelection( PlayerNumber pn, bool bAllowFail ) const
{
	for( unsigned i=0; i<m_vbSelected[pn].size(); i++ )
		if( m_vbSelected[pn][i] )
			return i;

	ASSERT( bAllowFail );	// shouldn't call this if not expecting one to be selected
	return -1;
}

int OptionRow::GetOneSharedSelection( bool bAllowFail ) const
{
	return GetOneSelection( (PlayerNumber)0, bAllowFail );
}

void OptionRow::SetOneSelection( PlayerNumber pn, int iChoice )
{
	if( m_vbSelected[pn].empty() )
		return;
	for( unsigned i=0; i<(unsigned)m_vbSelected[pn].size(); i++ )
		m_vbSelected[pn][i] = false;
	m_vbSelected[pn][iChoice] = true;
}

void OptionRow::SetOneSharedSelection( int iChoice )
{
	FOREACH_PlayerNumber( pn )
		SetOneSelection( pn, iChoice );
}

void OptionRow::SetOneSharedSelectionIfPresent( const CString &sChoice )
{
	for( unsigned i=0; i<m_RowDef.m_vsChoices.size(); i++ )
	{
		if( sChoice == m_RowDef.m_vsChoices[i] )
		{
			SetOneSharedSelection( i );
			break;
		}
	}
}

int OptionRow::GetChoiceInRowWithFocus( PlayerNumber pn ) const
{
	if( m_RowDef.m_bOneChoiceForAllPlayers )
		pn = PLAYER_1;
	if( m_RowDef.m_vsChoices.empty() )
		return -1;
	int iChoice = m_iChoiceInRowWithFocus[pn];
	return iChoice; 
}

int OptionRow::GetChoiceInRowWithFocusShared() const
{
	return GetChoiceInRowWithFocus( (PlayerNumber)0 );
}

void OptionRow::SetChoiceInRowWithFocus( PlayerNumber pn, int iChoice )
{
	if( m_RowDef.m_bOneChoiceForAllPlayers )
		pn = PLAYER_1;
	ASSERT(iChoice >= 0 && iChoice < (int)m_RowDef.m_vsChoices.size());
	m_iChoiceInRowWithFocus[pn] = iChoice;
}

void OptionRow::SetChoiceInRowWithFocusShared( int iChoice )
{
	FOREACH_PlayerNumber( pn )
		SetChoiceInRowWithFocus( pn, iChoice );
}


void OptionRow::SetExitText( CString sExitText )
{
	BitmapText *bt = m_textItems.back();
	bt->SetText( sExitText );
}

void OptionRow::Reload( const OptionRowDefinition &def )
{
	switch( GetRowType() )
	{
	case OptionRow::ROW_NORMAL:
		{
			vector<PlayerNumber> vpns;
			FOREACH_HumanPlayer( p )
				vpns.push_back( p );

			// TODO: Nothing uses this yet and it causes skips when changing options.
			//if( m_RowDef.m_bExportOnChange )
			//{
			//	bool bRowHasFocus[NUM_PLAYERS];
			//	ZERO( bRowHasFocus );
			//	ExportOptions( vpns, bRowHasFocus );
			//}

			if( m_pHand == NULL )
				m_RowDef = def;
			else
				m_pHand->Reload( m_RowDef );
			ASSERT( !m_RowDef.m_vsChoices.empty() );

			FOREACH_PlayerNumber( p )
				m_vbSelected[p].resize( m_RowDef.m_vsChoices.size(), false );

			// TODO: Nothing uses this yet and it causes skips when changing options.
			//ImportOptions( vpns );

			switch( m_RowDef.m_selectType )
			{
			case SELECT_ONE:
				FOREACH_HumanPlayer( p )
				{
					m_iChoiceInRowWithFocus[p] = GetOneSelection(p, true);
					if( m_iChoiceInRowWithFocus[p] == -1 )
						m_iChoiceInRowWithFocus[p] = 0;
				}
				break;
			case SELECT_MULTIPLE:
				FOREACH_HumanPlayer( p )
					CLAMP( m_iChoiceInRowWithFocus[p], 0, m_RowDef.m_vsChoices.size()-1 );
				break;
			default:
				ASSERT(0);
			}

			// TODO: Nothing uses this yet and it causes skips when changing options.
			//if( m_RowDef.m_bExportOnChange )
			//{
			//	bool bRowHasFocus[NUM_PLAYERS];
			//	ZERO( bRowHasFocus );
			//	ExportOptions( vpns, bRowHasFocus );
			//}

			UpdateEnabledDisabled();

			/* Update the text to show the options we just updated. */
			InitText();

			FOREACH_HumanPlayer( p )
				PositionUnderlines( p );
		}
		break;
	case OptionRow::ROW_EXIT:
		// nothing to do
		break;
	default:
		ASSERT(0);
	}
}

void OptionRow::SetEnabledRowForAllPlayers( bool bEnabledForAllPlayers )
{
	OptionRowDefinition def = m_RowDef;
	if( bEnabledForAllPlayers )
	{
		OptionRowDefinition temp; 
		def.m_vEnabledForPlayers = temp.m_vEnabledForPlayers;
	}
	else
	{
		def.m_vEnabledForPlayers.clear();
	}
	Reload( def );
}

void OptionRow::HandleMessage( const CString& sMessage )
{
	Reload( m_RowDef );
}


/* Hack: the NextRow entry is never set, and should be transparent.  Remove
 * it, and readd it below. */
#define ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( vbSelected ) \
	if( GetFirstItemGoesDown() ) \
		vbSelected.erase( vbSelected.begin() );
#define INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( vbSelected ) \
	if( GetFirstItemGoesDown() ) \
		vbSelected.insert( vbSelected.begin(), false );

void OptionRow::ImportOptions( const vector<PlayerNumber> &vpns )
{
	if( m_pHand == NULL )
		return;

	ASSERT( m_RowDef.m_vsChoices.size() > 0 );

	FOREACH_CONST( PlayerNumber, vpns, iter )
	{
		PlayerNumber p = *iter;

		FOREACH( bool, m_vbSelected[p], b )
			*b = false;

		ASSERT( m_vbSelected[p].size() == m_RowDef.m_vsChoices.size() );
		ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[p] );
	}
	
	m_pHand->ImportOption( m_RowDef, vpns, m_vbSelected );

	FOREACH_CONST( PlayerNumber, vpns, iter )
	{
		PlayerNumber p = *iter;

		INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[p] );
		VerifySelected( m_RowDef.m_selectType, m_vbSelected[p], m_RowDef.m_sName );
	}
}

int OptionRow::ExportOptions( const vector<PlayerNumber> &vpns, bool bRowHasFocus[NUM_PLAYERS] )
{
	if( m_pHand == NULL )
		return 0;

	ASSERT( m_RowDef.m_vsChoices.size() > 0 );

	int iChangeMask = 0;
	
	FOREACH_CONST( PlayerNumber, vpns, iter )
	{
		PlayerNumber p = *iter;
		bool bFocus = bRowHasFocus[p];

		VerifySelected( m_RowDef.m_selectType, m_vbSelected[p], m_RowDef.m_sName );
		ASSERT( m_vbSelected[p].size() == m_RowDef.m_vsChoices.size() );
		ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[p] );

		// SELECT_NONE rows get exported if they have focus when the user presses 
		// Start.
		int iChoice = GetChoiceInRowWithFocus( p );
		if( m_RowDef.m_selectType == SELECT_NONE && bFocus )
			m_vbSelected[p][iChoice] = true;
	}

	iChangeMask |= m_pHand->ExportOption( m_RowDef, vpns, m_vbSelected );
	
	FOREACH_CONST( PlayerNumber, vpns, iter )
	{
		PlayerNumber p = *iter;
		bool bFocus = bRowHasFocus[p];

		int iChoice = GetChoiceInRowWithFocus( p );
		if( m_RowDef.m_selectType == SELECT_NONE && bFocus )
			m_vbSelected[p][iChoice] = false;
		
		INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[p] );
	}

	return iChangeMask;
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
