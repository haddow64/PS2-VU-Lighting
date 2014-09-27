;/*
;* "PS2" Application Framework
;*
;* University of Abertay Dundee
;* May be used for educational purposed only
;*
;* Author - Dr Henry S Fortuna
;*
;* $Revision: 1.2 $
;* $Date: 2007/08/19 12:45:13 $
;*
;*/

; The static (or initialisation buffer) i.e. stuff that doesn't change for each
; time this code is called.
Scales		.assign 0
LightDirs	.assign 1
LightCols	.assign 5
Transform	.assign 9 
LightTrans	.assign 13
PLightCol	.assign 17
PLightPos	.assign 18
CamPos	.assign 19

; The input buffer (relative the the start of one of the double buffers)
NumVerts	.assign 0 
GifPacket  	.assign	1 
UVStart	.assign 2
NormStart	.assign 3
VertStart	.assign 4

; The output buffer (relative the the start of one of the double buffers)
GifPacketOut	.assign 248
UVStartOut		.assign 249
NormStartOut	.assign 250
VertStartOut	.assign 251


; Note that we have 4 data buffers: InputBuffer0, OutputBuffer0, InputBuffer1, and OutputBuffer1.
; Each buffer is 248 quad words. The different buffers are selected by reading the current offset
; from xtop (which is swapped automatically by the PS2 after each MSCALL / MSCNT).


.include "vcl_sml.i"

.init_vf_all
.init_vi_all
.syntax new

.vu

--enter
--endenter

; The START or Init code, that is only called once per frame.
START:
	fcset		0x000000

	lq			fTransform[0], Transform+0(vi00)
	lq			fTransform[1], Transform+1(vi00)
	lq			fTransform[2], Transform+2(vi00)
	lq			fTransform[3], Transform+3(vi00)
	lq			fLightTrans[0], LightTrans+0(vi00)
	lq			fLightTrans[1], LightTrans+1(vi00)
	lq			fLightTrans[2], LightTrans+2(vi00)
	lq			fLightTrans[3], LightTrans+3(vi00)
	lq			fScales, Scales(vi00)

; Transform the light position into world space
	lq			LightPos, PLightPos(vi00)
	mul         acc,  fLightTrans[0], LightPos[x]; Transform it
      madd        acc,  fLightTrans[1],  LightPos[y]
      madd        acc,  fLightTrans[2],  LightPos[z]; missing a multiplication to save speed
	;MatrixMultiplyVertex , fLightTrans , LightPos	; This begin code is called once per batch

begin:
	xtop		iDBOffset		; Load the address of the current buffer (will either be QW 16 or QW 520)
	ilw.x		iNumVerts, NumVerts(iDBOffset)
	iadd		iNumVerts, iNumVerts, iDBOffset
	iadd		Counter, vi00, iDBOffset

loop:
	lq			Vert, VertStart(Counter)		; Load the vertex from the input buffer

	mul  		Vertw , Vert, vf00[w]; store a copy of the vertex in local space

;transform to clip space
	mul         acc,  fTransform[0],  Vert[x]	; Transform it
      madd        acc,  fTransform[1],  Vert[y]
      madd        acc,  fTransform[2],  Vert[z]
      madd        Vert, fTransform[3],  Vert[w]

      clipw.xyz	Vert, Vert						; Clip it
	fcand		vi01, 0x3FFFF
	iaddiu		iADC, vi01, 0x7FFF
	ilw.w		iNoDraw, UVStart(Counter)		; Load the iNoDraw flag. If true we should set the ADC bit so the vert isn't drawn
	iadd		iADC, iADC, iNoDraw
	isw.w		iADC, VertStartOut(Counter)
	div         q,    vf00[w], Vert[w]			
	lq			UV,   UVStart(Counter)			; Handle the tex-coords
	mul			UV,   UV, q
	sq			UV,   UVStartOut(Counter)
	mul.xyz     Vert, Vert, q					; Scale the final vertex to fit to the screen.
	mula.xyz	acc, fScales, vf00[w]
	madd.xyz	Vert, Vert, fScales
	ftoi4.xyz	Vert, Vert
	sq.xyz		Vert, VertStartOut(Counter)		; And store in the output buffer
	
; transform verticies to world space
	mul         acc,  fLightTrans[0],  Vertw[x]	; Transform it
      madd        acc,  fLightTrans[1],  Vertw[y]
      madd        acc,  fLightTrans[2],  Vertw[z]
	

;transform normals to world space
	lq			Norm, NormStart(Counter)				; Load the normal
	mul         acc,  fLightTrans[0],  Norm[x]	; Transform it
      madd        acc,  fLightTrans[1],  Norm[y]
      madd        acc,  fLightTrans[2],  Norm[z]
	;MatrixMultiplyVertex Norm, fLightTrans, Norm		; Transform the normal into world space (important that norm.w is 0)

; find the to light vector
	sub.xyz		ToLight, LightPos, Vertw					; Get the vector from the vert to the light
	VectorNormalizeXYZ ToLight, ToLight					; Normalise this vector (and it becomes the light direction, just as in directional lighting)



;Diffuse lighting
    lq.xyz		fLightDirs[0], LightDirs+0(vi00)		; Load the light directions
    lq.xyz		fLightDirs[1], LightDirs+1(vi00)
    lq.xyz		fLightDirs[2], LightDirs+2(vi00)
    mula.xyz	acc, fLightDirs[0], Norm[x]				; "Transform" the normal by the light direction matrix
    madd.xyz	acc, fLightDirs[1], Norm[y]				; This has the effect of outputting a vector with all
    madd.xyz	fIntensities, fLightDirs[2], Norm[z]	; four intensities, one for each light.
    mini.xyz	fIntensities, fIntensities, vf00[w]		; Clamp the intensity to 0..1
    max.xyz		fIntensities, fIntensities, vf00[x]
    lq.xyz		fLightCols[0], LightCols+0(vi00)		; Load the light colours
    lq.xyz		fLightCols[1], LightCols+1(vi00)
    lq.xyz		fLightCols[2], LightCols+2(vi00)
    lq.xyz		fAmbient, LightCols+3(vi00)
    mula.xyz	acc, fLightCols[0], fIntensities[x]		; Transform the intensities by the light colour matrix
    madda.xyz	acc, fLightCols[1], fIntensities[y]		; This gives the final total directional light colour
    madda.xyz	acc, fLightCols[2], fIntensities[z]
    madd.xyz	fIntensities, fAmbient, vf00[w]

;Point Light Calculations
	VectorDotProduct  fIntensity, ToLight, Norm			; Get the intensity

;specular highlights
	lq.xyz		Cam, CamPos(vi00)
	sub.xyz		Cam, Cam, Vert
	VectorNormalizeXYZ Cam, Cam
	add.xyz		HalfVec, Cam, ToLight
	VectorNormalizeXYZ HalfVec, HalfVec
	VectorDotProduct  fSpecIntensity, HalfVec, Norm
	max.x		fSpecIntensity, fSpecIntensity, vf00			; Clamp to > 0
	mul.x		fSpecIntensity, fSpecIntensity, fSpecIntensity	; Square it
	mul.x		fSpecIntensity, fSpecIntensity, fSpecIntensity	; 4th power
	mul.x		fSpecIntensity, fSpecIntensity, fSpecIntensity	; 8th power
	mul.x		fSpecIntensity, fSpecIntensity, fSpecIntensity	; 16th power
	mul.x		fSpecIntensity, fSpecIntensity, fSpecIntensity	; 32nd power
	add.x		fIntensity, fIntensity, fSpecIntensity	; add the specular and point intensities


;calculate the final intensity
	max.x			fIntensity, fIntensity, vf00			; Clamp to > 0
	mini.x		fIntensity, fIntensity, vf00[w]			; Clamp to < 1
	lq			fLightCol, PLightCol(vi00)				; Load the light colour
	mul.xyz		fLightCol, fLightCol, fIntensity[x]		; Scale the colour by the intensity
	
	add.xyz  	 fIntensities, fLightCol, fIntensities

    loi			128										; Load 128 and put it into the alpha value
	addi.w		fIntensities, vf00, i
    ftoi0		fIntensities, fIntensities
	sq			fIntensities, NormStartOut(Counter)		; And write to the output buffer
	iaddiu		Counter, Counter, 3
	ibne		Counter, iNumVerts, loop				; Loop until all of the verts in this batch are done.
	iaddiu		iKick, iDBOffset, GifPacketOut
	lq			GP, GifPacket(iDBOffset)				; Copy the GIFTag to the output buffer
	sq			GP, GifPacketOut(iDBOffset)
	xgkick		iKick									; and render!
	
--cont
														; --cont is like end, but it really means pause, as this is where the code
														; will pick up from when MSCNT is called.
	b			begin									; Which will make it hit this code which takes it back to the start, but
														; skips the initialisation which we don't want done twice.

--exit
--endexit
