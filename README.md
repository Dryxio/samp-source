# SA-MP 0.3.7-R5 — matching strict

Nouveau dépôt local indépendant des anciens projets. Objectif : reconstruire
des sources maintenables produisant les octets exacts du R5 gelé.

## Résultat du premier pilote

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

Ce n'est **pas** un DLL complet lié, ni un client jouable. Les deux fonctions
externes `ActorPool::Delete` et `Unprotect` sont seulement identifiées par leurs
adresses et restent à reconstruire. Elles sont simulées dans les tests émulateur.
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

## Prochain jalon

Ajouter une tranche avec dépendances **reconstruites et liées ensemble**, puis
aborder jump tables, SEH/CRT et layout des sections. Le résultat final à viser
reste l'identité du fichier entier, métadonnées comprises ; les succès actuels
portent sur les fonctions, leurs références et les données constantes associées.
