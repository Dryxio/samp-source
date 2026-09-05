# SA-MP 0.3.7-R5 — matching strict

Nouveau dépôt local indépendant des anciens projets. Objectif : reconstruire
des sources maintenables produisant les octets exacts du R5 gelé.

## Checkpoint 3.2 — en cours

La couverture cumulée atteint **29 955 octets de code uniques (3,2184 % de
`.text`)**. Le lot Entity courant valide 124 régions de code/données, dont
85 fonctions complètes (5 903 octets de code, **386 nouveaux** par rapport au
lot précédent, sans doublons).

Les nouvelles fonctions couvrent force, audio, RenderWare, contrôle natif,
cible native, immobilité et avancement de position. Ce dernier corps est exact,
mais sa dépendance SetMatrixAndUpdate reste nonmatching et exclue, comme
FUNC_1009EC80, GetEulerAngles, AddEntry et PushBack. La capsule liée et les tests
ABI force/audio passent ; le checkpoint et le DLL complet restent incomplets.

```sh
.venv/bin/python tools/accept_actor_closure.py --run cp32-entity-tail-linked \
  --contract config/checkpoint32/entity-tail-contract.json \
  --report entity-tail-acceptance.json \
  --previous evidence/checkpoint32/closure-acceptance.json \
  --previous evidence/checkpoint32/actor-expansion-acceptance.json
```

Voir [les preuves actuelles](evidence/checkpoint32/entity-tail-acceptance.json),
[la couverture actuelle](evidence/checkpoint32/coverage-current.json) et
[le travail en cours](evidence/checkpoint32/PROGRESS.md).
Le [premier lot historique](evidence/checkpoint32/REPORT.md) couvrait 44 fonctions
et comportait 59 002 contrôles natifs sur deux builds indépendants.

## Checkpoint 3.1 — socle et dépendances validé

**50 unités sources, trois bibliothèques liées et 262 régions RAW_LINKED_EXACT** :
RakNet, DXUT et utilitaires MD5/SHA-1. Les deux builds indépendants passent
**19 622 contrôles natifs** au total, y compris les imports réels D3DX et BASS.
Leurs sections de code/données et fixups symboliques sont reproductibles.

La cartographie recense 8 020 entrées de fonctions, 320 imports et les dépendances
restantes. Dix structures communes disposent de layouts et de niveaux de preuve
explicites. La couverture exacte cumulée atteint **23 948 octets de code**, soit
**2,573 % de `.text`**. Compiler une bibliothèque ne compte pas comme la matcher.

```sh
.venv/bin/python tools/run_checkpoint31.py
# Avec les artefacts locaux existants :
.venv/bin/python tools/run_checkpoint31.py --skip-build
```

Voir [le rapport et ses limites](evidence/checkpoint31/REPORT.md),
[la couverture détaillée](evidence/checkpoint31/coverage.json),
[les preuves d’acceptation](evidence/checkpoint31/acceptance.json) et
[les itérations](evidence/checkpoint31/ITERATIONS.md).
Les interfaces spécifiques au jeu restent à implémenter au 3.2 ; ce DLL de test
n’est pas un client SA-MP complet.

## Checkpoint 2 — reconstruction liée validée

Une capsule DLL est maintenant compilée et liée par MSVC/MS LINK 2003, puis
exécutée dans Windows sans charger le DLL original. **17 régions complètes**
correspondent au R5 après résolution explicite des références : **622 octets de
code et 142 octets de données/padding**, dont table de branchement et métadonnées
d’exceptions C++. Deux entrées CRT Microsoft totalisant 59 octets sont vérifiées
séparément. Le cumul CP1 + CP2 est de **996 octets de code uniques**.

Deux builds aux bases `0x10000000` et `0x30000000` passent chacun 1 338 contrôles
natifs. Les appels liés, le dispatch virtuel, la table complète et le déroulement
d’exceptions sont exercés. Huit mutations incorrectes sont rejetées ; les
324 contrôles émulateur du premier checkpoint passent toujours.

```sh
.venv/bin/python tools/run_checkpoint2.py
# Revérification des artefacts existants :
.venv/bin/python tools/run_checkpoint2.py --skip-build
```

Voir [le périmètre et les limites](evidence/checkpoint2/REPORT.md),
[l’acceptation avec empreintes](evidence/checkpoint2/acceptance.json) et
[les itérations](evidence/checkpoint2/ITERATIONS.md). **Ce résultat valide le
workflow sur ces régions ; ce n’est pas un client SA-MP complet.**

## Résultat du premier pilote (historique)

**15 fonctions C++, 540 octets de code uniques validés**, avec MSVC 13.10.3077
dans la VM Parallels `Windows 11`. Aucun assembleur, `_emit` ou corps original
recopié dans les sources.

| Domaine | Fonctions | Octets |
|---|---:|---:|
| PlayerPool / LocalPlayer : bornes, pointeurs, touches | 3 | 117 |
| ActorPool : constructeur, boucle déroulée, recherche, appels | 4 | 190 |
| Caméra : globals, tableaux, constantes, x87 | 6 | 177 |
| Jeu : météo, appel de protection mémoire / gravité | 2 | 56 |
| Total | **15** | **540** |

- **7 RAW_EXACT** : corps objets directement identiques, sans fixup.
- **8 RESOLVED_FUNCTION_EXACT** : identiques après résolution explicite des
  relocations COFF aux adresses R5. Les constantes sont vérifiées et les sites
  d'adresses absolues doivent correspondre aux relocations du PE original.
- Deux builds indépendants, dans des dossiers Windows distincts, produisent les
  mêmes corps complets. Les timestamps COFF ne sont pas comptés comme du code.
- Les mêmes comparaisons passent après rebase de `0x10000000` à `0x30000000`.
- 7 familles de contrôles négatifs rejettent les mauvaises cibles, instructions
  modifiées, tailles tronquées/allongées, symboles absents et fixups invalides.
- 81 contrôles de comportement en émulation x86, sur original et candidat à deux
  bases : **324 PASS**, avec contrôle de pile et registres préservés à chaque appel.

Au checkpoint 1, ce n’était **pas** un DLL complet lié, ni un client jouable. Les deux fonctions
externes `ActorPool::Delete` et `Unprotect` étaient seulement identifiées par leurs
adresses. Elles sont désormais reconstruites au checkpoint 2 ; les tests
historiques du checkpoint 1 continuent de les simuler.
Les tests ne chargent aucun candidat dans GTA. La couverture validée est d'environ
**0,058 %** des 930 756 octets virtuels de `.text`, sans compter le code dépendant.

Voir [l'acceptation vérifiable](evidence/acceptance.json),
[les itérations](evidence/ITERATIONS.md) et [la provenance](config/provenance.json).

## Reproduire

La référence locale est gelée sous `private/samp.dll`, ignorée par Git :

```text
SHA-256 b72b5dbe725f81864ca3f78bc7063bda56cc05fc7188af822fa7a754432553a2
MD5     5ba5f0be7af99dfd03fb39e88a970a2b
Taille  1 204 224 octets
```

Sur ce Mac, depuis ce dépôt :

```sh
python3.13 -m venv .venv
.venv/bin/python -m pip install -r requirements.txt
.venv/bin/python tools/run.py
```

Le runner compile `baseline` puis `repeat`, vérifie les corps, exécute les
contrôles négatifs et les tests émulateur, puis actualise `evidence/acceptance.json`.
Une erreur à n'importe quelle étape termine avec un code non nul. La version,
les hashes du compilateur/backend et les options sont épinglés dans
`config/toolchain.json`. Les sources et objets de chaque build sont hashés ; un
objet périmé ne peut pas valider des sources modifiées.

Pour revérifier les artefacts existants sans compiler :

```sh
.venv/bin/python tools/run.py --skip-build
```

Prérequis VM : partage Parallels `C:\Mac\Home`, installation
`C:\Program Files (x86)\Microsoft Visual Studio .NET 2003`, accès `prlctl`.
Le build s'effectue uniquement dans `C:\dev\samp-r5-matching\<run>`.

`private/`, `build/` et `.venv/` ne sont pas versionnés. Après un clone neuf,
fournir sa propre copie de la référence exacte dans `private/samp.dll`.

## Ajouter une fonction

1. Examiner le R5 gelé, noter RVA, limite complète, branches, retours, appels et
   références de données. Les exports IDA/Ghidra et anciens sources sont des pistes.
2. Écrire du C++ ordinaire dans un module limité. Documenter les dépendances non
   reconstruites et les noms sémantiques encore hypothétiques.
3. Ajouter le contrat dans `config/manifest.json` : symbole COFF exact, taille
   complète, hash, fonctionnalités exercées et provenance.
4. Déclarer chaque symbole externe dans le manifeste, avec cible justifiée. Ne
   jamais ajouter un masque d'octets pour faire disparaître un écart.
5. Compiler, expliquer les écarts, puis ajouter les cas comportementaux pertinents.
   Exécuter le runner et ne compter que les fonctions entièrement acceptées.

Le lecteur COFF volontairement restreint exige une COMDAT `.text` dédiée par
fonction et ne supporte que DIR32/REL32. Le contrôle des limites du premier pilote
exige des branches locales, un retour terminal et un INT3 après le corps original.
Les jump tables, tail calls, fonctions réparties sur plusieurs sections et SEH
nécessitent d'étendre explicitement ces contrats et leurs tests ; ils ne doivent
pas être acceptés par suppression des contrôles.

## Checkpoints restants

- **3.2 — Client reconstruit** : pools, jeu, synchronisation, RPC et interface,
  avec dépendances réelles et matching vérifié. Remplacer les frontières de test.
- **3.3 — Intégration et validation** : client autonome et scénarios de jeu
  sur un environnement de test ; écarts restants précisément inventoriés.
- **4 — Identité du fichier** : layout, ressources, métadonnées et SHA-256
  strictement identiques au DLL R5 de référence.

Le matching complet reste l’objectif ; les succès du socle ne le garantissent
pas encore.
