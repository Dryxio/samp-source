# FontSize and command registration closure

Baseline387041, pending independently matched layout214. New proposed normal bodies: cmdFontSize68670/164, RefreshApplicationUiFontsC5430/106, SetupCommands69110/593. Root separately owns final toggles36. Total remainder899, or1113 including layout214. No additional EH is expected in these normal bodies. Exact matches only after root compilation/review.

Sources: closure_cmd_font_size_proposed.cpp, closure_refresh_ui_fonts_proposed.cpp, closure_setup_commands_proposed.cpp. Root's closure_cmd_final_toggles proposal supplies cmdCmpStat too; the separate cmd_cmpstat draft must not be compiled alongside it.

Required one-line header friend: chat-create-fonts-access-proposal.patch. This preserves the actual private CreateFonts AAE symbol, unlike changing member access. Refresh uses the exact R5TransferDialogLayoutView declaration from the already exact214 TU, including public realfield names and offsets. FontSize uses exact R5ChatPagingView declaration from accepted owner, not a differently laid out shadow class. Neither view is allocated.

FontSize164: signed range[-3,5]; true CConfig SetInt(fontsize), CFontRender::CreateFonts, private CChatWindow::CreateFonts, accepted ConfigureScrollBar, CDeathWindow::CreateFonts, then UI refresh. Failure string is exactly `Valid fontsize: -3 to 5` atE9E28 (23 bytes includingNUL), not an invented usage string. fontsize keyE9E40/9. No aliases, new fake methods or provider substitutions.

Seven true4-byte globals initiallyzero, each minimal own TU to preserve whole sections:

| source suffix | symbol | R5RVA | original identity |
| --- | --- | --- | --- |
| ui_dialog_scoreboard | r5UiDialog_scoreboard |26EC28| C5620 allocates actualCDXUTDialog1EA and passes it to scoreboard Reset6EAB0 |
| ui_dialog_chat_command | r5UiDialog_chat_command |26EC24| passed to Chat Reset678A0 and Cmd Reset69840 |
| ui_dialog_spawn | r5UiDialog_spawn |26EC2C| actualCDXUTDialog with `<<`, `>>`, `Spawn` buttons |
| ui_dialog_application3 | r5UiDialog_application3 |26EC30| passed to actual owner26EB50 Reset6FA20, corresponding existing CUnkClass3 |
| ui_dialog_application4 | r5UiDialog_application4 |26EC34| passed to actual owner26EB54 Reset716A0, corresponding existing CUnkClass4 |
| ui_dialog_transfer | r5UiDialog_transfer |26EC38| passed to actual owner26EB58 Reset6AAC0, corresponding existing CUnkClass5 |
| transfer_window | pUnkClass5 |26EB58| true existing main.cpp pointer, its definition must transfer to extern only after sole newstore accepted |

The two application3/4 labels deliberately use existing class associations; their precise user-facing purpose is not inferred. All six resources are definitely CDXUTDialog from actual constructor8CB60, managerinit908C0 and destructor8CC30 calls. None of C5620, those actual allocations, or application lifetime is counted through these pointerstores. C5430 null-checks each pointer and calls accepted UpdateFont8D740/60, then null-checks the application owner and calls actual UpdateLayout6A760/214.

Setup593 is copied from the base whole definition with actual command handlers declared externally. The original command table includes cmdCmpStat689F0 with PUSH at692FD / immediate operand692FE (verified final call sequence), which establishes the specific one-byte no-op identity; arbitrary RET homonyms must not substitute. All other handler addresses/strings must be qualified against fresh COFF, including DebugLabels/ObjectLight root's latestproviders. It retains the release-only tSettings.bDebug conditional around six debug commands. None of these863bytes receives direct025 credit; existingbase/derivedR5 code.
