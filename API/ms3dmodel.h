/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.2 $
* $Date: 2007/08/19 12:45:10 $
*
*/

#ifndef __MS3DMODEL_H__
#define __MS3DMODEL_H__

#include <stdio.h>
#include "pipeline.h"
#include "ps2matrix4x4.h"

// Internal structure used by the Loading code
struct MSVertex
{
	// flags, x, y, z, u, v, bone index
	int flags;
	float x, y, z, u, v;
	int bone;
	int _pad; // 16 byte align
};

// Internal structure used by the Loading code
struct MSTriangle
{
	// triangle: flags, vertex index1, vertex index2, vertex index3, 
	// normal index1, normal index 2, normal index 3, smoothing group
	int flags, vert1, vert2, vert3,
			   norm1, norm2, norm3, smoothinggroup;
};

// Internal structure used by the Loading code
struct Vector3
{
	float x, y, z;
};

// Internal structure used by the Loading code
struct MSMesh
{
	// mesh: name, flags, material index
	char strName[256];
	int flags, matindex;

	int nTriangles,
		nNormals,
		nVerts;

	MSVertex   * pVertices;
	Vector3    * pNormals;
	MSTriangle * pTriangles;
};

class CMs3dModel
{
public:
	CMs3dModel(void);
	~CMs3dModel(void);
	
	void SetWorldMatrix(const Matrix4x4 & matWorld);
	
	void SetWVPMatrix(const Matrix4x4 & matWVP);

	bool LoadModelData(const char * strFilename, bool bTransparent = false);
	
	void Render(void);

	char * GetTextureName( )
	{ return m_strTexname; }

protected:
	void GetNextLOC(char *pStr1024);
	void InitialiseDMA(bool bTransparent);

	MSMesh * m_pMeshes;

	char m_strTexname[256];

	int m_iStaticAddr;
	int m_nFrames, m_nCurrentFrame, m_nMeshes;
	FILE * m_fp;
	
	// The world and WorldViewProjection Matrices
	Matrix4x4 m_World, m_WVP;
	
	bool m_bWorldLoaded;
};

#endif
