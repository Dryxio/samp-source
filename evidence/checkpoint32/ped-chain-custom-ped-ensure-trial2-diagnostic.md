# EnsurePedModelReady:94 bytes, only inlined bounds helper layout differs

Original D2CF..D2DB has CMP index,count; JB valid; XOR ESI,ESI; JMP join; valid: load items/index. Trial2 same94-byte body uses JAE invalid; load items/index; JMP join; invalid: XOR ESI,ESI. Bytes beforeD2CF and afterD2DC agree once the actual C650 relocation resolves. No fields, constants, argument order or provider mismatch.

Motivated C++ correction: replace ternary GetAt(index<count ? items[index] :0) with explicit early out if(index>=count) return0; returnitems[index]. This puts invalid case first in helper source, matching the original block arrangement rather than requesting register/padding tricks. No class layout change and all LoadPedModelFiles source stays identical. Only partial pointer-view member implementation changes; no new emitted helper credit.

Proposed closure_custom_ped_load_manager_trial2_proposed.cpp. Fresh94-byte comparison required; this is one bounded syntax-form trial, not accepted coverage.
