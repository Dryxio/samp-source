# Preview owner: independent normal-C++ proposal361

Baseline229399. Whole functions Init6BE40/333 and constructor6CCC0/28 are
unaccepted; their15 direct calls resolve to already accepted real RenderWare
providers. No EH/tables. Complete actual allocation size28 proven C4D32 push1C,
operator newC627A and constructor6CCC0, result stored26EBA8 atC4D64. Source
class R5PreviewRenderer describes all seven4-byte slots; unknown18 is named by
offset only. This is a real complete allocation owner, not a pointer-only view.

camera0,frame4,light8,textureC,raster10,zbuffer14,unknown18.
Ctor zeros0,4,C,18,8 then calls Initialize. It does not separately zero10/14.
Init creates directional light type2/color1111,256x256 raster flags505 and
zbufferflag1, camera+frame at0,0,50 rotated90 onaxis0, sets buffers/frame,
far300/near0.01, viewwindow0.5/0.5, projection1, adds camera toworld, texture.
Early failure semantics retained exactly, no added cleanup/nullguard.

All float constants are original immediate stores/pushes, not inferred relocations:
1=3F800000;50=42480000;90=42B40000;300=43960000;
0.01=3C23D70A;0.5=3F000000. Original uses bool ALreturn, notBOOL EAX.
All15 callsites match native wrapper names in vehicle-statistics-contract.
Real BSS store4 proposal isolated, referenced by creation and all three original
outer-preview renderer calls. Actual startup function C4790 is not proposed or
counted as covered merely because this owner constructor becomes implemented.

0.2.5 direct credit0: preview subsystem absent. Main remaining outer update
blocker is RenderObject694, previously parked for register/scheduling mismatch.
Outer513 and itsPoolPass36 can be compiled before694, but not accepted without
that actual provider. All other direct update targets now accepted.
