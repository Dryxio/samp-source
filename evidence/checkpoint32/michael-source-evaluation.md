# Local 0.2.5 reference evaluation

Result: one of three previously unmatched R5 functions reproduces its complete
reference bytes after explicitly reviewed COFF relocation resolution. This is
217 bytes of function-level matching evidence, not a linked-capsule acceptance.
The accepted coverage counter remains unchanged at 34,071 bytes.

Reference: `../samp-r5-preservation/references/michael-fa-samp`, commit
`dc9eb80d80e26cf91e2b92697b8918208f052542`. File hashes, object hashes,
probe metadata hashes, relocation identities and comparison results are in
`michael-source-evaluation.json`.

## Controlled experiment

The three targets were selected because they are existing unmatched gaps with
an identifiable counterpart in the local reference, including a missing model
method and two chat methods whose current reconstructions were already close.
They are not a random sample and do not support a whole-project speed multiplier.

All candidates use our existing packed R5 headers and MSVC 13.10.3077 incremental
probe workflow, `/Ox /Og /Oi /Ot /Oy /MT /Zp1 /EHsc /Gy /GF`, with both `/Ob1`
and `/Ob2`. The minimal port preserves the older algorithm while translating
member names and declarations to compile against R5 layouts; it is not a build
of the original 0.2.5 executable. The adapted port adds behavior independently
identified in the R5 binary. Complete function COMDAT sections are compared;
no masked bytes, prefix comparison, copied machine bytes or truncated bodies.

| R5 function | R5 bytes | Minimal port Ob1 / Ob2 | R5-adapted Ob1 / Ob2 | Result |
|---|---:|---:|---:|---|
| CChatWindow::PushBack, RVA 67450 | 24 | 39 / 39 | 26 / 26 | No match: complete length differs |
| CChatWindow::AddEntry, RVA 67BE0 (0.2.5 AddToChatWindowBuffer) | 538 | 419 / 435 | 538 / 538 | No match: six byte differences after full relocation resolution |
| CEntity::SetModelIndex, RVA 9EF50 | 217 | 144 / 144 | 217 / 217 | Exact resolved COFF function in both profiles; counted once |

The current `closure_chat` was also compiled as the control with `/Ob2`: it
produces 26 and 538 bytes, with the same six differences for AddEntry. The older
reference therefore does not improve these two existing reconstructions.

## Adaptations and concrete benefit

- **PushBack:** 0.2.5 moves entries one at a time in the opposite direction. R5
  shifts the entire 100-entry buffer in one memcpy. Changing the algorithm gives
  our existing 26-byte result, still two bytes too long. This adaptation replaces
  the useful core of the old implementation; no incremental matching benefit.
- **AddEntry:** rename the old method and entry fields; use the packed R5 entry
  and 100-entry layout; write entry 99; change chat type 1 to 2, split length 64
  to 72 and copy limit 128 to 144; add the prefix length rejection, timestamp,
  logging and dirty flag. Its split-at-space logic transfers directly. After
  those changes the generated function reaches exactly the same six-byte LEA
  scheduling difference as our existing reconstruction. Useful historical
  explanation, but no improvement over the current base.
- **SetModelIndex:** keep the original symbolic inline assembly for the two GTA
  virtual calls. Add the R5 helper at B45A0, a signed polling counter and the
  `++attempts > 200` timeout, optional chat warning, and BOOL return indicating
  loading performed here. Null entity and timeout return FALSE. The older source
  supplies the missing method skeleton and the precise native call sequence.
  This produces the full 217-byte R5 function without assembly copied from R5.
  The source already contains symbolic assembly, so this is not a pure C++ match.

The pinned dashr9230 import at `cd3ec8b7ba297ca02402c7f22699dfd5e72e20bd`
has no definition of these three methods in its corresponding entity/chat source
files. Our current base had independently reconstructed the two chat functions;
SetModelIndex was still missing. The measurable incremental benefit is therefore
one newly reconstructed exact function, rather than three newly solved gaps.

## Validation scope and remaining work

The verifier resolves all 15 SetModelIndex relocations through an explicit
identity map, checks the complete warning-string object, the Sleep import, the
existing accepted game/chat/global identities and the original PE relocation
offsets. It pins the complete 40-byte B45A0 helper and its stdcall return boundary.
Changing a function byte or the helper target makes the comparison fail in both
compiler profiles. All compiled source and header hashes are checked.

The isolated `EvaluationEntity` inherits the current CEntity layout without
adding fields or virtual methods; it avoids changing existing headers. These
candidate objects have not been integrated into the linked capsule or exercised
with the runtime ABI harness. B45A0 and its A7A40 dependency remain unmatched;
neither contributes any accepted bytes. **Accepted coverage increment: zero.**
The 217-byte result is explicitly a resolved function match, not whole-file DLL
identity or completed dependency closure.

All pre-existing modified and untracked files were hashed before the experiment
and checked unchanged afterward, including the camera/audio candidates,
game-initialization evidence and coverage counter. Candidate units are separate
`client/saco/eval_michael_*.cpp` files, absent from the production selection and
export manifest. Failed candidates and successful probes remain under
`build/cp32-michael-eval-ob1` and `build/cp32-michael-eval-ob2`.

Recheck the recorded experiment with:

```
.venv/bin/python tools/evaluate_michael_reference.py
```

Conclusion: keep dashr9230 and our verified R5 layouts as the main base. Consult
0.2.5 first for missing old methods and native-call wrappers. This experiment
shows a concrete reconstruction benefit for one such gap, but does not show that
it accelerates the remaining compiler-scheduling work or justify starting over.
