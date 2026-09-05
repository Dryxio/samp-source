// Generated complete definitions from game/util.cpp; see tools/prepare_actor_closure.py.
#include <time.h>
#include <math.h>
#include "main.h"
#include "game/util.h"
extern CChatWindow *pChatWindow;
extern DWORD CamFrameBuffer;
extern DWORD FrameBufferTexture;
extern int Width;
extern int Height;
extern int Depth;
extern DWORD dwRwImage;
extern DWORD CamFrameBuffer2;
extern DWORD CamZBuffer2;


void _VehicleEnvMappingFromScreen()
{
	/*
	if(FrameBufferTexture) {
		_asm push FrameBufferTexture
		_asm mov edx, 0x7F3820
		_asm call edx
		_asm pop edx
		FrameBufferTexture = 0;
	}*/

	// get the frame buffer from the main RwCamera
	_asm mov eax, 0xC402D8
	_asm mov ebx, [eax]
	_asm mov CamFrameBuffer, ebx

	_asm push CamFrameBuffer
	_asm mov edx, 0x7F37C0 // RwTextureCreate
	_asm call edx
	_asm pop edx
	_asm mov FrameBufferTexture, eax

	if(!FrameBufferTexture) {
		pChatWindow->AddDebugMessage("No texture!");
	}

	*(DWORD *)0xB4E690 = FrameBufferTexture;
	*(DWORD *)0xB4E68C = FrameBufferTexture;
	*(DWORD *)0xB4E47C = FrameBufferTexture;
	*(DWORD *)0xB4E3EC = FrameBufferTexture;
	*(float *)0x8A7780 = 2.0f;
}

void WriteRwRasterToFile(DWORD raster,char *filename)
{
	// RwImageCreate(width,height,depth)
	_asm push Depth
	_asm push Height
	_asm push Width
	_asm mov edx, 0x8026E0
	_asm call edx
	_asm mov dwRwImage, eax
	_asm pop edx
	_asm pop edx
	_asm pop edx

	// RwImageAllocatePixels
	_asm push dwRwImage
	_asm mov edx, 0x8027A0
	_asm call edx
	_asm pop edx

	// RwImageSetFromRaster
	_asm push raster
	_asm push dwRwImage
	_asm mov edx, 0x804250
	_asm call edx
	_asm pop edx
	_asm pop edx

	// RtBmpImageWrite
	_asm push filename
	_asm push dwRwImage
	_asm mov edx, 0x7CE990
	_asm call edx
	_asm pop edx
	_asm pop edx

	// RwImageDestroy
	_asm push dwRwImage
	_asm mov edx, 0x802740
	_asm call edx
	_asm pop edx
}

void SaveCameraRaster(char *filename)
{
	// get the frame buffer from the main RwCamera
	//_asm mov ebx, 0xC1703C
	//_asm mov esi, [ebx]
	//_asm mov ebx, [esi+96]

	_asm mov eax, 0xC402D8
	_asm mov ebx, [eax]
	_asm mov CamFrameBuffer, ebx

	_asm mov esi, CamFrameBuffer
	_asm mov eax, [esi+20]
	_asm mov Depth, eax
	_asm mov eax, [esi+16]
	_asm mov Height, eax
	_asm mov eax, [esi+12]
	_asm mov Width, eax

	if(CamFrameBuffer) {
		WriteRwRasterToFile(CamFrameBuffer,"Raster1.bmp");
		pChatWindow->AddDebugMessage("Width %u Height: %u Depth: %u",Width,Height,Depth);
	}

	//_asm mov eax, 0xC7C71C
	//_asm mov ebx, [eax]
	//_asm mov CamFrameBuffer, ebx

	if(!CamFrameBuffer2) return;

	_asm mov esi, CamFrameBuffer2
	_asm mov eax, [esi+20]
	_asm mov Depth, eax
	_asm mov eax, [esi+16]
	_asm mov Height, eax
	_asm mov eax, [esi+12]
	_asm mov Width, eax

	if(CamFrameBuffer2) {
		WriteRwRasterToFile(CamFrameBuffer2,"Raster2.bmp");
		pChatWindow->AddDebugMessage("Width %u Height: %u Depth: %u",Width,Height,Depth);
	}
}

void CreateCameraRaster()
{
	if(CamFrameBuffer2 || CamZBuffer2) return;

	_asm mov eax, 0xC402D8
	_asm mov ebx, [eax]
	_asm mov CamFrameBuffer, ebx

	_asm mov esi, CamFrameBuffer
	_asm mov eax, [esi+20]
	_asm mov Depth, eax
	_asm mov eax, [esi+16]
	_asm mov Height, eax
	_asm mov eax, [esi+12]
	_asm mov Width, eax

	// RwRasterCreate
	_asm push 5
	_asm push Depth
	_asm push 1024
	_asm push 2048
	_asm mov edx, 0x7FB230
	_asm call edx
	_asm mov CamFrameBuffer2, eax
	_asm add esp, 16

	// RwRasterCreate
	_asm push 1
	_asm push Depth
	_asm push 1024
	_asm push 2048
	_asm mov edx, 0x7FB230
	_asm call edx
	_asm mov CamZBuffer2, eax
	_asm add esp, 16
}
