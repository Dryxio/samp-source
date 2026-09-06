# Isolated ListBox header correction: concrete impact audit before application

DO NOT apply yet: root owns milestone scheduling and the required fresh header rebuild. No active file changed. This audit concerns only CDXUTListBox member order, not CDXUTScrollBar or CDXUTComboBox member order; neither item header nor base Control layout needs to change.

Current declared layout: base CDXUTControl0..4C, column fields4D..5C, m_rcText5D..6C, m_rcSelection6D..7C, m_ScrollBar7D..12E, remaining members12F..157. Actual R5: same base/columns, m_ScrollBar5D..10E, m_rcText10F..11E, m_rcSelection11F..12E, same remainder12F..157. Complete sizeof158 remains unchanged. ScrollBar complete sizeofB2 is already accepted via existing ctor/vtable/field consumers. Exact original ctor8ED90 explicitly constructs scrollbar at5D and item array14C; accepted UpdateRects88C80 and interaction methods reference the two RECTs10F/11F. Existing R5ListBoxScrollView offsetof assertions already express those three actual offsets.

All eight accepted CDXUTListBox method owners found in current contract, with whole regions:

| Method | Original RVA/whole region | Owner | Consequence of reorder |
|---|---|---|---|
| HandleKeyboard |8AA80/480|closure_gui_listbox|All scrollbar accesses already use R5ListBoxScrollView; remaining native members after12F unchanged|
| HandleMouse |8AC60/1171|closure_gui_listbox|All scrollbar/text/selection accesses already use R5ListBoxScrollView; complete selected item view298 unchanged|
| SelectItem |8A9F0/134|closure_gui_listbox|ShowItem uses view scrollbar, selection/items fields unchanged|
| GetSelectedIndex |88E70/85|closure_gui_listbox|Only style/selection/items and item selected view; no displaced member access|
| GetItem |8A9C0/33|closure_gui_listbox|Only array14C, unchanged|
| RemoveItem |88D80/113|closure_gui_listbox|Scrollbar SetTrackRange via view; array/selection fields unchanged|
| RemoveAllItems |8C7F0/73|closure_gui_listbox|Scrollbar SetTrackRange via view; array14C unchanged|
| UpdateRects |88C80/256|closure_gui_widget_lifetime|Every scrollbar/text/selection use explicitly through R5ListBoxScrollView; other fields unchanged|

The eight original complete ranges sum2345 (includes switch tables); predicted source address changes: NONE. This is a prediction, not a replacement for new compiler hashes/full comparison. Current closure_gui_controls also declares the same stable R5 view; ListBoxUpdateRects was already moved to widget_lifetime, so do not reintroduce its old definition.

Current inline methods: OnInit refers directly to m_ScrollBar and WILL change from wrong+7D to correct+5D, but its original8EE40/13 is entirely unaccepted and part of new closure. CanHaveFocus only basefields, unchanged but original8EE20/23 unaccepted. Constructor/dtor/Render are unaccepted and are exactly the intended newly corrected members. All later inline setters/getters (style13F, selected143, scrollbar-width12F, border133, margin137, array14C) retain offsets and signatures.

Accepted app users: R5ListBoxItemOwnerView (AddItem254/SetColumn106) uses arrays14C and columnCount4D plus raw scrollbar5D; unchanged. Scoreboard actions/update use existing GUI methods, base virtual slots, and explicit pointer+5D; unchanged. Their allocation-free pointer views remain unchanged. Accepted initialization sources creating ComboBox/Slider/Edit controls use their own classes, not the moved ListBox fields. Currently accepted code has no ListBox constructor allocation; its ctor144, deleting destructor30, OnInit13, Render1014 all remain uncovered in complete code union. Existing source main/unkclass/scoreboard may contain legacy pending ListBox allocations, but these are not accepted providers and must not be accidentally pulled into coverage.

Additional proposed header declaration: add fourth DWORDcolor parameter with default0 to the currently unimplemented ListBox InsertItem method. Original8A8F0 ret16 and field299 store prove fourth argument. No currently accepted InsertItem caller/body identified; accepted AddItem belongs to separate actual R5ListBoxItemOwnerView and is untouched. This is nonvirtual: no vtable reordering.

Compile effects: shared header hash invalidates all dependent cached objects (~380 per root), even where layouts are unchanged. Do not claim cache reuse across it. Root should schedule a single deliberate fresh milestone and compare all accepted code/targets as required. Closest bodies at risk from compiler visibility (not shifted fields) are widget_lifetime functions if new constructor/destructor ownership is consolidated there, and RemoveAllItems if moved from listbox TU to provide destructor visibility. Those separate source-ownership effects must be tracked in addition to header changes. No current source/header application has been performed.
