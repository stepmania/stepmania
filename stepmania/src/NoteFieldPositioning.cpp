#include "global.h"
#include "NoteFieldPositioning.h"
#include "RageDisplay.h"
#include "RageDisplayInternal.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "RageMath.h"

/* This is similar in style to Actor::Command.  However, Actors don't store
 * matrix stacks; they only store offsets and scales, and compound them into
 * a single transformations at once.  This makes some things easy, but it's not
 * convenient for generic 3d transforms.  For that, we have this, which has the
 * small subset of the actor commands that applies to raw matrices, and we apply
 * commands in the order given.  "scale,2;x,1;" is very different from
 * "x,1;scale,2;". */
static CString GetParam( const CStringArray& sParams, int iIndex, int& iMaxIndexAccessed )
{
	iMaxIndexAccessed = max( iIndex, iMaxIndexAccessed );
	if( iIndex < int(sParams.size()) )
		return sParams[iIndex];
	else
		return "";
}

void MatrixCommand(CString sCommandString, RageMatrix &mat)
{
	CStringArray asCommands;
	split( sCommandString, ";", asCommands, true );
	
	for( unsigned c=0; c<asCommands.size(); c++ )
	{
		CStringArray asTokens;
		split( asCommands[c], ",", asTokens, true );

		int iMaxIndexAccessed = 0;

#define sParam(i) (GetParam(asTokens,i,iMaxIndexAccessed))
#define fParam(i) ((float)atof(sParam(i)))
#define iParam(i) (atoi(sParam(i)))
#define bParam(i) (iParam(i)!=0)

		CString& sName = asTokens[0];
		sName.MakeLower();

		RageMatrix b;
		// Act on command
		if( sName=="x" )					RageMatrixTranslation( &b, fParam(1),0,0 );
		else if( sName=="y" )				RageMatrixTranslation( &b, 0,fParam(1),0 );
		else if( sName=="z" )				RageMatrixTranslation( &b, 0,0,fParam(1) );
		else if( sName=="zoomx" )			RageMatrixScaling(&b, fParam(1),1,1 );
		else if( sName=="zoomy" )			RageMatrixScaling(&b, 1,fParam(1),1 );
		else if( sName=="zoomz" )			RageMatrixScaling(&b, 1,1,fParam(1) );
		else if( sName=="rotationx" )		RageMatrixRotationX( &b, fParam(1) );
		else if( sName=="rotationy" )		RageMatrixRotationY( &b, fParam(1) );
		else if( sName=="rotationz" )		RageMatrixRotationZ( &b, fParam(1) );
		else
		{
			CString sError = ssprintf( "Unrecognized matrix command name '%s' in command string '%s'.", sName.GetString(), sCommandString.GetString() );
			LOG->Warn( sError );
#if defined(WIN32) // XXX arch?
			if( DISPLAY->IsWindowed() )
				MessageBox(NULL, sError, "MatrixCommand", MB_OK);
#endif
			continue;
		}


		if( iMaxIndexAccessed != (int)asTokens.size()-1 )
		{
			CString sError = ssprintf( "Wrong number of parameters in command '%s'.  Expected %d but there are %d.", join(",",asTokens).GetString(), iMaxIndexAccessed+1, (int)asTokens.size() );
			LOG->Warn( sError );
#if defined(WIN32) // XXX arch?
			if( DISPLAY->IsWindowed() )
				MessageBox(NULL, sError, "MatrixCommand", MB_OK);
#endif
			continue;
		}

		RageMatrix a(mat);
		RageMatrixMultiply(&mat, &a, &b);
	}
	
}

NoteFieldPositioning::NoteFieldPositioning()
{
	Init();
}

NoteFieldPositioning::~NoteFieldPositioning()
{
}

void NoteFieldPositioning::Init()
{
	for( int tn=0; tn<MAX_NOTE_TRACKS; tn++ )
	{
		RageMatrixIdentity(&m_Position[tn]);
		RageMatrixIdentity(&m_PerspPosition[tn]);
		m_fFov[tn] = 0;
	}
}
/*
void NoteFieldPositioning::LoadFromFile(CString fn)
{
	Init();

}
*/
void NoteFieldPositioning::LoadFromStyleDef(const StyleDef *s, PlayerNumber pn)
{
	Init();

	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		/* Set up the normal position of each track. */
		const float fPixelXOffsetFromCenter = s->m_ColumnInfo[pn][t].fXOffset;
		RageMatrixTranslation(&m_Position[t], fPixelXOffsetFromCenter, 0, 0);
	}
}

void NoteFieldPositioning::Update(float fDeltaTime)
{
}


void NoteFieldPositioning::BeginDrawTrack(int tn)
{
	// XXX
#if 0
	FILE *f = fopen("test.dat", "r");
	if(f) for(int t = 0; t <= tn/*MAX_NOTE_TRACKS*/; ++t)
	{
		char buf[1000];
		fgets(buf, 1000, f);

		CString b(buf);
		TrimRight(b);
		
		RageMatrixIdentity(&m_Position[t]);
		MatrixCommand(b, m_Position[t]);

		fgets(buf, 1000, f);
		b=buf;
		TrimRight(b);

		RageMatrixIdentity(&m_PerspPosition[t]);
		MatrixCommand(b, m_PerspPosition[t]);

		fgets(buf, 1000, f);
		sscanf(buf, "%f", &m_fFov[t]);
	}
	fclose(f);
// XXX
#endif

	DISPLAY->PushMatrix();

	glMultMatrixf((float *) m_Position[tn]);

	if(m_fFov[tn])
		DISPLAY->EnterPerspective(m_fFov[tn]);
	
	glMultMatrixf((float *) m_PerspPosition[tn]);
}

void NoteFieldPositioning::EndDrawTrack(int tn)
{
	if(m_fFov[tn])
		DISPLAY->ExitPerspective();

	DISPLAY->PopMatrix();
}

