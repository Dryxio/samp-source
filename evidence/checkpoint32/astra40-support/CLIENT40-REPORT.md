# Client/Game/UI — reprise Astra 40 minutes

État au 2026-09-09 21:14 UTC : base acceptée `astra40-three`, commit `fdc201b`, 434468/930756. Cette reprise a livré267 octets nouveaux normaux C++ effectivement acceptés : GetCount29, taux120, RemoveAttachedObject81, GetAttachedObjectSlotState37. Sources nouvelles dans l’isolation, compilation/intégration exclusivement par parent. Aucune VM, compilation, modification active, commit ou sous-agent effectué par ce chercheur.

## Gain accepté267

`stats-attachments267-proposal.json`, compilé par intégrateur dans `cp32-astra40-probe-three`. Trois unités, cinq racines, code267 et vrais taux DWORD8, six régions et huit bindings exacts. Aucun nouvel import/EH/alias ou propriétaire concurrent. Les sources initiales ratées restent intactes. `while(index!=1000)` restaure le compteur simple29 au lieu du déroulage62. Corps positif `if(ped)` restitue exactement les branches60; `.r5rates` contient les deux véritables int30 adjacents. Objets attachés : complete vue tailleCPlayerPed, état4C, info52x10 à74, pointeurs27C vers vrai propriétaire CObject via baseCEntity0, suppression virtuelle réelle. Bornes signées et égalité état==1 exactes.

## Lots baseline remis, acceptation finale encore à confirmer

- SDK std::string543 :13 racines complètes, sources/objets100% préexistants. 46 instances de propriétaires COMDAT dans cinq unités identiques sur sections, types et cibles. Fermeture sélectionnée21 régions ; preuve exhaustive57 régions séparée. _Copy316, son handler10 et données132 sont déjà acceptés et entièrement revérifiés. Un octet interchunks de _Copy est exclu : gain543, jamais544.
- DXUT/D3DX446 : proposition historique460 dont `_powf14` retiré par intégrateur après rejet du lien.13 racines restantes ; aucun nouveau stockage/import. Les dépendances SDK/état complet/math sont déjà acceptées. Preuve complète des propriétaires séparée de fermeture sélectionnée81 régions (avant retrait14).
- Feuilles UI134 : six régions198, dont réel StringLengthWorkerA64 antérieurement couvert checkpoint31 et zérocrédit. Wrapper40, ObjectPool GetAt37, Menu GetSlotState26, ShowNameTag17, nomlocal14. Ces sources préexistantes préservent les bornes originales inclusives id<=1000 et menu<=128, même aux indices limites ; ShowNameTag normalise BYTE!=0 en BOOL. `sdk-ui134-proposal.json` contient seulement la fermeture sélectionnée ; `sdk-ui134-owner-proof.json` est séparé.

Potentiel remis encore non confirmé :543+446+134=1123 octets, tous préexistants ; aucun crédit source0.2.5 exclusif. Les trois propositions reposent sur `astra40-three` et sur les SHA actuels sources/snapshots/objets/métadonnées du build. Aucun double crédit de code accepté, padding, donnée ou EH.

Une tentative pipeline a rejeté33 régions des anciennes revues allowners parce qu’elles n’étaient pas atteintes depuis les seules racines sélectionnées. Les octets des copies étaient exacts mais elles appartiennent à une preuve d’ownership séparée, pas au manifeste de fermeture du draft. Intégrateur a conservé ces preuves, filtré des régions entières selon le draft et relancé. Pour les futures remises : manifeste de fermeture uniquement atteinte + sidecar tous propriétaires. Les demandes canonical et scripts sont conservés.

`_powf` : le lien five ne résout pas son export. Vérification locale : symbole global COFF storage2/type32/section5 dans baseline et builds five (pas storage3). La .def contient littéralement `_powf` ligne2963. Hypothèse de décoration cdecl versus export `powf`, à qualifier séparément. Aucun wrapper, source synthétique, modification du run ou faux export ajouté. Retrait14 conserve un lot fermé.

## Draw statistiques1310 garé

V1 émet1327, V2 1323, V3 1326. Les sources et demandes sont séparées ; `stats-draw-*-reviewed-manifest.json` et disassemblages conservent les divergences. V2 corrige réellement PROCESS_MEMORY_COUNTERS40 via memset (agrégat={0} émettait un store cb immédiat séparé). La partie initiale jusqu’au comptage et le rendu final correspondent ; restent allocations/scheduling des compteurs et copies de résultats. Deux variantes de déclarations guidées par les changements observés n’atteignent pas1310. Pas d’autre variante sans preuve nouvelle, pas de volatile/faux alignement/asm/registres imposés.

Vrais buffers isolés BSS : drawLine1024@125A60, stats16384@125E60, display16384@129E60, shot DWORD4@118990. Propriétaires/fixups/usage sont dans STATISTICS.md et stats-global-xrefs.json. La source0.2.5 consultée se trouve réellement dans le dépôt préservé voisin `/Users/salimtrouve/Documents/GitHub/samp-r5-preservation/references/michael-fa-samp/client/netstats.cpp`; elle fournit la structure, pas les ajouts R5. GetProcessMemoryInfo passe par vrai thunkC6208/IAT E5288 de PSAPI, SDK qualifié par intégrateur ; GetCurrentProcess IAT E5218 demeure nouvelle entrée à qualifier si Draw devient exact. Aucun octet Draw accepté.

EditMsgProc597 demeure garé avec21 différences de registres ; aucune nouvelle tentative. ChatDraw662, ListBox constructeur/render, Label755 ont été relus et laissés garés faute d’hypothèse nouvelle. Les petits getters HUD14 ont une provenance0.2.5 possible mais n’ont pas été compilés ni comptés pendant cette fenêtre.
