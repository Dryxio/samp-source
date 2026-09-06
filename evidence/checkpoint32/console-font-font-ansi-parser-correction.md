# Grounded ANSI parser correction

Current compiled source257 differs from original271 because it calls IsHexChar for all six positions. Original B5E99..B5EC7 expands the first check in place (signed char comparisons against0..9,A..F,a..f), and calls the genuine IsHexCharB5E10 exactly five times for positions2..6. The remainder, including17-byte memset, strncpy6, strtoul16 and stack cleanup, is structurally identical shifted14 bytes.

The isolated full closure_util proposal expands only position1 into the exact ordinary C++ predicate already implemented by IsHexChar(char), retaining all five true calls. This changes no semantics, signature, constants, shared headers, or other methods. It is a source-level rendering of the observed selective inlining, not a frame/padding permutation. It requires a new bounded probe and all accepted siblings revalidation before replacement. ANSI parser still carries zero credit until exact271 and true imports/providers are verified.
