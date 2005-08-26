#include "global.h"
#include "ScreenCredits.h"
#include "GameSoundManager.h"
#include "RageLog.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "Sprite.h"
#include "song.h"
#include "BitmapText.h"
#include "ActorUtil.h"
#include "SongUtil.h"
#include "RageUtil.h"


#define BACKGROUNDS_SPACING_X				THEME->GetMetricF("ScreenCredits","BackgroundsSpacingX")
#define BACKGROUNDS_SPACING_Y				THEME->GetMetricF("ScreenCredits","BackgroundsSpacingY")
#define BACKGROUNDS_SCROLL_SECONDS_PER_ITEM	THEME->GetMetricF("ScreenCredits","BackgroundsScrollSecondsPerItem")
#define BACKGROUNDS_WIDTH					THEME->GetMetricF("ScreenCredits","BackgroundsWidth")
#define BACKGROUNDS_HEIGHT					THEME->GetMetricF("ScreenCredits","BackgroundsHeight")
#define TEXTS_INTRO_COMMAND					THEME->GetMetricA("ScreenCredits","TextsIntroCommand")
#define TEXTS_HEADER_COMMAND				THEME->GetMetricA("ScreenCredits","TextsHeaderCommand")
#define TEXTS_NORMAL_COMMAND				THEME->GetMetricA("ScreenCredits","TextsNormalCommand")
#define TEXTS_ZOOM							THEME->GetMetricF("ScreenCredits","TextsZoom")
#define TEXTS_SPACING_X						THEME->GetMetricF("ScreenCredits","TextsSpacingX")
#define TEXTS_SPACING_Y						THEME->GetMetricF("ScreenCredits","TextsSpacingY")
#define TEXTS_SCROLL_SECONDS_PER_ITEM		THEME->GetMetricF("ScreenCredits","TextsScrollSecondsPerItem")

const int NUM_BACKGROUNDS = 20;

struct CreditLine
{
	int colorIndex;
	const char* text;
};

static const CreditLine CREDIT_LINES[] = 
{
	{1,"STEPMANIA TEAM"},
	{0,""},
	{0,""},
	{0,""},
	{2,"☆☆GRAPHICS☆☆"},
	{0,"Lucas “v1ral” Tang"}, 
	// "Who?" = "what is this person's real name", not "what did they do".
	// Who knows? Some people might not want to reveal their real name. - Friez.
	// Several people have requested that their real names not be shown. -Chris
	// This isn't a good place to keep a master list of contributors because 
	// it's not a very complete list. -Chris
	{0,"SPiGuMuS"}, // who?
	{0,"Visage"}, // who? makes para para theme graphics for me, wants to do BM and IIDX graphics too. - Friez
	{0,"Ryan “DJ McFox” McKanna"},
	{0,"Lamden “Cloud34” Travis"}, // ddrmaniax admin / did a graphic or two for me - friez
	{0,"Michael “Redcrusher” Curry"}, // ez2 graphics - friez
	{0,"Steve “healing_vision” Klise"},
	{0,"Mauro Panichella"},
	{0,"Popnko"}, // who? makes popnmusic graphics for me - Friez
	{0,"Griever (Julian)"}, // who?
	{0,"Miguel Moore"},
	{0,"Dj “AeON ExOdus” Washington"},
	{0,"Xelf"}, // who?
	{0,"James “SPDS” Sanders"},
	{0,"k0ldx"}, // who?
	{0,""},
	{0,""},
	{0,""},
	{2,"☆☆WEB DESIGN☆☆"},
	{0,"Brian “Bork” Bugh"},
	{0,""},
	{0,""},
	{0,""},
	{2,"☆☆SOUND☆☆"},
	{0,"Kyle “KeeL” Ward"},
	{0,"Jim “Izlude” Cabeen"},
	{0,""},
	{0,""},
	{0,""},
	{2,"☆☆PROGRAMMING☆☆"},
	{0,"Chris Danford"},
	{0,"Frieza"},
	{0,"Glenn Maynard"},
	{0,"Bruno Figueiredo"},
	{0,"Peter “Dro Kulix” May"},
	{0,"Jared “nmspaz” Roberts"},
	{0,"Brendan “binarys” Walker"},
	{0,"Lance “Neovanglist” Gilbert"},
	{0,"Michel Donais"},
	{0,"Ben “Mantis” Nordstrom"},
	{0,"Chris “Parasyte” Gomez"},
	{0,"Michael “dirkthedaring” Patterson"},
	{0,"Sauleil “angedelamort” Lamarre"},

	
	{0,"Edwin Evans"},
	{0,"Brian “Bork” Bugh"},
	{0,"Joel “Elvis314” Maher"},
	{0,"Garth “Kefabi” Smith"},
	{0,"Pkillah"}, // who?
	{0,"Ryan “DJ McFox” McKanna"},
	{0,"Robert Kemmetmueller"},
	{0,"Ben “Shabach” Andersen"},
	{0,"Will “SlinkyWizard” Valladao"},
	{0,"TheChip"}, // who?
	{0,"David “WarriorBob” H"}, // who? // He does documentation on Stepmania when a release comes out. Thats who - Frieza
	{0,"Mike Waltson"},
	{0,"Kevin “Miryokuteki” Slaughter"},
	{0,"Thad “Coderjoe” Ward"},
	{0,"Steve Checkoway"},
	{0,"Sean Burke"},
	{0,"XPort"},
	{0,"Charles Lohr"},
	{0,"Josh “Axlecrusher” Allen"},
	{0,""},
	{0,""},
	{0,""},
	{2,"☆☆SPECIAL THANKS TO☆☆"},
	{0,"SimWolf"}, // dwi
	{0,"DJ DraftHorse"}, // also dwi
	{0,"Dance With Intensity"}, // DWI! 
	{0,"BeMaNiRuler"},
	{0,"DDRLlama"},
	{0,"DDRManiaX"}, // boards and all round support
	{0,"NMR"},
	{0,"DJ Paranoia"}, // pop'n stuff, general help - friez
	{0,"DJ Yuz"}, // same as above
	{0,"Reo"}, // pop'n stuff.
	{0,"Random Hajile"}, // sent me 3DDX stuff - friez
	{0,"Chocobo Ghost"}, // sent me ez2 stuff - friez
	{0,"Tyma"}, // sent me popnmusic stuff - friez
	{0,"Deluxe"}, // para para noteskins - friez
	{0,"Lagged"},
	{0,"The Melting Pot"},
	{0,"DDRJamz Global BBS"},
	{0,"Eric “WaffleKing” Webster"},
	{0,"Gotetsu"},
	{0,"Mark “Foobly” Verrey"},
	{0,"Mandarin Blue"},
//	{0,"SailorBob"}, // isnt this the same as warrior bob?  // I think so.  -Chris
	{0,"Kenny “AngelTK” Lai"},
	{0,"curewater"},
	{0,"Bill “DMAshura” Shillito"},
	{0,"Illusionz - Issaquah, WA"},
	{0,"Quarters - Kirkland, WA"},
	{0,"Pier Amusements - Bournemouth, UK"}, // Pump inspiration
	{0,"Westcliff Amusements - Bournemouth, UK"}, // Ez2dancer inspiriation
	{0,"Naoki"},
	{0,"Konami Computer Entertainment Japan"},
	{0,""},
	{0,""},
	{0,""},
	{0,""},
	{0,""},
	{0,""},
	{0,""},
	{0,""},
	{1,"Join the StepMania team"},
	{1,"and help us out!"},
};

REGISTER_SCREEN_CLASS( ScreenCredits );
ScreenCredits::ScreenCredits( CString sName ) : ScreenAttract( sName )
{
}

void ScreenCredits::Init()
{
	ScreenAttract::Init();

	vector<Song*> arraySongs;
	SONGMAN->GetSongs( arraySongs );
	SongUtil::SortSongPointerArrayByTitle( arraySongs );

	// FIXME:  Redo this screen with a BGA
	CString sBackgroundsTransformFunction = ssprintf(
		"function(self,offset,itemIndex,numItems) "
		"	self:x(%f*offset); "
		"	self:y(%f*offset); "
		"end",
		(float)BACKGROUNDS_SPACING_X, 
		(float)BACKGROUNDS_SPACING_Y );

	m_ScrollerBackgrounds.SetName( "Backgrounds" );
	m_ScrollerBackgrounds.Load3(
		BACKGROUNDS_SCROLL_SECONDS_PER_ITEM,
		4,
		false,
		sBackgroundsTransformFunction,
		false,
		false );
	SET_XY( m_ScrollerBackgrounds );
	this->AddChild( &m_ScrollerBackgrounds );

	m_ScrollerFrames.SetName( "Backgrounds" );
	m_ScrollerFrames.Load3(
		BACKGROUNDS_SCROLL_SECONDS_PER_ITEM,
		4,
		false,
		sBackgroundsTransformFunction,
		false,
		false );
	SET_XY( m_ScrollerFrames );
	this->AddChild( &m_ScrollerFrames );

	float fTime = 0;
	{
		for( int i=0; i<NUM_BACKGROUNDS; i++ )
		{
			Song* pSong = NULL;
			for( int j=0; j<50; j++ )
			{
				pSong = arraySongs[ rand()%arraySongs.size() ];
				if( pSong->HasBackground() )
					break;
			}

			Sprite* pBackground = new Sprite;
			pBackground->LoadBG( pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathG("Common","fallback background") );
			pBackground->ScaleToClipped( BACKGROUNDS_WIDTH, BACKGROUNDS_HEIGHT );
			m_ScrollerBackgrounds.AddChild( pBackground );

			Sprite* pFrame = new Sprite;
			pFrame->Load( THEME->GetPathG("ScreenCredits","background frame") );
			m_ScrollerFrames.AddChild( pFrame );
		}
		float fFirst = -2;
		float fLast = NUM_BACKGROUNDS+2;
		m_ScrollerBackgrounds.SetCurrentAndDestinationItem( fFirst );
		m_ScrollerBackgrounds.SetDestinationItem( fLast );

		m_ScrollerFrames.SetCurrentAndDestinationItem( fFirst );
		m_ScrollerFrames.SetDestinationItem( fLast );

		fTime = max( fTime, BACKGROUNDS_SCROLL_SECONDS_PER_ITEM*(fLast-fFirst) );
	}
	
	CString sTextsTransformFunction = ssprintf(
		"function(self,offset,itemIndex,numItems) "
		"	self:x(%f*offset); "
		"	self:y(%f*offset); "
		"end",
		(float)TEXTS_SPACING_X, 
		(float)TEXTS_SPACING_Y );

	m_ScrollerTexts.SetName( "Texts" );
	m_ScrollerTexts.Load3(
		TEXTS_SCROLL_SECONDS_PER_ITEM,
		40,
		false,
		sTextsTransformFunction,
		false,
		false );
	SET_XY( m_ScrollerTexts );
	this->AddChild( &m_ScrollerTexts );
	
	{
		for( unsigned i=0; i<ARRAYSIZE(CREDIT_LINES); i++ )
		{
			BitmapText* pText = new BitmapText;
			pText->LoadFromFont( THEME->GetPathF("ScreenCredits","titles") );
			pText->SetText( CREDIT_LINES[i].text );
			switch( CREDIT_LINES[i].colorIndex )
			{
			case 1:	pText->RunCommands( TEXTS_INTRO_COMMAND );		break;
			case 2:	pText->RunCommands( TEXTS_HEADER_COMMAND );	break;
			case 0:	pText->RunCommands( TEXTS_NORMAL_COMMAND );	break;
			default:	ASSERT(0);
			}
			pText->SetZoom( TEXTS_ZOOM );
			m_ScrollerTexts.AddChild( pText );
		}

		float fFirst = -10;
		float fLast = ARRAYSIZE(CREDIT_LINES)+10;
		m_ScrollerTexts.SetCurrentAndDestinationItem( fFirst );
		m_ScrollerTexts.SetDestinationItem( fLast );
		fTime = max( fTime, TEXTS_SCROLL_SECONDS_PER_ITEM*(fLast-fFirst) );
	}

	this->SortByDrawOrder();

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow
	this->PostScreenMessage( SM_BeginFadingOut, fTime );
	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("credits") );
}

ScreenCredits::~ScreenCredits()
{
	m_ScrollerBackgrounds.DeleteAllChildren();
	m_ScrollerFrames.DeleteAllChildren();
	m_ScrollerTexts.DeleteAllChildren();
}


void ScreenCredits::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenAttract::HandleScreenMessage( SM );
}

/*
 * (c) 2003-2004 Chris Danford, Glenn Maynard
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
