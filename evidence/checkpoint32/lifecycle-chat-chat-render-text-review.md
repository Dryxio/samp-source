# Chat RenderText851 proposal, uncompiled

RVA67470/851, thiscall text*, RECT by value16, DWORD color; RET24. Proposed
R5ChatTextRenderView only observes mode8, CFontRender*63A2 and sprite63A6;
packed pointer view, never allocated. Those offsets are actual CChatWindow
header fields and original accesses. Full source C++ normal, no header change.

All direct dependencies closed: strncpyC77F0/292 (CRT),
RemoveColorEmbedsFromStringB60C0/61. strlen expands normally. Real foreground
font uses CFontRender field0 (ID3DXFontHook), shadow font uses field8
(ID3DXFont). All13 DrawTextA calls use real COM slot38, and real sprite63A6.
NativeScreenWidth C17044 is signed int, matching game.h GetScreenWidth and
original signed JLE1280. No synthetic native function or global required.

Mode2 renders black shadow after bounded strncpy511 into zeroed512 buffer,
stripping color embeds. Full color text keeps original text and its original
length and ORs alphaFF. No null guard added. Below/equal1280 width four passes;
above1280 eight passes adjust full RECT left/right/top/bottom; original ordering
and coordinate restoration preserved. Twelve shadow sites plus foreground.
No FP/stringconstant/newdata/EH expected; normal512 automatic scratch buffer.

Source025 client/chatwindow.cpp324 supplies earlier four-direction shadow
shape, but its font owner, color stripping, length behavior and high resolution
branch differ. Current dashr base lacks this RenderText body. This is a hybrid
reconstruction and direct025 accepted byte attribution must be0.

Original full capture chat-render-text-original.asm. Prospective851 unique
code, no proposed acceptance until integral COFF/data/provider review. Scope
independent of pending label785 and parked Object/Sprite; all direct providers
already covered in reliability-preview baseline.
