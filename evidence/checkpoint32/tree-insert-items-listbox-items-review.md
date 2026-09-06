# Newly bounded full item lifetime400, uncompiled

ScoreboardUpdate892 remains blocked by several player providers and AddItem.
We do not propose its whole closure prematurely. Newly audited AddItem itself
permits a concrete400-byte closure: AddItem8A7F0/254,SetColumnText88E00/106,
CGrowableArray<DXUTListBoxItem*>::Add89B50/40. Real downstream
SetSizeInternal86C60/142 already owned by closure_gui_listbox. It is exactly
this existing specialization, not an alias from another pointer template.

Entire actual allocated item size29E is explicitly present in operatornew
argument8A7F4. Full packed layout partitions every byte: text0/257;
three column buffers101/129 each; data284/4;activeRECT288/16;selected298/1;
color299/4;marked29D/1. SetColumnText independently uses column stride81 and
base101; selectionconsumers already establish298. Compile assertions enforce
sizeof29E and data284/color299. No allocation of the outer unknown ListBox
class or vtable coverage. Source uses actual complete item only for new/delete.

Important original behavior: StringCchCopyA capacity256 into text257 (last
storage byte may remain uninitialized). Constructor initializes only first64
bytes of each129-column buffer. SetColumnText zeros128 then strncpy128, without
adding an explicit terminator. These behaviors are preserved; no zero-fill of
wholeallocation or uncharacterized bytes is claimed. Bounds use actual runtime
columnCount4D, not a newly added hardcoded3 guard.

Outer pointer-view accesses real itemarray14C and scrollbar5D. Array stores
DXUTListBoxItem* values using old named pointer type, but never allocates or
sizes that stale header's smaller item. Actual R5CompleteListBoxItem cast is
only pointer conversion at offset0; delete acts on true allocated type.
Existing GetItem/RemoveAllItems consumers use pointer/ordinary-delete behavior,
not a substitute destructor/vtable. Items array true Add40 source instantiated
from bundled DXUTmisc; no wrapper around its call. Root must keep existing
SetSizeInternal owner if compiler additionally emits same COMDAT.

Closed providers: operatornewC627A,deleteC648A,strncpyC77F0,SetRect importE5300,
SetTrackRange88C50 and SetSizeInternal86C60. StringCchCopyA is actual inline
SDK source. No EH/strings/floats/newruntimeglobal. Observe template Add
out-of-line call; Ob1 suitable first probe. Direct025 credit0: known SDK/base
AddItem source supplies outline, R5 larger buffers/color/flag add adaptation.
No masks, rawbytes, fake partial allocation, or claims beyond actual400 bodies.
