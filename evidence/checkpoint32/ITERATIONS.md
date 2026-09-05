# Itérations conservées

Les logs et snapshots sont dans `build/` (non versionnés).

1. `cp32-actor-probe` : échec, en-têtes `archive/pkey.h` et `skey.h` absents.
2. `cp32-actor-probe2` : ActorPed/Entity compilent ; util échoue sur trois
   conventions d'appel incohérentes entre déclarations et définitions.
3. `cp32-actor-probe3` : quatre TU complètes compilent après correction.
4. `cp32-actor-link1` : tentative de link sans fichier d'exports, non acceptée.
5. `cp32-actor-link2` : les TU complètes entraînent des dépendances client encore
   non résolues. Aucun fournisseur factice n'a été ajouté.
6. `cp32-actor-link3/4` : unités sources incrémentales, essais `/noentry` rejetés
   à cause de l'intégration du CRT (`__fltused`, puis `_main`).
7. `cp32-actor-link5` : link DLL avec initialisation CRT normale, 27 fonctions
   exactes pour 1 044 octets. Probe, pas preuve finale.
8. `cp32-native1` : 9 231 contrôles natifs sur le premier périmètre.
9. `cp32-expanded1` : compilation du test interrompue par la déclaration absente
   de IsNumeric. `cp32-expanded2` la corrige dans le test.
10. `cp32-pool1` : ajout des méthodes ActorPool, 44 régions et 29 501 contrôles
    natifs. Un `.DS_Store` créé par le Finder a révélé qu'il fallait exclure ce
    fichier de métadonnées du contrôle de l'inventaire source.
11. `cp32-baseline` et `cp32-repeat` : couples de preuves finales, deux adresses
    de chargement, tests négatifs, émulation et régressions. Voir acceptance.json.

La déclaration de `_VectorNormalise` est corrigée pour compiler l'amont ; cette
fonction n'est pas acceptée dans le lot. Les échecs ne sont jamais de la couverture.
