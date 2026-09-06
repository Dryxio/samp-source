# Independent ListBox insertion287

Current actual run cp32-listbox-header-ob1, closure_gui_listbox_complete_insert.obj; two entire sections byte-exact after7 real provider fixups, with original PE relocation equality. No EH or new data. Manifest listbox-insert287-reviewed-manifest.json and reproducible script preflight_listbox_insert287.py.

CDXUTListBox::InsertItem8A8F0/195 has true four stack arguments (index,text,data,color), ret16. It allocates the complete previously proven R5CompleteListBoxItem29E using operatornewC627A, checks allocation, copies primary text with capacity256 through the inline checked copy, stores user data284, zeros RECT288 through true SetRect IAT E5300, sets selected298=false and color299; it does not fabricate initialization of columns or marked29D. Typed Insert89B80/92 updates the same pointer array at this14C and reuses accepted SetSizeInternal86C60/142 plus memmove. On insertion failure it frees the complete item with operator deleteC648A; on success it updates the actual embedded scrollbar5D via accepted SetTrackRange88C50. Current full class declaration has real size158 and shifted fields correctly; this method never allocates a ListBox. No ctor/VFT/renderer or reset dependency, so this287 can close independently.

COMDAT SetSizeInternal may be emitted in both widget and insert; same actual target86C60, zero new unique coverage. Root transfer seeds choose widget sole accepted owner. Root gate validates chosen physical copies; no second RVA credit from identical templates.

Base already supplies vendor Insert logic; R5 adds real per-item color/layout and fourth argument. Strict direct025 source credit zero. No additional source needed beyond the proposed now-active insert TU.
