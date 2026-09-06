# UI/Rcon/Skin/CreateVehicle preflight

Actual profile: cp32-policy-ui-ob1. Baseline rsa-commands accepted377473. Complete COFF sections compare against original R5 with every relocation target and literal checked. 52 sections,171 bindings. 1535 principal bytes plus Rcon handler21 =1556 new unique code bytes. Root NewVehicle142/handler21 is a separate163-byte proof. None is accepted by this preflight alone.

## Identity and ABI

The command entrypoints correspond to their actual command semantics, not interchangeable matching homonyms. NameTag command68720 calls8E90, which updates CNetGame byte234 and the persisted inverse nonametagstatus flag. Camera command685E0 toggles the actual byte1189A8. Mem688B0 directly reads the native GTA memory-use DWORD8A5A80; this is not an unowned invented SAMP variable.

Rcon69030 constructs a real RakNet::BitStream on the stack, serializes packet201, DWORD command length and command bytes, and calls the actual RakClientInterface Send virtual slot18 with HIGH_PRIORITY1, RELIABLE8 and channel0. Both pointer guards are retained. No network operation was executed. The complete21-byte handlerE38C0 includes its cleanup and dispatch tails; entry anchor is+11, FuncInfoFB770 is a complete36-byte section whose FuncInfo anchor is+8. All local targets, FS exception-list pseudo-symbol0 and PE relocations were checked.

Skin68D00 uses actual FindPlayerPed1010 and the accepted nonvirtual R5PreviewPedModelView::SetModelIndexAFF50. It does not substitute a differently named CPlayerPed virtual method. Its view is only used for pointer access, never allocated.

CreateVehicle68D70 calls the real NewVehicleA0250 provider, with six parameters including R5 creationFlag0. The pointer-only creation view matches the factory's definition exactly; full CVehicle184 allocation and constructorB83D0 belong to the real separately reviewed factory. This command then calls the real object's virtual Add slot4. The original unchecked return and the apparently unused TYPE_%d formatting are retained, as are blank[9] and the actual RequestModel default flags2. Float literals EA16C/4=5.0f and E5B2C/4=1.0f were checked as entire4-byte sections, not merely relocations.

## Stores and full types

Full GAME_SETTINGS1297 has the actual three BOOLs and five257-byte strings. Original InitSettingsC45B0 clears324 DWORDs plus one byte at26DFE8, independently establishing extent1297. It is a complete real object, not a debug-flag stand-in. Root transferred main.cpp's definition to the minimal store TU; main now declares extern. Original hook bool bHudScaleFix117470 is initialized1; root transferred its sole owner and hooks now declares extern. Camera bool1189A8 is initialized0. All three complete sections were checked; the settings section is actual PE virtual zero-fill, camera is backed zero data. No unrelated16-byte RakNet sentinel section is used.

Chat and game views are pointer-only and create no objects. Existing accepted full implementations provide config setters/getters, chat paging, info/debug, model requests and matrix access. AddEntry538 and PushBack24 remain implemented but unmatched downstream chat functions; no coverage is claimed for either.

## Duplicate ownership and attribution

Skin and CreateVehicle each emit FindPlayerPed94/handler21/FuncInfo36. Each complete normalized section has the same SHA256 as every matching previously accepted owner, including closure_player_lifetime. These six physical sections are explicitly present in the manifest but gain zero unique coverage. The selected linked COMDAT owner must still be verified by the root link/gate; a same-name shorter implementation must not be silently substituted.

Unused GetRakClient3 and Write template17 copies are not seeded or credited. All referenced literals are whole sections. Direct0.2.5 credit is0: HudScale/Mem already exist in dashr; Rcon adds the R5 network guard; Skin adapts the true model setter; CreateVehicle adapts its factory argument and API. These are useful hybrid reconstructions, not direct missing-source transfers.
