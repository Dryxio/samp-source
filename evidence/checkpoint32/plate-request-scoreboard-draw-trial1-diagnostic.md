# Draw790 trial1: bounded source diagnosis

Principal790 has correct frame60, full helper inlining already achieved under Ob1, and actual API/field identities. New request151 is independently exact and fully preflighted. No blanket Ob2 need demonstrated.

After resolving real providers and data, remaining instruction ordering differs in three windows:
- offset94..101 / original6F10E: original JLE width guard precedes MOV height fromC17048; source initializes screenHeight before the width condition, so compiler emits that read before the branch. Trial2 moves the actual height read into the width>0 block. This respects the observed control dependency, not a dummy lifetime.
- offset491..500 / original6F29B: original stores truncated headerHeight to stack14 before loading pDefaultFont; trial loads font first then stores the same integer. Same values and final frame locations.
- offset769..776 / original6F3B1: original PUSH0 before loading pDefaultFont into ECX; trial reverses that scheduling. Same receiver and argument.

No artificial variables, barrier, padding, or integer type permutation proposed for the latter two windows. Trial2 changes only the grounded conditional-read ordering. All earlier whole original data evidence remains required; code-only relocation discovery never establishes final literal identity.

Trial2 outcome: source conditional-read correction changes exactly the eight bytes at94..101 and now matches that original window. No other section byte changes; full length790 still. Remaining two windows are header local store vs pDefaultFont load (491..500) and final pDefaultFont load vs PUSH0 (769..776). Neither changes live ranges, values, ABI, or branch structure. No additional source-based hypothesis is established; keep Draw unaccepted/parked rather than permuting unrelated types or introducing artificial barriers. Request172 remains independently closed.
