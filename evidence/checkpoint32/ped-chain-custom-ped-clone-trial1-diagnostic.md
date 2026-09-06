# Clone trial1: accurate semantic body, missing actual same-TU clobber knowledge

COFF ClonePedModelInfo is77 bytes; original A7A80 is76. Trial saves sourceModel inEDI across actual stdcallGetModelInfo; original usesEDX because real body leavesEDX untouched. This advances PUSH EDI to prologue, shifts argument offsets, and adds POP EDI tofailurepath. Copy68, calloc(68,1), boolguard targetB3DD0 and store114B08 otherwise follow original.

Single grounded C++ trial2 exposes the identical existing GetModelInfo provider body before Clone, under current /Ob1 (non-inline ordinary function). That allows MSVC same-TU register-preservation analysis, while still producing a real call to A7A40. No fake ABI declaration, alternative target or wrapper. New TU contributes duplicate provider only for isolated probe; integration must move the existing provider definition into this TU or otherwise ensure one actual owner, leaving ModelInfoLoaded in its existing TU. Root must check the emitted GetModelInfo remains exactly the accepted provider, including real useExtendedModelInfos1A25AC, extendedModelInfos114B08 and native pointer-tableA9B0C8.

Proposed source closure_custom_ped_model_trial2_proposed.cpp retains Install81 exactly unchanged. No extra coverage credited for GetModelInfo. Fresh complete Clone76 comparison plus originaldata/provider review required.
