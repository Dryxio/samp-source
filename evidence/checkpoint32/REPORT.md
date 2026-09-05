# Checkpoint 3.2 — premier lot accepté, checkpoint en cours

44 fonctions complètes, 1 715 octets de code et une constante de 4 octets passent
la comparaison stricte avec le R5 et avec deux DLL réellement liées par MS LINK
2003. L'ajout net est de 1 428 octets : les 287 octets d'ActorPool recoupent des
régions déjà acceptées dans les checkpoints précédents.

La couverture cumulée passe de 23 948 à **25 376 octets**, soit **2,7264 % de
`.text`**. Ce pourcentage mesure les octets de code uniques, pas la proportion
de fonctionnalités, de fonctions nommées ou du fichier DLL complet.

## Périmètre accepté

- ActorPool : constructeur, UpdateCount, Delete, FindIDFromGtaPtr, DeleteAll.
- ActorPed : Destroy, santé/armure en lecture, armure en écriture, état, mort,
  action et présence dans un véhicule.
- Entity : matrices, vitesses, index de modèle, présence/retrait du monde,
  collision, gravité, streaming, transition des tunnels et drapeaux physiques.
- Utilitaires : accès aux pools GTA, ajout/retrait du monde, différences et
  distances flottantes, validation de chaînes numériques.

La liste exhaustive, les frontières et les fixups nommés sont dans
[regions.json](../../config/checkpoint32/regions.json). Chaque corps utilise une
section COFF complète et une frontière de fonction complète de l'inventaire R5.
Aucun masquage d'adresse, `_emit` ou recopiage d'instructions binaires n'est compté.
Les fixups sont résolus à la fois aux adresses R5 et aux adresses réellement
produites par le linker, avec égalité des ensembles de relocations PE.

## Validation

- Builds indépendants `cp32-baseline` et `cp32-repeat`, bases `0x10000000` et
  `0x30000000`, également confirmées pendant l'exécution native.
- **29 501 contrôles natifs par build**, donc 59 002 au total : layouts, champs,
  matrices et padding, états, distances, slots et limites d'ActorPool.
- 16 scénarios émulateur par build : les quatre branches de Destroy et les
  quatre branches de Remove, exécutées sur le R5 et sur le code lié.
- Dans Destroy, le véritable corps de GamePool_Ped_GetAt s'exécute. Seuls la
  méthode du pool GTA, le destructeur natif GTA et le retrait du monde GTA sont
  interceptés. Pointeurs, identifiant conservé/effacé, arguments, pile et
  registres non volatils sont contrôlés.
- Huit tests du vérificateur, dont corruption du dernier octet de chacune des
  44 fonctions dans le COFF puis dans le PE lié, constante altérée, dépendance
  absente, cible incorrecte, troncature et relocation PE supprimée.
- Toutes les sections COFF hors débogage et leurs fixups symboliques sont
  reproductibles. Les gates CP1, CP2 et CP3.1 restent valides.

Les preuves et empreintes figurent dans [acceptance.json](acceptance.json).
Les binaires, objets, snapshots et logs restent dans `build/`, ignoré par Git.

## Sources et assembleur

Les sources complètes importées se trouvent dans `client/saco` et les deux
en-têtes d'archive dans `client/archive`. Leurs empreintes originales et le
commit amont sont conservés dans
[provenance.json](../../config/checkpoint32/provenance.json).
L'import n'accorde aucune couverture aux fonctions non listées dans le contrat.

ActorPool a reçu les méthodes manquantes en C++ ordinaire, avec des champs typés
aux offsets R5. Les déclarations de deux distances ont été corrigées en
`__stdcall` pour correspondre à leurs définitions et au R5. La déclaration de
`_VectorNormalise` a été alignée sur sa définition `__cdecl` pour permettre la
compilation de la TU complète ; cette fonction ne fait pas partie du lot accepté.

Huit fonctions acceptées conservent l'assembleur symbolique de l'amont : Destroy
et sept ponts vers GTA. Elles sont marquées `symbolic-asm` dans le contrat.
Il décrit explicitement les registres, arguments et adresses de l'interface
GTA ; ce n'est pas une reconstruction exclusivement C++. Les autres fonctions
acceptées sont en C++.

`prepare_checkpoint32.py` extrait des **définitions sources complètes**, sans lire
le binaire, dans `client/matching`. Ces unités incrémentales permettent de lier
le périmètre accepté sans résoudre les nombreux corps encore incomplets des
fichiers amont. Les sources générées et leurs en-têtes sont compilés et hashés.
Le client complet et ses initialisations ne sont pas encore intégrés.

## Limites et suite

**Le 3.2 n'est pas terminé.** La capsule `actor.dll` n'est pas un remplacement de
`samp.dll` et n'a pas été déployée dans GTA. Aucun hash du fichier complet ne
correspond au R5.

Le corps d'ActorPool::Delete est exact ; sa cible virtuelle ActorPed n'est pas
encore intégrée. Les tests du pool ne prétendent donc pas valider une destruction
complète via sa vtable. Le test séparé de Destroy exécute déjà son véritable
accès au pool GTA.

La prochaine fermeture de dépendances concerne ActorPed : constructeur,
destructeur, destructeur supprimant, tables virtuelles, EH, Entity::Add/Teleport,
chargement de modèles et commandes de script. Les 22 corps émis dans la TU
ActorPed complète présentent les tailles et instructions hors fixups attendues,
mais cette observation exploratoire **n'est pas une acceptation** de ces corps.
L'ancienne base nomme même le destructeur ActorPed comme une classe de flux C++ :
les noms restent des pistes, les appels et les données font foi.

Les autres sous-systèmes client et la majorité des bibliothèques restent ensuite
à reconstruire et à mesurer.

## Reproduction

```sh
.venv/bin/python tools/run_checkpoint32.py --skip-build
# Nouveau couple de builds sans écraser les tentatives existantes :
.venv/bin/python tools/run_checkpoint32.py --prefix cp32-next
```

La référence privée, les outils VS2003 épinglés et la VM Parallels `Windows 11`
sont nécessaires. Un nom de build existant est refusé pour conserver les preuves.
