#include "global.h"

/* This handler is used for odd cases where we don't use SDL for input. */

#include "InputHandler_Linux_tty.h"
#include "InputHandler_Linux_tty_keys.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"

#include "archutils/Unix/SignalHandler.h"

#include "SDL_utils.h"

#include <errno.h>

#include <sys/ioctl.h>
#include <fcntl.h>

#include <linux/kd.h>
#include <linux/keyboard.h>
#include <termios.h>


/* Map from keys (ignoring shifts) to SDLK values. */
static int keys[NR_KEYS];
static termios saved_kbd_termios;
static int saved_kbd_mode;

/* This is normally a singleton.  Keep track of it, so we can access it
 * from our signal handler. */
static InputHandler_Linux_tty *handler = NULL;

void InputHandler_Linux_tty::OnCrash(int signo)
{
	/* Make sure we delete the input handler if we crash, so we don't leave
	 * the terminal in raw mode. */
	delete handler;
	handler = NULL;
}


InputHandler_Linux_tty::InputHandler_Linux_tty()
{
	fd = open("/dev/tty", O_RDWR);
	if(fd == -1)
		RageException::Throw("open(\"/dev/tty\"): %s", strerror(errno));
	
	if (tcgetattr(fd, &saved_kbd_termios) == -1)
		RageException::Throw("tcgetattr(%i) failed: %s", fd, strerror(errno));
	termios keyboard_termios = saved_kbd_termios;
	keyboard_termios.c_lflag &= ~(ICANON | ECHO | ISIG);
	keyboard_termios.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
	keyboard_termios.c_cc[VMIN] = 0;
	keyboard_termios.c_cc[VTIME] = 0;
	if (tcsetattr(fd, TCSAFLUSH, &keyboard_termios) == -1)
		RageException::Throw("tcsetattr(%i, TCSAFLUSH) failed: %s", fd, strerror(errno));

	if (ioctl(fd, KDGKBMODE, &saved_kbd_mode) == -1)
		RageException::Throw("ioctl(%i, KDGKBMODE) failed: %s", fd, strerror(errno));
	if (ioctl(fd, KDSKBMODE, K_MEDIUMRAW) == -1)
		RageException::Throw("ioctl(%i, KDSKBMODE, K_MEDIUMRAW) failed: %s", fd, strerror(errno));

	if (ioctl(fd, KDSETMODE, KD_GRAPHICS) == -1 )
		RageException::Throw("ioctl(%i, KDSETMODE, KD_GRAPHICS) failed: %s", fd, strerror(errno));

	memset(keys, 0, sizeof(keys));

	for (int i = 0; i < NR_KEYS; ++i)
	{
		switch(i)
		{
		case SCANCODE_PRINTSCREEN:			keys[i] = SDLK_PRINT;	continue;
		case SCANCODE_BREAK:				keys[i] = SDLK_BREAK;	continue;
		case SCANCODE_BREAK_ALTERNATIVE:	keys[i] = SDLK_PAUSE;	continue;
		case SCANCODE_LEFTSHIFT:			keys[i] = SDLK_LSHIFT;	continue;
		case SCANCODE_RIGHTSHIFT:			keys[i] = SDLK_RSHIFT;	continue;
		case SCANCODE_LEFTCONTROL:			keys[i] = SDLK_LCTRL;	continue;
		case SCANCODE_RIGHTCONTROL:			keys[i] = SDLK_RCTRL;	continue;
		case SCANCODE_RIGHTWIN:				keys[i] = SDLK_RSUPER;	continue;
		case SCANCODE_LEFTWIN:				keys[i] = SDLK_LSUPER;	continue;
		case 127:							keys[i] = SDLK_MENU;	continue;
		}

		kbentry entry;

		entry.kb_table = 0;
		entry.kb_index = i;
		if (ioctl(fd, KDGKBENT, &entry))
			continue; /* error */

		const int kern_map = entry.kb_value;

		switch(kern_map)
		{
		case K_ENTER:	keys[i] = SDLK_RETURN; 		break;
		case K_F1:		keys[i] = SDLK_F1; 			break;
		case K_F2:		keys[i] = SDLK_F2; 			break;
		case K_F3:		keys[i] = SDLK_F3; 			break;
		case K_F4:		keys[i] = SDLK_F4; 			break;
		case K_F5:		keys[i] = SDLK_F5; 			break;
		case K_F6:		keys[i] = SDLK_F6; 			break;
		case K_F7:		keys[i] = SDLK_F7; 			break;
		case K_F8:		keys[i] = SDLK_F8; 			break;
		case K_F9:		keys[i] = SDLK_F9; 			break;
		case K_F10:		keys[i] = SDLK_F10;			break;
		case K_F11:		keys[i] = SDLK_F11;			break;
		case K_F12:		keys[i] = SDLK_F12;			break;
		case K_UP:		keys[i] = SDLK_UP;			break;
		case K_DOWN:	keys[i] = SDLK_DOWN;		break;
		case K_LEFT:	keys[i] = SDLK_LEFT;  		break;
		case K_RIGHT:	keys[i] = SDLK_RIGHT;		break;
		case K_P0:     	keys[i] = SDLK_KP0;			break;
		case K_P1:     	keys[i] = SDLK_KP1;			break;
		case K_P2:     	keys[i] = SDLK_KP2;			break;
		case K_P3:     	keys[i] = SDLK_KP3;			break;
		case K_P4:     	keys[i] = SDLK_KP4;			break;
		case K_P5:     	keys[i] = SDLK_KP5;			break;
		case K_P6:     	keys[i] = SDLK_KP6;			break;
		case K_P7:     	keys[i] = SDLK_KP7;			break;
		case K_P8:     	keys[i] = SDLK_KP8;			break;
		case K_P9:     	keys[i] = SDLK_KP9; 		break;
		case K_PPLUS:  	keys[i] = SDLK_KP_PLUS;		break;
		case K_PMINUS: 	keys[i] = SDLK_KP_MINUS;	break;
		case K_PSTAR:  	keys[i] = SDLK_KP_MULTIPLY;	break;
		case K_PSLASH: 	keys[i] = SDLK_KP_DIVIDE;	break;
		case K_PENTER: 	keys[i] = SDLK_KP_ENTER;	break;
		case K_PDOT:   	keys[i] = SDLK_KP_PERIOD;	break;
		case K_ALT:		keys[i] = SDLK_LALT;  		break;
		case K_ALTGR:	keys[i] = SDLK_RALT;  		break;
		case K_INSERT:	keys[i] = SDLK_INSERT;		break;
		case K_REMOVE:	keys[i] = SDLK_DELETE;  	break;
		case K_PGUP:	keys[i] = SDLK_PAGEUP;  	break;
		case K_PGDN:	keys[i] = SDLK_PAGEDOWN;	break;
		case K_FIND:	keys[i] = SDLK_HOME;    	break;
		case K_SELECT:	keys[i] = SDLK_END;     	break;
		case K_NUM:		keys[i] = SDLK_NUMLOCK;  	break;
		case K_CAPS:	keys[i] = SDLK_CAPSLOCK; 	break;
		case K_F13:		keys[i] = SDLK_PRINT;    	break;
		case K_HOLD:	keys[i] = SDLK_SCROLLOCK;	break;
		case K_PAUSE:	keys[i] = SDLK_PAUSE;    	break;
		case 127:		keys[i] = SDLK_BACKSPACE;	break;
		default: keys[i] = KVAL(kern_map);
		}
	}

	handler = this;
	SignalHandler::OnClose(OnCrash);
}


InputHandler_Linux_tty::~InputHandler_Linux_tty()
{
	LOG->Trace("~InputHandler_Linux_tty");
	ioctl(fd, KDSETMODE, KD_TEXT);
	ioctl(fd, KDSKBMODE, saved_kbd_mode);
	tcsetattr(fd, TCSAFLUSH, &saved_kbd_termios);
	close(fd);
	
	handler = NULL;
}

void InputHandler_Linux_tty::Update()
{
	while (1)
	{
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(fd, &fdset);

		struct timeval zero = {0,0};
		if ( select(fd+1, &fdset, NULL, NULL, &zero) <= 0 )
			return;

		unsigned char keybuf[BUFSIZ];
		SDL_keysym keysym;

		int ret = read(fd, keybuf, BUFSIZ);
		for ( int i=0; i < ret; ++i ) {
			const int key = keybuf[i] & 0x7F;
			const int butno = keys[key];
			const bool pressed = !(keybuf[i] & 0x80);
			
			ButtonPressed(DeviceInput(DEVICE_KEYBOARD, butno), pressed);
		}
	}

	InputHandler::UpdateTimer();
}

void InputHandler_Linux_tty::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	vDevicesOut.push_back( InputDevice(DEVICE_KEYBOARD) );
	vDescriptionsOut.push_back( "Keyboard" );
}

/*
 * (c) 2003-2004 Glenn Maynard
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
