# True Chat constructor279, ready probe

closure_chat_constructor_proposed.cpp uses the actual existing CChatWindow
class and actual existing constructor signature(device,font,logpath). No new
header/view allocation or alternate constructor owner. Root must exclude old
chatwindow.cpp constructor definition from the capsule; this TU is sole owner.

Existing full class size0x63EA/25578 is independently confirmed by original
startup push63EA atC489D, operatornewC48AB, constructor68380 callC48D8, then
pChatWindow26EB80 storeC48EA. Constructor's own code279 has no vtable, EH,
subobject cleanup handlers, or scalar destructor to add.

Source is base constructor with only the 100 individual-entry clear loop
replaced by memset of the true entire array25200bytes. Original REP STOSD
count189C=6300DWORD at683CA proves complete whole-array initialization from
this132. Base field_11 clears261bytes, copiesMAX_PATH260, fopen"w", on success
sets loggingflag1 and closes. It deliberately leaves loggingflag unchanged
when path absent/fopen fails; no extra initialization added.

Other stores match base: device63AE,font63A2,mode8=2,twosprites63A6/63AA,
colors122/126/12A, pageSize0=10,timestampsC=0; surface/texture/renderToSurface
63BE/63BA/63B6=NULL,GetTickCount→63D2,lastScroll63DE=1,scrollbar11E/UI116/
field11A=0. Finally actual CChatWindow::CreateFonts681D0 (pending exact419).

Closed dependencies D3DXCreateSpriteC5C74,strncpyC77F0,fopenC7F49,
fcloseC7E58,GetTickCountE5238; new exact resources419+117 supplies final call.
String mode'w\0' complete2 atE9B9C (verify full literal section at probe).
No float/native memory inputs or new global owner required. Direct025 credit0:
base already had this body, and its sole change is explicit R5 array clear.
