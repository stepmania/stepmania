#ifndef SAMPLE_3D_OBJECT
#define SAMPLE_3D_OBJECT

#include "ActorFrame.h"

class Sample3dObject: public ActorFrame
{
	float rot;
public:
	Sample3dObject();
	
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
