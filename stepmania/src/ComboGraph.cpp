#include "global.h"
#include "ComboGraph.h"
#include "RageLog.h"
#include "StageStats.h"
#include "ActorUtil.h"
#include "BitmapText.h"
#include "XmlFile.h"

const int MinComboSizeToShow = 5;

REGISTER_ACTOR_CLASS( ComboGraph )

ComboGraph::ComboGraph()
{
	m_pNormalCombo = NULL;
	m_pMaxCombo = NULL;
}

ComboGraph::~ComboGraph()
{
	delete m_pNormalCombo;
	delete m_pMaxCombo;
	this->DeleteAllChildren();
}

void ComboGraph::LoadFromNode( const CString& sDir, const XNode* pNode )
{
	ActorFrame::LoadFromNode( sDir, pNode );

	const XNode *pChild = pNode->GetChild( "MaxComboText" );
	if( pChild == NULL )
		RageException::Throw( ssprintf("ComboGraph in \"%s\" is missing the node \"MaxComboText\"", sDir.c_str()) );
	m_MaxComboText.LoadFromNode( sDir, pChild );

	pChild = pNode->GetChild( "NormalCombo" );
	if( pChild == NULL )
		RageException::Throw( ssprintf("ComboGraph in \"%s\" is missing the node \"NormalCombo\"", sDir.c_str()) );
	m_pNormalCombo = ActorUtil::LoadFromActorFile( sDir, pChild );

	pChild = pNode->GetChild( "MaxCombo" );
	if( pChild == NULL )
		RageException::Throw( ssprintf("ComboGraph in \"%s\" is missing the node \"MaxCombo\"", sDir.c_str()) );
	m_pMaxCombo = ActorUtil::LoadFromActorFile( sDir, pChild );
}

void ComboGraph::Load( const StageStats &s, const PlayerStageStats &pss )
{
	ASSERT( m_SubActors.size() == 0 );

	const float fFirstSecond = 0;
	const float fLastSecond = s.GetTotalPossibleStepsSeconds();

	/* Find the largest combo. */
	int MaxComboSize = 0;
	for( unsigned i = 0; i < pss.ComboList.size(); ++i )
		MaxComboSize = max( MaxComboSize, pss.ComboList[i].GetStageCnt() );

	for( unsigned i = 0; i < pss.ComboList.size(); ++i )
	{
		const PlayerStageStats::Combo_t &combo = pss.ComboList[i];
		if( combo.GetStageCnt() < MinComboSizeToShow )
			continue; /* too small */

		const bool IsMax = (combo.GetStageCnt() == MaxComboSize);

		LOG->Trace("combo %i is %f+%f", i, combo.fStartSecond, combo.fSizeSeconds);
		Actor *sprite = IsMax? m_pMaxCombo->Copy():m_pNormalCombo->Copy();

		const float start = SCALE( combo.fStartSecond, fFirstSecond, fLastSecond, 0.0f, 1.0f );
		const float size = SCALE( combo.fSizeSeconds, 0, fLastSecond-fFirstSecond, 0.0f, 1.0f );
		sprite->SetCropLeft ( SCALE( size, 0.0f, 1.0f, 0.5f, 0.0f ) );
		sprite->SetCropRight( SCALE( size, 0.0f, 1.0f, 0.5f, 0.0f ) );

		sprite->SetCropLeft( start );
		sprite->SetCropRight( 1 - (size + start) );

		this->AddChild( sprite );
	}

	for( unsigned i = 0; i < pss.ComboList.size(); ++i )
	{
		const PlayerStageStats::Combo_t &combo = pss.ComboList[i];
		if( combo.GetStageCnt() < MinComboSizeToShow )
			continue; /* too small */
	
		if( !MaxComboSize )
			continue;

		const bool IsMax = (combo.GetStageCnt() == MaxComboSize);
		if( !IsMax )
			continue;

		BitmapText *text = (BitmapText *) m_MaxComboText.Copy(); // XXX Copy should be covariant

		const float start = SCALE( combo.fStartSecond, fFirstSecond, fLastSecond, 0.0f, 1.0f );
		const float size = SCALE( combo.fSizeSeconds, 0, fLastSecond-fFirstSecond, 0.0f, 1.0f );

		const float fWidth = m_pNormalCombo->GetUnzoomedWidth();
		const float CenterPercent = start + size/2;
		const float CenterXPos = SCALE( CenterPercent, 0.0f, 1.0f, -fWidth/2.0f, fWidth/2.0f );
		text->SetX( CenterXPos );

		text->SetText( ssprintf("%i",combo.GetStageCnt()) );

		this->AddChild( text );
	}
}

// lua start
#include "LuaBinding.h"

class LunaComboGraph: public Luna<ComboGraph>
{
public:
	LunaComboGraph() { LUA->Register( Register ); }

	static int LoadFromStats( T* p, lua_State *L )
	{
		StageStats *pStageStats = Luna<StageStats>::check( L, 1 );
		PlayerStageStats *pPlayerStageStats = Luna<PlayerStageStats>::check( L, 2 );
		p->Load( *pStageStats, *pPlayerStageStats );
		return 0;
	}

	static void Register(lua_State *L) 
	{
		ADD_METHOD( LoadFromStats );

		Luna<T>::Register( L );
	}
};

LUA_REGISTER_DERIVED_CLASS( ComboGraph, ActorFrame )
// lua end

/*
 * (c) 2003 Glenn Maynard
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
