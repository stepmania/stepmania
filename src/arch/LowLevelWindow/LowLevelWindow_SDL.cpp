#include "global.h"
#include "LowLevelWindow_SDL.h"
#include "RageLog.h"
#include "RageException.h"
#if defined(X11_FOUND)
#include "archutils/Unix/X11Helper.h"
#endif

#if defined (WINDOWS)
#include "archutils/Win32/GraphicsWindow.h"
#include "archutils/Win32/Crash.h"
#endif

#include "PrefsManager.h" // XXX
#include "DisplaySpec.h"
#include "RageDisplay.h" // VideoModeParams
#include "DisplayResolutions.h"
#include "LocalizedString.h"
#include "arch/InputHandler/InputHandler_SDL.h"

#include "RageDisplay_OGL_Helpers.h"
using namespace RageDisplay_Legacy_Helpers;

#include <stack>
#include <math.h>	// ceil()
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>



#include "arch/ArchHooks/ArchHooks.h"


static SDL_Window* g_DisplayWindow;
static SDL_DisplayMode g_DisplayMode;
static SDL_GLContext g_Context;
static SDL_Event g_Event;

static LocalizedString FAILED_WINDOW_SDL( "LowLevelWindow_SDL", "Failed to create an SDL window" );
LowLevelWindow_SDL::LowLevelWindow_SDL()
{
    if ( SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
         RageException::Throw( "SDL Error in %s: %s",__FUNCTION__, SDL_GetError());

    //SDL_GetDesktopDisplayMode(1, &g_DisplayMode);

}

LowLevelWindow_SDL::~LowLevelWindow_SDL()
{
    SDL_Quit(); // Shutting down SDL properly (will restore resolution automatically
}

int LowLevelWindow_SDL::GetSDLDisplayNum( const std::string displayId ) const
{
    auto target = 0; // Default to using SDL display 0
    try
    {
        int requested = std::stoi( displayId );
        int maxvalid = SDL_GetNumVideoDisplays();
        if (requested > maxvalid)
        {
            throw std::invalid_argument( "Display index out of range" );
        }
        target = requested;
    } catch (...)
    {
        LOG->Warn( "Unrecognized Display ID %s", displayId );
    }
    return target;
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
        g_DisplayWindow = SDL_CreateWindow( p.sWindowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g_DisplayMode.w, g_DisplayMode.h, SDL_WINDOW_OPENGL );
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
#if defined(X11_FOUND)
            X11Helper::Win = info.info.x11.window;
            X11Helper::Dpy = info.info.x11.display;
#endif
#if defined(WINDOWS)
			GraphicsWindow::SetHwnd(info.info.win.window); // So that DInput knows about our SDL created window
			//CrashHandler::SetForegroundWindow(info.info.win.window);
#endif
        }
        else
        {
            LOG->Warn("Could not retrieve Window information: %s", SDL_GetError());
        }

    }

	
    if (p.windowed && !p.bWindowIsFullscreenBorderless)
    {
        SDL_SetWindowFullscreen(g_DisplayWindow, 0);
        SDL_SetWindowSize(g_DisplayWindow, g_DisplayMode.w, g_DisplayMode.h);
		SDL_SetWindowPosition(g_DisplayWindow, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED);
    }
    else
    {
		auto target = GetSDLDisplayNum( p.sDisplayId );
        SDL_DisplayMode cur_mode;
        SDL_Rect cur_bounds;
        // Reset fullscreen, move window onto desired monitor, then set fullscreen mode accordingly
        SDL_SetWindowFullscreen( g_DisplayWindow, 0 );
        SDL_GetCurrentDisplayMode( target, &cur_mode );
        SDL_GetDisplayBounds( target, &cur_bounds );
        SDL_SetWindowPosition( g_DisplayWindow, cur_bounds.x, cur_bounds.y );

        if (p.bWindowIsFullscreenBorderless)
        {
            SDL_SetWindowSize( g_DisplayWindow, cur_mode.w, cur_mode.h );
            g_DisplayMode.w = cur_mode.w;
            g_DisplayMode.h = cur_mode.h;
            if (SDL_SetWindowFullscreen( g_DisplayWindow, SDL_WINDOW_FULLSCREEN_DESKTOP ) != 0)
                return fmt::sprintf( "Failed to set borderless fullscreen mode: %s", SDL_GetError());
        }
        else // Fullscreen exclusive
        {
            // Display mode takes effect at next transition to fullscreen
            if (SDL_SetWindowDisplayMode( g_DisplayWindow, &g_DisplayMode ) != 0)
                LOG->Warn( "Error setting DisplayMode %s", SDL_GetError());
            if (SDL_SetWindowFullscreen( g_DisplayWindow, SDL_WINDOW_FULLSCREEN ) != 0)
                return fmt::sprintf( "Failed to set display mode: %s", SDL_GetError() );
        }
    }
    CurrentParams = p;
    CurrentParams.windowWidth = g_DisplayMode.w;
    CurrentParams.windowHeight = g_DisplayMode.h;
    if (CurrentParams.windowHeight != CurrentParams.height || CurrentParams.windowWidth != CurrentParams.width)
    {
        CurrentParams.renderOffscreen = true;
    }

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
    if (!has_sdl_input) {
        while (SDL_PollEvent(&g_Event) != 0)
        {
            if (g_Event.type == SDL_QUIT)
            {
                LOG->Trace("SDL_QUIT: shutting down");
                ArchHooks::SetUserQuit();
            }
        }
    }
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
            DisplayMode res = {static_cast<unsigned int> (mode.w), static_cast<unsigned int> (mode.h),
                               static_cast<double> (mode.refresh_rate)};
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


