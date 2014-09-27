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


#include "ms3dmodel.h"
#include "dma.h"
#include <memory.h>

// Uncomment next line to check loading of model data
//#define MSVERBOSE


CMs3dModel::CMs3dModel(void)
:m_pMeshes(NULL)
{	
	// Initialise some default vaules
	m_pMeshes = NULL;
	m_iStaticAddr = 0;
	m_World = Matrix4x4::IDENTITY;
	m_WVP = Matrix4x4::IDENTITY;
	m_bWorldLoaded = false;
	
}

CMs3dModel::~CMs3dModel(void)
{
	for(int nMesh = 0; nMesh < m_nMeshes; nMesh++)
	{
		if(m_pMeshes[nMesh].pVertices != NULL)
		{
			#ifdef MSVERBOSE
			printf("Delete m_pMeshes[%d].pVertices\n", nMesh);
			#endif
			delete [] m_pMeshes[nMesh].pVertices;
			m_pMeshes[nMesh].pVertices = NULL;
		}
		
		if(m_pMeshes[nMesh].pNormals != NULL)
		{
			#ifdef MSVERBOSE
			printf("Delete m_pMeshes[%d].pNormals\n", nMesh);
			#endif
			delete [] m_pMeshes[nMesh].pNormals;
			m_pMeshes[nMesh].pNormals = NULL;
		}
		
		if(m_pMeshes[nMesh].pTriangles != NULL)
		{
			#ifdef MSVERBOSE
			printf("Delete m_pMeshes[%d].pTriangles\n", nMesh);
			#endif
			delete [] m_pMeshes[nMesh].pTriangles;
			m_pMeshes[nMesh].pTriangles = NULL;
		}
	}

	if(m_pMeshes != NULL)
	{
		#ifdef MSVERBOSE
		printf("Delete m_pMeshes\n");
		#endif
		delete [] m_pMeshes;
		m_pMeshes = NULL;
	}	
}

void CMs3dModel::SetWorldMatrix(const Matrix4x4 & matWorld)
{
	m_World = matWorld;
	m_WVP = m_World * Pipeline.GetViewProjection();
	m_bWorldLoaded = true;
}
	
void CMs3dModel::SetWVPMatrix(const Matrix4x4 & matWVP)
{
	m_WVP = matWVP;
}

// This function reads a whole line (maximum of 1024 characters) from the file
void CMs3dModel::GetNextLOC(char *pStr1024)
{
	bool foundcode = false;

	while(foundcode != true)
	{
		fgets(pStr1024, 1024, m_fp);

		if((pStr1024[0] != '/' &&
		   pStr1024[1] != '/') &&
		   pStr1024[0] != '\r')
		   foundcode = true;
	}
}

bool CMs3dModel::LoadModelData(const char *strFilename, bool bTransparent)
{
	m_fp = fopen(strFilename, "ra");

	if(m_fp == NULL)
	{
		printf("Failed to load model %s.\n", strFilename);
		exit(-1);
	}

	// The array that will be used to read the file one line at a time
	char strLine[1024];

	// Frames
	GetNextLOC(strLine);
	sscanf(strLine, "Frames: %d", &m_nFrames);
	
#ifdef MSVERBOSE
	printf("Model: %s\nFrames: %d\n", strFilename, m_nFrames);
#endif

	// Current frame
	GetNextLOC(strLine);
	sscanf(strLine, "Frame: %d", &m_nCurrentFrame);
	
#ifdef MSVERBOSE
	printf("Current Frame: %d\n", m_nCurrentFrame);
#endif

	// No. meshes
	GetNextLOC(strLine);
	sscanf(strLine, "Meshes: %d", &m_nMeshes);
	
#ifdef MSVERBOSE
	printf("Meshes: %d\n", m_nMeshes);
#endif

	m_pMeshes = new MSMesh[m_nMeshes];
	memset(m_pMeshes, 0, m_nMeshes * sizeof(MSMesh));

	for(int nMesh = 0; nMesh < m_nMeshes; nMesh++)
	{
		GetNextLOC(strLine);
		// mesh: name, flags, material index
		sscanf(strLine,	"\"%[^\"]\" %d %d", m_pMeshes[nMesh].strName,
						&m_pMeshes[nMesh].flags,
						&m_pMeshes[nMesh].matindex);

#ifdef MSVERBOSE
		printf("%s %d %d\n",	m_pMeshes[nMesh].strName,
								m_pMeshes[nMesh].flags,
								m_pMeshes[nMesh].matindex);
#endif


		// No. vertices
		GetNextLOC(strLine);
		sscanf(strLine, "%d", &m_pMeshes[nMesh].nVerts);

#ifdef MSVERBOSE
		printf("Vertices: %d\n", m_pMeshes[nMesh].nVerts);
#endif

		m_pMeshes[nMesh].pVertices = new MSVertex[m_pMeshes[nMesh].nVerts];

		// The vertices
		for(int nVert = 0; nVert < m_pMeshes[nMesh].nVerts; nVert++)
		{
			GetNextLOC(strLine);
			// flags, x, y, z, u, v, bone index
			sscanf(strLine, "%d %f %f %f %f %f %d", 
				&m_pMeshes[nMesh].pVertices[nVert].flags,
				&m_pMeshes[nMesh].pVertices[nVert].x,
				&m_pMeshes[nMesh].pVertices[nVert].y,
				&m_pMeshes[nMesh].pVertices[nVert].z,
				&m_pMeshes[nMesh].pVertices[nVert].u,
				&m_pMeshes[nMesh].pVertices[nVert].v,
				&m_pMeshes[nMesh].pVertices[nVert].bone);

#ifdef MSVERBOSE
			printf("%d %f %f %f %f %f %d\n", 
				m_pMeshes[nMesh].pVertices[nVert].flags,
				m_pMeshes[nMesh].pVertices[nVert].x,
				m_pMeshes[nMesh].pVertices[nVert].y,
				m_pMeshes[nMesh].pVertices[nVert].z,
				m_pMeshes[nMesh].pVertices[nVert].u,
				m_pMeshes[nMesh].pVertices[nVert].v,
				m_pMeshes[nMesh].pVertices[nVert].bone);
#endif
				
				
		}

		// No. normals
		GetNextLOC(strLine);
		sscanf(strLine, "%d", &m_pMeshes[nMesh].nNormals);

#ifdef MSVERBOSE
		printf("Normals: %d\n", m_pMeshes[nMesh].nNormals);
#endif


		m_pMeshes[nMesh].pNormals = new Vector3[m_pMeshes[nMesh].nNormals];

		// The normals
		for(int nNormal = 0; nNormal < m_pMeshes[nMesh].nNormals; nNormal++)
		{
			GetNextLOC(strLine);

			// x, y, z
			sscanf(strLine, "%f %f %f",
				&m_pMeshes[nMesh].pNormals[nNormal].x,
				&m_pMeshes[nMesh].pNormals[nNormal].y,
				&m_pMeshes[nMesh].pNormals[nNormal].z);
				
#ifdef MSVERBOSE
			printf("%f %f %f\n",
				m_pMeshes[nMesh].pNormals[nNormal].x,
				m_pMeshes[nMesh].pNormals[nNormal].y,
				m_pMeshes[nMesh].pNormals[nNormal].z);
#endif				
				

		}

		// No. triangles
		GetNextLOC(strLine);
		sscanf(strLine, "%d", &m_pMeshes[nMesh].nTriangles);
		
#ifdef MSVERBOSE
		printf("Triangles: %d\n", m_pMeshes[nMesh].nTriangles);
#endif

		m_pMeshes[nMesh].pTriangles = new MSTriangle[m_pMeshes[nMesh].nTriangles];

		// The triangles
		for(int nTriangle = 0; nTriangle < m_pMeshes[nMesh].nTriangles; nTriangle++)
		{
			GetNextLOC(strLine);

			// triangle: flags, vertex index1, vertex index2, vertex index3, 
			// normal index1, normal index 2, normal index 3, smoothing group
			sscanf(strLine, "%d %d %d %d %d %d %d %d",
				&m_pMeshes[nMesh].pTriangles[nTriangle].flags,
				&m_pMeshes[nMesh].pTriangles[nTriangle].vert1,
				&m_pMeshes[nMesh].pTriangles[nTriangle].vert2,
				&m_pMeshes[nMesh].pTriangles[nTriangle].vert3,
				&m_pMeshes[nMesh].pTriangles[nTriangle].norm1,
				&m_pMeshes[nMesh].pTriangles[nTriangle].norm2,
				&m_pMeshes[nMesh].pTriangles[nTriangle].norm3,
				&m_pMeshes[nMesh].pTriangles[nTriangle].smoothinggroup);

#ifdef MSVERBOSE
			printf("%d %d %d %d %d %d %d %d\n",
				m_pMeshes[nMesh].pTriangles[nTriangle].flags,
				m_pMeshes[nMesh].pTriangles[nTriangle].vert1,
				m_pMeshes[nMesh].pTriangles[nTriangle].vert2,
				m_pMeshes[nMesh].pTriangles[nTriangle].vert3,
				m_pMeshes[nMesh].pTriangles[nTriangle].norm1,
				m_pMeshes[nMesh].pTriangles[nTriangle].norm2,
				m_pMeshes[nMesh].pTriangles[nTriangle].norm3,
				m_pMeshes[nMesh].pTriangles[nTriangle].smoothinggroup);
#endif				

		}
	}

	// Get the number of materials (we don't care, we'll just use the first one)
	GetNextLOC(strLine);

	// The material name (enclosed in double quotes)
	GetNextLOC(strLine);
	sscanf(strLine, "\"%[^\"]\"", m_strTexname);

#ifdef MSVERBOSE
	printf("%s\n", m_strTexname);
#endif


	fclose(m_fp);

	InitialiseDMA(bTransparent);

	return true;
}

void CMs3dModel::InitialiseDMA(bool bTransparent)
{
	m_iStaticAddr = VIFStaticDMA.GetPointer();

	VIFStaticDMA.Add32(FLUSH);			// Make sure VU1 isn't busy
	VIFStaticDMA.Add32(STCYCL(1,1));	// Unpack linearly, i.e. don't skip any spaces
	VIFStaticDMA.Add32(BASE(32));		// The double buffers start at VU1 address 16 (giving us 16 QW to store data that won't change)
	VIFStaticDMA.Add32(OFFSET(496));	// The size of each buffer.

	VIFStaticDMA.AddUnpack(V4_32, 0, 1);	// Unpack 8QW of data that won't change
	VIFStaticDMA.AddVector(Pipeline.GetScaleVector());	// 0: The scales


	int iNumTrisPerChunk = 27;
	bool bFirst = true;
	
	//printf("m_nMeshes = %d\n", m_nMeshes);
	
	for(int nMesh = 0; nMesh < m_nMeshes; nMesh++)
	//for(int nMesh = 0; nMesh < 1; nMesh++)
	{
	
		MSMesh & pMesh = m_pMeshes[nMesh];
		int iNumTris = pMesh.nTriangles;
		int iAbsoluteTri = 0;
	
		
	
		while(iNumTris > 0)
		{
			int iTrisThisChunk;
	
			if(iNumTris > iNumTrisPerChunk)
			{
				iTrisThisChunk = iNumTrisPerChunk;
				iNumTris -= iTrisThisChunk;
			}
			else
			{
				iTrisThisChunk = iNumTris;
				iNumTris -= iTrisThisChunk;
			}
	
			VIFStaticDMA.AddUnpack(V4_32, 0, iTrisThisChunk * 9 + 2, 1);
	
			VIFStaticDMA.Add128(iTrisThisChunk * 9);
	
			VIFStaticDMA.Add128(
				GS_GIFTAG_BATCH(iTrisThisChunk * 3, // NLOOP
						1,	// EOP
						1,	// PRE
						// GS_PRIM(    PRIM, IIP, TME, FGE,  ABE         , AA1, FST, CTXT, FIX)
						GS_PRIM(GS_TRIANGLE,  1,   1,   0 , bTransparent ,   0,   0,    0,   0),	// PRIM
						GIF_FLG_PACKED,
						GS_BATCH_3(GIF_REG_ST, GIF_REG_RGBAQ, GIF_REG_XYZ2)));
	
			for(int iTriangle = 0; iTriangle < iTrisThisChunk; iTriangle++)
			{
				MSTriangle & pTriangle = pMesh.pTriangles[iAbsoluteTri++];
	
				MSVertex & pVert1 = pMesh.pVertices[pTriangle.vert1];
				MSVertex & pVert2 = pMesh.pVertices[pTriangle.vert2];
				MSVertex & pVert3 = pMesh.pVertices[pTriangle.vert3];
	
				Vector3 & pNorm1 = pMesh.pNormals[pTriangle.norm1];
				Vector3 & pNorm2 = pMesh.pNormals[pTriangle.norm2];
				Vector3 & pNorm3 = pMesh.pNormals[pTriangle.norm3];
	
				VIFStaticDMA.AddVector(Vector4(pVert1.u, pVert1.v, 1, 0));
				VIFStaticDMA.AddVector(Vector4(pNorm1.x, pNorm1.y, pNorm1.z, 0));
				VIFStaticDMA.AddVector(Vector4(pVert1.x, pVert1.y, pVert1.z, 1));
			
				VIFStaticDMA.AddVector(Vector4(pVert2.u, pVert2.v, 1, 0));
				VIFStaticDMA.AddVector(Vector4(pNorm2.x, pNorm2.y, pNorm2.z, 0));
				VIFStaticDMA.AddVector(Vector4(pVert2.x, pVert2.y, pVert2.z, 1));
	
				VIFStaticDMA.AddVector(Vector4(pVert3.u, pVert3.v, 1, 0));
				VIFStaticDMA.AddVector(Vector4(pNorm3.x, pNorm3.y, pNorm3.z, 0));
				VIFStaticDMA.AddVector(Vector4(pVert3.x, pVert3.y, pVert3.z, 1));
			}
			
			if(bFirst)
			{
				VIFStaticDMA.Add32(MSCALL(0));
				bFirst = false;
			}
			else
				VIFStaticDMA.Add32(MSCNT);
		}
	}

	VIFStaticDMA.Add32(FLUSH);

	VIFStaticDMA.DMARet();
}

void CMs3dModel::Render(void)
{	
	assert(m_bWorldLoaded && m_iStaticAddr);
	
	// Wait for all to be idle in VU1 land
	// Before uploading the data
	VIFDynamicDMA.Add32(FLUSH);
	
	// Upload the matrices in the dynamic buffer
	// because they can change every frame.
	VIFDynamicDMA.AddUnpack(V4_32, 1, 16);
	VIFDynamicDMA.AddMatrix(Pipeline.GetLightDirs());
	VIFDynamicDMA.AddMatrix(Pipeline.GetLightCols());
	VIFDynamicDMA.AddMatrix(m_WVP);
	VIFDynamicDMA.AddMatrix(m_World);
	
	// Call the packet with the static data that doesn't change per frame.
	VIFDynamicDMA.DMACall(m_iStaticAddr);
}
