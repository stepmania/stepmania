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
#include "RageModelGeometry.h"

struct msModel;


class Model : public Actor
{
public:
	Model ();
	virtual ~Model ();

public:
	void	Clear ();
	void	Load( CString sFile );

	void	LoadPieces( CString sMeshesPath, CString sMaterialsPath, CString sBomesPath );
	void	LoadFromModelFile( CString sFile );
	void	LoadMilkshapeAscii( CString sFile );
	void 	LoadMaterialsFromMilkshapeAscii( CString sPath );
	void 	LoadMilkshapeAsciiBones( CString sAniName, CString sPath );
	void	PlayAnimation( CString sAniName, float fPlayRate = 1 );

	virtual void	Update( float fDelta );
	virtual bool	EarlyAbortDraw();
	virtual void	DrawPrimitives();

	void	AdvanceFrame (float dt);

	virtual void SetState( int iNewState );
	float GetCurFrame();
	void SetFrame( float fNewFrame );
	virtual int GetNumStates();
	CString		GetDefaultAnimation() { return m_sDefaultAnimation; };
	void		SetDefaultAnimation( CString sAnimation, float fPlayRate = 1 );
	bool		m_bRevertToDefaultAnimation;

	virtual void HandleCommand( const ParsedCommand &command );

	int m_iRefCount;

private:
    RageModelGeometry	*m_pGeometry;

    vector<msMaterial>			m_Materials;
	map<CString,msAnimation>	m_mapNameToAnimation;
	msAnimation*				m_pCurAnimation;

	vector<myBone_t>	m_vpBones;

	// true if any vertex has a bone weight.
	// If true, then render from m_vTempVerticesByBone.  
	// Otherwise, render directly from the mesh's vertices
	bool m_bUseTempVertices;
	
	typedef vector<RageModelVertex>	RageModelVertexVector;
	vector<RageModelVertexVector>	m_vTempVerticesByMesh;
	
	float		m_fCurrFrame;
	CString		m_sDefaultAnimation;
	float		m_fDefaultAnimationRate;
	float		m_fCurAnimationRate;

};



#endif
