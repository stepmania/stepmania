/* LowLevelWindow_SDL - SDL2 Window driver*/

#ifndef LOW_LEVEL_WINDOW_SDL_H
#define LOW_LEVEL_WINDOW_SDL_H

#include "RageDisplay.h" // VideoModeParams
#include "LowLevelWindow.h"

class LowLevelWindow_SDL : public LowLevelWindow
{
public:
    LowLevelWindow_SDL();
    ~LowLevelWindow_SDL();

    void *GetProcAddress(std::string s){ return nullptr; } // TODO: Windows
    std::string TryVideoMode(const VideoModeParams &p, bool &bNewDeviceOut);
    bool IsSoftwareRenderer( std::string &sError );
    void SwapBuffers();

    const ActualVideoModeParams GetActualVideoModeParams() const { return CurrentParams; }

    void GetDisplaySpecs(DisplaySpecs &out) const;

    bool SupportsFullscreenBorderlessWindow() const;

private:
	int GetSDLDisplayNum( const std::string displayID ) const;
    ActualVideoModeParams CurrentParams;
};

#ifdef ARCH_LOW_LEVEL_WINDOW
#error "More than one LowLevelWindow selected!"
#endif
#define ARCH_LOW_LEVEL_WINDOW LowLevelWindow_SDL

#endif
