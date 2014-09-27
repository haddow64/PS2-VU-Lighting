#include <sps2lib.h>
#include <sps2tags.h>
#include <sps2util.h>
#include <sps2regstructs.h>
#include <assert.h>
#include <memory.h>
#include <signal.h>
#include "PS2Defines.h"
#include "ps2maths.h"
#include "ps2matrix4x4.h"
#include "pad.h"
#include "sps2wrap.h"
#include "dma.h"
#include "texture.h"
#include "font.h"
#include "pipeline.h"
#include "entities.h"
#include "vumanager.h"
#include "timer.h"
#include "ms3dmodel.h"
#include "terrain.h"

// These two external function pointers are exported by the VU code
// and we can now use them as pointers to the compiled VU data
extern "C" uint64 VU_vu1code_start;
extern "C" uint64 VU_vu1code_end;


bool g_bLoop = true;

void sig_handle(int sig)
{
	g_bLoop = false;
}

class program : public CPipeline {//inherits from pipeline.cpp 
public:
	program();
	
	void Initalise();
	void Lighting();

	void Load();
	void Draw();
	void Cycle();

private:
	// Model to render
	CMs3dModel Model[3];
	CTexture ModelTex;

	// Terrain to render (in this case the floor)
	CTerrain Terrain;
	// Load the texture for the ground plane
	CTexture Test;
	CFont Font;
	CTexture FontTex;
	// Performance timer
	CTimer Timer;

	Matrix4x4 matWorld, matTrans, matScaling, matRotLocal, matRotOrigin, modelTrans[3];
	
	// Two cubes to render
	Cube C1, C2;

	int Frame;
	float fps;
	float bright,x,z;
};

int main(void)
{	
	// Make sure these four lines are put first since some of the 
	// other classes need the managers to be initialised so they 
	// can pick up the correct data.	
	SPS2Manager.Initialise(4096);	// 4096 * 4K Pages = 16MB Total
	VIFStaticDMA.Initialise(3072);	// 1536 * 4K Pages = 6MB Static DMA
	VIFDynamicDMA.Initialise(256);	// 256 * 4K Pages * 2 Buffers =
							// 2MB Dynamic DMA

	program Program;// the implementation of the application class
	Program.Initalise();
	Program.Load();
			
	// The main Render loop
	while(g_bLoop)
	{
  		Program.Cycle();
		SPS2Manager.BeginScene();
		Program.Draw();
		SPS2Manager.EndScene();	
		if(pad[0].pressed & PAD_TRI)SPS2Manager.ScreenShot();		
	}

	pad_cleanup(PAD_0);
	return 0;
}

program::program(){
	Initialise();			// Initialise graphics pipline class
	bright=0, x=0, z=0;
	// set the initial colour and position of the lights
	SetLight1(Vector4( 30.0f, 210.2f, 0.0f, 0.0f), Vector4(40.0f,0.0f,0.0f,0.0f));
	SetLight2(Vector4(40.0f,210.2f, 0.0f, 0.0f), Vector4(0.0f,0.0f,40.0f,0.0f));
	SetLight3(Vector4( 0.0f, 0.0f,-1.0f, 0.0f), Vector4(0.0f,0.0f,0.0f,0.0f));
	//Ambient lighting
	SetAmbient(Vector4(60.0f,60.0f,60.0f,0.0f));
}

void program::Lighting(){
	VIFDynamicDMA.AddUnpack(V4_32, 1, 8);
	VIFDynamicDMA.AddMatrix(GetLightDirs());
	VIFDynamicDMA.AddMatrix(GetLightCols());
	VIFDynamicDMA.AddUnpack(V4_32, 17, 3);
	VIFDynamicDMA.AddVector(Vector4(0.0f,128.0f,0.0f,0.0f));
	VIFDynamicDMA.AddVector(Vector4(x,bright,z,0.0f));
	VIFStaticDMA.AddVector(Vector4(0, 1, 0, 0));
	VIFDynamicDMA.AddVector(Vector4(GetCameraX(),GetCameraY(),GetCameraZ(),0.0f));
}

void program::Load(){
	// Load the model itself.
	for (int i=0;i<3;i++){
	if(!Model[i].LoadModelData("ninja.txt", false))
		g_bLoop = false;
	// Now load the model's texture.
	if(!ModelTex.LoadBitmap(Model[i].GetTextureName()))
		g_bLoop = false;
	}
	
	//load the texture for the ground
	if(!Test.LoadBitmap("test.bmp"))
	{
		printf("Can't load Texture\n");
		g_bLoop = false;
	}
	
	// Load the font bitmap and data
	if(!Font.Load("font.dat", true))
	{
		g_bLoop = false;
		printf("Can't load font.dat\n");
	}
	if(!FontTex.LoadBitmap("font.bmp", false, true))
	{
		printf("Can't load font.bmp\n");
		g_bLoop = false;
	}
	// Upload these once to VRAM since
	// they are not going to change
	//Test.Upload(TEXBUF480);
	FontTex.Upload(TEXBUF496);
};

void program::Initalise(){// Initialise control pad 0
	if(!pad_init(PAD_0, PAD_INIT_LOCK | PAD_INIT_ANALOGUE | PAD_INIT_PRESSURE))
	{
		printf("Failed to initialise control pad\n");
		pad_cleanup(PAD_0);
		exit(0);
	}	
	
	// Initialise the VU1 manager class
	CVU1MicroProgram VU1MicroCode(&VU_vu1code_start, &VU_vu1code_end);
	// Upload the microcode
	VU1MicroCode.Upload();

	// Register our signal function for every possible signal (i.e. ctrl + c)
	for(int sig=0; sig<128; sig++)
		signal(sig, sig_handle);	

	// Set up the DMA packet to clear the screen. We want to clear to black.
	SPS2Manager.InitScreenClear(0x0, 0x0, 0x0);

	//Pipeline.PositionCamera(Vector4(0.0f, 35.0f, 80.0f, 1.0f), 0.0f, 0.0f);
	PositionCamera(Vector4(0.0f, 80.0f, 180.0f, 1.0f), Vector4(0.0f, 40.0f, 0.0f, 1.0f));
	}

void program::Cycle(){
		//scale vector from 
		VIFStaticDMA.AddUnpack(V4_32, 0, 1);	// Unpack 1QW - the scale vector
		VIFStaticDMA.AddVector(GetScaleVector());

		Timer.PrimeTimer();

		x+=	pad[0].pressures[PAD_PRIGHT];
		x-= pad[0].pressures[PAD_PLEFT];
		bright+= pad[0].pressures[PAD_PCIRCLE];
		bright-=	pad[0].pressures[PAD_PCROSS];
		z+= pad[0].pressures[PAD_PDOWN];
		z-= pad[0].pressures[PAD_PUP];

		Lighting();
		VIFDynamicDMA.Fire();
		
		pad_update(PAD_0);
		
		// Check for exit condition
		if((pad[0].buttons & PAD_START)&&(pad[0].buttons & PAD_SELECT)) g_bLoop = false;
		
		// Set up a world matrix that rotates the cube about it's own axis, and then again
		// around the origin.
		static float sfRotOrigin = 0.0f;
		static float sfRotLocal = 0.0f;
		sfRotOrigin += pad[0].pressures[PAD_PR1] * 0.1f;
	    sfRotOrigin -= pad[0].pressures[PAD_PR2] * 0.1f;
		sfRotLocal += 0.00f;

		matTrans.Translation(0, 0, -8.0f);
		matScaling.Scaling(1,1,1);
		matRotOrigin.RotationY(sfRotOrigin);
		matRotLocal.RotationY(sfRotLocal);

		matWorld =  matRotLocal * matRotOrigin;//matTrans
		
		// Get any camera movements
		// Get any requested translations
		float Strafe =  2.0f *pad[0].axes[PAD_AXIS_LX];
		float Advance = 2.0f *pad[0].axes[PAD_AXIS_LY];
		float UpDown =  (pad[0].pressures[PAD_PL1] - pad[0].pressures[PAD_PL2]);
		
		// Get requested rotations
		float YRot = pad[0].axes[PAD_AXIS_RY];
		float XRot = pad[0].axes[PAD_AXIS_RX];
				
		// Reset camera to default position if requested
		if(pad[0].buttons & PAD_R3)
		{
			//PositionCamera(Vector4(0.0f, 0.0f, 0.0f, 1.0f), 0.0f, 0.0f);
			PositionCamera(Vector4(0.0f, 80.0f, 180.0f, 1.0f), Vector4(0.0f, 40.0f, 0.0f, 1.0f));
		}
		
		// Update the Camera and viewProjection matrices
		Update(Strafe, Advance, UpDown, YRot, XRot);
	}

void program::Draw(){

		// Select the texture
		Test.Upload(TEXBUF480);
		Test.Select();

		// Render terrain
		Matrix4x4 A;
		A.Scaling(20, 20, 20);
		Terrain.SetWorldMatrix(A, GetViewProjection());
		//Terrain.SetWorldMatrix(Matrix4x4::IDENTITY);
		Terrain.Render();


		// Select the model texture
		ModelTex.Upload(TEXBUF480);
		ModelTex.Select();		
		
		// Render Model
		//matWorld(3,1) = 60.0f;
		for (int i=0;i<3;i++){
		modelTrans[i].Translation(30*i, 0, 0);
		//Model.SetWorldMatrix(matWorld);
		//Model.Render();
		Model[i].SetWorldMatrix(modelTrans[i]);
		Model[i].Render();
		}

		// Select the Font texture
		FontTex.Upload(TEXBUF496);	
		FontTex.Select();
		
		// Render some text
		Font.printfL(  	-300, -240, 127, 127, 127, 127, 
						"Camera Position (x, y, z) = (%3.1f, %3.1f, %3.1f)", 
						GetCameraX(), GetCameraY(), GetCameraZ());
		Font.printfL(  	-300, -210, 127, 127, 127, 127, 
						"Camera Rotation in Degrees (XRot, YRot) = (%3.1f, %3.1f)", 
						RadToDeg(GetCameraXRot()), RadToDeg(GetCameraYRot()));
		Font.printfC(  200, -240, 127, 127, 127, 127, "Frame: %d\nFPS: %.1f" , Frame++, fps);
		//calculate the current frame rate in frames per second	
		fps = Timer.GetFPS();

		Font.printfC(  -290, 180, 127, 127, 127, 127, "\nLight Intensity: %.1f", bright);
}