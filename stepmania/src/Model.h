#ifndef Model_H
#define Model_H
/*
-----------------------------------------------------------------------------
 Class: Model

 Desc: A 3D model.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include "RageTypes.h"
#include "ModelTypes.h"
#include <vector>
#include <map>

struct msModel;

typedef float matrix_t[3][4];
typedef struct
{
	matrix_t	mRelative;
	matrix_t	mAbsolute;
	matrix_t	mRelativeFinal;
	matrix_t	mFinal;
} myBone_t;


class Model : public Actor
{
public:
	Model ();
	virtual ~Model ();

public:
	void	Clear ();
	void	Load( CString sFile )
	{
		if( sFile == "" ) return;
		if( sFile.Right(6) == ".model" )
			LoadFromModelFile( sFile );
		else 
			LoadMilkshapeAscii( sFile );
	};

	bool	LoadFromModelFile( CString sFile );
	bool	LoadMilkshapeAscii( CString sFile );
	bool	LoadMilkshapeAsciiBones( CString sAniName, CString sFile );
	void	PlayAnimation( CString sAniName );

	virtual void	Update( float fDelta );
	virtual void	DrawPrimitives();

	void	AdvanceFrame (float dt);

	virtual void SetState( int iNewState );
	float GetCurFrame();
	void SetFrame( float fNewFrame );
	virtual int GetNumStates();
	CString		GetDefaultAnimation() { return m_sDefaultAnimation; };
	void		SetDefaultAnimation( CString sAnimation );
	bool		m_bRevertToDefaultAnimation;


private:
    vector<msMesh>				m_Meshes;
    vector<msMaterial>			m_Materials;
	map<CString,msAnimation>	m_mapNameToAnimation;
	msAnimation*				m_pCurAnimation;

	RageVector3			m_vMins, m_vMaxs;
	myBone_t			*m_pBones;
	typedef vector<RageModelVertex>	RageModelVertexVector;
	vector<RageModelVertexVector>	m_vTempVerticesByBone;
	float		m_fCurrFrame;
	CString		m_sDefaultAnimation;
	CString		m_sMostRecentAnimation;
};



#endif
