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

#ifndef _ENTITIES_H_
#define _ENTITIES_H_

#include "PS2Defines.h"
#include "pipeline.h"

class Cube
{
public:
	Cube();
	~Cube();
	
	void Render(void) const;
	
	void SetWorldMatrix(const Matrix4x4 & matWorld, const Matrix4x4 & VP );
	
	static void LoadCubeData(void);
	
	inline float GetX(void) const {return World(3,0);}
	
	inline float GetY(void) const {return World(3,1);}
	
	inline float GetZ(void) const {return World(3,2);}


protected:
	
	// Pointer to the data in Static DMA memory
	static int iStaticAddr;
	
	// The world and WorldViewProjection Matrices
	Matrix4x4 World, WVP;
	
	// Check variables
	bool WorldLoaded;
};


#endif