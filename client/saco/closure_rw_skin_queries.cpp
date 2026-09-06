// Native ABI bridges follow existing game/rwstuff.cpp wrapper idiom.
// No encoded instructions, fake callbacks, padding, or allocated partial views.
#include "main.h"
#include "game/rwstuff.h"
extern int iGtaVersion;
struct RpGeometry;
struct RpSkin;
struct RpHAnimHierarchy;
RpAtomic *GetFirstAtomic(RpClump *clump) {
 RpAtomic *result=0;
 DWORD nativeFunction=(iGtaVersion==1)?0x734820:0x734880;
 __asm {
  push clump
  mov eax,nativeFunction
  call eax
  mov result,eax
  pop eax
 }
 return result;
}
RpSkin *RpSkinGeometryGetSkin(RpGeometry *geometry) {
 RpSkin *result=0;
 DWORD nativeFunction=(iGtaVersion==1)?0x7c7550:0x7c7590;
 __asm {
  push geometry
  mov eax,nativeFunction
  call eax
  mov result,eax
  pop eax
 }
 return result;
}
// Real RpAtomic geometry pointer at0x18, never an allocated RpAtomic definition.
struct R5AtomicGeometryView { BYTE unknown0[0x18]; RpGeometry *geometry; };
RpSkin *GetClumpSkin(RpClump *clump) {
 RpAtomic *atomic=GetFirstAtomic(clump);
 return RpSkinGeometryGetSkin(((R5AtomicGeometryView*)atomic)->geometry);
}
RpHAnimHierarchy *GetAnimHierarchyFromClump(RpClump *clump) {
 RpHAnimHierarchy *result=0;
 DWORD nativeFunction=0x734b10;
 __asm {
  push clump
  mov eax,nativeFunction
  call eax
  mov result,eax
  pop eax
 }
 return result;
}
