// BOUNDED SYMBOLIC-ASSEMBLY EXCEPTION: only CPlayerPed::RemoveAllAttachedObjects.
// Use only after the three retained normal-C++ induction experiments fail.
// Full 86-byte function required. No copied bytes, _emit, incbin or masks.
#include "main.h"
#include <stddef.h>
enum {
 R5PedAttachedCount=10,
 R5PedActiveOffset=offsetof(CPlayerPed,field_4C),
 R5PedAttachmentInfoOffset=offsetof(CPlayerPed,field_74),
 R5PedAttachedObjectOffset=offsetof(CPlayerPed,field_27C),
 R5PedObjectFromActive=R5PedAttachedObjectOffset-R5PedActiveOffset,
 R5PedAttachmentInfoBytes=sizeof(struc_97),
 R5PedAttachmentInfoDwords=sizeof(struc_97)/sizeof(DWORD),
 R5PedActiveStride=sizeof(((CPlayerPed*)0)->field_4C[0])
};
typedef char R5AttachedSlotLayout[(R5PedActiveOffset==0x4c &&
 R5PedAttachmentInfoOffset==0x74 && R5PedAttachedObjectOffset==0x27c &&
 R5PedAttachmentInfoBytes==52 && R5PedActiveStride==4)?1:-1];
// Real ownership: B0C58 calls object ctorA9110, B0C69 stores its return in27C.
// Its CEntity offset0 vtableECD74 slot0 is deleting destructorAA100 ->A9430.
// delete flags1 invokes that accepted closure and then pinned operator delete.
__declspec(naked) void CPlayerPed::RemoveAllAttachedObjects()
{
 __asm {
  push ebx
  push ebp
  push esi
  push edi
  xor ebx,ebx
  lea esi,[ecx+R5PedActiveOffset]
  lea ebp,[ecx+R5PedAttachmentInfoOffset]
  ALIGN 16
 attached_slot_loop:
  test ebx,ebx
  jl attached_slot_next
  cmp ebx,R5PedAttachedCount
  jge attached_slot_next
  cmp dword ptr[esi],1
  jne attached_slot_next
  mov ecx,[esi+R5PedObjectFromActive]
  test ecx,ecx
  je attached_slot_clear_info
  mov eax,[ecx]
  push 1
  call dword ptr[eax]
  mov dword ptr[esi+R5PedObjectFromActive],0
 attached_slot_clear_info:
  xor eax,eax
  mov ecx,R5PedAttachmentInfoDwords
  mov edi,ebp
  rep stosd
  mov dword ptr[esi],eax
 attached_slot_next:
  inc ebx
  add esi,R5PedActiveStride
  add ebp,R5PedAttachmentInfoBytes
  cmp ebx,R5PedAttachedCount
  jl attached_slot_loop
  pop edi
  pop esi
  pop ebp
  pop ebx
  ret
 }
}
