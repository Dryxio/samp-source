# Scoreboard actions441 independent closure proposal

Sources closure_scoreboard_actions_proposed.cpp + scalar store
closure_store_rpc_click_player_proposed.cpp. Three principals GetRect66,
Hide72,ClickSelected282 plus genuine BitStream EH21 =441 unique new bytes
against full accepted lifecycle-chat union253737. No ListBox allocation or
constructor required by these pointer consumers; do not count/construct unknown
R5 item layouts. Entire original ranges and fixups in original manifest.

R5ScoreboardActionsView mirrors actual CScoreBoard layout0..40: enabled0,
left8,topC,width14,height18,dialog34,listbox38. Never allocated. GetRect truncates
left and width separately before adding; likewise top/height. Hidebool returns
without touching when !enabled/!dialog; otherwise dialog visiblebyte13 false,
ListBox virtualSetEnabled38/SetVisible40,false; optionally restores key inputs,
then enabled0. Does not invent a ListBox nullguard.

ClickSelected obtains actual GetSelectedIndex88E70(default-1),GetItem8A9C0.
Only actual NUL-terminated narrow text at returned item offset0 is passed to
atoiC6D26; no use of pData or sizeof(TCHAR)/item allocation. R5 selected flag at
item298 remains unrelated. Parse int→WORD write16bits, followed by zero BYTE
write8bits. Actual RPCClient vslot64 from pNetGame26EB94:GetRakClient offset0,
RPC23 at EA6A8, HIGH_PRIORITY1,RELIABLE_ORDERED9,channel0,false. After selected
item handling it hides with input restoration even if no item selected.

Closed providers: GetSelectedIndex,GetItem,atoi,BitStream ctor1F730,
WriteBits1FFE0,dtor1F8D0,__CxxFrameHandlerC6526,ToggleInputsA06F0,
pNetGame/pGame stores. Own Hide72 should inline in ClickSelected under Ob2,
consistent original inlined hide with known true flag. Use proper wholeTU Ob2,
not conflicting inline annotations. No frame-forcing/padding directives.

EH complete21 E3A00 includes cleanup11 then handler10 E3A0B; xdata36 FB8D8,
info FB8E0. Scalar4 EA6A8=23 true shared outgoing player-click ID, source-owned
minimal TU avoids header sentinel pollution. No float/string data. Parent
matching must inspect template writes and every handler/xdata relocation.

Direct025 credit0: original missing methods reconstructed from R5; old
scoreboard context is not equivalent source copied. This441 provides real UI
interactions while larger ListBox ctor/render remains explicitly unimplemented.
