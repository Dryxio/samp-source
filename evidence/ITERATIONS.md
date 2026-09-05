# Itérations du premier pilote — 2026-09-05

Les trois premières compilations `a`, `b`, `c` sont conservées localement dans
`build/`, avec les hashes de sources/objets enregistrés et leurs snapshots source.
Les builds finaux `baseline` et `repeat` sont indépendants de ces essais.

## A — expressions naturelles : 13 / 15

La source caméra initiale utilisait une seule expression :

```cpp
*cameraZoom = remoteZoom[id] * 35.0f + 35.0f;
```

MSVC réassociait l'expression en `(zoom + 1) * 35`. R5 multiplie puis additionne.
Le code produit était donc différent malgré une formule algébriquement
équivalente ; l'arrondi flottant peut lui aussi différer. L'écart n'a pas été masqué.

La météo écrivait la valeur courante avant de lire le champ de condition.
R5 lit et teste ce champ avant l'écriture. Là encore, le matching échouait.

## B — caméra exacte, allocation de registre météo différente : 14 / 15

Deux instructions C++ distinctes évitent la réassociation caméra :

```cpp
float zoom = remoteZoom[id] * 35.0f;
zoom += 35.0f;
*cameraZoom = zoom;
```

La météo avec une variable locale capturée avant l'écriture obtenait le bon
ordre, mais conservait ECX là où R5 utilisait EAX. Ce résultat restait rejeté.

## C puis baseline / repeat — 15 / 15

Exprimer explicitement les deux branches météo, avec l'écriture commune dans
chacune, conduit MSVC à produire exactement l'ordre et les registres R5.

Le constructeur ActorPool, la recherche et `UpdateCount` passent directement.
La boucle C++ simple de `UpdateCount` est automatiquement déroulée par MSVC en
groupes de cinq et reproduit **85 octets complets**. C'est notamment plus long que
l'ancien contrat de 48 octets du projet précédent : les anciennes tailles ne
constituent pas une référence fiable.

## Limites examinées

- `GetAt` accepte l'index 1004 dans le binaire, contrairement à `GetSlotState`.
  Ce comportement, y compris l'accès au-delà du tableau logique reconstruit, est
  conservé et testé dans une mémoire émulateur contrôlée. Il n'a pas été corrigé
  silencieusement au détriment du matching.
- Les noms de champs sont des interprétations. En particulier `weatherLocked`
  désigne le champ à l'offset `0x69` et son effet sur les écritures observées ;
  ce pilote n'établit pas son nom ou son sens original complet.
- Les tests flottants couvrent des valeurs finies usuelles. NaN, infinis et tous
  les modes de contrôle x87 ne sont pas couverts par les cas comportementaux.
- Les appels externes sont des mocks dans l'émulateur. Leur ABI et destination
  sont contrôlées, leur véritable implémentation n'est pas validée.
- L'émulation x86 et l'égalité après résolution des symboles ne remplacent pas
  une validation native dans GTA ni la reconstruction du linkage complet.

Conclusion : le workflow discrimine des erreurs réelles, permet des itérations
C++ explicables et reproduit les résultats. Il est confirmé sur ce pilote.
