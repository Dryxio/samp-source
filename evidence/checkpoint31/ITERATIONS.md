# Checkpoint 3.1 — investigations and rejected attempts

1. The pinned upstream IDB describes the same reference MD5 as the frozen DLL.
   Its 8,020 exported function entries and every chunk's bytes were checked
   against that DLL. Previous projects' matching statuses were discarded.
2. The old vcproj listed SocketDataEncryptor.cpp, which is absent. It is not
   treated as an implemented dependency. The actual selected network source
   set was expanded from linker evidence, including TEA, factory dependencies,
   Router and SystemAddressList. The final list has 40 network units.
3. The strict COFF reader initially rejected Rand.obj's uninitialized `.bss`,
   whose logical size exceeds its file length. BSS now has an explicit
   uninitialized state and no file bytes. It cannot be accepted as code or
   silently read as initialized constants. Dedicated negative controls cover it.
4. Full source compilation succeeded for eight DXUT units and two utility
   units. The preserved upstream DXUT code emits C4700 for RenderArrow's
   cPasses because V expands to nothing. That function remains unaccepted.
5. The first DLL link exposed game font/window dependencies, missing factory
   dependencies, and a wrong SDK search result for dxguid.lib. Real network
   sources were added; SDK input paths/hashes were made explicit. Game-only
   calls are fail-fast test boundaries, not production replacements.
6. ABI assertions rejected the initial fixture header order: Windows headers
   changed packing despite /Zp1. An explicit push/pop of pack(1) around network
   declarations restores the ABI shown by the exact R5 bodies. BitStream is
   273 bytes; CheckSum.sum is at offset 6. No library body was changed.
7. The first BitStream test incorrectly expected unused low bits of a partial
   left-aligned byte to be cleared. The R5-exact ReadBits body preserves them.
   The test now checks that original behavior plus right-aligned reads, instead
   of modifying source to satisfy an incorrect expectation.
8. Whole-body raw discovery proposed 263 distinct R5 regions. A file-local
   SortModesCallback cannot be rooted by a public linker export and was left
   out of this batch. The frozen contract contains 262 regions. Discovery
   produces ignored proposals only and cannot update acceptance automatically.
9. The integration probe passed 9,771 native checks. The final test adds digest
   padding/block-boundary vectors, a real D3DX import call and a BASS version
   query. Final results and source fingerprints are in acceptance.json.

Intermediate snapshots/logs remain locally under ignored `build/cp31-*`.
Only the final gate result is an acceptance; a failed rerun writes FAIL rather
than leaving a stale PASS at the acceptance path.
