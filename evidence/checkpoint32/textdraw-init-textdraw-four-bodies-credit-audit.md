# Binding preflight and strict 0.2.5 contribution

33 inferred bindings from trial1 constructor/pool and trial2 SetText/LoadTexture were independently checked against selection-model-motion-contract providers, semantic literal contents, and the actual new bodies. All33 identities agree; no Shop-style inversion detected. Machine-readable rows: textdraw-four-bodies-binding-preflight.json. This is an identity audit, not replacement for linked-byte acceptance.

Readiness queries have the correct meanings: D150 HasTextureModel returns presence for type2, D190 IsModelReady reads readiness flag, D320 EnsureModelReady actually loads and sets flags, D280 GetTextureId returns texture identifier. Source calls them in precisely that order and reloads the real manager pointer26EB98 between calls, rather than interchanging getters with compatible-length bodies. Returned ID formats %X as dictionary name. EnsureTextdrawDictionary takes dictionaryName,file,sampDirectory (true for samaps/vehicles128, false for general files), while ReadTextdrawDictionaryTexture takes dictionaryName,textureName. The vehicleprev dictionary string and vehicles128 file string are correctly distinct. Textdraw texture output goes to real store26B568 at its textureSlot. The constructor calls SetText before style4 allocation because initialslot=-1, preventing accidental doubleallocation; SetText later updates only previously allocated style4. PoolNew calls actual Delete1E7F0, newC627A and ctorB36E0; the EH deleteC648A concerns failed allocationconstruction cleanup, not textdrawresource cleanup. No resource provider is swapped.

Remaining acceptance requirement: _strncmp CB030/57 is correctly identified from argument protocol and full bounded unsigned-byte-comparison semantics, but is absent from the previous contract. New linked MAP must identify its actual pinned vendor member and compare all57 bytes; do not infer vendor ownership solely from matching the callsite. All other referenced functional providers are already in the prior contract or these exact new bodies. EHhandlerE173B and __except_list0 are the expected compiler exception registration identities, not arbitrary external aliases.

Strict direct 0.2.5 principal credit for this lot, only after successful acceptance:

| Function | R5 bytes | New direct025 bytes | Reason |
|---|---:|---:|---|
| PoolNew |155|155|Complete control flow comes from025 CTextDrawPool::New, absent from dashr base. Adapt actual pool2304/offset2400, R5 object9D6/transmit63/types and accepted Delete identity. New-null/store/return flow unchanged.|
| Constructor |402|0|Hybrid:025 supplies standard assignment order, but substantial R5 packet/preview fields, SetText/resources/selection state and layout are newly reconstructed. Do not count entire402 as direct transfer or partial instruction ranges as accepted025 functions.|
| SetText |298|0|025 has only strncpy+terminator skeleton; R5 memset/limits/tilde/keycodes/texture lifecycle are absent. Full body R5 reconstruction.|
| LoadTexture |630|0|Absent from025. Dictionary handling and custommodel protocol reconstructed fromR5.|

Thus strict additional direct025 principal credit155, not1485/1506. Compiler-generated EH21 is not counted as source-transfer credit. Reused accepted dependencies, old125 poolcleanup and earlier727 DrawText receive no additional credit. Qualitative acceleration from025 ctor is real but left unquantified in unique code bytes.

Base evidence: matching-decomp/saco/net/textdrawpool.cpp contains only poolconstructor (SHA256 cb50da02b819aa44e610c2540b18efb72f89217b5c55852533f11344e17b2c2b), no New implementation. michael-fa-samp/client/net/textdrawpool.cpp provides complete New. Its client/game/textdraw.cpp provides standard constructor assignments and old two-line SetText, as described. No base implementation reclassified as025 gain.
