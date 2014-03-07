//
//  LunaSteps.h
//  stepmania
//
//  Created by Jason Felds on 3/6/14.
//
//

#ifndef __stepmania__LunaSteps__
#define __stepmania__LunaSteps__

#include "LuaBinding.h"
#include "Steps.h"

class LunaSteps: public Luna<Steps>
{
public:
	DEFINE_METHOD( GetStepsType,	m_StepsType )
	DEFINE_METHOD( GetDifficulty,	GetDifficulty() )
	DEFINE_METHOD( GetDescription,	GetDescription() )
	DEFINE_METHOD( GetChartStyle,	GetChartStyle() )
	DEFINE_METHOD( GetAuthorCredit, GetCredit() )
	DEFINE_METHOD( GetMeter,	GetMeter() )
	DEFINE_METHOD( GetFilename,	GetFilename() )
	DEFINE_METHOD( IsAutogen,	IsAutogen() )
	DEFINE_METHOD( IsAnEdit,	IsAnEdit() )
	DEFINE_METHOD( IsAPlayerEdit,	IsAPlayerEdit() )
    
	static int HasSignificantTimingChanges( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->HasSignificantTimingChanges());
		return 1;
	}
	static int HasAttacks( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->HasAttacks());
		return 1;
	}
	static int GetRadarValues( T* p, lua_State *L )
	{
		PlayerNumber pn = PLAYER_1;
		if (!lua_isnil(L, 1)) {
			pn = Enum::Check<PlayerNumber>(L, 1);
		}
		
		RadarValues &rv = const_cast<RadarValues &>(p->GetRadarValues(pn));
		rv.PushSelf(L);
		return 1;
	}
	static int GetTimingData( T* p, lua_State *L )
	{
		p->GetTimingData()->PushSelf(L);
		return 1;
	}
	static int GetHash( T* p, lua_State *L ) { lua_pushnumber( L, p->GetHash() ); return 1; }

	static int GetChartName(T *p, lua_State *L)
	{
		lua_pushstring(L, p->GetChartName());
		return 1;
	}
	static int GetDisplayBpms( T* p, lua_State *L )
	{
		DisplayBpms temp;
		p->GetDisplayBpms(temp);
		float fMin = temp.GetMin();
		float fMax = temp.GetMax();
		vector<float> fBPMs;
		fBPMs.push_back( fMin );
		fBPMs.push_back( fMax );
		LuaHelpers::CreateTableFromArray(fBPMs, L);
		return 1;
	}
	static int IsDisplayBpmSecret( T* p, lua_State *L )
	{
		DisplayBpms temp;
		p->GetDisplayBpms(temp);
		lua_pushboolean( L, temp.IsSecret() );
		return 1;
	}
	static int IsDisplayBpmConstant( T* p, lua_State *L )
	{
		DisplayBpms temp;
		p->GetDisplayBpms(temp);
		lua_pushboolean( L, temp.BpmIsConstant() );
		return 1;
	}
	static int IsDisplayBpmRandom( T* p, lua_State *L )
	{
		lua_pushboolean( L, p->GetDisplayBPM() == DISPLAY_BPM_RANDOM );
		return 1;
	}
	DEFINE_METHOD( PredictMeter, PredictMeter() )
	static int GetDisplayBPMType( T* p, lua_State *L )
	{
		LuaHelpers::Push( L, p->GetDisplayBPM() );
		return 1;
	}
    
	LunaSteps()
	{		
		ADD_METHOD( GetAuthorCredit );
		ADD_METHOD( GetChartStyle );
		ADD_METHOD( GetDescription );
		ADD_METHOD( GetDifficulty );
		ADD_METHOD( GetFilename );
		ADD_METHOD( GetHash );
		ADD_METHOD( GetMeter );
		ADD_METHOD( HasSignificantTimingChanges );
		ADD_METHOD( HasAttacks );
		ADD_METHOD( GetRadarValues );
		ADD_METHOD( GetTimingData );
		ADD_METHOD( GetChartName );
		//ADD_METHOD( GetSMNoteData );
		ADD_METHOD( GetStepsType );
		ADD_METHOD( IsAnEdit );
		ADD_METHOD( IsAutogen );
		ADD_METHOD( IsAPlayerEdit );
		ADD_METHOD( GetDisplayBpms );
		ADD_METHOD( IsDisplayBpmSecret );
		ADD_METHOD( IsDisplayBpmConstant );
		ADD_METHOD( IsDisplayBpmRandom );
		ADD_METHOD( PredictMeter );
		ADD_METHOD( GetDisplayBPMType );
	}
	
	static void PushSelf( lua_State *L, Steps *p) {
		// Luna<Steps>::PushObject(L, "Steps", p);
		void **pData = (void **) lua_newuserdata( L, sizeof(void *) );
		*pData = p;
		LuaBinding::ApplyDerivedType( L, "Steps", p );
	}
	
	static LuaReference CreateFromPush( Steps *p )
	{
		Lua *L = LUA->Get();
		LuaReference ref;
		PushSelf(L, p);
		ref.SetFromStack( L );
		LUA->Release( L );
		
		return ref;
	}
};

#endif /* defined(__stepmania__LunaSteps__) */

/*
 * (c) 2001-2014 Chris Danford, Glenn Maynard, David Wilson
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
