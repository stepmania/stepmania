#ifndef SAMPLE_3D_OBJECT
#define SAMPLE_3D_OBJECT

#include "ActorFrame.h"

class Sample3dObject: public ActorFrame
{
	float rot;
	float fDeltaTime;
	float tX;
	float tY;
	float tZ;
public:
	Sample3dObject();
	void Update();
	void DrawPrimitives();
	
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
