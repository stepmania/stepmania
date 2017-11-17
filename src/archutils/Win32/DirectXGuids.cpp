#include "global.h"

// The purpose of this file is to define the various GUIDs we need, which
// we used to get via dxguid.lib in the DirectX SDK. The DirectX SDK has
// been discontinued and (mostly) rolled into the Windows SDK, but it is
// missing a few features. One of which is dxguid.lib.

// if you wind up running into other GUIDs you need, add them here

#define INITGUID

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
