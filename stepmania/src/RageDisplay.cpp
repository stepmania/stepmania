#include "global.h"
#include "RageDisplay.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageMath.h"
#include "RageUtil.h"
#include "RageFile.h"
#include "RageSurface_Save_BMP.h"
#include "RageSurface_Save_JPEG.h"
#include "RageSurfaceUtils_Zoom.h"
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
static RageTimer g_LastFrameEndedAt( RageZeroTimer );

static int g_iTranslateX = 0, g_iTranslateY = 0, g_iAddWidth = 0, g_iAddHeight = 0;

RageDisplay*		DISPLAY	= NULL;

Preference<bool>  LOG_FPS( "LogFPS", true );
Preference<float> g_fFrameLimitPercent( "FrameLimitPercent", 0.0f );

static const char *PixelFormatNames[] = {
	"RGBA8",
	"RGBA4",
	"RGB5A1",
	"RGB5",
	"RGB8",
	"PAL",
	"BGR8",
	"A1BGR5",
};
XToString( PixelFormat, NUM_PixelFormat );

/* bNeedReloadTextures is set to true if the device was re-created and we need
 * to reload textures.  On failure, an error message is returned. 
 * XXX: the renderer itself should probably be the one to try fallback modes */
RString RageDisplay::SetVideoMode( VideoModeParams p, bool &bNeedReloadTextures )
{
	RString err;
	err = this->TryVideoMode(p,bNeedReloadTextures);
	if( err == "" )
		return RString();
	LOG->Trace( "TryVideoMode failed: %s", err.c_str() );
	
	// fall back
	if( this->TryVideoMode(p,bNeedReloadTextures) == "" )
		return RString();
	p.bpp = 16;
	if( this->TryVideoMode(p,bNeedReloadTextures) == "" )
		return RString();
	p.width = 640;
	p.height = 480;
	if( this->TryVideoMode(p,bNeedReloadTextures) == "" )
		return RString();

	return ssprintf( "SetVideoMode failed: %s", err.c_str() );
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
		{
			RString sStats = GetStats();
			sStats.Replace( "\n", ", " );
			LOG->Trace( "%s", sStats.c_str() );
		}
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

RString RageDisplay::GetStats() const
{
	RString s;
	/* If FPS == 0, we don't have stats yet. */
	if( !GetFPS() )
		s = "-- FPS\n-- av FPS\n-- VPF";

	s = ssprintf( "%i FPS\n%i av FPS\n%i VPF", GetFPS(), GetCumFPS(), GetVPF() );
	
	s += "\n"+this->GetApiDescription();
	
	return s;
}

void RageDisplay::EndFrame()
{
	ProcessStatsOnFlip();
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
	for( int i = 0; i < iNumVerts-1; ++i )
		DrawPolyLine(v[i], v[i+1], LineWidth);

	/* Join the lines with circles so we get rounded corners. */
	for( int i = 0; i < iNumVerts; ++i )
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
		const float fX = RageFastCos(fRotation) * radius;
		const float fY = -RageFastSin(fRotation) * radius;
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
	SetZBias( 0 );
	LoadMenuPerspective( 0, 640, 480, 320, 240 ); // 0 FOV = ortho
	ChangeCentering(0,0,0,0);
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

	void SkewX( float fAmount )
	{		
		RageMatrix m;
		RageMatrixSkewX( &m, fAmount );
		MultMatrixLocal( m );
	}

	// Obtain the current matrix at the top of the stack
	const RageMatrix* GetTop() { return &stack.back(); }
};


static MatrixStack g_ProjectionStack;
static MatrixStack g_ViewStack;
static MatrixStack g_WorldStack;
static MatrixStack g_TextureStack;

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

void RageDisplay::SkewX( float fAmount )
{
	g_WorldStack.SkewX( fAmount );
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

void RageDisplay::TextureTranslate( float x, float y )
{
	g_TextureStack.TranslateLocal(x, y, 0);
}


void RageDisplay::LoadMenuPerspective( float fovDegrees, float fWidth, float fHeight, float fVanishPointX, float fVanishPointY )
{
	/* fovDegrees == 0 gives ortho projection. */
	if( fovDegrees == 0 )
	{
 		float left = 0, right = fWidth, bottom = fHeight, top = 0;
		g_ProjectionStack.LoadMatrix( GetOrthoMatrix(left, right, bottom, top, -1000, +1000) );
 		g_ViewStack.LoadIdentity();
	}
	else
	{
		CLAMP( fovDegrees, 0.1f, 179.9f );
		float fovRadians = fovDegrees / 180.f * PI;
		float theta = fovRadians/2;
		float fDistCameraFromImage = fWidth/2 / tanf( theta );

		fVanishPointX = SCALE( fVanishPointX, 0, fWidth, fWidth, 0 );
		fVanishPointY = SCALE( fVanishPointY, 0, fHeight, fHeight, 0 );

		fVanishPointX -= fWidth/2;
		fVanishPointY -= fHeight/2;


		/* It's the caller's responsibility to push first. */
		g_ProjectionStack.LoadMatrix(
			GetFrustumMatrix(
			  (fVanishPointX-fWidth/2)/fDistCameraFromImage,
			  (fVanishPointX+fWidth/2)/fDistCameraFromImage,
			  (fVanishPointY+fHeight/2)/fDistCameraFromImage,
			  (fVanishPointY-fHeight/2)/fDistCameraFromImage,
			  1,
			  fDistCameraFromImage+1000	) );

		g_ViewStack.LoadMatrix( 
			RageLookAt(
				-fVanishPointX+fWidth/2, -fVanishPointY+fHeight/2, fDistCameraFromImage,
				-fVanishPointX+fWidth/2, -fVanishPointY+fHeight/2, 0,
				0.0f, 1.0f, 0.0f) );
	}
}


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
void RageDisplay::LoadLookAt( float fFOV, const RageVector3 &Eye, const RageVector3 &At, const RageVector3 &Up )
{
	float fAspect = GetActualVideoModeParams().fDisplayAspectRatio;
	g_ProjectionStack.LoadMatrix( GetPerspectiveMatrix(fFOV, fAspect, 1, 1000) );

	/* Flip the Y coordinate, so positive numbers go down. */
	g_ProjectionStack.Scale( 1, -1, 1 );

	g_ViewStack.LoadMatrix( RageLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z) );
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

PixelFormat RageDisplay::FindPixelFormat( int iBPP, int iRmask, int iGmask, int iBmask, int iAmask, bool bRealtime )
{
	PixelFormatDesc tmp = { iBPP, { iRmask, iGmask, iBmask, iAmask } };

	FOREACH_ENUM2( PixelFormat, iPixFmt )
	{
		const PixelFormatDesc *pf = GetPixelFormatDesc( PixelFormat(iPixFmt) );
		if( !SupportsTextureFormat(PixelFormat(iPixFmt), bRealtime) )
			continue;

		if( memcmp(pf, &tmp, sizeof(tmp)) )
			continue;
		return iPixFmt;
	}

	return PixelFormat_INVALID;
}
	
/* These convert to OpenGL's coordinate system: -1,-1 is the bottom-left, +1,+1 is the
 * top-right, and Z goes from -1 (viewer) to +1 (distance).  It's a little odd, but
 * very well-defined. */
RageMatrix RageDisplay::GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf )
{
	RageMatrix m(
		2/(r-l),      0,            0,           0,
		0,            2/(t-b),      0,           0,
		0,            0,            -2/(zf-zn),   0,
		-(r+l)/(r-l), -(t+b)/(t-b), -(zf+zn)/(zf-zn),  1 );
	return m;
}

RageMatrix RageDisplay::GetFrustumMatrix( float l, float r, float b, float t, float zn, float zf )
{
	// glFrustum
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

void RageDisplay::ResolutionChanged()
{
	/* The centering matrix depends on the resolution. */
	UpdateCentering();
}

void RageDisplay::ChangeCentering( int iTranslateX, int iTranslateY, int iAddWidth, int iAddHeight )
{
	g_iTranslateX = iTranslateX;
	g_iTranslateY = iTranslateY;
	g_iAddWidth = iAddWidth;
	g_iAddHeight = iAddHeight;

	UpdateCentering();
}

void RageDisplay::UpdateCentering()
{
	// in screen space, left edge = -1, right edge = 1, bottom edge = -1. top edge = 1
	float fWidth = (float) GetActualVideoModeParams().width;
	float fHeight = (float) GetActualVideoModeParams().height;
	float fPercentShiftX = SCALE( g_iTranslateX, 0, fWidth, 0, +2.0f );
	float fPercentShiftY = SCALE( g_iTranslateY, 0, fHeight, 0, -2.0f );
	float fPercentScaleX = SCALE( g_iAddWidth, 0, fWidth, 1.0f, 2.0f );
	float fPercentScaleY = SCALE( g_iAddHeight, 0, fHeight, 1.0f, 2.0f );

	RageMatrix m1;
	RageMatrix m2;
	RageMatrixTranslation( 
		&m1, 
		fPercentShiftX, 
		fPercentShiftY, 
		0 );
	RageMatrixScaling( 
		&m2, 
		fPercentScaleX, 
		fPercentScaleY, 
		1 );
	RageMatrixMultiply( &m_Centering, &m1, &m2 );
}

bool RageDisplay::SaveScreenshot( RString sPath, GraphicsFileFormat format )
{
	RageSurface* surface = this->CreateScreenshot();

	/* Unless we're in lossless, resize the image to 640x480.  If we're saving lossy,
	 * there's no sense in saving 1280x960 screenshots, and we don't want to output
	 * screenshots in a strange (non-1) sample aspect ratio. */
	if( format != SAVE_LOSSLESS )
	{
		/* Maintain the DAR. */
		ASSERT( GetActualVideoModeParams().fDisplayAspectRatio > 0 );
		int iHeight = lrintf( 640 / GetActualVideoModeParams().fDisplayAspectRatio );
		LOG->Trace( "%ix%i -> %ix%i (%.3f)", surface->w, surface->h, 640, iHeight, GetActualVideoModeParams().fDisplayAspectRatio );
		RageSurfaceUtils::Zoom( surface, 640, iHeight );
	}

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
		bSuccess = RageSurfaceUtils::SaveBMP( surface, out );
		break;
	case SAVE_LOSSY_LOW_QUAL:
		bSuccess = RageSurfaceUtils::SaveJPEG( surface, out, false );
		break;
	case SAVE_LOSSY_HIGH_QUAL:
		bSuccess = RageSurfaceUtils::SaveJPEG( surface, out, true );
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

void RageDisplay::FrameLimitBeforeVsync( int iFPS )
{
	ASSERT( iFPS != 0 );

	if( g_LastFrameEndedAt.IsZero() )
		return;

	if( g_fFrameLimitPercent.Get() == 0.0f )
		return;

	float fFrameTime = g_LastFrameEndedAt.GetDeltaTime();
	float fExpectedTime = 1.0f / iFPS;

	/* This is typically used to turn some of the delay that would normally
	 * be waiting for vsync and turn it into a usleep, to make sure we give
	 * up the CPU.  If we overshoot the sleep, then we'll miss the vsync,
	 * so allow tweaking the amount of time we expect a frame to take.
	 * Frame limiting is disabled by setting this to 0. */
	fExpectedTime *= g_fFrameLimitPercent.Get();
	float fExtraTime = fExpectedTime - fFrameTime;
	if( fExtraTime > 0 )
		usleep( int(fExtraTime * 1000000) );
}

void RageDisplay::FrameLimitAfterVsync()
{
	if( g_fFrameLimitPercent.Get() == 0.0f )
		return;

	g_LastFrameEndedAt.Touch();
}


RageCompiledGeometry::~RageCompiledGeometry()
{
	m_bNeedsNormals = false;
}

void RageCompiledGeometry::Set( const vector<msMesh> &vMeshes, bool bNeedsNormals )
{
	m_bNeedsNormals = bNeedsNormals;

	size_t totalVerts = 0;
	size_t totalTriangles = 0;

	m_bAnyNeedsTextureMatrixScale = false;

	m_vMeshInfo.resize( vMeshes.size() );
	for( unsigned i=0; i<vMeshes.size(); i++ )
	{
		const msMesh& mesh = vMeshes[i];
		const vector<RageModelVertex> &Vertices = mesh.Vertices;
		const vector<msTriangle> &Triangles = mesh.Triangles;

		MeshInfo& meshInfo = m_vMeshInfo[i];
		meshInfo.m_bNeedsTextureMatrixScale = false;

		meshInfo.iVertexStart = totalVerts;
		meshInfo.iVertexCount = Vertices.size();
		meshInfo.iTriangleStart = totalTriangles;
		meshInfo.iTriangleCount = Triangles.size();

		totalVerts += Vertices.size();
		totalTriangles += Triangles.size();

		for( unsigned j = 0; j < Vertices.size(); ++j )
		{
			if( Vertices[j].TextureMatrixScale.x != 1.0f || Vertices[j].TextureMatrixScale.y != 1.0f )
			{
				meshInfo.m_bNeedsTextureMatrixScale = true;
				m_bAnyNeedsTextureMatrixScale = true;
			}
		}
	}

	this->Allocate( vMeshes );

	Change( vMeshes );
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

