#include "global.h"
#include "ActorFrame.h"
#include "arch/Dialog/Dialog.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "XmlFile.h"
#include "ActorUtil.h"
#include "LuaBinding.h"
#include "ActorUtil.h"
#include "RageDisplay.h"
#include "ScreenDimensions.h"
#include "Foreach.h"

/* Tricky: We need ActorFrames created in Lua to auto delete their children.
 * We don't want classes that derive from ActorFrame to auto delete their 
 * children.  The name "ActorFrame" is widely used in Lua, so we'll have
 * that string instead create an ActorFrameAutoDeleteChildren object.
 */
//REGISTER_ACTOR_CLASS( ActorFrame );
REGISTER_ACTOR_CLASS_WITH_NAME( ActorFrameAutoDeleteChildren, ActorFrame );
ActorFrame *ActorFrame::Copy() const { return new ActorFrame(*this); }


ActorFrame::ActorFrame()
{
	m_bPropagateCommands = false;
	m_bDeleteChildren = false;
	m_bDrawByZPosition = false;
	m_DrawFunction.SetFromNil();
	m_UpdateFunction.SetFromNil();
	m_fUpdateRate = 1;
	m_fFOV = -1;
	m_fVanishX = SCREEN_CENTER_X;
	m_fVanishY = SCREEN_CENTER_Y;
	m_bOverrideLighting = false;
	m_bLighting = false;
	m_ambientColor = RageColor(1,1,1,1);
	m_diffuseColor = RageColor(1,1,1,1);
	m_specularColor = RageColor(1,1,1,1);
	m_lightDirection = RageVector3(0,0,1);
}

ActorFrame::~ActorFrame()
{
	if( m_bDeleteChildren )
		DeleteAllChildren();
}

ActorFrame::ActorFrame( const ActorFrame &cpy ):
	Actor( cpy )
{
#define CPY(x) this->x = cpy.x;
	CPY( m_bPropagateCommands );
	CPY( m_bDeleteChildren );
	CPY( m_bDrawByZPosition );
	CPY( m_DrawFunction );
	CPY( m_UpdateFunction );
	CPY( m_fUpdateRate );
	CPY( m_fFOV );
	CPY( m_fVanishX );
	CPY( m_fVanishY );
	CPY( m_bOverrideLighting );
	CPY( m_bLighting );
	CPY( m_ambientColor );
	CPY( m_diffuseColor );
	CPY( m_specularColor );
	CPY( m_lightDirection );
#undef CPY

	/* If m_bDeleteChildren, we own our children and it's up to us to copy
	 * them.  If not, the derived class owns the children.  This must preserve
	 * the current order of m_SubActors. */
	if( m_bDeleteChildren )
	{
		for( unsigned i = 0; i < cpy.m_SubActors.size(); ++i )
		{
			Actor *pActor = cpy.m_SubActors[i]->Copy();
			this->AddChild( pActor );
		}
	}
}

void ActorFrame::InitState()
{
	FOREACH( Actor*, m_SubActors, a )
		(*a)->InitState();
	Actor::InitState();
}

void ActorFrame::LoadFromNode( const XNode* pNode )
{
	if( AutoLoadChildren() )
		LoadChildrenFromNode( pNode );

	Actor::LoadFromNode( pNode );

	pNode->GetAttrValue( "UpdateRate", m_fUpdateRate );
	pNode->GetAttrValue( "FOV", m_fFOV );
	pNode->GetAttrValue( "VanishX", m_fVanishX );
	pNode->GetAttrValue( "VanishY", m_fVanishY );
	m_bOverrideLighting = pNode->GetAttrValue( "Lighting", m_bLighting );
	// new lighting values (only ambient color seems to work?) -aj
	RString sTemp1,sTemp2,sTemp3;
	pNode->GetAttrValue( "AmbientColor", sTemp1 );
	m_ambientColor.FromString(sTemp1);
	pNode->GetAttrValue( "DiffuseColor", sTemp2 );
	m_diffuseColor.FromString(sTemp2);
	pNode->GetAttrValue( "SpecularColor", sTemp3 );
	m_specularColor.FromString(sTemp3);
	// Values need to be converted into a RageVector3, so more work needs to be done...
	//pNode->GetAttrValue( "LightDirection", m_lightDirection );
}

void ActorFrame::LoadChildrenFromNode( const XNode* pNode )
{
	// Shouldn't be calling this unless we're going to delete our children.
	ASSERT( m_bDeleteChildren );

	// Load children
	const XNode* pChildren = pNode->GetChild("children");
	bool bArrayOnly = false;
	if( pChildren == NULL )
	{
		bArrayOnly = true;
		pChildren = pNode;
	}

	FOREACH_CONST_Child( pChildren, pChild )
	{
		if( bArrayOnly && !IsAnInt(pChild->GetName()) )
			continue;

		Actor* pChildActor = ActorUtil::LoadFromNode( pChild, this );
		if( pChildActor )
			AddChild( pChildActor );
	}
	SortByDrawOrder();
}

void ActorFrame::AddChild( Actor *pActor )
{
#ifdef DEBUG
	// check that this Actor isn't already added.
	vector<Actor*>::iterator iter = find( m_SubActors.begin(), m_SubActors.end(), pActor );
	if( iter != m_SubActors.end() )
		Dialog::OK( ssprintf("Actor \"%s\" adds child \"%s\" more than once", GetLineage().c_str(), pActor->GetName().c_str()) );
#endif

	ASSERT( pActor != NULL );
	ASSERT( (void*)pActor != (void*)0xC0000005 );
	m_SubActors.push_back( pActor );

	pActor->SetParent( this );
}

void ActorFrame::RemoveChild( Actor *pActor )
{
	vector<Actor*>::iterator iter = find( m_SubActors.begin(), m_SubActors.end(), pActor );
	if( iter != m_SubActors.end() )
		m_SubActors.erase( iter );
}

void ActorFrame::TransferChildren( ActorFrame *pTo )
{
	FOREACH( Actor*, m_SubActors, i )
		pTo->AddChild( *i );
	RemoveAllChildren();
}

Actor* ActorFrame::GetChild( const RString &sName )
{
	FOREACH( Actor*, m_SubActors, a )
	{
		if( (*a)->GetName() == sName )
			return *a;
	}
	return NULL;
}

void ActorFrame::RemoveAllChildren()
{
	m_SubActors.clear();
}

void ActorFrame::MoveToTail( Actor* pActor )
{
	vector<Actor*>::iterator iter = find( m_SubActors.begin(), m_SubActors.end(), pActor );
	if( iter == m_SubActors.end() )	// didn't find
		FAIL_M("Nonexistent actor");

	m_SubActors.erase( iter );
	m_SubActors.push_back( pActor );
}

void ActorFrame::MoveToHead( Actor* pActor )
{
	vector<Actor*>::iterator iter = find( m_SubActors.begin(), m_SubActors.end(), pActor );
	if( iter == m_SubActors.end() )	// didn't find
		FAIL_M("Nonexistent actor");

	m_SubActors.erase( iter );
	m_SubActors.insert( m_SubActors.begin(), pActor );
}

void ActorFrame::BeginDraw()
{
	Actor::BeginDraw();
	if( m_fFOV != -1 )
	{
		DISPLAY->CameraPushMatrix();
		DISPLAY->LoadMenuPerspective( m_fFOV, SCREEN_WIDTH, SCREEN_HEIGHT, m_fVanishX, m_fVanishY );
	}

	if( m_bOverrideLighting )
	{
		DISPLAY->SetLighting( m_bLighting );
		if( m_bLighting )
			DISPLAY->SetLightDirectional( 0,m_ambientColor,m_diffuseColor,m_specularColor,m_lightDirection );
	}
}


void ActorFrame::DrawPrimitives()
{
	ASSERT_M( !m_bClearZBuffer, "ClearZBuffer not supported on ActorFrames" );

	// Don't set Actor-defined render states because we won't be drawing 
	// any geometry that belongs to this object.
	// Actor::DrawPrimitives();

	if( unlikely(!m_DrawFunction.IsNil()) )
	{
		Lua *L = LUA->Get();
		m_DrawFunction.PushSelf( L );
		ASSERT( !lua_isnil(L, -1) );
		this->PushSelf( L );
		RString sError;
		if( !LuaHelpers::RunScriptOnStack(L, sError, 1, 0) ) // 1 arg, 0 results
			LOG->Warn( "Error running DrawFunction: %s", sError.c_str() );
		LUA->Release(L);
		return;
	}

	// draw all sub-ActorFrames while we're in the ActorFrame's local coordinate space
	if( m_bDrawByZPosition )
	{
		vector<Actor*> subs = m_SubActors;
		ActorUtil::SortByZPosition( subs );
		for( unsigned i=0; i<subs.size(); i++ )
			subs[i]->Draw();
	}
	else
	{
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->Draw();
	}
}


void ActorFrame::EndDraw()
{
	if( m_bOverrideLighting )
	{
		// TODO: pop state instead of turning lighting off
		DISPLAY->SetLightOff( 0 );
		DISPLAY->SetLighting( false );
	}

	if( m_fFOV != -1 )
	{
		DISPLAY->CameraPopMatrix();
	}
	Actor::EndDraw();
}

void ActorFrame::PushChildrenTable( lua_State *L )
{
	lua_newtable( L );
	FOREACH( Actor*, m_SubActors, a )
	{
		LuaHelpers::Push( L, (*a)->GetName() );
		(*a)->PushSelf( L );
		lua_rawset( L, -3 );
	}
}

void ActorFrame::PlayCommandOnChildren( const RString &sCommandName, const LuaReference *pParamTable )
{
	const apActorCommands *pCmd = GetCommand( sCommandName );
	if( pCmd != NULL )
		RunCommandsOnChildren( *pCmd, pParamTable );
}

void ActorFrame::PlayCommandOnLeaves( const RString &sCommandName, const LuaReference *pParamTable )
{
	const apActorCommands *pCmd = GetCommand( sCommandName );
	if( pCmd != NULL )
		RunCommandsOnLeaves( **pCmd, pParamTable );
}

void ActorFrame::RunCommandsRecursively( const LuaReference& cmds, const LuaReference *pParamTable )
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->RunCommandsRecursively( cmds, pParamTable );
	Actor::RunCommandsRecursively( cmds, pParamTable );
}

void ActorFrame::RunCommandsOnChildren( const LuaReference& cmds, const LuaReference *pParamTable )
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->RunCommands( cmds, pParamTable );
}

void ActorFrame::RunCommandsOnLeaves( const LuaReference& cmds, const LuaReference *pParamTable )
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->RunCommandsOnLeaves( cmds, pParamTable );
}

void ActorFrame::UpdateInternal( float fDeltaTime )
{
//	LOG->Trace( "ActorFrame::Update( %f )", fDeltaTime );

	fDeltaTime *= m_fUpdateRate;

	Actor::UpdateInternal( fDeltaTime );

	// update all sub-Actors
	for( vector<Actor*>::iterator it=m_SubActors.begin(); it!=m_SubActors.end(); it++ )
	{
		Actor *pActor = *it;
		pActor->Update(fDeltaTime);
	}

	if( unlikely(!m_UpdateFunction.IsNil()) )
	{
		Lua *L = LUA->Get();
		m_UpdateFunction.PushSelf( L );
		ASSERT( !lua_isnil(L, -1) );
		this->PushSelf( L );
		lua_pushnumber( L, fDeltaTime );
		RString sError;

		if( !LuaHelpers::RunScriptOnStack(L, sError, 2, 0) ) // 1 args, 0 results
			LOG->Warn( "Error running m_UpdateFunction: %s", sError.c_str() );
		LUA->Release(L);
	}
}

#define PropagateActorFrameCommand( cmd ) \
	void ActorFrame::cmd()				\
	{									\
		Actor::cmd();					\
										\
		/* set all sub-Actors */		\
		for( unsigned i=0; i<m_SubActors.size(); i++ ) \
			m_SubActors[i]->cmd();		\
	}

#define PropagateActorFrameCommand1Param( cmd, type ) \
	void ActorFrame::cmd( type f )		\
	{									\
		Actor::cmd( f );				\
										\
		/* set all sub-Actors */		\
		for( unsigned i=0; i<m_SubActors.size(); i++ ) \
			m_SubActors[i]->cmd( f );	\
	}

PropagateActorFrameCommand( FinishTweening )
PropagateActorFrameCommand1Param( SetDiffuse,		RageColor )
PropagateActorFrameCommand1Param( SetZTestMode,		ZTestMode )
PropagateActorFrameCommand1Param( SetZWrite,		bool )
PropagateActorFrameCommand1Param( HurryTweening,	float )
PropagateActorFrameCommand1Param( SetDiffuseAlpha,	float )
PropagateActorFrameCommand1Param( SetBaseAlpha,		float )


float ActorFrame::GetTweenTimeLeft() const
{
	float m = Actor::GetTweenTimeLeft();

	for( unsigned i=0; i<m_SubActors.size(); i++ )
	{
		const Actor* pActor = m_SubActors[i];
		m = max(m, m_fHibernateSecondsLeft + pActor->GetTweenTimeLeft());
	}

	return m;

}

static bool CompareActorsByDrawOrder(const Actor *p1, const Actor *p2)
{
	return p1->GetDrawOrder() < p2->GetDrawOrder();
}

void ActorFrame::SortByDrawOrder()
{
	// Preserve ordering of Actors with equal DrawOrders.
	stable_sort( m_SubActors.begin(), m_SubActors.end(), CompareActorsByDrawOrder );
}

void ActorFrame::DeleteAllChildren()
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		delete m_SubActors[i];
	m_SubActors.clear();
}

void ActorFrame::RunCommands( const LuaReference& cmds, const LuaReference *pParamTable )
{
	if( m_bPropagateCommands )
		RunCommandsOnChildren( cmds, pParamTable );
	else
		Actor::RunCommands( cmds, pParamTable );
}

void ActorFrame::SetPropagateCommands( bool b )
{
	m_bPropagateCommands = b;
}

void ActorFrame::HandleMessage( const Message &msg )
{
	Actor::HandleMessage( msg );

	/* Don't propagate broadcasts.  They'll receive it directly if they're subscribed. */
	if( msg.IsBroadcast() )
		return;

	for( unsigned i=0; i<m_SubActors.size(); i++ ) 
	{
		Actor* pActor = m_SubActors[i];
		pActor->HandleMessage( msg );
	}
}

void ActorFrame::SetDrawByZPosition( bool b )
{
	m_bDrawByZPosition = b;
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ActorFrame. */ 
class LunaActorFrame : public Luna<ActorFrame>
{
public:
	static int playcommandonchildren( T* p, lua_State *L )		{ p->PlayCommandOnChildren(SArg(1)); return 0; }
	static int playcommandonleaves( T* p, lua_State *L )		{ p->PlayCommandOnLeaves(SArg(1)); return 0; }
	static int runcommandsonleaves( T* p, lua_State *L )
	{
		luaL_checktype( L, 1, LUA_TFUNCTION );
		LuaReference cmds;
		cmds.SetFromStack( L );

		p->RunCommandsOnLeaves( cmds );
		return 0;
	}
	static int RunCommandsOnChildren( T* p, lua_State *L )
	{
		luaL_checktype( L, 1, LUA_TFUNCTION );
		lua_pushvalue( L, 2 );
		LuaReference ParamTable;
		ParamTable.SetFromStack( L );

		lua_pushvalue( L, 1 );
		LuaReference cmds;
		cmds.SetFromStack( L );

		p->RunCommandsOnChildren( cmds, &ParamTable );
		return 0;
	}
	static int propagate( T* p, lua_State *L )			{ p->SetPropagateCommands( BIArg(1) ); return 0; }
	static int fov( T* p, lua_State *L )				{ p->SetFOV( FArg(1) ); return 0; }
	static int SetUpdateRate( T* p, lua_State *L )			{ p->SetUpdateRate( FArg(1) ); return 0; }
	static int SetFOV( T* p, lua_State *L )				{ p->SetFOV( FArg(1) ); return 0; }
	static int vanishpoint( T* p, lua_State *L )			{ p->SetVanishPoint( FArg(1), FArg(2) ); return 0; }
	static int GetChild( T* p, lua_State *L )
	{
		Actor *pChild = p->GetChild( SArg(1) );
		if( pChild )
			pChild->PushSelf( L );
		else
			lua_pushnil( L );
		return 1;
	}
	static int GetChildren( T* p, lua_State *L )
	{
		p->PushChildrenTable( L );
		return 1;
	}
	static int GetNumChildren( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetNumChildren() ); return 1; }
	static int SetDrawByZPosition( T* p, lua_State *L )	{ p->SetDrawByZPosition( BArg(1) ); return 1; }
	static int SetDrawFunction( T* p, lua_State *L )
	{
		luaL_checktype( L, 1, LUA_TFUNCTION );

		LuaReference ref;
		lua_pushvalue( L, 1 );
		ref.SetFromStack( L );
		p->SetDrawFunction( ref );
		return 0;
	}
	static int GetDrawFunction( T* p, lua_State *L )
	{
		p->GetDrawFunction().PushSelf(L);
		return 1;
	}
	static int SetUpdateFunction( T* p, lua_State *L )
	{
		luaL_checktype( L, 1, LUA_TFUNCTION );

		LuaReference ref;
		lua_pushvalue( L, 1 );
		ref.SetFromStack( L );
		p->SetUpdateFunction( ref );
		return 0;
	}
	static int SortByDrawOrder( T* p, lua_State *L )		{ p->SortByDrawOrder(); return 0; }

	//static int CustomLighting( T* p, lua_State *L )			{ p->SetCustomLighting(BArg(1)); return 0; }
	static int SetAmbientLightColor( T* p, lua_State *L )		{ RageColor c; c.FromStackCompat( L, 1 ); p->SetAmbientLightColor( c ); return 0; }
	static int SetDiffuseLightColor( T* p, lua_State *L )		{ RageColor c; c.FromStackCompat( L, 1 ); p->SetDiffuseLightColor( c ); return 0; }
	static int SetSpecularLightColor( T* p, lua_State *L )	{ RageColor c; c.FromStackCompat( L, 1 ); p->SetSpecularLightColor( c ); return 0; }
	static int SetLightDirection( T* p, lua_State *L )
	{
		luaL_checktype( L, 1, LUA_TTABLE );
		lua_pushvalue( L, 1 );
		vector<float> coords;
		LuaHelpers::ReadArrayFromTable( coords, L );
		lua_pop( L, 1 );
		if( coords.size() !=3 )
		{
			//error
		}
		RageVector3 vTmp = RageVector3( coords[0], coords[1], coords[2] );
		p->SetLightDirection( vTmp );
		return 0;
	}

	static int AddChildFromPath( T* p, lua_State *L )
	{
		// this one is tricky, we need to get an Actor from Lua.
		Actor *pActor = ActorUtil::MakeActor( SArg(1) );
		if ( pActor == NULL )
		{
			lua_pushboolean( L, 0 );
			return 1;
		}
		p->AddChild( pActor );
		lua_pushboolean( L, 1 );
		return 1;
	}

	static int RemoveChild( T* p, lua_State *L )
	{
		Actor *pChild = p->GetChild( SArg(1) );
		if( pChild )
			p->RemoveChild( pChild );
		else
			lua_pushnil( L );
		return 1;
	}
	static int RemoveAllChildren( T* p, lua_State *L )				{ p->RemoveAllChildren( ); return 0; }

	LunaActorFrame()
	{
		ADD_METHOD( playcommandonchildren );
		ADD_METHOD( playcommandonleaves );
		ADD_METHOD( runcommandsonleaves );
		ADD_METHOD( RunCommandsOnChildren );
		ADD_METHOD( propagate ); // deprecated
		ADD_METHOD( fov );
		ADD_METHOD( SetUpdateRate );
		ADD_METHOD( SetFOV );
		ADD_METHOD( vanishpoint );
		ADD_METHOD( GetChild );
		ADD_METHOD( GetChildren );
		ADD_METHOD( GetNumChildren );
		ADD_METHOD( SetDrawByZPosition );
		ADD_METHOD( SetDrawFunction );
		ADD_METHOD( GetDrawFunction );
		ADD_METHOD( SetUpdateFunction );
		ADD_METHOD( SortByDrawOrder );
		//ADD_METHOD( CustomLighting );
		ADD_METHOD( SetAmbientLightColor );
		ADD_METHOD( SetDiffuseLightColor );
		ADD_METHOD( SetSpecularLightColor );
		ADD_METHOD( SetLightDirection );
		ADD_METHOD( AddChildFromPath );
		ADD_METHOD( RemoveChild );
		ADD_METHOD( RemoveAllChildren );
		
	}
};

LUA_REGISTER_DERIVED_CLASS( ActorFrame, Actor )
// lua end

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
