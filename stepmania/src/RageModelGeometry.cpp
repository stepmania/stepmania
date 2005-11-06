#include "global.h"
#include "RageModelGeometry.h"
#include "RageUtil.h"
#include "RageFile.h"
#include "RageMath.h"
#include "RageDisplay.h"

#define MS_MAX_NAME	32

RageModelGeometry::RageModelGeometry ()
{
	m_iRefCount = 1;
	m_pCompiledGeometry = DISPLAY->CreateCompiledGeometry();
}

RageModelGeometry::~RageModelGeometry ()
{
	DISPLAY->DeleteCompiledGeometry( m_pCompiledGeometry );
}

void RageModelGeometry::OptimizeBones()
{
	for (unsigned i = 0; i < m_Meshes.size(); i++)
	{
		msMesh& mesh = m_Meshes[i];

		if( mesh.Vertices.empty() )
			continue;	// nothing to optimize

		// check to see if all vertices have the same bone index
		bool bAllVertsUseSameBone = true;
		
		char nBoneIndex	= mesh.Vertices[0].bone;

		for (unsigned j = 1; j < mesh.Vertices.size(); j++)
		{
			if( mesh.Vertices[j].bone != nBoneIndex )
			{
				bAllVertsUseSameBone = false;
				break;
			}
		}

		if( bAllVertsUseSameBone )
		{
			mesh.nBoneIndex = nBoneIndex;

			// clear all vertex/bone associations;
			for (unsigned j = 0; j < mesh.Vertices.size(); j++)
			{
				mesh.Vertices[j].bone = -1;
			}
		}
	}
}

void RageModelGeometry::MergeMeshes( int iFromIndex, int iToIndex )
{
	msMesh& meshFrom = m_Meshes[ iFromIndex ];
	msMesh& meshTo = m_Meshes[ iToIndex ];

	int iShiftTriangleVertexIndicesBy = meshTo.Vertices.size();
	int iStartShiftingAtTriangleIndex = meshTo.Triangles.size();

	meshTo.Vertices.insert( meshTo.Vertices.end(), meshFrom.Vertices.begin(), meshFrom.Vertices.end() );
	meshTo.Triangles.insert( meshTo.Triangles.end(), meshFrom.Triangles.begin(), meshFrom.Triangles.end() );

	for( unsigned i=iStartShiftingAtTriangleIndex; i<meshTo.Triangles.size(); i++ )
	{
		for( int j=0; j<3; j++ )
		{
			uint16_t &iIndex = meshTo.Triangles[i].nVertexIndices[j];
			iIndex = uint16_t(iIndex + iShiftTriangleVertexIndicesBy);
		}
	}
}

bool RageModelGeometry::HasAnyPerVertexBones() const
{
	for( unsigned i = 0; i < m_Meshes.size(); ++i )
	{
		const msMesh& mesh = m_Meshes[i];
		for( unsigned j = 0; j < mesh.Vertices.size(); ++j )
			if( mesh.Vertices[j].bone != -1 )
				return true;
	}

	return false;
}

#define THROW RageException::Throw( "Parse error in \"%s\" at line %d: '%s'", sPath.c_str(), iLineNum, sLine.c_str() )

void RageModelGeometry::LoadMilkshapeAscii( const CString& _sPath, bool bNeedsNormals )
{
	CString sPath = _sPath;
	FixSlashesInPlace(sPath);
	const CString sDir = Dirname( sPath );

	RageFile f;
	if( !f.Open( sPath ) )
		RageException::Throw( "RageModelGeometry::LoadMilkshapeAscii Could not open \"%s\": %s", sPath.c_str(), f.GetError().c_str() );

	CString sLine;
	int iLineNum = 0;
	char szName[MS_MAX_NAME];
	int nFlags, nIndex;

	RageVec3ClearBounds( m_vMins, m_vMaxs );

	while( f.GetLine( sLine ) > 0 )
	{
		iLineNum++;

		if( !strncmp(sLine, "//", 2) )
			continue;

		int nFrame;
		if( sscanf(sLine, "Frames: %d", &nFrame) == 1 )
		{
			// ignore
			// m_pRageModelGeometry->nTotalFrames = nFrame;
		}
		if( sscanf(sLine, "Frame: %d", &nFrame) == 1 )
		{
			// ignore
			// m_pRageModelGeometry->nFrame = nFrame;
		}

		int nNumMeshes = 0;
		if( sscanf(sLine, "Meshes: %d", &nNumMeshes) == 1 )
		{
			ASSERT( m_Meshes.empty() );
			m_Meshes.resize( nNumMeshes );

			for( int i = 0; i < nNumMeshes; i++ )
			{
				msMesh &mesh = m_Meshes[i];
				vector<RageModelVertex> &Vertices = mesh.Vertices;
				vector<msTriangle> &Triangles = mesh.Triangles;

				if( f.GetLine( sLine ) <= 0 )
					THROW;

				// mesh: name, flags, material index
				if( sscanf (sLine, "\"%[^\"]\" %d %d",szName, &nFlags, &nIndex) != 3 )
					THROW;

				mesh.sName = szName;
				// mesh.nFlags = nFlags;
				mesh.nMaterialIndex = (uint8_t) nIndex;

				mesh.nBoneIndex = -1;

				//
				// vertices
				//
				if( f.GetLine( sLine ) <= 0 )
					THROW;

				int nNumVertices = 0;
				if( sscanf (sLine, "%d", &nNumVertices) != 1 )
					THROW;

				Vertices.resize( nNumVertices );

				for( int j = 0; j < nNumVertices; j++ )
				{
					RageModelVertex &v = Vertices[j];

					if( f.GetLine( sLine ) <= 0 )
						THROW;

					if( sscanf(sLine, "%d %f %f %f %f %f %d",
								&nFlags,
								&v.p[0], &v.p[1], &v.p[2],
								&v.t[0], &v.t[1],
								&nIndex
						   ) != 7 )
					{
						THROW;
					}

					// vertex.nFlags = nFlags;
					if( nFlags & 1 )
						v.TextureMatrixScale.x = 0;
					if( nFlags & 2 )
						v.TextureMatrixScale.y = 0;
					v.bone = (uint8_t) nIndex;
					RageVec3AddToBounds( v.p, m_vMins, m_vMaxs );
				}


				//
				// normals
				//
				if( f.GetLine( sLine ) <= 0 )
					THROW;

				int nNumNormals = 0;
				if( sscanf(sLine, "%d", &nNumNormals) != 1 )
					THROW;

				vector<RageVector3> Normals;
				Normals.resize( nNumNormals );
				for( int j = 0; j < nNumNormals; j++ )
				{
					if( f.GetLine( sLine ) <= 0 )
						THROW;

					RageVector3 Normal;
					if( sscanf(sLine, "%f %f %f", &Normal[0], &Normal[1], &Normal[2]) != 3 )
						THROW;

					RageVec3Normalize( (RageVector3*)&Normal, (RageVector3*)&Normal );
					Normals[j] = Normal;
				}

				//
				// triangles
				//
				if( f.GetLine( sLine ) <= 0 )
					THROW;

				int nNumTriangles = 0;
				if( sscanf (sLine, "%d", &nNumTriangles) != 1 )
					THROW;

				Triangles.resize( nNumTriangles );

				for( int j = 0; j < nNumTriangles; j++ )
				{
					if( f.GetLine( sLine ) <= 0 )
						THROW;

					uint16_t nIndices[3];
					uint16_t nNormalIndices[3];
					if( sscanf (sLine, "%d %hd %hd %hd %hd %hd %hd %d",
								&nFlags,
								&nIndices[0], &nIndices[1], &nIndices[2],
								&nNormalIndices[0], &nNormalIndices[1], &nNormalIndices[2],
								&nIndex
						   ) != 8 )
					{
						THROW;
					}

					// deflate the normals into vertices
					for( int k=0; k<3; k++ )
					{
						RageModelVertex& vertex = Vertices[ nIndices[k] ];
						RageVector3& normal = Normals[ nNormalIndices[k] ];
						vertex.n = normal;
						//mesh.Vertices[nIndices[k]].n = Normals[ nNormalIndices[k] ];
					}

					msTriangle& Triangle = Triangles[j];
					// Triangle.nFlags = nFlags;
					memcpy( &Triangle.nVertexIndices, nIndices, sizeof(Triangle.nVertexIndices) );
					// Triangle.nSmoothingGroup = nIndex;
				}
			}
		}
	}

	OptimizeBones();

	if( DISPLAY->SupportsPerVertexMatrixScale() )
	{
		if( m_Meshes.size() == 2  &&  m_Meshes[0].sName == m_Meshes[1].sName )
		{
			MergeMeshes( 1, 0 );
		}
	}

	// send the finalized vertices to the graphics card
	m_pCompiledGeometry->Set( m_Meshes, bNeedsNormals );

	f.Close();
}

/*
 * Copyright (c) 2001-2002 Chris Danford
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
