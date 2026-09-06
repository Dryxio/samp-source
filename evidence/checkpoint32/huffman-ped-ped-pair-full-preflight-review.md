# Independent full COFF preflight: remote ped ctor + ped renderer

Run cp32-huffman-ped-trial1, baseline death-vehicle217741. **1367 new code bytes** =694+634 principals +18+21 attached EH. Script preflight_ped_pair.py and machine report ped-pair-full-preflight.json independently verify six complete sections (two .text, two .text$x, two .xdata$x), **all78 section fixups**,47 distinct principal symbol bindings and9 complete data payloads. Every symbol binds to a real semantic provider from death-vehicle or the specifically reviewed newctor/descriptors/EH. No truncated comparison or masked bytes. This is preflight, not acceptance.

Ctor694 exact atB0CE0; EH .text$x18 mapsE3F50 and .xdata$x36 mapsFBEAC. Preview634 exact6C140; EH21 mapsE3960 and .xdata$x36 mapsFB824. Cleanup targets: ctor uses actualCEntitydtor9C280; renderer allocation cleanup uses operator deleteC648A. Both handlers target CxxFrameHandlerC6526. Source-local FuncInfo/UnwindMap fixups resolved by their own actual section/value. EH includes entire contiguous36bytes each (8UnwindMap+28FuncInfo), not36 bytes starting atFuncInfo.

VtableED140/12 in actual constructor COFF also fully resolved and equals original: __ECPlayerPed weak-alias to already established __GCPlayerPed atB13D0, CEntityAdd9F300, CEntityRemove9F400. Root should preserve that existing E→G alias in the new unit's reviewed mapping; no second table/function provider. Constructor real classsize32D/new request32D and preview virtualslot0/4/8 agree. AFF50 is true nonvirtual R5PreviewPedModelView::SetModelIndex; no mistaken CPlayerPed virtualbinding. Remaining preview calls use the true matrix173/render138 and animation providers, not stand-ins.

## Static descriptor whitelist for ctor

Four internally linked constants in ctor.rdata section46 need explicit reuse approval by exact symbol/body:
- _set_actor_immunities: section46 offset0,18 bytes →ECE70.
- _set_actor_can_be_decapitated: offset20,18 →ECE84.
- _set_actor_weapon_droppable: offset40,18 →ECFC4.
- _set_actor_money: offset60,18 →ECFD8.
Each18-byte payload was compared in full to original, and each is an already implemented actual scriptrecord. Section46 totals78 (four18 records plus alignment gaps); do not map this whole discontiguous set as one contiguous78-byte original region. No extra code/data credit for reused records or source alignment gaps.

Newdescriptor TUs each contain exactly one18-byte .rdata section, no relocations and no main.h/RakNet sentinel data: r5CreatePreviewPlayer→ECDA8 (0053,vfffv), r5CreatePreviewActorFromPlayer→ECDBC (01F5,vv). Both source payloads fully match. Newliteral0.15f constructor __real3E19999A→ED278 matches9A99193E. Preview -2.25f→EA62C matches000010C0 andzero→E5940 matches00000000. Complete634 comparison confirms specialheight immediate42489999 (50.05f+0.1f), not wrong literal50.15f/4248999A. No forced data substitution.

The usual header-emitted unused COMDAT helpers and writable RakNet sentinels in principal TUs are not new coverage/providers. Keep selective existing-owner policy; emitted CEntityctor25, CEntitydtor7, scalardeletors and helper bodies are excluded from gain. These discoveries have many same-byte homologues and must never enlarge this1367.

Direct0.2.5 numeric credit remains conservative0 for hybridctor and newrenderer whole bodies. Useful real input:025 gave missing remoteconstructor lifecycle and exact newcommandrecords; R5 adaptations supply substantial tail/state/position behavior. Source-guided first-probe success is concrete acceleration, not1367 directsource-transfer bytes.
