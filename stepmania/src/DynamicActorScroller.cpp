#include "global.h"
#include "DynamicActorScroller.h"
#include "XmlFile.h"
#include "LuaManager.h"
#include "ActorUtil.h"
#include "RageUtil.h"
#include "LuaBinding.h"

Actor *DynamicActorScroller::Copy() const { return new DynamicActorScroller(*this); }

void DynamicActorScroller::LoadFromNode( const RString &sDir, const XNode *pNode )
{
	ActorScroller::LoadFromNode( sDir, pNode );

	/*
	 * All of our children are identical, since they must be interchangeable.
	 * The <children> node loads only one; we copy the rest.
	 *
	 * Make one extra copy if masking is enabled.
	 */
	if( m_SubActors.size() != 1 )
		RageException::Throw( "%s: DynamicActorScroller: loaded %i nodes; require exactly one", ActorUtil::GetWhere(pNode).c_str(), (int)m_SubActors.size() );

	int iNumCopies = (int) m_fNumItemsToDraw;
	if( !m_quadMask.GetHidden() )
		iNumCopies += 1;
	for( int i = 1; i < iNumCopies; ++i )
	{
		Actor *pCopy = m_SubActors[0]->Copy();
		this->AddChild( pCopy );
	}

	RString sLoadFunction;
	pNode->GetAttrValue( "LoadFunction", sLoadFunction );
	m_LoadFunction.SetFromExpression( sLoadFunction );

	/* Call the expression with line = nil to find out the number of lines. */
	{
		Lua *L = LUA->Get();
		m_LoadFunction.PushSelf( L );
		ASSERT( !lua_isnil(L, -1) );
		lua_pushnil( L );
		lua_pushnil( L );
		lua_call( L, 2, 1 ); // 2 args, 1 results

		m_iNumItems = (int) luaL_checknumber( L, -1 );
		lua_pop( L, 1 );
		LUA->Release(L);
	}

	/*
	 * Reconfigure all items, so the loaded actors actually correspond with
	 * m_iFirstSubActorIndex.
	 */
	ShiftSubActors( INT_MAX );
}

/*
 * Shift m_SubActors forward by iDist, and then fill in the new entries.
 *
 * Important: under normal scrolling, with or without m_bLoop, at most one
 * object is created per update, and this normally only happens when an
 * object comes on screen.  Extra actor updates are avoided for efficiency.
 */
void DynamicActorScroller::ShiftSubActors( int iDist )
{
	ActorScroller::ShiftSubActors( iDist );

	if( iDist == 0 )
		return;

	if( m_bLoop )
	{
		/* Optimization: in a loop of 10, when we loop around from 9 to 0,
		 * iDist will be -9.  Moving -9 is equivalent to moving +1, and
		 * reconfigures much fewer actors. */
		int iWrapped = iDist;
		wrap( iWrapped, m_iNumItems );
		if( abs(iWrapped) < abs(iDist) )
			iDist = iWrapped;
	}

	int iFirstToReconfigure = 0;
	int iLastToReconfigure = (int)m_SubActors.size();
	if( iDist > 0 && iDist < (int) m_SubActors.size() )
		iFirstToReconfigure = m_SubActors.size()-iDist;
	else if( iDist < 0 && -iDist < (int) m_SubActors.size() )
		iLastToReconfigure = -iDist;

	for( int i = iFirstToReconfigure; i < iLastToReconfigure; i++ )
	{
		int iIndex = i; // index into m_SubActors
		int iItem = i + m_iFirstSubActorIndex;
		if( m_bLoop )
		{
			wrap( iIndex, m_SubActors.size() );
			wrap( iItem, m_iNumItems );
		}
		else if( iIndex < 0 || iIndex >= m_iNumItems || iItem < 0 || iItem >= m_iNumItems )
			continue;

		{
			Lua *L = LUA->Get();
			lua_pushnumber( L, i );
			m_SubActors[iIndex]->m_pLuaInstance->Set( L, "ItemIndex" );
			LUA->Release(L);
		}

		ConfigureActor( m_SubActors[iIndex], iItem );
	}
}

void DynamicActorScroller::ConfigureActor( Actor *pActor, int iItem )
{
	Lua *L = LUA->Get();
	m_LoadFunction.PushSelf( L );
	ASSERT( !lua_isnil(L, -1) );
	pActor->PushSelf( L );
	LuaHelpers::Push( L, iItem );
	lua_call( L, 2, 0 ); // 2 args, 0 results
	LUA->Release(L);
}

REGISTER_ACTOR_CLASS_WITH_NAME( DynamicActorScrollerAutoDeleteChildren, DynamicActorScroller )

/*
 * (c) 2005 Glenn Maynard
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
