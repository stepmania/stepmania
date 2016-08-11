#include "global.h"
#include "ModIconRow.h"
#include "RageMath.hpp"

#include <array>

#include "ThemeManager.h"
#include "PlayerOptions.h"
#include "GameState.h"
#include "RageLog.h"
#include "PlayerState.h"
#include "ActorUtil.h"
#include "XmlFile.h"
#include "LuaManager.h"

using std::vector;

int OptionToPreferredColumn( std::string sOptionText );

REGISTER_ACTOR_CLASS( ModIconRow );

ModIconRow::ModIconRow()
{
	m_pn = PlayerNumber_Invalid;

	this->SubscribeToMessage( "PlayerOptionsChanged" );
}

ModIconRow::~ModIconRow()
{
	for (auto *p: m_vpModIcon)
	{
		Rage::safe_delete( p );
	}
	this->RemoveAllChildren();
}

void ModIconRow::Load( const std::string &sMetricsGroup, PlayerNumber pn )
{
	ASSERT_M( m_pn == PlayerNumber_Invalid, "Multiple calls to Load" );

	m_sMetricsGroup = sMetricsGroup;
	m_pn = pn;

	SPACING_X.Load(sMetricsGroup,"SpacingX");
	SPACING_Y.Load(sMetricsGroup,"SpacingY");
	NUM_OPTION_ICONS.Load(sMetricsGroup,"NumModIcons");
	OPTION_ICON_METRICS_GROUP.Load(sMetricsGroup,"ModIconMetricsGroup");

	for( int i=0; i<NUM_OPTION_ICONS; i++ )
	{
		ModIcon *p = new ModIcon;
		p->SetName( "ModIcon" );
		float fOffset = Rage::scale( i + 0.f, 0.f, NUM_OPTION_ICONS-1.f, -(NUM_OPTION_ICONS-1)/2.0f, (NUM_OPTION_ICONS-1)/2.0f );
		p->SetXY( fOffset*SPACING_X, fOffset*SPACING_Y );
		p->Load( OPTION_ICON_METRICS_GROUP.GetValue() );
		ActorUtil::LoadAllCommands( p, sMetricsGroup );
		m_vpModIcon.push_back( p );
		this->AddChild( p );
	}

	SetFromGameState();
}

void ModIconRow::HandleMessage( const Message &msg )
{
	if( msg.GetName() == "PlayerOptionsChanged" )
		SetFromGameState();

	ActorFrame::HandleMessage( msg );
}


struct OptionColumnEntry
{
	std::string szString;
	int iSlotIndex;

	OptionColumnEntry(std::string str, int slot): szString(str), iSlotIndex(slot) {}
	//void FromStack( lua_State *L, int iPos );
};

// todo: metric these? -aj
static std::array<OptionColumnEntry, 33> const g_OptionColumnEntries =
{
	{
		OptionColumnEntry {"Boost", 0},
		OptionColumnEntry {"Brake", 0},
		OptionColumnEntry {"Wave", 0},
		OptionColumnEntry {"Expand", 0},
		OptionColumnEntry {"Boomerang", 0},
		//--------------------//
		OptionColumnEntry {"Drunk", 1},
		OptionColumnEntry {"Dizzy", 1},
		OptionColumnEntry {"Mini", 1},
		OptionColumnEntry {"Flip", 1},
		OptionColumnEntry {"Tornado", 1},
		//--------------------//
		OptionColumnEntry {"Hidden", 2},
		OptionColumnEntry {"Sudden", 2},
		OptionColumnEntry {"Stealth", 2},
		OptionColumnEntry {"Blink", 2},
		OptionColumnEntry {"RandomVanish", 2},
		//--------------------//
		OptionColumnEntry {"Mirror", 3},
		OptionColumnEntry {"Left", 3},
		OptionColumnEntry {"Right", 3},
		OptionColumnEntry {"Shuffle", 3},
		OptionColumnEntry {"SuperShuffle", 3},
		//--------------------//
		OptionColumnEntry {"Little", 4},
		OptionColumnEntry {"NoHolds", 4},
		OptionColumnEntry {"Dark", 4},
		OptionColumnEntry {"Blind", 4},
		//--------------------//
		OptionColumnEntry {"Reverse", 5},
		OptionColumnEntry {"Split", 5},
		OptionColumnEntry {"Alternate", 5},
		OptionColumnEntry {"Cross", 5},
		OptionColumnEntry {"Centered", 5},
		//--------------------//
		OptionColumnEntry {"Incoming", 6},
		OptionColumnEntry {"Space", 6},
		OptionColumnEntry {"Hallway", 6},
		OptionColumnEntry {"Distant", 6}
	}
};

int OptionToPreferredColumn( std::string sOptionText )
{
	// Speedups always go in column 0. digit ... x
	if( sOptionText.size() > 1 &&
		isdigit(sOptionText[0])    &&
		tolower(sOptionText[sOptionText.size()-1]) == 'x' )
	{
		return 0;
	}

	for (auto const &entry : g_OptionColumnEntries)
	{
		if ( entry.szString == sOptionText )
		{
			return entry.iSlotIndex;
		}
	}

	// This warns about C1234 and noteskins.
//	LOG->Warn("Unknown option: '%s'", sOptionText.c_str() );
	return 0;
}

void ModIconRow::SetFromGameState()
{
	PlayerNumber pn = m_pn;

	std::string sOptions = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetStage().GetString();
	auto vsOptions = Rage::split(sOptions, ", ", Rage::EmptyEntries::skip);

	vector<std::string> vsText;	// fill these with what will be displayed on the tabs
	vsText.resize( m_vpModIcon.size() );

	// for each option, look for the best column to place it in
	for (auto sOption: vsOptions)
	{
		int iPerferredCol = OptionToPreferredColumn( sOption );
		Rage::clamp( iPerferredCol, 0, static_cast<int>(m_vpModIcon.size()) - 1 );

		if( iPerferredCol == -1 )
			continue;	// skip

		// search for a vacant spot
		for( int j=iPerferredCol; j<NUM_OPTION_ICONS; j++ )
		{
			if( vsText[j] != "" )
			{
				continue;
			}
			else
			{
				vsText[j] = sOption;
				break;
			}
		}
	}

	for( unsigned i=0; i<m_vpModIcon.size(); i++ )
		m_vpModIcon[i]->Set( vsText[i] );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ModIconRow. */
class LunaModIconRow: public Luna<ModIconRow>
{
public:
	static int Load( T* p, lua_State *L )		{ p->Load( SArg(1), Enum::Check<PlayerNumber>(L, 2) ); COMMON_RETURN_SELF; }

	LunaModIconRow()
	{
		ADD_METHOD( Load );
	}
};

LUA_REGISTER_DERIVED_CLASS( ModIconRow, ActorFrame )

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
