#include "global.h"
#include "ScreenTestSound.h"
#include "RageDisplay.h"
#include "ScreenManager.h"
#include "RageSoundManager.h"
#include "ThemeManager.h"
#include "RageUtil.h"

ScreenTestSound::ScreenTestSound( CString sClassName ) : Screen( sClassName )
{	
	this->AddChild(&HEEEEEEEEELP);

	HEEEEEEEEELP.SetXY(450, 400);
	HEEEEEEEEELP.LoadFromFont( THEME->GetPathToF("Common normal") );
	HEEEEEEEEELP.SetZoom(.5);
	HEEEEEEEEELP.SetText(
		"p  Play\n"
		"s  Stop\n"
		"l  Set looping\n"
		"a  Set autostop\n"
		"c  Set continue");

	for( int i = 0; i < nsounds; ++i )
	{
		this->AddChild(&s[i].txt);
		s[i].txt.LoadFromFont( THEME->GetPathToF("Common normal") );
		s[i].txt.SetZoom(.5);
	}

	s[0].txt.SetXY(150, 100);
	s[1].txt.SetXY(450, 100);
	s[2].txt.SetXY(150, 250);
	s[3].txt.SetXY(450, 250);
	s[4].txt.SetXY(150, 400);

	s[0].s.Load("Themes/default/Sounds/_common menu music (loop).ogg");
	s[1].s.Load("Themes/default/Sounds/ScreenTitleMenu change.ogg");
	s[2].s.Load("Themes/default/Sounds/ScreenEvaluation try extra1.ogg");
	s[3].s.Load("Themes/default/Sounds/ScreenGameplay oni die.ogg");
	s[4].s.Load("Themes/default/Sounds/Common back.ogg");

//s[0].s.SetStartSeconds(45);
//s[0].s.SetPositionSeconds();
// s[4].s.SetLengthSeconds(1);
	RageSoundParams p;
	p.StopMode = RageSoundParams::M_STOP;
	// p.SetPlaybackRate( 1.20f );
	for( int i = 0; i < nsounds; ++i )
		s[i].s.SetParams( p );

//s[0].s.SetStopMode(RageSound::M_LOOP);
//s[0].s.Play();

	selected = 0;
	for( int i = 0; i < nsounds; ++i )
		UpdateText(i);
}


void ScreenTestSound::UpdateText(int n)
{
	CString fn = Basename( s[n].s.GetLoadedFilePath() );

	vector<RageSound *> snds;
	SOUNDMAN->GetCopies(s[n].s, snds);

	CString pos;
	for(unsigned p = 0; p < snds.size(); ++p)
	{
		if(p) pos += ", ";
		pos += ssprintf("%.3f", snds[p]->GetPositionSeconds());
	}

	s[n].txt.SetText(ssprintf(
		"%i: %s\n"
		"%s\n"
		"%s\n"
		"(%s)\n"
		"%s",
		n+1, fn.c_str(),
		s[n].s.IsPlaying()? "Playing":"Stopped",
		s[n].s.GetParams().StopMode == RageSoundParams::M_STOP?
			"Stop when finished":
		s[n].s.GetParams().StopMode == RageSoundParams::M_CONTINUE?
			"Continue until stopped":
			"Loop",
		pos.size()? pos.c_str(): "none playing",
		selected == n? "^^^^^^":""
		));
}

void ScreenTestSound::Update(float f)
{
	Screen::Update(f);
	for(int i = 0; i < nsounds; ++i)
		UpdateText(i);
}

void ScreenTestSound::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS )
		return;	// ignore

	switch( DeviceI.device)
	{
	case DEVICE_KEYBOARD:
		switch( DeviceI.button )
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5': selected = DeviceI.button - '0'-1; break;
		case 'p':
			s[selected].s.Play();
			break;
		case 's':
			s[selected].s.Stop();
			break;
		case 'l':
			{
				RageSoundParams p = s[selected].s.GetParams();
				p.StopMode = RageSoundParams::M_LOOP;
				s[selected].s.SetParams( p );
			}

			break;
		case 'a':
			{
				RageSoundParams p = s[selected].s.GetParams();
				p.StopMode = RageSoundParams::M_STOP;
				s[selected].s.SetParams( p );
			}
			break;
		case 'c':
			{
				RageSoundParams p = s[selected].s.GetParams();
				p.StopMode = RageSoundParams::M_CONTINUE;
				s[selected].s.SetParams( p );
			}
			break;

/*		case KEY_LEFT:
			obj.SetX(obj.GetX() - 10);
			break;
		case KEY_RIGHT:
			obj.SetX(obj.GetX() + 10);
			break;
		case KEY_UP:
			obj.SetY(obj.GetY() - 10);
			break;
		case KEY_DOWN:
			obj.SetY(obj.GetY() + 10);
			break;
*/
		}

	}

}
void ScreenTestSound::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );
}

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
