# Checkpoint 2 : capsule liée

Le gate valide 17 régions complètes provenant de 13 fonctions C++ ordinaires,
de deux sections de code auxiliaire EH et de deux sections de données EH.
La région du switch inclut ses 17 pointeurs de branchement et son padding.
Les noms Storage, Buffer et Layer sont des noms locaux de reconstruction ;
ils ne prétendent pas restituer les noms des classes originales.

| Périmètre | Preuve |
|---|---|
| Game::SetGravity → Unprotect → VirtualProtect | Deux fonctions réellement liées ; 12 exécutions Unicorn depuis les DLL candidats ; seule l’API terminale est simulée |
| Unprotect → API Windows | Protection d’une page réelle, écriture et libération dans le processus Windows |
| Layer → Buffer → Storage → operator delete | Chaîne C++ liée, seuils 0/1/32/512/513/1024, conservation/libération du stockage |
| Pool::DeleteAll → Delete → UpdateCount | Fonctions liées ; appel virtuel exécuté avec objets C++ de test |
| FormatBits | 58 octets de code, 2 de padding et 68 de table ; 1 026 entrées testées par build |
| Exceptions C++ | Prologues, helpers, 72 octets de données EH et fixups exacts ; 256 throws/catches par build avec destruction et restauration de FS:[0] |

Les compilations indépendantes utilisent les mêmes outils épinglés et les
bibliothèques CRT dont les hashes figurent dans `config/checkpoint2/libraries.json`.
Les journaux natifs constatent les bases effectivement chargées : 10000000 et
30000000. Chaque build passe 1 338 contrôles, soit 2 676 au total.

Le vérificateur rejoue chaque relocation COFF dans le layout R5 et dans le layout
effectivement produit par MS LINK. Les deux régions résultantes doivent être
entièrement identiques aux images correspondantes. Une cible source doit être
couverte par le manifeste ; les exceptions sont la CRT nommée, l’import Windows
identifié et le pseudo-symbole FS absolu zéro. Les données EH sont localisées en
suivant leurs références liées, sans rechercher une séquence d’octets similaire.
Les relocations PE doivent couvrir exactement les sites DIR32 attendus.

Huit contrôles négatifs rejettent les corruptions de table, données EH, cibles
d’appels source et EH, pseudo-symbole FS, identité IAT, code lié et adresse de
section liée. Les sept familles de contrôles du CP1 et ses 324 exécutions
comportementales passent encore.

## Couverture et limites

- CP2 : 622 octets de code + 142 octets de données/padding. CP1 et CP2 partagent
  166 octets ; leur union contient donc 996 octets de code, sans double compte.
- Deux entrées de la CRT fournie par Microsoft sont exactes après résolution de
  leur transfert : operator delete (5 octets) et CxxFrameHandler (54 octets).
  Le reste de la CRT est une dépendance liée, pas une reconstruction acceptée.
- Les auxiliaires de capsule, DllMain, exports de test, TestActor et son
  destructeur ne sont pas comptés. Le dispatch virtuel respecte l’interface,
  mais le véritable ActorPed R5 derrière cette interface reste à reconstruire.
- Les throws déclenchent le nettoyage d’un objet Layer local dans le test.
  Ils ne constituent pas une injection d’exception dans chaque état interne
  des destructeurs. Leurs tables et helpers sont comparés statiquement en entier.
- L’adresse GTA fixe de gravité peut être occupée dans un processus Windows
  autonome. Ce chemin est donc exécuté dans Unicorn, depuis le DLL candidat
  uniquement ; Unprotect est également testé nativement avec une page allouée.
- Le binaire R5 est utilisé par le comparateur, jamais pour fournir du code à
  la capsule compilée ou aux tests natifs. Aucun candidat n’est déployé dans GTA.
- LINKED_REGION_EXACT désigne une identité des régions avec références résolues
  et vérifiées. Le layout global, les ressources et le fichier complet diffèrent
  encore : aucune identité SHA-256 du DLL entier n’est revendiquée.

Les contrats RVA/taille/hash sont dans `config/checkpoint2/regions.json`.
Les objets, sources instantanées et journaux locaux sont dans `build/cp2-*`.
`acceptance.json` conserve les résultats et empreintes des sources/outils/tests.

## Reproduire

Depuis le dépôt : `.venv/bin/python tools/run_checkpoint2.py`.
Le build utilise uniquement les sources, la toolchain et les bibliothèques Windows.
La comparaison nécessite ensuite sa propre référence exacte dans `private/samp.dll`.
`--skip-build` exige les artefacts locaux existants et vérifie leur fraîcheur.
