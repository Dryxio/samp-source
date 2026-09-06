# Label bounded C++ trials, parked

Root Ob2 trials: trial1 produced757; trial2 produced756; trial3 produced755.
Begin16 and End14 are exact in every trial. Main Draw755 remains unaccepted;
no further variants proposed. Sources and three complete captures retained.

Trial1's rectangle alias was semantically wrong: centered text rectangle
modified status origin. Trial2 fixed this using a separate text rectangle.
Trial3 moved the copy before MeasureText2, matching actual register lifetime.
It recovered original frame9C and the same overall length755.

Remaining trial3 differences are instruction selection/scheduling, not newly
identified semantic/provider differences:
- Original after first ftol: ESI=EAX, EDI=EAX, store local-left. It delays the
  original-right store until preparing MeasureText2. Trial3 ESI=EAX, EDI=ESI,
  stores original-right immediately, then original-left. Similarly top/bottom
  stores reorder around the second result and MeasureText2 arguments.
- At auxiliary glyph measurement original loads [ESI] vtable before argument
  pushes, pushes ESI, then clears four symbolRect fields. Trial3 clears the
  same four fields before loading the vtable and pushing ESI. Stack-relative
  displacements differ because the clearing occurs at a different push depth.
- Remaining status rendering, rectangle arithmetic and final glyph offsets
  now converge; fixed-size inline RECT zeroing call scheduling remains.

No new substantive C++ hypothesis beyond these register/store schedules.
Keep label755 parked. Begin16/End14 are real dedicated sprite callbacks with
field8, BeginALPHABLEND16/slot20, Endslot2C; no direct dependency/fixup/data/EH.
Prefer bundle with eventual complete label unless root needs separate batch.
