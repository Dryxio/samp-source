# Update892: actual scratch allocation interval

At function entry original ESP points at return address. SUB ESP154 then four
saved registers EBP/EBX/ESI/EDI produce final stable ESP=entryESP-164. Saved
registers occupy offsets0..F; original allocated locals occupy10..163.
The sprintf destination is consistently stable ESP+60 (LEA ESP+64 after one
argument push, e.g.6EFC6). Thus trailing scratch storage can extend from60 up
to the return-address boundary164, a260-byte interval; no other live local
appears above it. Earlier locals occupy10..2F and whole45-byte swapped row
30..5C, leaving normal alignment before the scratch at60. There are no params
on the stack or additional hidden object lifetimes in this method.

Source trial2 char256 yields identical local offsets, SUB150 and matching
stable frame160. Its scratch occupies60..15F exactly256. All resolved body
bytes except the two frame-immediate low bytes match; moving those same real
scratch storage bounds to the observed164 reproduces the original allocation.
A source array capacity257,258,259 or260 could all round to this frame; original
spelling cannot be uniquely inferred. Choosing actual buffer260 uses the full
observed scratch allocation and invents no independent dummy field/variable or
alignment. No extra sprintf capacity promise exists: ordinary sprintf is used.

Bounded trial3 changes only this existing scratch array to260. This is a
consistent source reconstruction of a real buffer with nonunique declaration
capacity, not padding inserted into code or an unrelated frame-forcing object.
All control/data flow and provider bindings unchanged. Integral matching still
required, and this provenance must remain explicit after any acceptance.
