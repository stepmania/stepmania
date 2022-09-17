#include "global.h"
#include "NotesLoaderJson.h"
#include "json/json.h"
#include "TimingData.h"
#include "RageUtil.h"
#include "JsonUtil.h"
#include "BackgroundUtil.h"
#include "NoteData.h"
#include "Song.h"
#include "Steps.h"
#include "GameManager.h"

void NotesLoaderJson::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.json"), out );
}

static void Deserialize( TimingSegment *seg, const Json::Value &root )
{
	switch( seg->GetType() )
	{
		case SEGMENT_BPM:
		{
			float fBPM = root["BPM"].asDouble();
			static_cast<BPMSegment *>(seg)->SetBPM(fBPM);
			break;
		}
		case SEGMENT_STOP:
		{
			float fStop = root["Seconds"].asDouble();
			static_cast<StopSegment *>(seg)->SetPause(fStop);
			break;
		}
		default: break; // The rest are unused.
	}
}

static void Deserialize(BPMSegment &seg, const Json::Value &root)
{
	Deserialize( static_cast<TimingSegment*>(&seg), root );
}

static void Deserialize(StopSegment &seg, const Json::Value &root)
{
	Deserialize( static_cast<TimingSegment*>(&seg), root );
}

static void Deserialize(TimingData &td, const Json::Value &root)
{
	vector<BPMSegment*> vBPMs;
	vector<StopSegment*> vStops;
	JsonUtil::DeserializeVectorPointers( vBPMs, Deserialize, root["BpmSegments"] );
	JsonUtil::DeserializeVectorPointers( vStops, Deserialize, root["StopSegments"] );

	for( unsigned i = 0; i < vBPMs.size(); ++i )
	{
		td.AddSegment( *vBPMs[i] );
		delete vBPMs[i];
	}
	for( unsigned i = 0; i < vStops.size(); ++i )
	{
		td.AddSegment( *vStops[i] );
		delete vStops[i];
	}
}

static void Deserialize(LyricSegment &o, const Json::Value &root)
{
	o.m_fStartTime = (float)root["StartTime"].asDouble();
	o.m_sLyric = root["Lyric"].asString();
	o.m_Color.FromString( root["Color"].asString() );
}

static void Deserialize(BackgroundDef &o, const Json::Value &root)
{
	o.m_sEffect = root["Effect"].asString();
	o.m_sFile1 = root["File1"].asString();
	o.m_sFile2 = root["File2"].asString();
	o.m_sColor1 = root["Color1"].asString();
}

static void Deserialize(BackgroundChange &o, const Json::Value &root )
{
	Deserialize( o.m_def, root["Def"] );
	o.m_fStartBeat = (float)root["StartBeat"].asDouble();
	o.m_fRate = (float)root["Rate"].asDouble();
	o.m_sTransition = root["Transition"].asString();
}

static void Deserialize( TapNote &o, const Json::Value &root )
{
	//if( o.type != TapNoteType_Tap )
	if( root.isInt() )
		o.type = (TapNoteType)root["Type"].asInt();
	//if( o.type == TapNoteType_HoldHead )
		o.subType = (TapNoteSubType)root["SubType"].asInt();
	//root["Source"] = (int)source;
	//if( !o.sAttackModifiers.empty() )
		o.sAttackModifiers = root["AttackModifiers"].asString();
	//if( o.fAttackDurationSeconds > 0 )
		o.fAttackDurationSeconds = (float)root["AttackDurationSeconds"].asDouble();
	//if( o.bKeysound )
		o.iKeysoundIndex = root["KeysoundIndex"].asInt();
	//if( o.iDuration > 0 )
		o.iDuration = root["Duration"].asInt();
	//if( o.pn != PLAYER_INVALID )
		o.pn = (PlayerNumber)root["PlayerNumber"].asInt();
}

static void Deserialize( StepsType st, NoteData &nd, const Json::Value &root )
{
	int iTracks = nd.GetNumTracks();
	nd.SetNumTracks( iTracks );
	for( unsigned i=0; i<root.size(); i++ )
	{
		Json::Value root2 = root[i];
		float fBeat = (float)root2[(unsigned)0].asDouble();
		int iRow = BeatToNoteRow(fBeat);
		int iTrack = root2[1].asInt();
		const Json::Value &root3 = root2[2];
		TapNote tn;
		Deserialize( tn, root3 );
		nd.SetTapNote( iTrack, iRow, tn );
	}
}

static void Deserialize( RadarValues &o, const Json::Value &root )
{
	FOREACH_ENUM( RadarCategory, rc )
	{
		o[rc] = (float)root[ RadarCategoryToString(rc) ].asDouble();
	}
}

static void Deserialize( Steps &o, const Json::Value &root )
{
	o.m_StepsType = GAMEMAN->StringToStepsType(root["StepsType"].asString());

	o.Decompress();

	NoteData nd;
	Deserialize( o.m_StepsType, nd, root["NoteData"] );
	o.SetNoteData( nd );
	//o.SetHash( root["Hash"].asInt() );
	o.SetDescription( root["Description"].asString() );
	o.SetDifficulty( StringToDifficulty(root["Difficulty"].asString()) );
	o.SetMeter( root["Meter"].asInt() );

	RadarValues rv[NUM_PLAYERS];
	FOREACH_PlayerNumber( pn )
	{
		Deserialize( rv[pn], root["RadarValues"] );
	}
	o.SetCachedRadarValues( rv );
}

static void Deserialize( Song &out, const Json::Value &root )
{
	out.SetSongDir( root["SongDir"].asString() );
	out.m_sGroupName = root["GroupName"].asString();
	out.m_sMainTitle = root["Title"].asString();
	out.m_sSubTitle = root["SubTitle"].asString();
	out.m_sArtist = root["Artist"].asString();
	out.m_sMainTitleTranslit = root["TitleTranslit"].asString();
	out.m_sSubTitleTranslit = root["SubTitleTranslit"].asString();
	out.m_sGenre = root["Genre"].asString();
	out.m_sCredit = root["Credit"].asString();
	out.m_sBannerFile = root["Banner"].asString();
	out.m_sBackgroundFile = root["Background"].asString();
	out.m_sLyricsFile = root["LyricsFile"].asString();
	out.m_sCDTitleFile = root["CDTitle"].asString();
	out.m_sMusicFile = root["Music"].asString();
	out.m_SongTiming.m_fBeat0OffsetInSeconds = (float)root["Offset"].asDouble();
	out.m_fMusicSampleStartSeconds = (float)root["SampleStart"].asDouble();
	out.m_fMusicSampleLengthSeconds = (float)root["SampleLength"].asDouble();
	RString sSelectable = root["Selectable"].asString();
	if( sSelectable.EqualsNoCase("YES") )
		out.m_SelectionDisplay = out.SHOW_ALWAYS;
	else if( sSelectable.EqualsNoCase("NO") )
		out.m_SelectionDisplay = out.SHOW_NEVER;

	out.m_sSongFileName = root["SongFileName"].asString();
	out.m_bHasMusic = root["HasMusic"].asBool();
	out.m_bHasBanner = root["HasBanner"].asBool();
	out.m_fMusicLengthSeconds = (float)root["MusicLengthSeconds"].asDouble();

	RString sDisplayBPMType = root["DisplayBpmType"].asString();
	if( sDisplayBPMType == "*" )
		out.m_DisplayBPMType = DISPLAY_BPM_RANDOM;
	else
		out.m_DisplayBPMType = DISPLAY_BPM_SPECIFIED;

	if( out.m_DisplayBPMType == DISPLAY_BPM_SPECIFIED )
	{
		out.m_fSpecifiedBPMMin = (float)root["SpecifiedBpmMin"].asDouble();
		out.m_fSpecifiedBPMMax = (float)root["SpecifiedBpmMax"].asDouble();
	}

	Deserialize( out.m_SongTiming, root["TimingData"] );
	JsonUtil::DeserializeVectorObjects( out.m_LyricSegments, Deserialize, root["LyricSegments"] );

	{
		const Json::Value &root2 = root["BackgroundChanges"];
		FOREACH_BackgroundLayer( bl )
		{
			const Json::Value &root3 = root2[bl];
			vector<BackgroundChange> &vBgc = out.GetBackgroundChanges(bl);
			JsonUtil::DeserializeVectorObjects( vBgc, Deserialize, root3 );
		}
	}

	{
		vector<BackgroundChange> &vBgc = out.GetForegroundChanges();
		JsonUtil::DeserializeVectorObjects( vBgc, Deserialize, root["ForegroundChanges"] );
	}

	out.m_vsKeysoundFile = JsonUtil::DeserializeArrayStrings(root["KeySounds"]);

	{
		vector<Steps*> vpSteps;
		JsonUtil::DeserializeVectorPointersParam<Steps,Song*>( vpSteps, Deserialize, root["Charts"], &out );
		for (Steps *step : vpSteps)
			out.AddSteps( step );
	}
}

bool NotesLoaderJson::LoadFromJsonFile( const RString &sPath, Song &out )
{
	Json::Value root;
	if( !JsonUtil::LoadFromFileShowErrors(root,sPath) )
		return false;

	Deserialize(out, root);

	return true;
}

bool NotesLoaderJson::LoadFromDir( const RString &sPath, Song &out )
{
	return LoadFromJsonFile(sPath, out);
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
