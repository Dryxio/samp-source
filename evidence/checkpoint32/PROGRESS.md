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


## ActorPool::New et extension Entity — lot accepté

Build `cp32-expansion-linked1`, contrat `actor-expansion-contract.json` :
116 régions, 77 fonctions complètes, 5 517 octets de code. Gain unique :
**1 590 octets**, cumul **29 569 / 930 756 = 3,1769 %**. Le lot inclut les
régions du précédent contrat ; le calcul soustrait leur couverture et celle
des checkpoints historiques. L'ancien contrat et sa preuve restent conservés.

New (0x1900, 312 octets) utilise le payload compact de 27 octets, publie
l'acteur et ses flags puis appelle SetHealth, ToggleImmunity et UpdateCount.
Sa section EH complète (21 octets, cleanup + handler) et sa FuncInfo sont
comparées avec les mêmes règles que les autres sections, sans masquage.
Entity : matrices/vecteurs, distances caméra/point, limites du monde, angles
Euler, collision/gravité/streaming et ponts GTA. Les exceptions d'assembly
symbolique amont sont étiquetées dans le contrat ; aucune émission d'octets.
Les constantes flottantes et toutes les cibles de fixups passent le contrôle
intégral à la fois dans R5 et dans le DLL effectivement lié.

Le test ABI lève l'incertitude du payload compact et de la publication du pool :
le véritable New appelle le constructeur, encode les scripts et installe la
santé/immunité ; Delete traverse la suppression virtuelle. Allocation/libération
CRT et opérations GTA/Windows seulement sont interceptées. Les deux images
passent, ainsi que les six contrôles négatifs existants. Pas de test général
supplémentaire ni de double build. Le compilateur réutilise net/actorpool du
probe `cp32-pool-new1` ; les autres unités suivent leurs empreintes d'entrées.

Tentatives non adoptées conservées : `cp32-chat-os` (/Os aggrave AddEntry et
PushBack), `cp32-entity-expand` (TU Entity complète de découverte). Le probe
permet désormais les variantes /Os et CPU ; le vérificateur d'acceptation
continue d'exiger le profil /Ot original et ne les accepte pas implicitement.

Suite : reconstruire les régions Entity encore manquantes (mise à jour RenderWare
0x9EC80, historique de vitesse 0x9ED40, fonctions 0x9EF50 et suivantes) puis les
sous-systèmes voisins. Les distances au joueur local ont une forme de code
candidate exacte mais leur chaîne de dépendances est plus large et reste hors
couverture. AddEntry et PushBack diffèrent toujours. Le checkpoint 3.2 et la
DLL entière restent incomplets.


## Suite Entity — huit fonctions supplémentaires acceptées

`cp32-entity-tail-linked`, contrat `entity-tail-contract.json` : 124 régions,
85 fonctions complètes, 5 903 octets de code. Gain unique **386 octets**, cumul
**29 955 / 930 756 = 3,2184 %**. Les contrats/proofs précédents sont conservés.
Fonctions nouvelles : ApplyForce (48), PlayAudio (41), GetRwObject (12),
DeleteRwObject (33), ProcessControl (30), IsNativeTarget (21), AdvancePosition
(113), IsStationary (88). BYTE, et non bool, reproduit le retour AL de
IsNativeTarget. Les noms des nouvelles fonctions sont descriptifs ; les
identités sont ancrées dans les RVAs et les corps complets.

Les relocations d'AdvancePosition ciblent GetMatrix et SetMatrixAndUpdate.
Ce dernier est implémenté mais nonmatching : il ne compte pas dans la
couverture, tout comme son pont FUNC_1009EC80. Les 113 octets acceptés ne
prouvent donc pas une chaîne de mise à jour entièrement exacte. Les trois
références au zéro flottant d'IsStationary sont également vérifiées.

Le test ciblé ajoute les ABI force/audio aux deux images : six flottants dans
l'ordre natif pour 542A50, event + zéro pour 4F6420 sur l'objet natif +138.
Pile et registres non volatils restaurés ; seuls les appels moteur sont
interceptés. Les six contrôles négatifs existants passent. Pas de double build
ni de régression générale. Les ajouts au header ont renuméroté les labels EH
privés du compilateur ; leurs hashes et leurs cibles originales sont inchangés
et contrôlés avant la revue du nouveau contrat.

### Essais non adoptés à ne pas recommencer à l'identique

Les snapshots `cp32-motion-ob2`, `cp32-motion2-ob2` à `cp32-motion9-ob2`,
`cp32-motion-precise`, `cp32-bridges-ob2` sont conservés dans build/.
La routine 9ED40 attend 225 octets ; les variantes ont produit :
- VECTOR local + SetMoveSpeedVector : 275 ;
- accumulation dans trois paramètres scalaires : 150 ; avec boucle : 165 ;
- VECTOR local + écritures directes : 231 ;
- alias VECTOR sur &fX : 227, avec aliasage incorrect des paramètres (rejeté) ;
- VECTOR passé par valeur + initialisation champ par champ : 225 mais
  53 différences hors fixups ;
- affectation agrégée initiale : 233, puis variante 228.
La signature par valeur est une piste, pas une identité prouvée. La définition
amont et sa signature initiale sont restaurées ; aucune variante ne compte.

GetEulerAngles : littéraux float donnent 87 octets (multiplications fusionnées),
-1.0 double donne 105 mais trois opcodes FMUL qword au lieu de dword ; constante
extern séparée donne 111 ou 105 avec réordonnancement différent. Les pragmas p
on/off et /Op testés n'ont pas réglé l'écart dans ces essais. Aucun pragma ou
constante artificielle n'est conservé. Le probe expose /Op pour les essais,
mais le gate n'accepte toujours que les profils revus précédemment.

Les régions manquantes prioritaires restent 9EC80 (80/82), 9ED40 (146/225 dans
la source amont), GetEulerAngles (87/105) et SetMatrixAndUpdate (89/91). Les
fonctions autour de 9EF50 (modèle) dépendent du lookup A7A40/B45A0 et de sa
mémoire de modèles relocalisée ; les distances joueur dépendent de FindPlayerPed.
Élargir aussi les lots voisins plutôt que répéter longtemps les mêmes variantes.


## Utilitaires communs et CGame — lot accepté

`cp32-common2-linked`, contrat `common-contract.json` : 221 régions, 165
fonctions complètes, 9 403 octets de code. Le lot ajoute 80 corps aux 85 du lot
précédent, soit 3 500 octets ; après déduction des chevauchements historiques,
le gain est **3 069 octets uniques**. Cumul **33 024 / 930 756 = 3,5481 %**.

Pools natifs, tâches, angles/distances, sous-types véhicules, parseur Unicode,
conversion d'armes, temps/météo, streaming et marqueurs sont comparés avec leurs
cibles réelles. Le switch B3E20 comporte 371 octets de code dans une section
COFF de 556 octets : alignement et 46 entrées relocalisées de table sont
comparés intégralement, sans les compter comme code. Quatorze SCRIPT_COMMAND
supplémentaires passent sur leurs 18 octets entiers. VirtualProtect est bien
l'import kernel32.dll des deux images ; __stat, wcsncpy, wcstoul et strstr sont
fournis par le CRT et restent hors couverture source.

Le parseur ANSI GetColorFromEmbedCode reste différent (257 contre 271 octets)
et rejoint les cinq fournisseurs déjà exclus. Le corps appelant
RemoveColorEmbedsFromString peut être exact sans rendre cette dépendance exacte.
Le gate intègre cette distinction. L'exécution ABI existante et les six mutations
passent sur le nouveau link ; aucun test natif général supplémentaire ni double
build. Les objets inchangés sont réutilisés : la dernière compilation n'a refait
que closure_util et closure_models.

Extraction pilotée par `common-selection.json`, avec choix explicite des deux
surcharges pour IsHexChar, GetColorFromEmbedCode et SquaredDistanceBetweenPoints.
`prepare_actor_closure.py` régénère les unités complètes ; il reprend aussi le
typedef DrawZone_t original. L'échec initial sans ce typedef est conservé dans
`cp32-common-ob1`; `cp32-common2-ob1` a ajouté les unités contrôle/protection,
puis `cp32-common3-ob1` le switch armes et le traitement ~k~.

Le scan complet `cp32-game-util-scan` a produit 111 candidats à adresse unique,
5 198 octets avant vérification. Propositions et script de découverte conservés
sous `build/game-util-candidates.json`, `.txt` et `scan_game_util.py` ; ce scan
ignore les valeurs de fixups pour trouver des pistes et n'est jamais une preuve
d'acceptation. Il manque les tables accolées dans son filtre de taille : le
switch armes de 556 octets a donc été identifié séparément puis vérifié au gate.
Les prochains ensembles intéressants sont les registres de pointeurs joueurs,
les rasters/caméras et la gestion des modèles. Les données de ces ensembles
restent à contractualiser avant de les compter.

Observations modèle conservées pour la suite : A7A40 (51 octets) consulte le
flag 1A25AC et le pointeur 114B08, initialisé à 1825AC ; sinon il accepte les
indices 0..20000 inclus dans la table GTA A9B0C8. A7A00 efface 0xFFFF DWORDs à
partir de 1625B0 et recopie 20000 pointeurs GTA vers la table relocalisée. Cela
ne suffit pas encore à certifier la déclaration complète du stockage. B45A0
(40 octets) appelle A7A40 puis lit le champ +1C ; Entity 9EF50 en dépend.


## Registres joueurs et rasters — lot accepté

`cp32-records-rasters-linked`, contrat `records-rasters-contract.json` :
246 régions, 176 fonctions complètes, 10 221 octets de code. Gain unique
**818 octets**, cumul **33 842 / 930 756 = 3,6360 %**. Sept fonctions C++ de
registres joueurs (259 octets) et quatre helpers natifs de rasters (559 octets).

Les tableaux dwPlayerPedPtrs et VAR_1026C258 sont des objets complets de
840 et 3 360 octets, aux RVAs 26BF10 et 26C258. Bornes de 210 entrées,
strides 4/16 et tailles d'initialisation correspondent au R5. Les huit globals
de rasters sont des objets entiers de 4 octets, avec quatre chaînes entières.
Les objets sont définis dans des unités séparées pour conserver des sections
BSS complètes, sans découpage de données ni assouplissement du gate. Le
générateur contrôle la déclaration amont et émet une assertion sizeof.
Aucune donnée zéro ne compte comme code.

Le test ciblé exécute l'initialisation complète, l'entrée 209, le rejet gardé de
210 (y compris les 16 octets suivants), la publication d'un pointeur et les
recherches inverses par ped et RenderWare. Pile stdcall et registres restaurés
sur les deux images ; les six mutations passent. Pas d'I/O BMP ni rendu moteur
exécuté, pas de double build ou de test général supplémentaire.

Le probe réutilise dix objets et compile douze nouvelles unités ; l'objet
Entity Ob2 est réutilisé. Déclarations dans `common-selection.json`, identités
dans `records-rasters-seeds.json`. Les six fournisseurs nonmatching restent
inchangés et exclus. Preuve : `records-rasters-acceptance.json`.
Reproduction :

```sh
.venv/bin/python tools/accept_actor_closure.py --run cp32-records-rasters-linked \
  --contract config/checkpoint32/records-rasters-contract.json \
  --report records-rasters-acceptance.json \
  --previous evidence/checkpoint32/closure-acceptance.json \
  --previous evidence/checkpoint32/actor-expansion-acceptance.json \
  --previous evidence/checkpoint32/entity-tail-acceptance.json \
  --previous evidence/checkpoint32/common-acceptance.json
```

Suite disponible dans le scan : CGame::CGame (133), FindFirstFreePlayerPedSlot
(37), FUNC_100A00F0 (30), plus les constructeurs inline CAudio (13) et CCamera
(16). Leur tableau bUsedPlayerSlots[210], déclaré dans game.cpp, peut reprendre
la définition d'objet complet. Reprendre aussi A7A40/B45A0/Entity 9EF50 et les
régions manquantes.

## Game initialization and staged camera/audio work

Game-init acceptance adds 229 unique code bytes, bringing the certified union to 34,071 bytes. The complete packed CGame/audio/camera constructors and player-slot helpers passed the full gate, six mutation controls, and focused allocation/slot-boundary execution checks. Camera/audio extraction and its candidate probe are preserved but are not yet accepted.
