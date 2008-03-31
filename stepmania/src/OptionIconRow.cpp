#include "global.h"
#include "OptionIconRow.h"
#include "ThemeManager.h"
#include "PlayerOptions.h"
#include "GameState.h"
#include "RageLog.h"
#include "PlayerState.h"
#include "ActorUtil.h"
#include "XmlFile.h"
#include "LuaManager.h"
#include "Foreach.h"

REGISTER_ACTOR_CLASS( OptionIconRow )

OptionIconRow::OptionIconRow()
{
	m_pn = PlayerNumber_Invalid;

	this->SubscribeToMessage( "PlayerOptionsChanged" );
}

OptionIconRow::~OptionIconRow()
{
	FOREACH( OptionIcon*, m_vpOptionIcon, p )
	{
		SAFE_DELETE( *p );
	}
	this->RemoveAllChildren();
}

void OptionIconRow::Load( const RString &sMetricsGroup, PlayerNumber pn )
{
	ASSERT_M( m_pn == PlayerNumber_Invalid, "Multiple calls to Load" );

	m_sMetricsGroup = sMetricsGroup;
	m_pn = pn;

	SPACING_X.Load(sMetricsGroup,"SpacingX");
	SPACING_Y.Load(sMetricsGroup,"SpacingY");
	NUM_OPTION_ICONS.Load(sMetricsGroup,"NumOptionIcons");

	for( int i=0; i<NUM_OPTION_ICONS; i++ )
	{
		OptionIcon *p = new OptionIcon;
		p->SetXY( i*SPACING_X, i*SPACING_Y );
		p->Load( sMetricsGroup );		
		m_vpOptionIcon.push_back( p );
		this->AddChild( p );
	}

	SetFromGameState();
}

void OptionIconRow::HandleMessage( const Message &msg )
{
	if( msg.GetName() == "PlayerOptionsChanged" )
		SetFromGameState();

	ActorFrame::HandleMessage( msg );
}


struct OptionColumnEntry
{
	char szString[30];
	int iSlotIndex;
};

static const OptionColumnEntry g_OptionColumnEntries[] =
{
	{"Boost",		0},
	{"Brake",		0},
	{"Wave",		0},
	{"Expand",		0},
	{"Boomerang",		0},
	{"Drunk",		1},
	{"Dizzy",		1},
	{"Mini",		1},
	{"Flip",		1},
	{"Tornado",		1},
	{"Hidden",		2},
	{"Sudden",		2},
	{"Stealth",		2},
	{"Blink",		2},
	{"RandomVanish", 	2},
	{"Mirror",		3},
	{"Left",		3},
	{"Right",		3},
	{"Shuffle",		3},
	{"SuperShuffle",	3},
	{"Little",		4},
	{"NoHolds",		4},
	{"Dark",		4},
	{"Blind",		4},
	{"Reverse",		5},
	{"Split",		5},
	{"Alternate",		5},
	{"Cross",		5},
	{"Centered",		5},
	{"Incoming",		6},
	{"Space",		6},
	{"Hallway",		6},
	{"Distant",		6},
};

int OptionToPreferredColumn( RString sOptionText )
{
	/* Speedups always go in column 0. digit ... x */
	if( sOptionText.size() > 1 &&
		isdigit(sOptionText[0])    &&
		tolower(sOptionText[sOptionText.size()-1]) == 'x' )
	{
		return 0;
	}

	for( unsigned i=0; i<ARRAYLEN(g_OptionColumnEntries); i++ )
		if( g_OptionColumnEntries[i].szString == sOptionText )
			return g_OptionColumnEntries[i].iSlotIndex;

	/* This warns about C1234 and noteskins. */
//	LOG->Warn("Unknown option: '%s'", sOptionText.c_str() );
	return 0;
}

void OptionIconRow::SetFromGameState()
{
	PlayerNumber pn = m_pn;

	RString sOptions = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetStage().GetString();
	vector<RString> vsOptions;
	split( sOptions, ", ", vsOptions, true );

	vector<RString> vsText;	// fill these with what will be displayed on the tabs
	vsText.resize( m_vpOptionIcon.size() );
	
	// for each option, look for the best column to place it in
	for( unsigned i=0; i<vsOptions.size(); i++ )
	{
		RString sOption = vsOptions[i];
		int iPerferredCol = OptionToPreferredColumn( sOption );
		clamp( iPerferredCol, 0, (int)m_vpOptionIcon.size()-1 );

		if( iPerferredCol == -1 )
			continue;	// skip

		// search for a vacant spot
		for( int i=iPerferredCol; i<NUM_OPTION_ICONS; i++ )
		{
			if( vsText[i] != "" )
			{
				continue;
			}
			else
			{
				vsText[i] = sOption;
				break;
			}
		}
	}

	for( unsigned i=0; i<vsOptions.size(); i++ )
		m_vpOptionIcon[i]->Set( vsText[i] );
}

// lua start
#include "LuaBinding.h"

class LunaOptionIconRow: public Luna<OptionIconRow>
{
public:
	static int Load( T* p, lua_State *L )		{ p->Load( SArg(1), Enum::Check<PlayerNumber>(L, 2) ); return 0; }

	LunaOptionIconRow()
	{
		ADD_METHOD( Load );
	}
};

LUA_REGISTER_DERIVED_CLASS( OptionIconRow, ActorFrame )

// lua end

/*
 * (c) 2002-2004 Chris Danford
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
