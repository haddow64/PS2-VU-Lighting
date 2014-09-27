/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.2 $
* $Date: 2007/08/19 12:45:12 $
*
*/

#include <sps2lib.h>
#include <sps2tags.h>
#include <sps2util.h>
#include <sps2regstructs.h>
#include "PS2Defines.h"
#include "ps2matrix4x4.h"
#include "sps2wrap.h"
#include "dma.h"
#include "terrain.h"


CTerrain::CTerrain()
{
	// Initialise some default vaules
	m_iStaticAddr = 0;
	m_World = Matrix4x4::IDENTITY;
	m_WVP = Matrix4x4::IDENTITY;
	m_bWorldLoaded = false;
	LoadTerrainData();
}

CTerrain::~CTerrain()
{
	// Nothing to destruct
}

void CTerrain::SetWorldMatrix(const Matrix4x4 & matWorld, const Matrix4x4 & WVP )
{
	m_World = matWorld;
	m_WVP = m_World *WVP;
	m_bWorldLoaded = true;
}
	
void CTerrain::SetWVPMatrix(const Matrix4x4 & matWVP)
{
	m_WVP = matWVP;
}



void CTerrain::LoadTerrainData(void)
{
	// Get the address that we can call to later.
	m_iStaticAddr = VIFStaticDMA.GetPointer();

	VIFStaticDMA.Add32(FLUSH);			// Make sure VU1 isn't busy
	VIFStaticDMA.Add32(STCYCL(1,1));	// Unpack linearly, i.e. don't skip any spaces
	VIFStaticDMA.Add32(BASE(32));		// The double buffers start at VU1 address 16 (giving us 16 QW to store data that won't change)
	VIFStaticDMA.Add32(OFFSET(496));	// The size of each buffer.

	VIFStaticDMA.AddUnpack(V4_32, 0, 1);	// Unpack 8QW of data that won't change
	VIFStaticDMA.AddVector(Vector4(2048, 2048, (0xFFFFFF / 32.0f), 1));	// 0: The scales

	// Set the PRIM field here. This may look like it's for convenience, but actually it's the only
	// way we can do it. If we set this in the GIFTag it would reset the strip every buffer.
	// As we want one huge strip, we can't use the prim setting capabilites of the GIFTag.
	VIFStaticDMA.StartDirect();
	VIFStaticDMA.StartAD();
	VIFStaticDMA.AddAD(PRIM_SET(GS_TRISTRIP, 1, 1, 0, 0, 0, 0, 0, 0), PRIM);
	VIFStaticDMA.EndAD();
	VIFStaticDMA.EndDirect();
	

	// The verts are 3QW each, and the 504QW is split in half. Therefore we have 252QW for each buffer.
	// Each buffer also has 2QW of info (GIFTag + Numverts), so we can process 83 vertices (249QW)
	// in each buffer.
	int iNumVertsPerChunk = 82;

	// The number of triangles in each dimension.
	int iSizeX = 64;
	int iSizeY = 64;

	int iNumVerts = (iSizeX * 2) * (iSizeY - 1);

	int iX = 0, iY = 0;
	int bBottomRow = 0;
	bool bFirst = true;

	while(iNumVerts > 0)
	{
		int iVertsThisChunk;

		if(iNumVerts > iNumVertsPerChunk)
		{
			iVertsThisChunk = iNumVertsPerChunk;
			iNumVerts -= iVertsThisChunk;
		}
		else
		{
			iVertsThisChunk = iNumVerts;
			iNumVerts -= iVertsThisChunk;
		}

		VIFStaticDMA.AddUnpack(V4_32, 0, iVertsThisChunk * 3 + 2, 1);
		VIFStaticDMA.Add128(iVertsThisChunk * 3);
		VIFStaticDMA.Add128(
			GS_GIFTAG_BATCH(iVertsThisChunk,
					1, 0, 0, GIF_FLG_PACKED,
					GS_BATCH_3(GIF_REG_ST, GIF_REG_RGBAQ, GIF_REG_XYZ2)));

		// Zigzag from left to right across the patch making a huge triangle strip.
		//
		//	0	   	2		4		6
		//  |     / |     / |     / |
  		//  |   /   |   /   |   /   |
		//	1 /		3 /		5 /		7 etc.
		//
		for(int iVert = 0; iVert < iVertsThisChunk; iVert++)
		{
			float fX = (float)iX;
			float fY = (float)(iY + bBottomRow);

			// Disable the drawing kick when we move to the next row, so that we don't get a huge
			// polygon going across the patch.
			int iDontDraw = (iX == 0) ? 1 : 0;

			// Alternate between the current row and the one below.
			iX += bBottomRow;
			bBottomRow ^= 1;

			// If we have reached the end, start again on the next row.
			if(iX == iSizeX)
			{
				iX = 0;
				bBottomRow = 0;
				iY += 1;
			}

			// Add a huge number of vertices 
			VIFStaticDMA.AddVector(Vector4((fX / iSizeX)*2, (fY / iSizeY)*4, 1, *((float *)&iDontDraw)));	// Tex coords + in W, a flag to set ADC or not.
			VIFStaticDMA.AddVector(Vector4(0, 1, 0, 0));											// Normal
			VIFStaticDMA.AddVector(Vector4((fX - (iSizeX / 2)), 0, (fY - (iSizeY / 2)), 1));		// Position
		}

		if(bFirst)
		{
			bFirst = false;
			// If this is the first buffer then run the VU code from address 0 so that the initialisation
			// VU code is ran.
			VIFStaticDMA.Add32(MSCALL(0));
		}
		else
		{
			// For multiple times MSCNT means to continue where the VU code finished last time at the --cont.
			VIFStaticDMA.Add32(MSCNT);
		}

		// Notice that due to the quad buffer setup we don't need to flush here because we can upload, process, and render
		// all in parallel now. (the XGKick in the vector code will flush to make sure that we don't get ahead of ourselves)
	}

	VIFStaticDMA.Add32(FLUSH);
	VIFStaticDMA.DMARet();
	
	
	
	
}


void CTerrain::Render(void) const
{
	assert(m_bWorldLoaded && m_iStaticAddr);
	
	// Wait for all to be idle in VU1 land
	// Before uploading the data
	VIFDynamicDMA.Add32(FLUSH);
	
	// Upload the matrices in the dynamic buffer
	// because they can change every frame.
	VIFDynamicDMA.AddUnpack(V4_32, 9, 8);
	VIFDynamicDMA.AddMatrix(m_WVP);
	VIFDynamicDMA.AddMatrix(m_World);

	// Call the packet with the static data that doesn't change per frame.
	VIFDynamicDMA.DMACall(m_iStaticAddr);
}