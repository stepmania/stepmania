#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: RageDisplay

 Desc: Methods common to all RageDisplays

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
    Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "RageDisplay.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageMath.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"

//
// Statistics stuff
//
RageTimer			g_LastCheckTimer;
int					g_iNumVerts;
int					g_iFPS, g_iVPF, g_iCFPS;

int RageDisplay::GetFPS() const { return g_iFPS; }
int RageDisplay::GetVPF() const { return g_iVPF; }
int RageDisplay::GetCumFPS() const { return g_iCFPS; }

static int			g_iFramesRenderedSinceLastCheck,
					g_iFramesRenderedSinceLastReset,
					g_iVertsRenderedSinceLastCheck,
					g_iNumChecksSinceLastReset;

RageDisplay*		DISPLAY	= NULL;

CString PixelFormatToString( PixelFormat pixfmt )
{
	const CString s[NUM_PIX_FORMATS] = {
		"FMT_RGBA8",
		"FMT_RGBA4",
		"FMT_RGB5A1",
		"FMT_RGB5",
		"FMT_RGB8",
		"FMT_PAL" };
	return s[pixfmt];
};

void RageDisplay::ProcessStatsOnFlip()
{
	g_iFramesRenderedSinceLastCheck++;
	g_iFramesRenderedSinceLastReset++;

	if( g_LastCheckTimer.PeekDeltaTime() >= 1.0f )	// update stats every 1 sec.
	{
		g_LastCheckTimer.GetDeltaTime();
		g_iNumChecksSinceLastReset++;
		g_iFPS = g_iFramesRenderedSinceLastCheck;
		g_iCFPS = g_iFramesRenderedSinceLastReset / g_iNumChecksSinceLastReset;
		g_iVPF = g_iVertsRenderedSinceLastCheck / g_iFPS;
		g_iFramesRenderedSinceLastCheck = g_iVertsRenderedSinceLastCheck = 0;
		LOG->Trace( "FPS: %d, CFPS %d, VPF: %d", g_iFPS, g_iCFPS, g_iVPF );
	}
}

void RageDisplay::ResetStats()
{
	g_iFPS = g_iVPF = 0;
	g_iFramesRenderedSinceLastCheck = g_iFramesRenderedSinceLastReset = 0;
	g_iNumChecksSinceLastReset = 0;
	g_iVertsRenderedSinceLastCheck = 0;
	g_LastCheckTimer.GetDeltaTime();
}

void RageDisplay::StatsAddVerts( int iNumVertsRendered ) { g_iVertsRenderedSinceLastCheck += iNumVertsRendered; }

/* Draw a line as a quad.  GL_LINES with antialiasing off can draw line
 * ends at odd angles--they're forced to axis-alignment regardless of the
 * angle of the line. */
void RageDisplay::DrawPolyLine(const RageVertex &p1, const RageVertex &p2, float LineWidth )
{
	/* soh cah toa strikes strikes again! */
	float opp = p2.p.x - p1.p.x;
	float adj = p2.p.y - p1.p.y;
	float hyp = powf(opp*opp + adj*adj, 0.5f);

	float lsin = opp/hyp;
	float lcos = adj/hyp;

	RageVertex v[4];

	v[0] = v[1] = p1;
	v[2] = v[3] = p2;

	float ydist = lsin * LineWidth/2;
	float xdist = lcos * LineWidth/2;
	
	v[0].p.x += xdist;
	v[0].p.y -= ydist;
	v[1].p.x -= xdist;
	v[1].p.y += ydist;
	v[2].p.x -= xdist;
	v[2].p.y += ydist;
	v[3].p.x += xdist;
	v[3].p.y -= ydist;

	this->DrawQuad(v);
}


void RageDisplay::DrawLineStrip( const RageVertex v[], int iNumVerts, float LineWidth )
{
	ASSERT( iNumVerts >= 2 );

	/* Draw a line strip with rounded corners using polys.  This is used on
	 * cards that have strange allergic reactions to antialiased points and
	 * lines. */
	int i;
	for(i = 0; i < iNumVerts-1; ++i)
		DrawPolyLine(v[i], v[i+1], LineWidth);

	/* Join the lines with circles so we get rounded corners. */
	for(i = 0; i < iNumVerts; ++i)
		DrawCircle( v[i], LineWidth/2 );
}

void RageDisplay::DrawCircle( const RageVertex &p, float radius )
{
	const int subdivisions = 32;
	RageVertex v[subdivisions+2];
	v[0] = p;

	for(int i = 0; i < subdivisions+1; ++i) 
	{
		const float fRotation = float(i) / subdivisions * 2*PI;
		const float fX = cosf(fRotation) * radius;
		const float fY = -sinf(fRotation) * radius;
		v[1+i] = v[0];
		v[1+i].p.x += fX;
		v[1+i].p.y += fY;
	}

	this->DrawFan( v, subdivisions+2 );
}



void RageDisplay::SetDefaultRenderStates()
{
	SetLighting( false );
	SetBackfaceCull( false );
	SetZBuffer( false );
	SetAlphaTest( true );
	SetBlendMode( BLEND_NORMAL );
	SetTextureFiltering( true );
	LoadMenuPerspective(0);	// 0 FOV = ortho
}


//
// Matrix stuff
//
class MatrixStack
{
	vector<RageMatrix> stack;
public:

	MatrixStack()
	{
		stack.resize(1);
		LoadIdentity();
	}

    // Pops the top of the stack, returns the current top
    // *after* popping the top.
    void Pop()
	{
		stack.pop_back();
		ASSERT( stack.size() > 0 );	// underflow
	}

    // Pushes the stack by one, duplicating the current matrix.
    void Push()
	{
		stack.push_back( stack.back() );
		ASSERT( stack.size() < 100 );	// overflow
	}

    // Loads identity in the current matrix.
    void LoadIdentity()
	{
		RageMatrixIdentity( &stack.back() );
	}

    // Loads the given matrix into the current matrix
    void LoadMatrix( const RageMatrix& m )
	{
		stack.back() = m;
	}

    // Right-Multiplies the given matrix to the current matrix.
    // (transformation is about the current world origin)
    void MultMatrix( const RageMatrix& m )
	{
		RageMatrixMultiply( &stack.back(), &m, &stack.back() );
	}

    // Left-Multiplies the given matrix to the current matrix
    // (transformation is about the local origin of the object)
    void MultMatrixLocal( const RageMatrix& m )
	{
		RageMatrixMultiply( &stack.back(), &stack.back(), &m );
	}

    // Right multiply the current matrix with the computed rotation
    // matrix, counterclockwise about the given axis with the given angle.
    // (rotation is about the current world origin)
    void RotateX( float degrees )
	{
		RageMatrix m;
		RageMatrixRotationX( &m, degrees );
		MultMatrix( m );
	}
    void RotateY( float degrees )
 	{
		RageMatrix m;
		RageMatrixRotationY( &m, degrees );
		MultMatrix( m );
	}
	void RotateZ( float degrees )
	{
		RageMatrix m;
		RageMatrixRotationZ( &m, degrees );
		MultMatrix( m );
	}
	
	// Left multiply the current matrix with the computed rotation
    // matrix. All angles are counterclockwise. (rotation is about the
    // local origin of the object)
	void RotateXLocal( float degrees )
	{
		RageMatrix m;
		RageMatrixRotationX( &m, degrees );
		MultMatrixLocal( m );
	}
    void RotateYLocal( float degrees )
 	{
		RageMatrix m;
		RageMatrixRotationY( &m, degrees );
		MultMatrixLocal( m );
	}
	void RotateZLocal( float degrees )
	{
		RageMatrix m;
		RageMatrixRotationZ( &m, degrees );
		MultMatrixLocal( m );
	}

    // Right multiply the current matrix with the computed scale
    // matrix. (transformation is about the current world origin)
    void Scale( float x, float y, float z)
 	{
		RageMatrix m;
		RageMatrixScaling( &m, x, y, z );
		MultMatrix( m );
	}

    // Left multiply the current matrix with the computed scale
    // matrix. (transformation is about the local origin of the object)
    void ScaleLocal( float x, float y, float z)
 	{
		RageMatrix m;
		RageMatrixScaling( &m, x, y, z );
		MultMatrixLocal( m );
	}

    // Right multiply the current matrix with the computed translation
    // matrix. (transformation is about the current world origin)
    void Translate( float x, float y, float z)
 	{
		RageMatrix m;
		RageMatrixTranslation( &m, x, y, z );
		MultMatrix( m );
	}

    // Left multiply the current matrix with the computed translation
    // matrix. (transformation is about the local origin of the object)
    void TranslateLocal( float x, float y, float z)
 	{
		RageMatrix m;
		RageMatrixTranslation( &m, x, y, z );
		MultMatrixLocal( m );
	}

    // Obtain the current matrix at the top of the stack
    const RageMatrix* GetTop() { return &stack.back(); }
};


MatrixStack	g_ProjectionStack;
MatrixStack	g_ModelViewStack;

const RageMatrix* RageDisplay::GetProjection()
{
	return g_ProjectionStack.GetTop();
}

const RageMatrix* RageDisplay::GetModelViewTop()
{
	return g_ModelViewStack.GetTop();
}

void RageDisplay::PushMatrix() 
{ 
	g_ModelViewStack.Push();
}

void RageDisplay::PopMatrix() 
{ 
	g_ModelViewStack.Pop();
}

void RageDisplay::Translate( float x, float y, float z )
{
	g_ModelViewStack.TranslateLocal(x, y, z);
}

void RageDisplay::TranslateWorld( float x, float y, float z )
{
	g_ModelViewStack.Translate(x, y, z);
}

void RageDisplay::Scale( float x, float y, float z )
{
	g_ModelViewStack.ScaleLocal(x, y, z);
}

void RageDisplay::RotateX( float deg )
{
	g_ModelViewStack.RotateXLocal( deg );
}

void RageDisplay::RotateY( float deg )
{
	g_ModelViewStack.RotateYLocal( deg );
}

void RageDisplay::RotateZ( float deg )
{
	g_ModelViewStack.RotateZLocal( deg );
}

void RageDisplay::PostMultMatrix( const RageMatrix &m )
{
	g_ModelViewStack.MultMatrix( m );
}

void RageDisplay::PreMultMatrix( const RageMatrix &m )
{
	g_ModelViewStack.MultMatrixLocal( m );
}

void RageDisplay::LoadIdentity()
{
	g_ModelViewStack.LoadIdentity();
}


void RageDisplay::LoadMenuPerspective(float fovDegrees)
{
	/* fovDegrees == 0 looks the same as an ortho projection.  However,
	 * we don't want to mess with the ModelView stack because 
	 * EnterPerspectiveMode's preserve location feature expectes there 
	 * not to be any camera transforms.  So, do a true ortho projection
	 * if fovDegrees == 0.  Perhaps it would be more convenient to keep 
	 * separate model and view stacks like D3D?
	 */
	if( fovDegrees == 0 )
	{
 		float left = 0, right = SCREEN_WIDTH, bottom = SCREEN_HEIGHT, top = 0;
		g_ProjectionStack.LoadMatrix( GetOrthoMatrix(left, right, bottom, top, SCREEN_NEAR, SCREEN_FAR) );
 		g_ModelViewStack.LoadIdentity();
	}
	else
	{
		CLAMP( fovDegrees, 0.1f, 179.9f );
		float fovRadians = fovDegrees / 180.f * PI;
		float theta = fovRadians/2;
		float fDistCameraFromImage = SCREEN_WIDTH/2 / tanf( theta );

		/* It's the caller's responsibility to push first. */
		g_ProjectionStack.LoadMatrix(
			GetFrustrumMatrix(
			  -(SCREEN_WIDTH/2)/fDistCameraFromImage,
			  +(SCREEN_WIDTH/2)/fDistCameraFromImage,
			  +(SCREEN_HEIGHT/2)/fDistCameraFromImage,
			  -(SCREEN_HEIGHT/2)/fDistCameraFromImage,
			  1,
			  fDistCameraFromImage+1000	) );

		g_ModelViewStack.MultMatrixLocal( 
			RageLookAt(
				CENTER_X, CENTER_Y, fDistCameraFromImage,
				CENTER_X, CENTER_Y, 0,
				0.0f, 1.0f, 0.0f) );
	}
}


/* Switch from orthogonal to perspective view.
 *
 * Tricky: we want to maintain all of the zooms, rotations and translations
 * that have been applied already.  They're in our internal screen space (that
 * is, 640x480 ortho).  We can't simply leave them where they are, since they'll
 * be applied before the perspective transform, which means they'll be in the
 * wrong coordinate space.
 *
 * Handle translations (the right column of the transform matrix) to the viewport.
 * Move rotations and scaling (0,0 through 1,1) to after the perspective transform.
 * Actually, those might be able to stay where they are, I'm not sure; it's translations
 * that are annoying.  So, XXX: see if rots and scales can be left on the modelview
 * matrix instead of messing with the projection matrix.
 *
 * When finished, the current position will be the "viewpoint" (at 0,0).  negative
 * Z goes into the screen, positive X and Y is right and down.
 */
void RageDisplay::EnterPerspective(float fov, bool preserve_loc, float near_clip, float far_clip)
{
	g_ProjectionStack.Push();
	g_ModelViewStack.Push();

	float aspect = SCREEN_WIDTH/(float)SCREEN_HEIGHT;
	g_ProjectionStack.LoadMatrix( GetPerspectiveMatrix(fov, aspect, near_clip, far_clip) );
	/* Flip the Y coordinate, so positive numbers go down. */
	g_ProjectionStack.Scale(1, -1, 1);

	if( preserve_loc )
	{
		RageMatrix matTop = *g_ModelViewStack.GetTop();
		/* TODO: Come up with a more general way to handle this.  
		 * It looks kind of hacky. -Chris */
	//	{
	//		/* Pull out the 2d translation. */
	//		float x = matTop.m[3][0], y = matTop.m[3][1];
	//
	//		/* These values are where the viewpoint should be.  By default, it's in the
	//		* center of the screen, and these values are from the top-left, so subtract
	//		* the difference. */
	//		x -= SCREEN_WIDTH/2;
	//		y -= SCREEN_HEIGHT/2;
	//		SetViewport(int(x), int(y));
	//	}
	//
	//	/* Pull out the 2d rotations and scales. */
	//	{
	//		RageMatrix mat;
	//		RageMatrixIdentity(&mat);
	//		mat.m[0][0] = matTop.m[0][0];
	//		mat.m[0][1] = matTop.m[0][1];
	//		mat.m[1][0] = matTop.m[1][0];
	//		mat.m[1][1] = matTop.m[1][1];
	//		this->MultMatrix(mat);
	//	}
	//
	//	/* We can't cope with perspective matrices or things that touch Z.  (We shouldn't
	//	* have touched those while in 2d, anyway.) */
	//	ASSERT(matTop.m[0][2] == 0.f &&	matTop.m[0][3] == 0.f && matTop.m[1][2] == 0.f &&
	//		matTop.m[1][3] == 0.f && matTop.m[2][0] == 0.f && matTop.m[2][1] == 0.f &&
	//		matTop.m[2][2] == 1.f && matTop.m[3][2] == 0.f && matTop.m[3][3] == 1.f);
	//
	//	/* Reset the matrix back to identity. */
	//	glMatrixMode( GL_MODELVIEW );
	//	glLoadIdentity();
	}
}

void RageDisplay::ExitPerspective()
{
	g_ProjectionStack.Pop();
	g_ModelViewStack.Pop();
}


/* gluLookAt.  The result is pre-multiplied to the matrix (M = L * M) instead of
 * post-multiplied. */
void RageDisplay::LookAt(const RageVector3 &Eye, const RageVector3 &At, const RageVector3 &Up)
{
	PreMultMatrix(RageLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z));
}

RageMatrix RageDisplay::GetFrustrumMatrix( 
	float left,    
	float right,   
	float bottom,  
	float top,     
	float znear,   
	float zfar )	// see glFrustrum docs
{
	float A = (right+left) / (right-left);
	float B = (top+bottom) / (top-bottom);
	float C = -1 * (zfar+znear) / (zfar-znear);
	float D = -1 * (2*zfar*znear) / (zfar-znear);
	RageMatrix m(
		2*znear/(right-left), 0,                   0,  0,
		0,                   2*znear/(top-bottom), 0,  0,
		A,                   B,                    C,  -1,
		0,                   0,                    D,  0 );
	return m;
}

RageMatrix RageDisplay::GetPerspectiveMatrix(float fovy, float aspect, float zNear, float zFar)
{
   float ymax = zNear * tanf(fovy * PI / 360.0f);
   float ymin = -ymax;
   float xmin = ymin * aspect;
   float xmax = ymax * aspect;

   return GetFrustrumMatrix(xmin, xmax, ymin, ymax, zNear, zFar);
}
