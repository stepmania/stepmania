#include "global.h"
#include "OptionRow.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "Font.h"
#include "Foreach.h"
#include "OptionRowHandler.h"

static const CString SelectTypeNames[NUM_SELECT_TYPES] = {
	"SelectOne",
	"SelectMultiple",
	"SelectNone",
};
XToString( SelectType );
StringToX( SelectType );

static const CString LayoutTypeNames[NUM_LAYOUT_TYPES] = {
	"ShowAllInRow",
	"ShowOneInRow",
};
XToString( LayoutType );
StringToX( LayoutType );


const CString NEXT_ROW_NAME = "NextRow";

CString ITEMS_LONG_ROW_X_NAME( size_t p )		{ return ssprintf("ItemsLongRowP%dX",p+1); }
CString ICONS_X_NAME( size_t p )				{ return ssprintf("IconsP%dX",p+1); }

OptionRow::OptionRow()
{
	FOREACH_PlayerNumber( p )
		this->AddChild( &m_OptionIcons[p] );
	this->AddChild( &m_sprBullet );
	this->AddChild( &m_textTitle );

	m_pHand = NULL;
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

	for( unsigned i = 0; i < m_textItems.size(); ++i )
		SAFE_DELETE( m_textItems[i] );
	m_textItems.clear();
	FOREACH_PlayerNumber( p )
	{
		for( unsigned i = 0; i < m_Underline[p].size(); ++i )
			SAFE_DELETE( m_Underline[p][i] );
		m_Underline[p].clear();
	}

	ASSERT( m_pHand == NULL );

	m_bFirstItemGoesDown = false;
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
	ITEMS_ZOOM						.Load(m_sType,"ItemsZoom");
	ITEMS_START_X					.Load(m_sType,"ItemsStartX");
	ITEMS_END_X						.Load(m_sType,"ItemsEndX");
	ITEMS_GAP_X						.Load(m_sType,"ItemsGapX");
	ITEMS_LONG_ROW_X				.Load(m_sType,ITEMS_LONG_ROW_X_NAME,NUM_PLAYERS);
	ITEMS_LONG_ROW_SHARED_X			.Load(m_sType,"ItemsLongRowSharedX");
	ICONS_X							.Load(m_sType,ICONS_X_NAME,NUM_PLAYERS);
	COLOR_SELECTED					.Load(m_sType,"ColorSelected");
	COLOR_NOT_SELECTED				.Load(m_sType,"ColorNotSelected");
	COLOR_DISABLED					.Load(m_sType,"ColorDisabled");
	CAPITALIZE_ALL_OPTION_NAMES		.Load(m_sType,"CapitalizeAllOptionNames");
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

	if( !def.choices.size() )
		RageException::Throw( "Screen %s menu entry \"%s\" has no choices",
		m_sName.c_str(), def.name.c_str() );
	
	FOREACH_PlayerNumber( p )
	{
		vector<bool> &vbSelected = m_vbSelected[p];
		vbSelected.resize( m_RowDef.choices.size() );
		for( unsigned j=0; j<vbSelected.size(); j++ )
			vbSelected[j] = false;
		
		// set select the first item if a SELECT_ONE row
		if( m_RowDef.selectType == SELECT_ONE )
			vbSelected[0] = true;
	}

	// TRICKY:  Insert a down arrow as the first choice in the row.
	if( m_bFirstItemGoesDown )
	{
		m_RowDef.choices.insert( m_RowDef.choices.begin(), ENTRY_NAME(NEXT_ROW_NAME) );
		FOREACH_PlayerNumber( p )
			m_vbSelected[p].insert( m_vbSelected[p].begin(), false );
	}
}

void OptionRow::AfterImportOptions( 
	Font* pFont, 
	const CString &sTitle,
	float fY
	)
{
	/*
	ITEMS_START_X, 
	ITEMS_GAP_X, 
	ITEMS_END_X, 
	ITEMS_LONG_ROW_SHARED_X,
	ITEMS_LONG_ROW_X,
	ITEMS_ZOOM, 
	CAPITALIZE_ALL_OPTION_NAMES,
	THEME->GetPathF(m_sType,"item"),
	THEME->GetPathF(m_sType,"title"),
	THEME->GetPathG(m_sType,"bullet"),
	LABELS_X,
	ARROWS_X,
	LABELS_ON_COMMAND
*/

	
	// Make all selections the same if bOneChoiceForAllPlayers
	if( m_RowDef.bOneChoiceForAllPlayers )
	{
		for( int p=1; p<NUM_PLAYERS; p++ )
			m_vbSelected[p] = m_vbSelected[0];
	}

	FOREACH_PlayerNumber( p )
	{
		switch( m_RowDef.selectType )
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

				if( !bHasASelection )
					m_vbSelected[p][0] = true;
				
				m_iChoiceInRowWithFocus[p] = GetOneSelection(p);	// focus on the selection we just set
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
	{
		LoadOptionIcon( p, "" );
	}


	// If the items will go off the edge of the screen, then re-init with the "long row" style.
	{
		float fX = ITEMS_START_X;
		
		for( unsigned c=0; c<m_RowDef.choices.size(); c++ )
		{
			CString sText = m_RowDef.choices[c];
			if( CAPITALIZE_ALL_OPTION_NAMES )
				sText.MakeUpper();
			fX += ITEMS_ZOOM * pFont->GetLineWidthInSourcePixels( CStringToWstring(sText) );
			
			if( c != m_RowDef.choices.size()-1 )
				fX += ITEMS_GAP_X;

			if( fX > ITEMS_END_X ) 
			{
				m_RowDef.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
				break;
			}
		}
	}

	//
	// load m_textItems
	//
	switch( m_RowDef.layoutType )
	{
	case LAYOUT_SHOW_ONE_IN_ROW:
		// init text
		FOREACH_HumanPlayer( p )
		{
			BitmapText *bt = new BitmapText;
			m_textItems.push_back( bt );

			const int iChoiceInRowWithFocus = m_iChoiceInRowWithFocus[p];

			bt->LoadFromFont( THEME->GetPathF(m_sType,"item") );
			CString sText = m_RowDef.choices[iChoiceInRowWithFocus];
			if( CAPITALIZE_ALL_OPTION_NAMES )
				sText.MakeUpper();
			bt->SetText( sText );
			bt->SetZoom( ITEMS_ZOOM );
			bt->SetShadowLength( 0 );

			if( m_RowDef.bOneChoiceForAllPlayers )
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
			m_Underline[p].push_back( ul );
			ul->Load( p, true );
			int iWidth, iX, iY;
			GetWidthXY( p, 0, iWidth, iX, iY );
			ul->SetX( float(iX) );
			ul->SetWidth( float(iWidth) );
		}
		break;

	case LAYOUT_SHOW_ALL_IN_ROW:
		{
			float fX = ITEMS_START_X;
			for( unsigned c=0; c<m_RowDef.choices.size(); c++ )
			{
				// init text
				BitmapText *bt = new BitmapText;
				m_textItems.push_back( bt );
				bt->LoadFromFont( THEME->GetPathF(m_sType,"item") );
				CString sText = m_RowDef.choices[c];
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
					m_Underline[p].push_back( ul );
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

	for( unsigned c=0; c<m_textItems.size(); c++ )
		this->AddChild( m_textItems[c] );
	FOREACH_PlayerNumber( p )
		for( unsigned c=0; c<m_Underline[p].size(); c++ )
			this->AddChild( m_Underline[p][c] );


	m_textTitle.LoadFromFont( THEME->GetPathF(m_sType,"title") );
	m_textTitle.SetText( sTitle );
	m_textTitle.SetXY( LABELS_X, fY );
	m_textTitle.RunCommands( LABELS_ON_COMMAND );

	m_sprBullet.Load( THEME->GetPathG(m_sType,"bullet") );
	m_sprBullet.SetXY( ARROWS_X, fY );
	
	// set the Y position of each item in the line
	for( unsigned c=0; c<m_textItems.size(); c++ )
		m_textItems[c]->SetY( fY );

	//
	// HACK: Set focus to one item in the row, which is "go down"
	//
	if( m_bFirstItemGoesDown )
		FOREACH_PlayerNumber( p )
			m_iChoiceInRowWithFocus[p] = 0;	
}

void OptionRow::LoadExit()
{
	/*
			THEME->GetPathF(m_sType,"item"),
		THEME->GetMetric("OptionNames","Exit"),
		ITEMS_LONG_ROW_SHARED_X,
		ITEMS_ZOOM
*/
	m_RowType = OptionRow::ROW_EXIT;

	BitmapText *bt = new BitmapText;
	m_textItems.push_back( bt );

	bt->LoadFromFont( THEME->GetPathF(m_sType,"item") );
	bt->SetText( THEME->GetMetric("OptionNames","Exit") );
	bt->SetZoom( ITEMS_ZOOM );
	bt->SetShadowLength( 0 );
	bt->SetX( ITEMS_LONG_ROW_SHARED_X );
	this->AddChild( bt );

	FOREACH_PlayerNumber( p )
		m_OptionIcons[p].SetHidden( true );
	m_sprBullet.SetHidden( true );
	m_textTitle.SetHidden( true );
}

void OptionRow::PositionUnderlines( bool bShowUnderlines, float fTweenSeconds )
{
	if( m_RowType == ROW_EXIT )
		return;

	FOREACH_HumanPlayer( p )
	{
		vector<OptionsCursor*> &vpUnderlines = m_Underline[p];

		const int iNumUnderlines = (m_RowDef.layoutType == LAYOUT_SHOW_ONE_IN_ROW) ? 1 : vpUnderlines.size();
		
		for( int i=0; i<iNumUnderlines; i++ )
		{
			OptionsCursor& ul = *vpUnderlines[i];

			int iChoiceWithFocus = (m_RowDef.layoutType == LAYOUT_SHOW_ONE_IN_ROW) ? m_iChoiceInRowWithFocus[p] : i;

			/* Don't tween X movement and color changes. */
			int iWidth, iX, iY;
			GetWidthXY( p, iChoiceWithFocus, iWidth, iX, iY );
			ul.SetGlobalX( (float)iX );
			ul.SetGlobalDiffuseColor( RageColor(1,1,1, 1.0f) );

			ASSERT( m_vbSelected[p].size() == m_RowDef.choices.size() );

			bool bSelected = m_vbSelected[p][ iChoiceWithFocus ];
			bool bHidden = !bSelected || m_bHidden;
			if( !bShowUnderlines )
				bHidden = true;

			if( ul.GetDestY() != m_fY )
			{
				ul.StopTweening();
				ul.BeginTweening( fTweenSeconds );
			}

			ul.SetHidden( bHidden );
			ul.SetBarWidth( iWidth );
			ul.SetY( (float)iY );
		}
	}
}

void OptionRow::PositionIcons( float fTweenSeconds )
{
	/*
	ICONS_X
	*/
	if( m_RowType == OptionRow::ROW_EXIT )
		return;

	FOREACH_HumanPlayer( p )	// foreach player
	{
		OptionIcon &icon = m_OptionIcons[p];

		int iChoiceWithFocus = m_iChoiceInRowWithFocus[p];

		int iWidth, iX, iY;			// We only use iY
		GetWidthXY( p, iChoiceWithFocus, iWidth, iX, iY );
		icon.SetX( ICONS_X.GetValue(p) );

		if( icon.GetDestY() != m_fY )
		{
			icon.StopTweening();
			icon.BeginTweening( fTweenSeconds );
		}

		icon.SetY( (float)iY );
		/* XXX: this doesn't work since icon is an ActorFrame */
		icon.SetDiffuse( RageColor(1,1,1, m_bHidden? 0.0f:1.0f) );
	}
}

void OptionRow::UpdateText()
{
	/*
	CAPITALIZE_ALL_OPTION_NAMES
	*/
	switch( m_RowDef.layoutType )
	{
	case LAYOUT_SHOW_ONE_IN_ROW:
		FOREACH_HumanPlayer( pn )
		{
			int iChoiceWithFocus = m_iChoiceInRowWithFocus[pn];
			unsigned item_no = m_RowDef.bOneChoiceForAllPlayers ? 0 : pn;

			/* If player_no is 2 and there is no player 1: */
			item_no = min( item_no, m_textItems.size()-1 );

			CString sText = m_RowDef.choices[iChoiceWithFocus];
			if( CAPITALIZE_ALL_OPTION_NAMES )
				sText.MakeUpper();
			m_textItems[item_no]->SetText( sText );
		}
		break;
	}
}

void OptionRow::UpdateEnabledDisabled( 
	bool bThisRowHasFocus[NUM_PLAYERS], 
	float fTweenSeconds )
{
	/*
		COLOR_SELECTED, 
		COLOR_NOT_SELECTED, 
		COLOR_DISABLED, 
	*/

	bool bThisRowHasFocusByAny = false;
	FOREACH_HumanPlayer( p )
		bThisRowHasFocusByAny |= bThisRowHasFocus[p];

	bool bThisRowHasFocusByAll = true;
	FOREACH_HumanPlayer( p )
		bThisRowHasFocusByAll &= bThisRowHasFocus[p];
	
	float fDiffuseAlpha = m_bHidden? 0.0f:1.0f;

	/* Don't tween selection colors at all. */
	RageColor color = bThisRowHasFocusByAny ? COLOR_SELECTED:COLOR_NOT_SELECTED;
	m_sprBullet.SetGlobalDiffuseColor( color );
	m_textTitle.SetGlobalDiffuseColor( color );

	switch( m_RowDef.layoutType )
	{
	case LAYOUT_SHOW_ALL_IN_ROW:
		for( unsigned j=0; j<m_textItems.size(); j++ ) 	 
			m_textItems[j]->SetGlobalDiffuseColor( color ); 	 
		for( unsigned j=0; j<m_textItems.size(); j++ )
		{
			if( m_textItems[j]->GetDestY() == m_fY && 	 
				m_textItems[j]->DestTweenState().diffuse[0][3] == fDiffuseAlpha ) 	 
				continue;

			m_textItems[j]->StopTweening();
			m_textItems[j]->BeginTweening( fTweenSeconds );
			m_textItems[j]->SetDiffuseAlpha( fDiffuseAlpha );
			m_textItems[j]->SetY( m_fY );
		}
		break;
	case LAYOUT_SHOW_ONE_IN_ROW:
		FOREACH_HumanPlayer( pn )
		{
			bool bRowEnabled = m_RowDef.m_vEnabledForPlayers.find(pn) != m_RowDef.m_vEnabledForPlayers.end();
			
			if( bThisRowHasFocus[pn] )
				color = COLOR_SELECTED;
			else if( bRowEnabled )
				color = COLOR_NOT_SELECTED;
			else
				color = COLOR_DISABLED;

			color.a = (bRowEnabled && !m_bHidden) ? 1.0f:0.0f;

			unsigned item_no = m_RowDef.bOneChoiceForAllPlayers ? 0 : pn;

			// If player_no is 2 and there is no player 1:
			item_no = min( item_no, m_textItems.size()-1 );

			BitmapText &bt = *m_textItems[item_no];

			if( bt.GetDestY() != m_fY  ||  bt.DestTweenState().diffuse[0] != color )
			{
				bt.StopTweening();
				bt.BeginTweening( fTweenSeconds );
				bt.SetDiffuse( color );
				bt.SetY( m_fY );
				FOREACH_HumanPlayer( p )
				{
					OptionsCursor &ul = *m_Underline[p][item_no];
					ul.StopTweening();
					ul.BeginTweening( fTweenSeconds );
					ul.SetDiffuseAlpha( color.a );
					ul.SetY( m_fY );
				}
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

	if( m_sprBullet.GetDestY() != m_fY ) 	 
	{
		m_sprBullet.StopTweening();
		m_textTitle.StopTweening();
		m_sprBullet.BeginTweening( fTweenSeconds );
		m_textTitle.BeginTweening( fTweenSeconds );
		m_sprBullet.SetDiffuseAlpha( m_bHidden? 0.0f:1.0f );
		m_textTitle.SetDiffuseAlpha( m_bHidden? 0.0f:1.0f );
		m_sprBullet.SetY( m_fY );
		m_textTitle.SetY( m_fY );
	}
}

void OptionRow::LoadOptionIcon( PlayerNumber pn, const CString &sText )
{
	m_OptionIcons[pn].Load( pn, sText, false );
}

BitmapText &OptionRow::GetTextItemForRow( PlayerNumber pn, int iChoiceOnRow )
{
	if( m_RowType == OptionRow::ROW_EXIT )
		return *m_textItems[0];

	bool bOneChoice = m_RowDef.bOneChoiceForAllPlayers;
	int index = -1;
	switch( m_RowDef.layoutType )
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

int OptionRow::GetOneSharedSelection() const
{
	return GetOneSelection( (PlayerNumber)0 );
}

void OptionRow::SetOneSelection( PlayerNumber pn, int iChoice )
{
	for( unsigned i=0; i<(unsigned)m_vbSelected[pn].size(); i++ )
		m_vbSelected[pn][i] = false;
	m_vbSelected[pn][iChoice] = true;
}

void OptionRow::SetOneSharedSelection( int iChoice )
{
	FOREACH_HumanPlayer( pn )
		SetOneSelection( pn, iChoice );
}

void OptionRow::SetExitText( CString sExitText )
{
	BitmapText *bt = m_textItems.back();
	bt->SetText( sExitText );
}

void OptionRow::Reload()
{
	if( m_pHand == NULL )
		return;

	ExportOptions();

	OptionRowDefinition def;
	m_pHand->Reload( def );

	switch( GetRowType() )
	{
	case OptionRow::ROW_NORMAL:
		{
			ASSERT( m_RowDef.layoutType == LAYOUT_SHOW_ONE_IN_ROW );
			m_RowDef = def;
			UpdateText();
		}
		break;
	case OptionRow::ROW_EXIT:
		// nothing to do
		break;
	default:
		ASSERT(0);
	}

	ImportOptions();
}

void OptionRow::HandleMessage( const CString& sMessage )
{
	Reload();
}


/* Hack: the NextRow entry is never set, and should be transparent.  Remove
 * it, and readd it below. */
#define ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( vbSelected ) \
	if( GetFirstItemGoesDown() ) \
		vbSelected.erase( vbSelected.begin() );
#define INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( vbSelected ) \
	if( GetFirstItemGoesDown() ) \
		vbSelected.insert( vbSelected.begin(), false );
#define VERIFY_SELECTED( vbSelected ) if( m_RowDef.selectType == SELECT_ONE ) VerifyOneSelected(vbSelected);
static void VerifyOneSelected( const vector<bool> vbSelected )
{
	int iNumSelected = 0;
	for( unsigned e = 0; e < vbSelected.size(); ++e )
		if( vbSelected[e] )
			iNumSelected++;
	ASSERT( iNumSelected == 1 );
}

void OptionRow::ImportOptions()
{
	if( m_pHand == NULL )
		return;

	if( m_RowDef.bOneChoiceForAllPlayers )
	{
		ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[0] );
		m_pHand->ImportOption( m_RowDef, PLAYER_1, m_vbSelected[0] );
		INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[0] );
		VERIFY_SELECTED( m_vbSelected[0] );
	}
	else
	{
		FOREACH_HumanPlayer( p )
		{
			ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[p] );
			m_pHand->ImportOption( m_RowDef, p, m_vbSelected[p] );
			INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[p] );
			VERIFY_SELECTED( m_vbSelected[p] );
		}
	}
}

int OptionRow::ExportOptions()
{
	if( m_pHand == NULL )
		return 0;

	int iChangeMask = 0;

	if( m_RowDef.bOneChoiceForAllPlayers )
	{
		ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[0] );
		iChangeMask |= m_pHand->ExportOption( m_RowDef, PLAYER_1, m_vbSelected[0] );
		INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[0] );
	}
	else
	{
		FOREACH_HumanPlayer( p )
		{
			ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[p] );
			iChangeMask |= m_pHand->ExportOption( m_RowDef, p, m_vbSelected[p] );
			INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( m_vbSelected[p] );
		}
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
