# Next independent textdraw RPC: EditTextDraw

Only one new C++ proposal: closure_rpc_edit_textdraw_proposed.cpp. Ranked manifest remaining-textdraw-ranked-after-construction.json.

First choice Edit1CF70/268 + EH E1530/21 =289 unique potential. Direct providers: real char* BitStream ctor1F840,ReadBits1F9B0,Read(char*,int)1FEA0,dtor1F8D0; global pNetGame26EB94; SetTextB2F60 exact in root pending construction lot. Pool GetAt is inlined id<2304, activeDWORD array, pointer at2400; no independent getter bytes counted (its35-byte body alreadycovered elsewhere). Full net/pool view is pointer-only, never allocated. Compared current accepted union202690: none of these289 bytes overlap; pending parent3158 families do not include this RPC/EH.

Original ReadBits destination for length is a DWORD initialized0, followed by a16-bit read, low-word comparison and zero-extension. Proposal retains unsigned int local and Read(WORD&) rather than a32-bit read. Buffer is800 and original permits length<=800 before writing terminator atlength; preserve this boundary behavior, do not silently change to <800 or allocate801. Show RPC notably rejects>=800; do not reuse its different check. Data is char* to select exact1F840 rather than unsignedchar*1F7C0. No added read-result checks or pNetGame null checks.

EH handlerE153B points to FuncInfoF9004; unwindfunclet E1530 loads stream viaEBP-440 and tailcalls dtor1F8D0. UnwindmapF8FFC/8 points toE1530; FuncInfoF9004/28 has magic19930520,maxState1,mapF8FFC andzeros. Full21 code+36data required. No switch tables.

Source025 provides EditRPC skeleton, but its fixed-width800-byte read differs from R5 length-prefixed message. R5 also checks bounds/active and has the observed zeroedDWORD length. No full-body025 byte credit assigned in advance.

Second choice Show1CD90/297 needs actual AddInfoMessage68070/117. This is NOT accepted AddDebugMessage680F0/117 although both are117 bytes. Body68070 selects chat type4 and color atthis+126; its identity and source must be bound before Show can close. Avoid using length similarity as evidence. Including this helper and EH21 gives435 net potential, but it exceeds the request for alreadyclosed dependencies so no second proposal prepared.

PreviewUpdateB34A0/513 and PoolPass1E870/36 are further away: unimplemented direct targetsD110,D2C0,B3DB0,6C140,6C3C0,6C9B0 remain. The new construction/texture loader closure does not provide preview rendering. Sprite272 remains explicitly excluded; no further variants.
