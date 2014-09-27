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

#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "PS2Defines.h"
#include "pipeline.h"



class CTerrain
{
public:
	CTerrain();
	~CTerrain();
	
	void Render(void) const;
	
	void SetWorldMatrix(const Matrix4x4 & matWorld, const Matrix4x4 & WVP);
	
	void SetWVPMatrix(const Matrix4x4 & matWVP);
	
	void LoadTerrainData(void);
	
protected:

	// Pointer to the data in Static DMA memory
	int m_iStaticAddr;
	
	// The world and WorldViewProjection Matrices
	Matrix4x4 m_World, m_WVP;
	
	// Check variables
	bool m_bWorldLoaded;	
};


#endif