#include "global.h"
#include "HighScore.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "ThemeManager.h"
#include "XmlFile.h"
#include "RadarValues.h"

#include <algorithm>

ThemeMetric<RString> EMPTY_NAME("HighScore","EmptyName");


struct HighScoreImpl
{
	RString	sName;	// name that shows in the machine's ranking screen
	Grade grade;
	unsigned int iScore;
	float fPercentDP;
	float fSurviveSeconds;
	unsigned int iMaxCombo;			// maximum combo obtained [SM5 alpha 1a+]
	StageAward stageAward;	// stage award [SM5 alpha 1a+]
	PeakComboAward peakComboAward;	// peak combo award [SM5 alpha 1a+]
	RString	sModifiers;
	DateTime dateTime;		// return value of time() when screenshot was taken
	RString sPlayerGuid;	// who made this high score
	RString sMachineGuid;	// where this high score was made
	int iProductID;
	int iTapNoteScores[NUM_TapNoteScore];
	int iHoldNoteScores[NUM_HoldNoteScore];
	RadarValues radarValues;
	float fLifeRemainingSeconds;
	bool bDisqualified;

	HighScoreImpl();
	XNode *CreateNode() const;
	void LoadFromNode( const XNode *pNode );

	bool operator==( const HighScoreImpl& other ) const;
	bool operator!=( const HighScoreImpl& other ) const { return !(*this == other); }
};

bool HighScoreImpl::operator==( const HighScoreImpl& other ) const 
{
#define COMPARE(x)	if( x!=other.x )	return false;
	COMPARE( sName );
	COMPARE( grade );
	COMPARE( iScore );
	COMPARE( iMaxCombo );
	COMPARE( stageAward );
	COMPARE( peakComboAward );
	COMPARE( fPercentDP );
	COMPARE( fSurviveSeconds );
	COMPARE( sModifiers );
	COMPARE( dateTime );
	COMPARE( sPlayerGuid );
	COMPARE( sMachineGuid );
	COMPARE( iProductID );
	FOREACH_ENUM( TapNoteScore, tns )
		COMPARE( iTapNoteScores[tns] );
	FOREACH_ENUM( HoldNoteScore, hns )
		COMPARE( iHoldNoteScores[hns] );
	COMPARE( radarValues );
	COMPARE( fLifeRemainingSeconds );
	COMPARE( bDisqualified );
#undef COMPARE

	return true;
}

HighScoreImpl::HighScoreImpl()
{
	sName = "";
	grade = Grade_NoData;
	iScore = 0;
	fPercentDP = 0;
	fSurviveSeconds = 0;
	iMaxCombo = 0;
	stageAward = StageAward_Invalid;
	peakComboAward = PeakComboAward_Invalid;
	sModifiers = "";
	dateTime.Init();
	sPlayerGuid = "";
	sMachineGuid = "";
	iProductID = 0;
	ZERO( iTapNoteScores );
	ZERO( iHoldNoteScores );
	radarValues.MakeUnknown();
	fLifeRemainingSeconds = 0;
}

XNode *HighScoreImpl::CreateNode() const
{
	XNode *pNode = new XNode( "HighScore" );
	const bool bWriteSimpleValues = RadarValues::WRITE_SIMPLE_VALIES;
	const bool bWriteComplexValues = RadarValues::WRITE_COMPLEX_VALIES;

	// TRICKY:  Don't write "name to fill in" markers.
	pNode->AppendChild( "Name",			IsRankingToFillIn(sName) ? RString("") : sName );
	pNode->AppendChild( "Grade",			GradeToString(grade) );
	pNode->AppendChild( "Score",			iScore );
	pNode->AppendChild( "PercentDP",		fPercentDP );
	pNode->AppendChild( "SurviveSeconds",	fSurviveSeconds );
	pNode->AppendChild( "MaxCombo",			iMaxCombo );
	pNode->AppendChild( "StageAward",		StageAwardToString(stageAward) );
	pNode->AppendChild( "PeakComboAward",	PeakComboAwardToString(peakComboAward) );
	pNode->AppendChild( "Modifiers",		sModifiers );
	pNode->AppendChild( "DateTime",			dateTime.GetString() );
	pNode->AppendChild( "PlayerGuid",		sPlayerGuid );
	pNode->AppendChild( "MachineGuid",		sMachineGuid );
	pNode->AppendChild( "ProductID",		iProductID );
	XNode* pTapNoteScores = pNode->AppendChild( "TapNoteScores" );
	FOREACH_ENUM( TapNoteScore, tns )
		if( tns != TNS_None )	// HACK: don't save meaningless "none" count
			pTapNoteScores->AppendChild( TapNoteScoreToString(tns), iTapNoteScores[tns] );
	XNode* pHoldNoteScores = pNode->AppendChild( "HoldNoteScores" );
	FOREACH_ENUM( HoldNoteScore, hns )
		if( hns != HNS_None )	// HACK: don't save meaningless "none" count
			pHoldNoteScores->AppendChild( HoldNoteScoreToString(hns), iHoldNoteScores[hns] );
	pNode->AppendChild( radarValues.CreateNode(bWriteSimpleValues, bWriteComplexValues) );
	pNode->AppendChild( "LifeRemainingSeconds",	fLifeRemainingSeconds );
	pNode->AppendChild( "Disqualified",		bDisqualified);

	return pNode;
}

void HighScoreImpl::LoadFromNode( const XNode *pNode )
{
	ASSERT( pNode->GetName() == "HighScore" );

	RString s;

	pNode->GetChildValue( "Name", sName );
	pNode->GetChildValue( "Grade", s );
	grade = StringToGrade( s );
	pNode->GetChildValue( "Score",			iScore );
	pNode->GetChildValue( "PercentDP",		fPercentDP );
	pNode->GetChildValue( "SurviveSeconds",		fSurviveSeconds );
	pNode->GetChildValue( "MaxCombo",		iMaxCombo );
	pNode->GetChildValue( "StageAward",		s ); stageAward = StringToStageAward(s);
	pNode->GetChildValue( "PeakComboAward",	s ); peakComboAward = StringToPeakComboAward(s);
	pNode->GetChildValue( "Modifiers",		sModifiers );
	pNode->GetChildValue( "DateTime",		s ); dateTime.FromString( s );
	pNode->GetChildValue( "PlayerGuid",		sPlayerGuid );
	pNode->GetChildValue( "MachineGuid",		sMachineGuid );
	pNode->GetChildValue( "ProductID",		iProductID );
	const XNode* pTapNoteScores = pNode->GetChild( "TapNoteScores" );
	if( pTapNoteScores )
		FOREACH_ENUM( TapNoteScore, tns )
			pTapNoteScores->GetChildValue( TapNoteScoreToString(tns), iTapNoteScores[tns] );
	const XNode* pHoldNoteScores = pNode->GetChild( "HoldNoteScores" );
	if( pHoldNoteScores )
		FOREACH_ENUM( HoldNoteScore, hns )
			pHoldNoteScores->GetChildValue( HoldNoteScoreToString(hns), iHoldNoteScores[hns] );
	const XNode* pRadarValues = pNode->GetChild( "RadarValues" );
	if( pRadarValues )
		radarValues.LoadFromNode( pRadarValues );
	pNode->GetChildValue( "LifeRemainingSeconds",	fLifeRemainingSeconds );
	pNode->GetChildValue( "Disqualified",		bDisqualified);

	// Validate input.
	grade = clamp( grade, Grade_Tier01, Grade_Failed );
}

REGISTER_CLASS_TRAITS( HighScoreImpl, new HighScoreImpl(*pCopy) )

HighScore::HighScore()
{
	m_Impl = new HighScoreImpl;
}

void HighScore::Unset()
{
	m_Impl = new HighScoreImpl;
}

bool HighScore::IsEmpty() const
{
	if(	m_Impl->iTapNoteScores[TNS_W1] ||
		m_Impl->iTapNoteScores[TNS_W2] ||
		m_Impl->iTapNoteScores[TNS_W3] ||
		m_Impl->iTapNoteScores[TNS_W4] ||
		m_Impl->iTapNoteScores[TNS_W5] )
		return false;
	if( m_Impl->iHoldNoteScores[HNS_Held] > 0 )
		return false;
	return true;
}

RString	HighScore::GetName() const { return m_Impl->sName; }
Grade HighScore::GetGrade() const { return m_Impl->grade; }
unsigned int HighScore::GetScore() const { return m_Impl->iScore; }
unsigned int HighScore::GetMaxCombo() const { return m_Impl->iMaxCombo; }
StageAward HighScore::GetStageAward() const { return m_Impl->stageAward; }
PeakComboAward HighScore::GetPeakComboAward() const { return m_Impl->peakComboAward; }
float HighScore::GetPercentDP() const { return m_Impl->fPercentDP; }
float HighScore::GetSurviveSeconds() const { return m_Impl->fSurviveSeconds; }
float HighScore::GetSurvivalSeconds() const { return GetSurviveSeconds() + GetLifeRemainingSeconds(); }
RString HighScore::GetModifiers() const { return m_Impl->sModifiers; }
DateTime HighScore::GetDateTime() const { return m_Impl->dateTime; }
RString HighScore::GetPlayerGuid() const { return m_Impl->sPlayerGuid; }
RString HighScore::GetMachineGuid() const { return m_Impl->sMachineGuid; }
int HighScore::GetProductID() const { return m_Impl->iProductID; }
int HighScore::GetTapNoteScore( TapNoteScore tns ) const { return m_Impl->iTapNoteScores[tns]; }
int HighScore::GetHoldNoteScore( HoldNoteScore hns ) const { return m_Impl->iHoldNoteScores[hns]; }
const RadarValues &HighScore::GetRadarValues() const { return m_Impl->radarValues; }
float HighScore::GetLifeRemainingSeconds() const { return m_Impl->fLifeRemainingSeconds; }
bool HighScore::GetDisqualified() const { return m_Impl->bDisqualified; }

void HighScore::SetName( const RString &sName ) { m_Impl->sName = sName; }
void HighScore::SetGrade( Grade g ) { m_Impl->grade = g; }
void HighScore::SetScore( unsigned int iScore ) { m_Impl->iScore = iScore; }
void HighScore::SetMaxCombo( unsigned int i ) { m_Impl->iMaxCombo = i; }
void HighScore::SetStageAward( StageAward a ) { m_Impl->stageAward = a; }
void HighScore::SetPeakComboAward( PeakComboAward a ) { m_Impl->peakComboAward = a; }
void HighScore::SetPercentDP( float f ) { m_Impl->fPercentDP = f; }
void HighScore::SetAliveSeconds( float f ) { m_Impl->fSurviveSeconds = f; }
void HighScore::SetModifiers( RString s ) { m_Impl->sModifiers = s; }
void HighScore::SetDateTime( DateTime d ) { m_Impl->dateTime = d; }
void HighScore::SetPlayerGuid( RString s ) { m_Impl->sPlayerGuid = s; }
void HighScore::SetMachineGuid( RString s ) { m_Impl->sMachineGuid = s; }
void HighScore::SetProductID( int i ) { m_Impl->iProductID = i; }
void HighScore::SetTapNoteScore( TapNoteScore tns, int i ) { m_Impl->iTapNoteScores[tns] = i; }
void HighScore::SetHoldNoteScore( HoldNoteScore hns, int i ) { m_Impl->iHoldNoteScores[hns] = i; }
void HighScore::SetRadarValues( const RadarValues &rv ) { m_Impl->radarValues = rv; }
void HighScore::SetLifeRemainingSeconds( float f ) { m_Impl->fLifeRemainingSeconds = f; }
void HighScore::SetDisqualified( bool b ) { m_Impl->bDisqualified = b; }

/* We normally don't give direct access to the members.  We need this one
 * for NameToFillIn; use a special accessor so it's easy to find where this
 * is used. */
RString *HighScore::GetNameMutable() { return &m_Impl->sName; }

bool HighScore::operator<(HighScore const& other) const
{
	/* Make sure we treat AAAA as higher than AAA, even though the score
 	 * is the same. */
	if( PREFSMAN->m_bPercentageScoring )
	{
		if( GetPercentDP() != other.GetPercentDP() )
			return GetPercentDP() < other.GetPercentDP();
	}
	else
	{
		if( GetScore() != other.GetScore() )
			return GetScore() < other.GetScore();
	}

	return GetGrade() < other.GetGrade();
}

bool HighScore::operator>(HighScore const& other) const
{
	return other.operator<(*this);
}

bool HighScore::operator<=( const HighScore& other ) const
{
	return !operator>(other);
}

bool HighScore::operator>=( const HighScore& other ) const
{
	return !operator<(other);
}

bool HighScore::operator==( const HighScore& other ) const 
{
	return *m_Impl == *other.m_Impl;
}

bool HighScore::operator!=( const HighScore& other ) const
{
	return !operator==(other);
}

XNode* HighScore::CreateNode() const
{
	return m_Impl->CreateNode();
}

void HighScore::LoadFromNode( const XNode* pNode ) 
{
	m_Impl->LoadFromNode( pNode );
}

RString HighScore::GetDisplayName() const
{
	if( GetName().empty() )
		return EMPTY_NAME;
	else
		return GetName();
}

/* begin HighScoreList */
void HighScoreList::Init()
{
	iNumTimesPlayed = 0;
	vHighScores.clear();
	HighGrade = Grade_NoData;
}

void HighScoreList::AddHighScore( HighScore hs, int &iIndexOut, bool bIsMachine )
{
	int i;
	for( i=0; i<(int)vHighScores.size(); i++ )
	{
		if( hs >= vHighScores[i] )
			break;
	}
	const int iMaxScores = bIsMachine ? 
		PREFSMAN->m_iMaxHighScoresPerListForMachine : 
		PREFSMAN->m_iMaxHighScoresPerListForPlayer;
	if( i < iMaxScores )
	{
		vHighScores.insert( vHighScores.begin()+i, hs );
		iIndexOut = i;

		// Delete extra machine high scores in RemoveAllButOneOfEachNameAndClampSize
		// and not here so that we don't end up with less than iMaxScores after 
		// removing HighScores with duplicate names.
		//
		if( !bIsMachine )
			ClampSize( bIsMachine );
	}
	HighGrade = min( hs.GetGrade(), HighGrade );
}

void HighScoreList::IncrementPlayCount( DateTime _dtLastPlayed )
{
	dtLastPlayed = _dtLastPlayed;
	iNumTimesPlayed++;
}

const HighScore& HighScoreList::GetTopScore() const
{
	if( vHighScores.empty() )
	{
		static HighScore hs;
		hs = HighScore();
		return hs;
	}
	else
	{
		return vHighScores[0];
	}
}

XNode* HighScoreList::CreateNode() const
{
	XNode* pNode = new XNode( "HighScoreList" );

	pNode->AppendChild( "NumTimesPlayed", iNumTimesPlayed );
	pNode->AppendChild( "LastPlayed", dtLastPlayed.GetString() );
	if( HighGrade != Grade_NoData )
		pNode->AppendChild( "HighGrade", GradeToString(HighGrade) );

	for( unsigned i=0; i<vHighScores.size(); i++ )
	{
		const HighScore &hs = vHighScores[i];
		pNode->AppendChild( hs.CreateNode() );
	}

	return pNode;
}

void HighScoreList::LoadFromNode( const XNode* pHighScoreList )
{
	Init();

	ASSERT( pHighScoreList->GetName() == "HighScoreList" );
	FOREACH_CONST_Child( pHighScoreList, p )
	{
		const RString &name = p->GetName();
		if( name == "NumTimesPlayed" )
		{
			p->GetTextValue( iNumTimesPlayed );
		}
		else if( name == "LastPlayed" )
		{
			RString s;
			p->GetTextValue( s );
			dtLastPlayed.FromString( s );
		}
		else if( name == "HighGrade" )
		{
			RString s;
			p->GetTextValue( s );
			HighGrade = StringToGrade( s );
		}
		else if( name == "HighScore" )
		{
			vHighScores.resize( vHighScores.size()+1 );
			vHighScores.back().LoadFromNode( p );

			// ignore all high scores that are 0
			if( vHighScores.back().GetScore() == 0 )
				vHighScores.pop_back();
			else
				HighGrade = min( vHighScores.back().GetGrade(), HighGrade );
		}
	}
}

void HighScoreList::RemoveAllButOneOfEachName()
{
	for (vector<HighScore>::iterator i = vHighScores.begin(); i != vHighScores.end(); ++i)
	{
		for( vector<HighScore>::iterator j = i+1; j != vHighScores.end(); j++ )
		{
			if( i->GetName() == j->GetName() )
			{
				j--;
				vHighScores.erase( j+1 );
			}
		}
	}
}

void HighScoreList::ClampSize( bool bIsMachine )
{
	const int iMaxScores = bIsMachine ? 
		PREFSMAN->m_iMaxHighScoresPerListForMachine : 
		PREFSMAN->m_iMaxHighScoresPerListForPlayer;
	if( vHighScores.size() > unsigned(iMaxScores) )
		vHighScores.erase( vHighScores.begin()+iMaxScores, vHighScores.end() );
}

void HighScoreList::MergeFromOtherHSL(HighScoreList& other, bool is_machine)
{
	iNumTimesPlayed+= other.iNumTimesPlayed;
	if(other.dtLastPlayed > dtLastPlayed) { dtLastPlayed= other.dtLastPlayed; }
	if(other.HighGrade > HighGrade) { HighGrade= other.HighGrade; }
	vHighScores.insert(vHighScores.end(), other.vHighScores.begin(),
		other.vHighScores.end());
	std::sort(vHighScores.begin(), vHighScores.end());
	// Remove non-unique scores because they probably come from an accidental
	// repeated merge. -Kyz
	vector<HighScore>::iterator unique_end=
		std::unique(vHighScores.begin(), vHighScores.end());
	vHighScores.erase(unique_end, vHighScores.end());
	// Reverse it because sort moved the lesser scores to the top.
	std::reverse(vHighScores.begin(), vHighScores.end());
	ClampSize(is_machine);
}

XNode* Screenshot::CreateNode() const
{
	XNode* pNode = new XNode( "Screenshot" );

	// TRICKY:  Don't write "name to fill in" markers.
	pNode->AppendChild( "FileName",	sFileName );
	pNode->AppendChild( "MD5",	sMD5 );
	pNode->AppendChild( highScore.CreateNode() );

	return pNode;
}

void Screenshot::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->GetName() == "Screenshot" );

	pNode->GetChildValue( "FileName",	sFileName );
	pNode->GetChildValue( "MD5",		sMD5 );
	const XNode* pHighScore = pNode->GetChild( "HighScore" );
	if( pHighScore )
		highScore.LoadFromNode( pHighScore );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the HighScore. */
class LunaHighScore: public Luna<HighScore>
{
public:
	static int GetName( T* p, lua_State *L )			{ lua_pushstring(L, p->GetName() ); return 1; }
	static int GetScore( T* p, lua_State *L )			{ lua_pushnumber(L, p->GetScore() ); return 1; }
	static int GetPercentDP( T* p, lua_State *L )			{ lua_pushnumber(L, p->GetPercentDP() ); return 1; }
	static int GetDate( T* p, lua_State *L )			{ lua_pushstring(L, p->GetDateTime().GetString() ); return 1; }
	static int GetSurvivalSeconds( T* p, lua_State *L )			{ lua_pushnumber(L, p->GetSurvivalSeconds() ); return 1; }
	static int IsFillInMarker( T* p, lua_State *L )
	{
		bool bIsFillInMarker = false;
		FOREACH_PlayerNumber( pn )
			bIsFillInMarker |= p->GetName() == RANKING_TO_FILL_IN_MARKER[pn];
		lua_pushboolean( L, bIsFillInMarker );
		return 1;
	}
	static int GetMaxCombo( T* p, lua_State *L )			{ lua_pushnumber(L, p->GetMaxCombo() ); return 1; }
	static int GetModifiers( T* p, lua_State *L )			{ lua_pushstring(L, p->GetModifiers() ); return 1; }
	static int GetTapNoteScore( T* p, lua_State *L )			{ lua_pushnumber(L, p->GetTapNoteScore( Enum::Check<TapNoteScore>(L, 1) ) ); return 1; }
	static int GetHoldNoteScore( T* p, lua_State *L )			{ lua_pushnumber(L, p->GetHoldNoteScore( Enum::Check<HoldNoteScore>(L, 1) ) ); return 1; }
	static int GetRadarValues( T* p, lua_State *L )
	{
		RadarValues &rv = const_cast<RadarValues &>(p->GetRadarValues());
		rv.PushSelf(L);
		return 1;
	}
	DEFINE_METHOD( GetGrade, GetGrade() )
	DEFINE_METHOD( GetStageAward, GetStageAward() )
	DEFINE_METHOD( GetPeakComboAward, GetPeakComboAward() )

	LunaHighScore()
	{
		ADD_METHOD( GetName );
		ADD_METHOD( GetScore );
		ADD_METHOD( GetPercentDP );
		ADD_METHOD( GetDate );
		ADD_METHOD( GetSurvivalSeconds );
		ADD_METHOD( IsFillInMarker );
		ADD_METHOD( GetModifiers );
		ADD_METHOD( GetTapNoteScore );
		ADD_METHOD( GetHoldNoteScore );
		ADD_METHOD( GetRadarValues );
		ADD_METHOD( GetGrade );
		ADD_METHOD( GetMaxCombo );
		ADD_METHOD( GetStageAward );
		ADD_METHOD( GetPeakComboAward );
	}
};

LUA_REGISTER_CLASS( HighScore )

/** @brief Allow Lua to have access to the HighScoreList. */
class LunaHighScoreList: public Luna<HighScoreList>
{
public:
	static int GetHighScores( T* p, lua_State *L )
	{
		lua_newtable(L);
		for( int i = 0; i < (int) p->vHighScores.size(); ++i )
		{
			p->vHighScores[i].PushSelf(L);
			lua_rawseti( L, -2, i+1 );
		}

		return 1;
	}

	static int GetHighestScoreOfName( T* p, lua_State *L )
	{
		RString name= SArg(1);
		for(size_t i= 0; i < p->vHighScores.size(); ++i)
		{
			if(name == p->vHighScores[i].GetName())
			{
				p->vHighScores[i].PushSelf(L);
				return 1;
			}
		}
		lua_pushnil(L);
		return 1;
	}

	static int GetRankOfName( T* p, lua_State *L )
	{
		RString name= SArg(1);
		size_t rank= 0;
		for(size_t i= 0; i < p->vHighScores.size(); ++i)
		{
			if(name == p->vHighScores[i].GetName())
			{
				// Indices from Lua are one-indexed.  +1 to adjust.
				rank= i+1;
				break;
			}
		}
		// The themer is expected to check for validity before using.
		lua_pushnumber(L, rank);
		return 1;
	}

	LunaHighScoreList()
	{
		ADD_METHOD( GetHighScores );
		ADD_METHOD( GetHighestScoreOfName );
		ADD_METHOD( GetRankOfName );
	}
};

LUA_REGISTER_CLASS( HighScoreList )
// lua end

/*
 * (c) 2004 Chris Danford
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
