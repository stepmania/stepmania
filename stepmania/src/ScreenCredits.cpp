#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenCredits

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenCredits.h"
#include "ScreenManager.h"
#include "RageSounds.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include <math.h>
#include "Sprite.h"
#include "song.h"
#include "BitmapText.h"


#define BACKGROUNDS_BASE_X		THEME->GetMetricF("ScreenCredits","BackgroundsBaseX")
#define BACKGROUNDS_BASE_Y		THEME->GetMetricF("ScreenCredits","BackgroundsBaseY")
#define BACKGROUNDS_VELOCITY_X	THEME->GetMetricF("ScreenCredits","BackgroundsVelocityX")
#define BACKGROUNDS_VELOCITY_Y	THEME->GetMetricF("ScreenCredits","BackgroundsVelocityY")
#define BACKGROUNDS_SPACING_X	THEME->GetMetricF("ScreenCredits","BackgroundsSpacingX")
#define BACKGROUNDS_SPACING_Y	THEME->GetMetricF("ScreenCredits","BackgroundsSpacingY")
#define BACKGROUNDS_WIDTH		THEME->GetMetricF("ScreenCredits","BackgroundsWidth")
#define BACKGROUNDS_HEIGHT		THEME->GetMetricF("ScreenCredits","BackgroundsHeight")
#define TEXTS_COLOR_INTRO		THEME->GetMetricC("ScreenCredits","TextsColorIntro")
#define TEXTS_COLOR_HEADER		THEME->GetMetricC("ScreenCredits","TextsColorHeader")
#define TEXTS_COLOR_NORMAL		THEME->GetMetricC("ScreenCredits","TextsColorNormal")
#define TEXTS_ZOOM				THEME->GetMetricF("ScreenCredits","TextsZoom")
#define TEXTS_BASE_X			THEME->GetMetricF("ScreenCredits","TextsBaseX")
#define TEXTS_BASE_Y			THEME->GetMetricF("ScreenCredits","TextsBaseY")
#define TEXTS_VELOCITY_X		THEME->GetMetricF("ScreenCredits","TextsVelocityX")
#define TEXTS_VELOCITY_Y		THEME->GetMetricF("ScreenCredits","TextsVelocityY")
#define TEXTS_SPACING_X			THEME->GetMetricF("ScreenCredits","TextsSpacingX")
#define TEXTS_SPACING_Y			THEME->GetMetricF("ScreenCredits","TextsSpacingY")
#define NEXT_SCREEN				THEME->GetMetric ("ScreenCredits","NextScreen")

const int NUM_BACKGROUNDS = 20;

struct CreditLine
{
	int colorIndex;
	const char* text;
};

const CreditLine CREDIT_LINES[] = 
{
	{1,"STEPMANIA TEAM"},
	{0,""},
	{0,""},
	{0,""},
	{2,"☆☆GRAPHICS☆☆"},
	{0,"Lucas “v1ral” Tang"}, 
	// "Who?" = "what is this person's real name", not "what did they do".
	// Who knows? Some people might not want to reveal their real name. - Friez.
	// FYI: Several people have requested that their real names not be shown. -Chris
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
	{0,"Andrew “Frieza” Livy"},
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
	{0,"Random Hajile"}, // sent me 3DDX stuff - friez
	{0,"Chocobo Ghost"}, // sent me ez2 stuff - friez
	{0,"Tyma"}, // sent me popnmusic stuff - friez
	{0,"Lagged"},
	{0,"The Melting Pot"},
	{0,"DDRJamz Global BBS"},
	{0,"Eric “WaffleKing” Webster"},
	{0,"Gotetsu"},
	{0,"Mark “Foobly” Verrey"},
	{0,"Mandarin Blue"},
	{0,"Anne Kiel"},
	{0,"BeMaNiFiNiNaTiC (Jeremy Hine)"},
	{0,"Garett Sakamoto"},
	{0,"SailorBob"}, // isnt this the same as warrior bob?
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
const unsigned NUM_CREDIT_LINES = sizeof(CREDIT_LINES) / sizeof(CreditLine);


ScreenCredits::ScreenCredits( CString sName ) : Screen( sName )
{
	LOG->Trace( "ScreenCredits::ScreenCredits()" );

	int i;

	GAMESTATE->Reset();		// so that credits message for both players will show


	m_Background.LoadFromAniDir( THEME->GetPathToB("ScreenCredits background") );
	this->AddChild( &m_Background );

	vector<Song*> arraySongs;
	SONGMAN->GetSongs( arraySongs );
	SortSongPointerArrayByTitle( arraySongs );

	for( i=0; i<NUM_BACKGROUNDS; i++ )
	{
		Song* pSong = NULL;
		for( int j=0; j<50; j++ )
		{
			pSong = arraySongs[ rand()%arraySongs.size() ];
			if( pSong->HasBackground() )
				break;
		}

		Sprite* pBackground = new Sprite;
		pBackground->LoadBG( pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathToG("Common fallback background") );
		pBackground->ScaleToClipped( BACKGROUNDS_WIDTH, BACKGROUNDS_HEIGHT );
		m_vBackgrounds.push_back( pBackground );

		Sprite* pFrame = new Sprite;
		pFrame->Load( THEME->GetPathToG("ScreenCredits background frame") );
		m_vFrames.push_back( pFrame );
	}
	m_ScrollerBackgrounds.Load( m_vBackgrounds, BACKGROUNDS_BASE_X, BACKGROUNDS_BASE_Y, BACKGROUNDS_VELOCITY_X, BACKGROUNDS_VELOCITY_Y, BACKGROUNDS_SPACING_X, BACKGROUNDS_SPACING_Y );
	this->AddChild( &m_ScrollerBackgrounds );
	m_ScrollerFrames.Load( m_vFrames, BACKGROUNDS_BASE_X, BACKGROUNDS_BASE_Y, BACKGROUNDS_VELOCITY_X, BACKGROUNDS_VELOCITY_Y, BACKGROUNDS_SPACING_X, BACKGROUNDS_SPACING_Y );
	this->AddChild( &m_ScrollerFrames );
	

	for( i=0; i<int(NUM_CREDIT_LINES); i++ )
	{
		BitmapText* pText = new BitmapText;
		pText->LoadFromFont( THEME->GetPathToF("ScreenCredits titles") );
		pText->SetText( CREDIT_LINES[i].text );
		switch( CREDIT_LINES[i].colorIndex )
		{
		case 1:	pText->SetDiffuse( TEXTS_COLOR_INTRO );		break;
		case 2:	pText->SetDiffuse( TEXTS_COLOR_HEADER );	break;
		case 0:	pText->SetDiffuse( TEXTS_COLOR_NORMAL );	break;
		default:	ASSERT(0);
		}
		pText->SetZoom( TEXTS_ZOOM );
		m_vTexts.push_back( pText );
	}
	m_ScrollerTexts.Load( m_vTexts, TEXTS_BASE_X, TEXTS_BASE_Y, TEXTS_VELOCITY_X, TEXTS_VELOCITY_Y, TEXTS_SPACING_X, TEXTS_SPACING_Y );
	this->AddChild( &m_ScrollerTexts );
	

	m_Overlay.LoadFromAniDir( THEME->GetPathToB("ScreenCredits overlay") );
	this->AddChild( &m_Overlay );

	m_In.Load( THEME->GetPathToB("ScreenCredits in") );
	this->AddChild( &m_In );	// draw and update manually
	m_In.StartTransitioning();

	m_Out.Load( THEME->GetPathToB("ScreenCredits out") );
	this->AddChild( &m_Out );	// draw and update manually

	this->PostScreenMessage( SM_BeginFadingOut, m_ScrollerTexts.GetTotalSecsToScroll() );

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("credits") );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenCredits music") );
}

ScreenCredits::~ScreenCredits()
{
	unsigned i;
	
	for( i=0; i<m_vBackgrounds.size(); i++ )
		delete m_vBackgrounds[i];
	m_vBackgrounds.clear();

	for( i=0; i<m_vFrames.size(); i++ )
		delete m_vFrames[i];
	m_vFrames.clear();

	for( i=0; i<m_vTexts.size(); i++ )
		delete m_vTexts[i];
	m_vTexts.clear();
}

void ScreenCredits::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}	


void ScreenCredits::DrawPrimitives()
{
	Screen::DrawPrimitives();
}	


void ScreenCredits::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenCredits::Input()" );

	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenCredits::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:
		if( !m_Out.IsTransitioning() )
			m_Out.StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		SONGMAN->SaveMachineScoresToDisk();
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

void ScreenCredits::MenuStart( PlayerNumber pn )
{
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
}

void ScreenCredits::MenuBack( PlayerNumber pn )
{
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
}

