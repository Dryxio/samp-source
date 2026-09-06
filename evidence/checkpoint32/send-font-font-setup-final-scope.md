# Scope for the header milestone

Baseline packet-registration387041. This UI closure has1113 new unique code bytes: previously exact transfer layout214, final toggles36, FontSize164, UI refresh106, command registration593. Source0.2.5 direct credit0. Network additions are outside this figure. No pointer/data bytes are counted as code.

The friend-only header change exposes no method publicly. `friend void cmdFontSize(PCHAR szCmd);` grants that function access to the existing private member; its actual COFF call remains ?CreateFonts@CChatWindow@@AAEXXZ at681D0. The accepted provider owner closure_chat_resources and its machine calling convention remain the same. Header hashes require full Ob1/Ob2 profile regeneration; root performs this milestone, and all previously accepted regions must survive the gate.

Exact shared-view text has been independently checked in font-setup-view-odr-audit.json. R5ChatPagingView in cmd_font_size, cmd_ui_settings and chat_paging_resources is identical. R5TransferDialogLayoutView in transfer_dialog_layout and refresh_ui_fonts is identical. Both are pointer-only views; no class-view allocation or lifetime substitutes for the actual application objects. CChatWindow stays the true complete original class, with the single friend declaration applied everywhere via its header.

True owner transfers:
- pUnkClass5 is existing main.cpp application pointer, now extern; closure_store_transfer_window defines the same CUnkClass5* once, full4-byte zero-initialized storage26EB58. Its existing startup allocation remains outside this closure's coverage.
- bShowDebugLabels is existing main.cpp bool, now extern; closure_store_debug_labels_flag defines the same bool once, full1-byte zero-initialized storage26EBB8.
- The six new CDXUTDialog pointer stores each own a full zero-initialized4-byte global, and root's object-light store owns the whole actual BOOL1166C0 initial1. No storage is supplied by an import alias.
- Earlier pDeathWindow, bQuitGame and dwStartQuitTick transfers were accepted in packet-registration. This lot does not transfer them again. The unrelated inherited pLabel work is outside scope.

Actual pointer provenance is preserved in ui-font-dialog-pointer-provenance.json: C5620 builds genuine CDXUTDialog objects using allocation1EA/constructor8CB60 and manager init908C0. It binds26EC28 to scoreboard,26EC24 to both chat/command,26EC2C to the Spawn buttons,26EC30/34 to the existing application3/4 owners and26EC38 to the transfer application. These prove real pointers and consumer identity; they do not grant coverage for C5620 or its allocations/lifetimes. C5430 null-checks them and calls the already accepted CDXUTDialog::UpdateFont8D740, then the newly exact214-byte layout on pUnkClass5.

Setup593 supplies the true original code pointer cmdCmpStat689F0 through operand692FE, establishing its specific no-op identity despite many unrelated RET homologues. Every other command binding, full literal section and relocation was checked in font-setup-reviewed-manifest.json. Final link still must qualify the actual sole owners and their complete initialization, plus inherited navigation's real import-only alias.
