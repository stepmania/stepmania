#include "global.h"
#include "RageDisplay.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageMath.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "RageFile.h"
#include "SDL_SaveJPEG.h"
#include "RageSurface_Save_BMP.h"
#include "SDL_rotozoom.h"
#include "RageSurface.h"
#include "Preference.h"

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

Preference<bool>  LOG_FPS( Debug, "LogFPS", true );

CString RageDisplay::PixelFormatToString( PixelFormat pixfmt )
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

// Return true if device was re-created and we need to reload textures.
bool RageDisplay::SetVideoMode( VideoModeParams p )
{
	/* Round to the nearest valid fullscreen resolution */
	/* Don't do this: we might be manually set to a less common resolution,
	 * like 1280x1024 or 640x240, which are occasionally useful. XXX: how
	 * should we do this sanity check?  A list of acceptable height/widths,
	 * forcing unknown ones? */
	/*
	if( !p.windowed )
	{
		if(      p.width <= 320 )	p.width = 320;
		else if( p.width <= 400 )	p.width = 400;
		else if( p.width <= 512 )	p.width = 512;
		else if( p.width <= 640 )	p.width = 640;
		else if( p.width <= 800 )	p.width = 800;
		else if( p.width <= 1024 )	p.width = 1024;
		else if( p.width <= 1280 )	p.width = 1280;

		switch( p.width )
		{
		case 320:	p.height = 240;	break;
		case 400:	p.height = 300;	break;
		case 512:	p.height = 384;	break;
		case 640:	p.height = 480;	break;
		case 800:	p.height = 600;	break;
		case 1024:	p.height = 768;	break;
		case 1280:	p.height = 960;	break;
		default:	ASSERT(0);
		}
	}
*/
	bool bNeedReloadTextures;
	
	CString err;
	err = this->TryVideoMode(p,bNeedReloadTextures);
	if( err == "" )
		return bNeedReloadTextures;
	
	// fall back
	p.windowed = false;
	if( this->TryVideoMode(p,bNeedReloadTextures) == "" )
		return bNeedReloadTextures;
	p.bpp = 16;
	if( this->TryVideoMode(p,bNeedReloadTextures) == "" )
		return bNeedReloadTextures;
	p.width = 640;
	p.height = 480;
	if( this->TryVideoMode(p,bNeedReloadTextures) == "" )
		return bNeedReloadTextures;

	RageException::ThrowNonfatal( "SetVideoMode failed: %s", err.c_str() );
}

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
		if( LOG_FPS )
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

/* Draw a line as a quad.  GL_LINES with SmoothLines off can draw line
 * ends at odd angles--they're forced to axis-alignment regardless of the
 * angle of the line. */
void RageDisplay::DrawPolyLine(const RageSpriteVertex &p1, const RageSpriteVertex &p2, float LineWidth )
{
	/* soh cah toa strikes strikes again! */
	float opp = p2.p.x - p1.p.x;
	float adj = p2.p.y - p1.p.y;
	float hyp = powf(opp*opp + adj*adj, 0.5f);

	float lsin = opp/hyp;
	float lcos = adj/hyp;

	RageSpriteVertex v[4];

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


void RageDisplay::DrawLineStripInternal( const RageSpriteVertex v[], int iNumVerts, float LineWidth )
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

void RageDisplay::DrawCircleInternal( const RageSpriteVertex &p, float radius )
{
	const int subdivisions = 32;
	RageSpriteVertex v[subdivisions+2];
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
	SetCullMode( CULL_NONE );
	SetZWrite( false ); 
	SetZTestMode( ZTEST_OFF );
	SetAlphaTest( true );
	SetBlendMode( BLEND_NORMAL );
	SetTextureFiltering( true );
	LoadMenuPerspective(0, CENTER_X, CENTER_Y);	// 0 FOV = ortho
	ChangeCentering(0,0,1,1);
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

    // Pops the top of the stack.
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
MatrixStack	g_ViewStack;
MatrixStack	g_WorldStack;
MatrixStack	g_TextureStack;

const RageMatrix* RageDisplay::GetProjectionTop()
{
	return g_ProjectionStack.GetTop();
}

const RageMatrix* RageDisplay::GetViewTop()
{
	return g_ViewStack.GetTop();
}

const RageMatrix* RageDisplay::GetWorldTop()
{
	return g_WorldStack.GetTop();
}

const RageMatrix* RageDisplay::GetTextureTop()
{
	return g_TextureStack.GetTop();
}

void RageDisplay::PushMatrix() 
{ 
	g_WorldStack.Push();
}

void RageDisplay::PopMatrix() 
{ 
	g_WorldStack.Pop();
}

void RageDisplay::Translate( float x, float y, float z )
{
	g_WorldStack.TranslateLocal(x, y, z);
}

void RageDisplay::TranslateWorld( float x, float y, float z )
{
	g_WorldStack.Translate(x, y, z);
}

void RageDisplay::Scale( float x, float y, float z )
{
	g_WorldStack.ScaleLocal(x, y, z);
}

void RageDisplay::RotateX( float deg )
{
	g_WorldStack.RotateXLocal( deg );
}

void RageDisplay::RotateY( float deg )
{
	g_WorldStack.RotateYLocal( deg );
}

void RageDisplay::RotateZ( float deg )
{
	g_WorldStack.RotateZLocal( deg );
}

void RageDisplay::PostMultMatrix( const RageMatrix &m )
{
	g_WorldStack.MultMatrix( m );
}

void RageDisplay::PreMultMatrix( const RageMatrix &m )
{
	g_WorldStack.MultMatrixLocal( m );
}

void RageDisplay::LoadIdentity()
{
	g_WorldStack.LoadIdentity();
}


void RageDisplay::TexturePushMatrix() 
{ 
	g_TextureStack.Push();
}

void RageDisplay::TexturePopMatrix() 
{ 
	g_TextureStack.Pop();
}

void RageDisplay::TextureTranslate( float x, float y, float z )
{
	g_TextureStack.TranslateLocal(x, y, z);
}


void RageDisplay::LoadMenuPerspective( float fovDegrees, float fVanishPointX, float fVanishPointY )
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
 		g_ViewStack.LoadIdentity();
	}
	else
	{
		CLAMP( fovDegrees, 0.1f, 179.9f );
		float fovRadians = fovDegrees / 180.f * PI;
		float theta = fovRadians/2;
		float fDistCameraFromImage = SCREEN_WIDTH/2 / tanf( theta );

		fVanishPointX = SCALE( fVanishPointX, SCREEN_LEFT, SCREEN_RIGHT, SCREEN_RIGHT, SCREEN_LEFT );
		fVanishPointY = SCALE( fVanishPointY, SCREEN_TOP, SCREEN_BOTTOM, SCREEN_BOTTOM, SCREEN_TOP );

		fVanishPointX -= CENTER_X;
		fVanishPointY -= CENTER_Y;


		/* It's the caller's responsibility to push first. */
		g_ProjectionStack.LoadMatrix(
			GetFrustumMatrix(
			  (fVanishPointX-SCREEN_WIDTH/2)/fDistCameraFromImage,
			  (fVanishPointX+SCREEN_WIDTH/2)/fDistCameraFromImage,
			  (fVanishPointY+SCREEN_HEIGHT/2)/fDistCameraFromImage,
			  (fVanishPointY-SCREEN_HEIGHT/2)/fDistCameraFromImage,
			  1,
			  fDistCameraFromImage+1000	) );

		g_ViewStack.LoadMatrix( 
			RageLookAt(
				-fVanishPointX+CENTER_X, -fVanishPointY+CENTER_Y, fDistCameraFromImage,
				-fVanishPointX+CENTER_X, -fVanishPointY+CENTER_Y, 0,
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
//void RageDisplay::EnterPerspective(float fov, bool preserve_loc, float near_clip, float far_clip)
//{
//	g_ProjectionStack.Push();
//	g_WorldStack.Push();
//
//	float aspect = SCREEN_WIDTH/(float)SCREEN_HEIGHT;
//	g_ProjectionStack.LoadMatrix( GetPerspectiveMatrix(fov, aspect, near_clip, far_clip) );
//	/* Flip the Y coordinate, so positive numbers go down. */
//	g_ProjectionStack.Scale(1, -1, 1);
//
//	if( preserve_loc )
//	{
//		
		//RageMatrix matTop = *g_WorldStack.GetTop();
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
//	}
//}

void RageDisplay::CameraPushMatrix()
{
	g_ProjectionStack.Push();
	g_ViewStack.Push();
}

void RageDisplay::CameraPopMatrix()
{
	g_ProjectionStack.Pop();
	g_ViewStack.Pop();
}


/* gluLookAt.  The result is pre-multiplied to the matrix (M = L * M) instead of
 * post-multiplied. */
void RageDisplay::LoadLookAt(float fov, const RageVector3 &Eye, const RageVector3 &At, const RageVector3 &Up)
{
	float aspect = SCREEN_WIDTH/(float)SCREEN_HEIGHT;
	g_ProjectionStack.LoadMatrix( GetPerspectiveMatrix(fov, aspect, 1, 1000) );
	/* Flip the Y coordinate, so positive numbers go down. */
	g_ProjectionStack.Scale(1, -1, 1);

	g_ViewStack.LoadMatrix(RageLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z));
}


RageMatrix RageDisplay::GetPerspectiveMatrix(float fovy, float aspect, float zNear, float zFar)
{
   float ymax = zNear * tanf(fovy * PI / 360.0f);
   float ymin = -ymax;
   float xmin = ymin * aspect;
   float xmax = ymax * aspect;

   return GetFrustumMatrix(xmin, xmax, ymin, ymax, zNear, zFar);
}

RageSurface *RageDisplay::CreateSurfaceFromPixfmt( PixelFormat pixfmt,
						void *pixels, int width, int height, int pitch )
{
	const PixelFormatDesc *tpf = GetPixelFormatDesc(pixfmt);

	RageSurface *surf = CreateSurfaceFrom(
		width, height, tpf->bpp, 
		tpf->masks[0], tpf->masks[1], tpf->masks[2], tpf->masks[3],
		(uint8_t *) pixels, pitch );

	return surf;
}

RageDisplay::PixelFormat RageDisplay::FindPixelFormat( 
	int bpp, int Rmask, int Gmask, int Bmask, int Amask, bool realtime )
{
	PixelFormatDesc tmp = { bpp, { Rmask, Gmask, Bmask, Amask } };

	for(int pixfmt = 0; pixfmt < NUM_PIX_FORMATS; ++pixfmt)
	{
		const PixelFormatDesc *pf = GetPixelFormatDesc(PixelFormat(pixfmt));
		if(!SupportsTextureFormat( PixelFormat(pixfmt), realtime ))
			continue;

		if(memcmp(pf, &tmp, sizeof(tmp)))
			continue;
		return PixelFormat(pixfmt);
	}

	return NUM_PIX_FORMATS;
}
	
RageMatrix RageDisplay::GetFrustumMatrix( float l, float r, float b, float t, float zn, float zf )
{
	// glFrustrum
	float A = (r+l) / (r-l);
	float B = (t+b) / (t-b);
	float C = -1 * (zf+zn) / (zf-zn);
	float D = -1 * (2*zf*zn) / (zf-zn);
	RageMatrix m(
		2*zn/(r-l), 0,          0,  0,
		0,          2*zn/(t-b), 0,  0,
		A,          B,          C,  -1,
		0,          0,          D,  0 );
	return m;
}

void RageDisplay::ChangeCentering( int trans_x, int trans_y, float scale_x, float scale_y )
{
	RageMatrix m1;
	RageMatrix m2;
	RageMatrixTranslation( &m1, float(trans_x), float(trans_y), 0 );
	RageMatrixScaling( &m2, scale_x, scale_y, 1 );
	RageMatrixMultiply( &m_Centering, &m1, &m2 );
}

bool RageDisplay::SaveScreenshot( CString sPath, GraphicsFileFormat format )
{
	RageSurface* surface = this->CreateScreenshot();

	/* Unless we're in lossless, resize the image to 640x480.  If we're saving lossy,
	 * there's no sense in saving 1280x960 screenshots, and if we're in a custom
	 * resolution (eg. 640x240), we don't want to output in that resolution. */
	if( format != SAVE_LOSSLESS && (surface->h != 640 || surface->w != 480) )
		RageSurfaceUtils::Zoom( surface, 640, 480 );

	RageFile out;
	if( !out.Open( sPath, RageFile::WRITE ) )
	{
		LOG->Trace("Couldn't write %s: %s", sPath.c_str(), out.GetError().c_str() );
		return false;
	}

	bool bSuccess = false;
	switch( format )
	{
	case SAVE_LOSSLESS:
		bSuccess = RageSurface_Save_BMP( surface, out );
		break;
	case SAVE_LOSSY_LOW_QUAL:
		bSuccess = IMG_SaveJPG_RW( surface, out, false );
		break;
	case SAVE_LOSSY_HIGH_QUAL:
		bSuccess = IMG_SaveJPG_RW( surface, out, true );
		break;
	default:
		ASSERT(0);
		return false;
	}

	delete surface;
	surface = NULL;

	if( !bSuccess )
	{
		LOG->Trace("Couldn't write %s: %s", sPath.c_str(), out.GetError().c_str() );
		return false;
	}

	return true;
}

void RageDisplay::DrawQuads( const RageSpriteVertex v[], int iNumVerts )
{
	ASSERT( (iNumVerts%4) == 0 );

	if(iNumVerts == 0)
		return;

	this->DrawQuadsInternal(v,iNumVerts);
	
	StatsAddVerts(iNumVerts);
}

void RageDisplay::DrawQuadStrip( const RageSpriteVertex v[], int iNumVerts )
{
	ASSERT( (iNumVerts%2) == 0 );

	if(iNumVerts < 4)
		return;

	this->DrawQuadStripInternal(v,iNumVerts);
	
	StatsAddVerts(iNumVerts);
}

void RageDisplay::DrawFan( const RageSpriteVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );

	this->DrawFanInternal(v,iNumVerts);
	
	StatsAddVerts(iNumVerts);
}

void RageDisplay::DrawStrip( const RageSpriteVertex v[], int iNumVerts )
{
	ASSERT( iNumVerts >= 3 );

	this->DrawStripInternal(v,iNumVerts);
	
	StatsAddVerts(iNumVerts); 
}

void RageDisplay::DrawTriangles( const RageSpriteVertex v[], int iNumVerts )
{
	if( iNumVerts == 0 )
		return;
	
	ASSERT( iNumVerts >= 3 );

	this->DrawTrianglesInternal(v,iNumVerts);

	StatsAddVerts(iNumVerts);
}

void RageDisplay::DrawCompiledGeometry( const RageCompiledGeometry *p, int iMeshIndex, const vector<msMesh> &vMeshes )
{
	this->DrawCompiledGeometryInternal( p, iMeshIndex );

	StatsAddVerts( vMeshes[iMeshIndex].Triangles.size() );	
}

void RageDisplay::DrawLineStrip( const RageSpriteVertex v[], int iNumVerts, float LineWidth )
{
	ASSERT( iNumVerts >= 2 );

	this->DrawLineStripInternal( v, iNumVerts, LineWidth );
}

void RageDisplay::DrawCircle( const RageSpriteVertex &v, float radius )
{
	this->DrawCircleInternal( v, radius );
}

/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
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

