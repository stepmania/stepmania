#ifndef SCREEN_SANDBOX_H
#define SCREEN_SANDBOX_H

#include "Screen.h"

class ScreenSandbox : public Screen
{
public:
	ScreenSandbox( CString sName );

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void Update(float f);
	virtual void DrawPrimitives();
};

#endif
