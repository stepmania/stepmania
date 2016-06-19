#include "global.h"
#include "LowLevelWindow_SDL.h"
#include "RageLog.h"
#include "RageException.h"
#include "archutils/Unix/X11Helper.h"
#include "PrefsManager.h" // XXX
#include "RageDisplay.h" // VideoModeParams
#include "DisplayResolutions.h"
#include "LocalizedString.h"

#include "RageDisplay_OGL_Helpers.h"
using namespace RageDisplay_Legacy_Helpers;

#include <stack>
#include <math.h>	// ceil()
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>


static SDL_Window* g_DisplayWindow;
static SDL_DisplayMode g_DisplayMode;
static SDL_GLContext g_Context;

static LocalizedString FAILED_WINDOW_SDL( "LowLevelWindow_SDL", "Failed to create an SDL window" );
LowLevelWindow_SDL::LowLevelWindow_SDL()
{
    if ( SDL_Init(SDL_INIT_VIDEO) != 0)
         RageException::Throw( "SDL Error in %s: %s",__FUNCTION__, SDL_GetError());

    //SDL_GetDesktopDisplayMode(1, &g_DisplayMode);

}

LowLevelWindow_SDL::~LowLevelWindow_SDL()
{
    SDL_Quit(); // Shutting down SDL properly (will restore resolution automatically
}

std::string LowLevelWindow_SDL::TryVideoMode( const VideoModeParams &p, bool &bNewDeviceOut )
{
    LOG->Trace("%s called", __FUNCTION__);




    g_DisplayMode.h = p.height;
    g_DisplayMode.w = p.width;
    g_DisplayMode.refresh_rate = p.rate;


    if (g_DisplayWindow == nullptr) // If there is no Window yet create one first.
    {

        //Create window
        g_DisplayWindow = SDL_CreateWindow( p.sWindowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g_DisplayMode.w, g_DisplayMode.h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
        if( g_DisplayWindow == NULL )
        {
            LOG->Warn( "%s SDL Error: %s\n",FAILED_WINDOW_SDL.GetValue().c_str(), SDL_GetError() );
            return "Failed to create SDL window";
        }
        //Create context
        auto g_Context = SDL_GL_CreateContext( g_DisplayWindow );
        if( g_Context == NULL )
        {
            LOG->Warn( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
            return "Failed to create OGL context";
        }

        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);

        if(SDL_GetWindowWMInfo(g_DisplayWindow,&info)) { // Retrieve the Window ID so that the X11 InputHandler can use this
            X11Helper::Win = info.info.x11.window;
            X11Helper::Dpy = info.info.x11.display;
        }
        else
        {
            LOG->Warn("Could not retrieve Window information: %s", SDL_GetError());
        }

    }


    if (p.windowed)
    {
        SDL_SetWindowFullscreen(g_DisplayWindow, 0);
        SDL_SetWindowSize(g_DisplayWindow, g_DisplayMode.w, g_DisplayMode.h);
    }
    else {
        SDL_SetWindowFullscreen(g_DisplayWindow, SDL_WINDOW_FULLSCREEN);
        if (SDL_SetWindowDisplayMode(g_DisplayWindow,&g_DisplayMode) != 0)
            LOG->Warn( "Error setting DisplayMode %s", SDL_GetError());
    }
    CurrentParams = p;

    if (g_DisplayMode.refresh_rate != 0) // If SDL does not specify a refresh rate assume 60
        CurrentParams.rate = g_DisplayMode.refresh_rate;
    else
        CurrentParams.rate = 60;

    if (p.vsync)
    {
        SDL_GL_SetSwapInterval(1);
    }
    else
    {
        SDL_GL_SetSwapInterval(0);
    }

    return ""; // Success
}

bool LowLevelWindow_SDL::IsSoftwareRenderer( std::string &sError )
{
    return false;
}

void LowLevelWindow_SDL::SwapBuffers()
{
    SDL_GL_SwapWindow(g_DisplayWindow);
}

void LowLevelWindow_SDL::GetDisplaySpecs( DisplaySpecs &out ) const
{
    int displayCount;
    SDL_DisplayMode mode;

    std::set<DisplayMode> outputSupported;

    displayCount = SDL_GetNumVideoDisplays();
    if (displayCount < 1)
    {
        LOG->Warn("SDL_GetNumVideoDisplays failed: %s", displayCount);
        return;
    }



    for ( int i = 0; i < displayCount; i++)
    {
        LOG->Trace("Checking modes for display %i", i);
        auto nummodes = SDL_GetNumDisplayModes(i);
        for (int j = 0; j < nummodes; j++){
            if (SDL_GetDisplayMode(i,j, &mode) != 0) {
                LOG->Warn("SDL_GetDisplayMode failed: %s", SDL_GetError());
                return;
            }
            LOG->Info(" Mode %d: %dx%d %dbpp %dHz", j, mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate);
            DisplayMode res = { mode.w, mode.h, mode.refresh_rate };
            outputSupported.insert( res );
        }
        const std::string outId(std::to_string(i));
        const std::string outName(SDL_GetDisplayName(i));
        out.insert( DisplaySpec( outId, outName, outputSupported ));
        outputSupported.clear();
    }

}

bool LowLevelWindow_SDL::SupportsFullscreenBorderlessWindow() const
{
    return true;
}


