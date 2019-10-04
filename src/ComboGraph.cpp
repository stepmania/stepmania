#include "global.h"
#include "ComboGraph.h"
#include "RageLog.h"
#include "StageStats.h"
#include "ActorUtil.h"
#include "BitmapText.h"
#include "XmlFile.h"

const int MinComboSizeToShow = 5;

REGISTER_ACTOR_CLASS( ComboGraph );

ComboGraph::ComboGraph()
{
	DeleteChildrenWhenDone( true );

	m_pNormalCombo = nullptr;
	m_pMaxCombo = nullptr;
	m_pComboNumber = nullptr;
}

void ComboGraph::Load( RString sMetricsGroup )
{
	BODY_WIDTH.Load( sMetricsGroup, "BodyWidth" );
	BODY_HEIGHT.Load( sMetricsGroup, "BodyHeight" );

	// These need to be set so that a theme can use zoomtowidth/zoomtoheight and get correct behavior.
	this->SetWidth(BODY_WIDTH);
	this->SetHeight(BODY_HEIGHT);

	Actor *pActor = nullptr;

	m_pBacking = ActorUtil::MakeActor( THEME->GetPathG(sMetricsGroup,"Backing") );
	if( m_pBacking != nullptr )
	{
		m_pBacking->ZoomToWidth( BODY_WIDTH );
		m_pBacking->ZoomToHeight( BODY_HEIGHT );
		this->AddChild( m_pBacking );
	}

	m_pNormalCombo = ActorUtil::MakeActor( THEME->GetPathG(sMetricsGroup,"NormalCombo") );
	if( m_pNormalCombo != nullptr )
	{
		m_pNormalCombo->ZoomToWidth( BODY_WIDTH );
		m_pNormalCombo->ZoomToHeight( BODY_HEIGHT );
		this->AddChild( m_pNormalCombo );
	}

	m_pMaxCombo = ActorUtil::MakeActor( THEME->GetPathG(sMetricsGroup,"MaxCombo") );
	if( m_pMaxCombo != nullptr )
	{
		m_pMaxCombo->ZoomToWidth( BODY_WIDTH );
		m_pMaxCombo->ZoomToHeight( BODY_HEIGHT );
		this->AddChild( m_pMaxCombo );
	}

	pActor = ActorUtil::MakeActor( THEME->GetPathG(sMetricsGroup,"ComboNumber") );
	if( pActor != nullptr )
	{
		m_pComboNumber = dynamic_cast<BitmapText *>( pActor );
		if( m_pComboNumber != nullptr )
			this->AddChild( m_pComboNumber );
		else
			LuaHelpers::ReportScriptErrorFmt( "ComboGraph: \"sMetricsGroup\" \"ComboNumber\" must be a BitmapText" );
	}
}	

void ComboGraph::Set( const StageStats &s, const PlayerStageStats &pss )
{
	const float fFirstSecond = 0;
	const float fLastSecond = s.GetTotalPossibleStepsSeconds();

	// Find the largest combo.
	int iMaxComboSize = 0;
	for( unsigned i = 0; i < pss.m_ComboList.size(); ++i )
		iMaxComboSize = max( iMaxComboSize, pss.m_ComboList[i].GetStageCnt() );

	for( unsigned i = 0; i < pss.m_ComboList.size(); ++i )
	{
		const PlayerStageStats::Combo_t &combo = pss.m_ComboList[i];
		if( combo.GetStageCnt() < MinComboSizeToShow )
			continue; // too small

		const bool bIsMax = (combo.GetStageCnt() == iMaxComboSize);

		LOG->Trace( "combo %i is %f+%f of %f", i, combo.m_fStartSecond, combo.m_fSizeSeconds, fLastSecond );
		Actor *pSprite = bIsMax? m_pMaxCombo->Copy() : m_pNormalCombo->Copy();

		const float fStart = SCALE( combo.m_fStartSecond, fFirstSecond, fLastSecond, 0.0f, 1.0f );
		const float fSize = SCALE( combo.m_fSizeSeconds, 0, fLastSecond-fFirstSecond, 0.0f, 1.0f );
		pSprite->SetCropLeft ( SCALE( fSize, 0.0f, 1.0f, 0.5f, 0.0f ) );
		pSprite->SetCropRight( SCALE( fSize, 0.0f, 1.0f, 0.5f, 0.0f ) );

		pSprite->SetCropLeft( fStart );
		pSprite->SetCropRight( 1 - (fSize + fStart) );

		this->AddChild( pSprite );
	}

	for( unsigned i = 0; i < pss.m_ComboList.size(); ++i )
	{
		const PlayerStageStats::Combo_t &combo = pss.m_ComboList[i];
		if( combo.GetStageCnt() < MinComboSizeToShow )
			continue; // too small
	
		if( !iMaxComboSize )
			continue;

		const bool bIsMax = (combo.GetStageCnt() == iMaxComboSize);
		if( !bIsMax )
			continue;

		BitmapText *pText = m_pComboNumber->Copy();

		const float fStart = SCALE( combo.m_fStartSecond, fFirstSecond, fLastSecond, 0.0f, 1.0f );
		const float fSize = SCALE( combo.m_fSizeSeconds, 0, fLastSecond-fFirstSecond, 0.0f, 1.0f );

		const float fCenterPercent = fStart + fSize/2;
		const float fCenterXPos = SCALE( fCenterPercent, 0.0f, 1.0f, -BODY_WIDTH/2.0f, BODY_WIDTH/2.0f );
		pText->SetX( fCenterXPos );

		pText->SetText( ssprintf("%i",combo.GetStageCnt()) );

		this->AddChild( pText );
	}

	// Hide the templates.
	m_pNormalCombo->SetVisible( false );
	m_pMaxCombo->SetVisible( false );
	m_pComboNumber->SetVisible( false );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ComboGraph. */ 
class LunaComboGraph: public Luna<ComboGraph>
{
public:
	static int Load( T* p, lua_State *L )
	{
		p->Load( SArg(1) );
		COMMON_RETURN_SELF;
	}
	static int Set( T* p, lua_State *L )
	{
		StageStats *pStageStats = Luna<StageStats>::check( L, 1 );
		PlayerStageStats *pPlayerStageStats = Luna<PlayerStageStats>::check( L, 2 );
		p->Set( *pStageStats, *pPlayerStageStats );
		COMMON_RETURN_SELF;
	}

	LunaComboGraph()
	{
		ADD_METHOD( Load );
		ADD_METHOD( Set );
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
