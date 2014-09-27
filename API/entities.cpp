/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.2 $
* $Date: 2007/08/19 12:45:09 $
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
#include "entities.h"

// Initialise static member
int Cube::iStaticAddr = 0;

Cube::Cube()
{
	// Initialise some default vaules
	World = Matrix4x4::IDENTITY;
	WVP = Matrix4x4::IDENTITY;
	WorldLoaded = false;
	LoadCubeData();
}

Cube::~Cube()
{
	// Nothing to destruct
}

// Load the world matrix
void Cube::SetWorldMatrix(const Matrix4x4 & matWorld, const Matrix4x4 & VP )
{
	World = matWorld;
	WVP = World * VP;
	WorldLoaded = true;
}

// Load the cube data into static DMA memory
void Cube::LoadCubeData(void)
{
	// Check if data already loaded - return if it is.
	// We only need the cube data loaded once.
	if(Cube::iStaticAddr)return;
	
	const int iDraw = 0;
	const int iDontDraw = 1;
		
	const float fDraw = *((float *)&iDraw);
	const float fDontDraw = *((float *)&iDontDraw);
		
	printf("Loading Cube Data\n");
	
	// Get the address of static data that we can call to later.
	Cube::iStaticAddr = VIFStaticDMA.GetPointer();
	
	VIFStaticDMA.Add32(FLUSH);			// Make sure VU1 isn't busy
	VIFStaticDMA.Add32(STCYCL(1,1));	// Unpack linearly, i.e. don't skip any spaces
	VIFStaticDMA.Add32(BASE(32));		// The double buffers start at VU1 address 32 (giving us 32 QW to store data that won't change)
	VIFStaticDMA.Add32(OFFSET(496));	// The size of each buffer.


	// The first face draws two triangle
	// Next faces draw four triangles
	// Five triangles needed for last face
	// Some triangles are "zero" area and will be culled by GS
	int iTriangles = 12;
	int iVerts = iTriangles * 3;
	
	// We want to unpack 3 quad words of misc data, and then 3QW per vertex starting at
	// VU mem location 32 (we will upload the matrices to the first 16QW per frame).
	VIFStaticDMA.AddUnpack(V4_32, 0, 2 + iVerts * 3, 1);
	
	// VU32 iVerts is in position "x" LSWord of the qword
	VIFStaticDMA.Add128(iVerts*3);
	
	// Add the GIFTag VU33
	VIFStaticDMA.Add128(GS_GIFTAG_BATCH(	iVerts,						// NLOOP
	 										1,							// EOP
	 										1,							// PRE
	 										GS_PRIM(PRIM_TRI,		 	// PRIM
													PRIM_IIP_GOURAUD, 
													PRIM_TME_ON,
													PRIM_FGE_OFF, 
													PRIM_ABE_ON, 
													PRIM_AA1_OFF, 
													PRIM_FST_STQ, 
													PRIM_CTXT_CONTEXT1, 
													PRIM_FIX_NOFIXDDA),					
	 										GIF_FLG_PACKED,				// FLG
	 										GS_BATCH_3(	GIF_REG_ST,
	 													GIF_REG_RGBAQ, 	// BATCH
	 													GIF_REG_XYZ2)));
	
	// Front face
	VIFStaticDMA.AddVector(Vector4(0, 0, 1, fDraw));			// TexCoord (STQ)	
	VIFStaticDMA.AddVector(Vector4( 0, 0, 1, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f, 1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0, 1, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f,  1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0, 1, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,-1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0, 1, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,-1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0, 1, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f,  1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1 , 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0, 1, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f, -1.0f, 1.0f, 1.0f));	// Vert (xyzw)

	// Right face
	VIFStaticDMA.AddVector(Vector4(0, 0, 1, fDraw));			// TexCoord (STQ)	
	VIFStaticDMA.AddVector(Vector4( 1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f, 1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f,  1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f,-1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f,-1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f, 1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1 , 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f, -1.0f, -1.0f, 1.0f));	// Vert (xyzw)

	// Back face
	VIFStaticDMA.AddVector(Vector4(1, 0, 1, fDraw));			// TexCoord (STQ)	
	VIFStaticDMA.AddVector(Vector4( 0, 0,-1, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f, 1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 1, 1, 0));				// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0,-1, fDraw));			// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,-1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 0, 1, 0));				// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0,-1, fDraw));			// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f,  1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 1, 1, 0));				// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0,-1, fDraw));			// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,-1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0 , 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0,-1, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f, -1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4( 0, 0,-1, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f,  1.0f, -1.0f, 1.0f));	// Vert (xyzw)

	// Left face
	VIFStaticDMA.AddVector(Vector4(1, 0, 1, fDraw));			// TexCoord (STQ)	
	VIFStaticDMA.AddVector(Vector4(-1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f, 1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(-1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,  1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(-1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,-1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(-1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,-1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(-1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f, 1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0 , 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(-1, 0, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f, -1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	
	// Top face
	VIFStaticDMA.AddVector(Vector4(1, 0, 1, fDraw));			// TexCoord (STQ)	
	VIFStaticDMA.AddVector(Vector4(0, 1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f, 1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, 1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f,  1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, 1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f, 1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, 1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f, 1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, 1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f, 1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0 , 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, 1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,  1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	
	// Bottom face
	VIFStaticDMA.AddVector(Vector4(0, 0, 1, fDraw));			// TexCoord (STQ)	
	VIFStaticDMA.AddVector(Vector4(0, -1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f, -1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, -1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f,  -1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, -1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,-1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(0, 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, -1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f,-1.0f, 1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1, 0, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, -1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(1.0f, -1.0f, -1.0f, 1.0f));	// Vert (xyzw)
	VIFStaticDMA.AddVector(Vector4(1 , 1, 1, fDraw));			// TexCoord (STQ)
	VIFStaticDMA.AddVector(Vector4(0, -1, 0, 0));				// Normal
	VIFStaticDMA.AddVector(Vector4(-1.0f, -1.0f, -1.0f, 1.0f));	// Vert (xyzw)

	
	// Flush to make sure VU1 isn't doing anything.
	VIFStaticDMA.Add32(FLUSH);
	// Then run the microcode.
	VIFStaticDMA.Add32(MSCALL(0));

	// Return back to the dynamic chain (note that we aren't aligned on a QWORD boundary, but
	// it doesn't matter because the packet class will sort that out for us).
	VIFStaticDMA.DMARet();
}
	

// Render the cube.
void Cube::Render(void) const
{
	// check world matrix has been loaded (at least once!)
	// and that the DMA data has been initialised
	// Bomb out if they havn't since something is wrong
	assert(WorldLoaded && (Cube::iStaticAddr));
	
	// Wait for all to be idle in VU1 land
	// Before uploading the data
	VIFDynamicDMA.Add32(FLUSH);
	
	// Upload the matrices in the dynamic buffer
	// because they can change every frame.
	VIFDynamicDMA.AddUnpack(V4_32, 9, 8);
	VIFDynamicDMA.AddMatrix(WVP);
	VIFDynamicDMA.AddMatrix(World);
	VIFDynamicDMA.DMACall(Cube::iStaticAddr);
}


