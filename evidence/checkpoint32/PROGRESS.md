# Travail autonome — fermeture ActorPed en cours

Commit de départ accepté : `3716bee` (44 fonctions liées, 25 376 octets cumulés).
Les nouvelles régions ci-dessous sont **des propositions**, pas une nouvelle
acceptation ni une augmentation de la couverture publiée.

- `tools/probe_client.ps1` compile par unité et réutilise les objets selon une clé
  de sources/en-têtes/options/outils. La tentative `cp32-chat-models2` recompile
  chatwindow et réutilise effectivement closure_models. Chaque tentative garde
  son snapshot et ses empreintes ; elle ne produit aucune acceptation.
- Entity : `/Ob2` corrige complètement les corps Add (207 octets) et TeleportTo
  (115). Le vcproj amont applique effectivement une optimisation spécifique au
  fichier Entity. Les réglages ne doivent plus être uniformes par défaut.
- ActorPed : `/GF` sépare correctement les chaînes des commandes de script.
  Le bloc de 138 octets de commandes correspond alors intégralement au R5.
- ScriptCommand : la section complète fait 565 octets, dont 518 de corps de
  fonction, suivis du padding et des tables de dispatch. Ne pas la tronquer à 518.
- Les tables virtuelles emploient les alias faibles MSVC `_E` avec repli `_G`.
  Leurs auxiliaires COFF sont lus explicitement ; aucune fausse fonction relais.
- `closure_models.cpp` contient quatre définitions sources complètes extraites
  de game.cpp. Leur bloc de commandes a la disposition originale. Ce fichier
  sert à fermer les dépendances sans embarquer les fonctions amont incomplètes.
- AddDebugMessage, auparavant vide, est reconstruit en C++ : tampon de 512 octets
  initialisé par memset, vsprintf, remplacement des contrôles ASCII strictement
  positifs, appel AddEntry. L'essai avec `{0}` donnait 120 octets ; memset donne les
  117 octets complets attendus, y compris l'alignement de boucle généré par MSVC.

Le graphe exploratoire actuel propose **39 sections entières**, sans différence
hors fixups ni conflit de placement. Il inclut constructeur/destructeur ActorPed,
helpers de suppression, vtables, EH, Entity Add/Teleport/Remove, wrappers modèles,
ScriptCommand/ExecuteScriptBuf, filtre d'exception et AddDebugMessage.
Les adresses obtenues en suivant les fixups restent à revoir : ce procédé de
**découverte** n'est pas une preuve autonome de l'identité des cibles.

Reproduction de la découverte :

```sh
.venv/bin/python tools/discover_client_graph.py config/checkpoint32/closure-seeds.json
```

La seule fonction source encore absente dans ce graphe est
`CChatWindow::AddEntry` (RVA `0x67BE0`, corps de 538 octets). La déclaration ajoutée
n'est pas une implémentation et n'apporte aucune couverture. Les autres sorties
sont des globals R5, Sleep et les entrées CRT/EH ; elles nécessitent des contrats
explicites et les vrais fournisseurs au link.

Prochaine action : reconstruire AddEntry depuis le R5, puis poursuivre ses
éventuelles dépendances. Fermer le link et vérifier les sections complètes avec
un manifeste revu avant d'accepter ce nouveau lot. Les snapshots des anciens lots
restent des preuves historiques ; les modifications de client/ demandent une
nouvelle validation pour toute acceptation actuelle.

## Étape suivante réalisée — capsule liée (état actuel)

La capsule `build/cp32-closure-linked2/closure.dll` se lie désormais avec de vraies
implémentations pour toute la branche de dépendances examinée. Aucun fournisseur
factice ni appel réseau de test. Les objets sont réutilisés au link ; aucune
seconde compilation intégrale n'a été lancée.

- AddEntry est maintenant implémenté en C++ (538 octets). Six octets diffèrent
  encore : les deux LEA initiaux chargent les bonnes adresses dans les bons
  registres, dans l'ordre inverse du R5. Cette fonction reste non acceptée.
- Log est reconstruit (193 octets). Le tampon local de 65 octets avec une limite
  strftime de 64 reproduit exactement le frame R5. Il ne dépend que du CRT.
- La structure de chat possède les offsets observés : timestamp, préfixe,
  stockage de texte, type et couleurs. La copie originale lit 100 entrées à
  partir de la deuxième ; ce comportement est conservé.
- PushBack a été identifié au RVA 0x67450 (24 octets dans R5). Notre helper inline
  a un corps émis de 26 octets et n'est pas accepté ; ses expansions dans
  AddEntry reproduisent tout sauf l'ordre des deux LEA du premier décalage.
- `prepare_actor_closure.py` extrait les définitions complètes nécessaires. Les
  globals zéro sont les définitions réelles reprises de main.cpp. Scripting,
  ActorPed et ActorPool utilisent leurs TU entières. World est compilé avec Ob2,
  les autres avec Ob1, et les chaînes sont poolées avec GF.

Tentatives conservées : `cp32-chat-entry1` (time.h manquant), 2–7 (frame Log,
variantes memcpy/memmove/affectation et helper inline), `cp32-chat-ob2`, puis
`cp32-closure-link-ob1`, `cp32-closure-link-ob2`, `cp32-closure-linked1/2`.
Le second link retire les exports explicites inutiles des destructeurs
supprimants ; leurs corps restent présents via les vtables.

**Prochaine action prioritaire :** vérifier le link avec des contrats revus.
Les constantes SCRIPT_COMMAND doivent être traitées comme objets complets de
18 octets lorsque le regroupement de la section amont diffère ; les globals
individuels comme données zéro typées. Ne pas accepter aveuglément les placements
inférés par discover_client_graph. Les sections code restent toujours entières,
y compris les tables accolées. Garder AddEntry et PushBack hors couverture tant
que les corps complets diffèrent, puis poursuivre ActorPool::New et les autres
sous-systèmes. L'ancienne couverture acceptée n'a pas encore augmenté.

Entrées actuelles : `config/checkpoint32/closure-linked-seeds.json`,
`evidence/checkpoint32/closure-progress.json` et les empreintes dans
`build/cp32-closure-linked2/link.json`.


## Contrôle du link terminé — lot accepté

`closure-contract.json` contient les identités revues. Le vérificateur rejoue les
fixups complets à la fois aux adresses R5 et aux adresses réellement liées, avec
contrôle exact des relocations PE, alias faibles, imports et fournisseurs CRT.
Les sections de code ne peuvent pas être déguisées en données pour être tronquées.
Les données SCRIPT_COMMAND sont des objets entiers de 18 octets et les globals
zéro des objets entiers de 2/4 octets, avec une disposition de section cohérente
au link. AddEntry et PushBack restent hors couverture.

Résultat : 84 régions acceptées, 50 corps complets, 3 432 octets de code dont
2 603 nouveaux. Cumul : 27 979 / 930 756 = 3,0061 % de `.text`.
Le test ciblé reproduit la création, trois requêtes de disponibilité du modèle,
Sleep, l'encodage des commandes de script et la suppression virtuelle via le
pool sur R5 et sur le DLL lié. Les seuls appels interceptés sont aux frontières
GTA/Windows/CRT. Les registres non volatils, la pile et la chaîne SEH restaurée
sont vérifiés ; il ne s'agit pas d'un test du déroulement d'une exception réelle.
Six mutations distinctes sont rejetées. Aucun rebuild n'était nécessaire.

Preuves : `closure-acceptance.json`, `coverage-current.json`. Reproduction :
`.venv/bin/python tools/accept_actor_closure.py`.
Prochaine action : reconstruire ActorPool::New (RVA 0x1900, 312 octets et EH),
puis étendre aux autres sous-systèmes. Revenir à l'ordre des LEA d'AddEntry sans
le compter entre-temps ; PushBack émis reste également nonmatching.
