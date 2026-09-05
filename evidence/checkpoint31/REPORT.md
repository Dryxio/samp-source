# Checkpoint 3.1 — socle et dépendances

**Validé sur le périmètre du socle.** Trois archives sont compilées avec MSVC
2003 puis intégrées dans une DLL de test, sans charger `samp.dll`. Le client
SA-MP complet et ses interfaces avec GTA restent le travail du checkpoint 3.2.

| Bibliothèque source | Unités compilées | Régions R5 exactes retenues | Octets de code |
|---|---:|---:|---:|
| RakNet, fork SA-MP et dépendances de sa factory | 40 | 153 | 13 996 |
| DXUT et utilitaires inline du SDK DirectX | 8 | 104 | 5 100 |
| MD5 et SHA-1 fhicl | 2 | 5 | 3 856 |
| Total du lot | **50** | **262** | **22 952** |

Le SHA-1 de RakNet est inclus dans la première ligne. Le cumul sans double
compte CP1 + CP2 + CP3.1 atteint **23 948 octets**, soit **2,572962 %** des
930 756 octets virtuels de `.text`. Les données/padding du CP2 et ses 59 octets
d’entrées CRT sont comptabilisés séparément, pas ajoutés à ce pourcentage.

## Cartographie

- 8 020 entrées de fonctions exportées de l’IDB associé à la référence exacte.
  Chaque plage de la base a été comparée aux octets du DLL gelé.
- 27 200 sites d’appels/sauts extraits aux instructions identifiées par IDA ;
  3 199 transferts indirects restent des dépendances dynamiques à résoudre.
- 320 imports, 11 DLL importées, 19 075 relocations et les cinq sections PE
  inventoriés directement depuis le binaire. Entrée : RVA `0xCBC90`.
- Les noms, classifications et frontières IDA restent des pistes. Les 6 882
  noms classés inconnus à l’extraction ne sont pas inventés ni marqués acceptés.
- Dix structures communes disposent de descriptions explicites et de contrôles
  d’ABI : BitStream, CheckSum, PlayerID, Packet, RPCParameters, GrowableArray,
  BlendColor, MD5Context, Vector3 et Matrix. Les niveaux de preuve sont distincts :
  témoins dans des corps exacts, ABI source seulement, ou hypothèse de layout.

Les détails sont dans `config/checkpoint31/inventory.json`, `pe.json`,
`structures.json` et `dependencies.json`. `coverage.json` liste les régions
sources et les dépendances externes des archives, ainsi que 100 priorités
fondées sur les appels entrants. Il recense 3 094 COMDAT de fonctions dans les
objets, **avec des duplications de templates** ; ce n’est pas un nombre de
fonctions R5 acceptées.

## Preuves d’acceptation

Les 262 régions de ce lot sont **RAW_LINKED_EXACT** : mêmes octets complets dans
l’objet compilé, dans le R5 et dans le DLL effectivement lié. Elles ne contiennent
aucune relocation COFF ou PE. Aucun masque, ajustement d’adresse ou comparaison
tronquée n’est utilisé. Les frontières complètes, branches locales, exports et
symboles MAP sont vérifiés. Les 43 transferts indirects présents dans 26 corps
acceptés restent déclarés : cette preuve du corps appelant ne certifie pas ses
destinations dynamiques.

Deux builds indépendants chargent réellement la DLL aux bases `0x10000000` et
`0x30000000`. Chacun passe **9 811 contrôles natifs**, soit **19 622** au total :
sérialisation au-delà du stockage interne, lecture bit par bit et partielle,
cycle de vie d’un client RakNet sans connexion, MD5 et les deux SHA-1 avec
frontières de blocs/padding, tableaux DXUT, couleur, timer et ABI. Une fonction
D3DX du DLL externe et la requête de version BASS sont également exécutées.
Aucun rendu GTA, écoute réseau, envoi de paquet ou lecture audio n’est lancé.

Les membres des trois archives doivent être exactement les objets vérifiés.
Les deux builds produisent les mêmes sections COFF de code/données et les mêmes
fixups symboliques pour les 50 unités. Seuls les enregistrements de debug, les
timestamps et les indices symboliques SafeSEH résolus par leur identité sont
hors comparaison brute de reproductibilité. Cela ne signifie pas que ces
bibliothèques entières correspondent déjà au R5.

Les contrôles négatifs rejettent la corruption du dernier octet de **chacune des
262 régions**, les tailles modifiées, mauvaise adresse liée, symbole inconnu,
hash de référence incorrect, relocation cachée, archive tronquée et unité
omise. Les cas BSS sont testés séparément. Les gates CP1/CP2 et leurs contrôles
passent encore, avec 324 exécutions CP1 et 12 exécutions CP2 en émulation.

## Limites et suite

Les 273 fichiers upstream sont conservés avec leurs empreintes et notices.
Leurs anciens commentaires « MATCH » ne valent aucune acceptation. Le code
compilé mais absent du contrat reste non accepté, notamment les nombreuses
fonctions qui nécessitent un contrat de relocations et de dépendances.

Les polices et la fenêtre du jeu sont des frontières explicites. Les fournisseurs
de test arrêtent le processus si un chemin de police non implémenté est appelé ;
ils ne sont jamais comptés comme du code reconstruit. Le warning DXUT RenderArrow
est conservé et documenté ; la fonction concernée n’est pas acceptée. Les DLL
BASS/D3DX et les bibliothèques Microsoft sont des dépendances épinglées, pas
des bibliothèques dont tout le code aurait été reconstruit.

Le 3.2 peut utiliser ce socle pour remplacer les interfaces de jeu, vérifier les
fonctions avec références, puis intégrer les pools et le protocole du client.
Le layout et l’identité SHA-256 du fichier entier restent le checkpoint 4.

## Reproduire

Depuis le dépôt :

```sh
.venv/bin/python tools/run_checkpoint31.py
# Ou revérifier les artefacts existants, avec contrôle de fraîcheur :
.venv/bin/python tools/run_checkpoint31.py --skip-build
```

Prérequis : VM et toolchain des checkpoints précédents, référence exacte dans
`private/samp.dll`, inputs privés aux hashes de `private-inputs.json`, et artefacts
CP1/CP2 pour les régressions. Au besoin, les produire avec `tools/run.py` puis
`tools/run_checkpoint2.py`. Les sources vendor sont incluses ; les binaires ne
le sont pas. Le build ne dépend pas des anciens dépôts locaux.

L’IDB n’est nécessaire que pour régénérer la cartographie avec
`tools/inventory_checkpoint31.py`. La commande normale vérifie la cartographie
gelée contre le DLL. `tools/discover_checkpoint31.py` écrit des propositions
dans `build/` et ne modifie jamais le contrat d’acceptation.
