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

## SetModelIndex integration and camera/audio acceptance

`model-camera-acceptance.json` adds 1,624 unique code bytes across 23 functions: the 217-byte 0.2.5 SetModelIndex port and 1,407 bytes of camera/audio/bounds helpers. Certified union: 35,695 / 930,756 bytes (3.83505%). Full original/linked comparison, reviewed targets, PE relocations, six negative controls and existing ABI exercise passed. ModelInfoLoaded remains a 16-byte semantic provider versus 40 bytes in R5 and is excluded, as are its transitive getter and provisional extended table. No copied bytes and no deployment.

The 0.2.5 evaluation remains historical object-level evidence; the model-camera acceptance now certifies the real CEntity method in the linked capsule. Adding that member declaration required recompiling the units that consume the shared headers; later source-only changes retain the incremental cache.

## Vehicle state and 0.2.5 turret transfer

Vehicle acceptance adds 903 unique code bytes (24 new vehicle functions), for a union of 36,598 / 930,756 (3.93207%). SetTankRot/GetTankRotX/GetTankRotY transfer directly from 0.2.5 and contribute 49 bytes. Source 0.2.5 contribution accepted so far: 266 bytes including SetModelIndex. The already covered 8-byte Entity GetModelIndex is included as a linked dependency without being counted twice. Script command identities distinguish the otherwise identical HasSunk/IsWrecked wrappers. Full gate, six mutation controls and existing linked ABI exercise passed. Seven pending regions remain excluded.

## Player methods, shared skills and complete state storage

Player acceptance adds 4,647 unique code bytes (4,887 batch code bytes before removing previously certified overlap). Union: 41,245 / 930,756 (4.43134%). The linked superset includes 69 selected player methods plus eight key/aim/skill helpers. Three direct 0.2.5 transfers add SetAmmo36, IsOnGround28 and SetWeaponModelIndex62 bytes, bringing direct 0.2.5 accepted contributions to392 bytes. Seven older pending regions remain excluded.

The initial linked draft exposed a rotation constant precision mismatch and unrelated RakNet initialized globals emitted by broad main.h inclusion in storage-only units. Minimal source-storage headers preserve whole COFF sections without slicing; the R5 float PI precision resolves the rotation constant. Failed cp32-player-linked and both source snapshots are preserved. Complete remote-key/aim/skill arrays and small scalar/string storage are independently bound at original and actual linked addresses. Strict gate, six mutation controls and existing capsule ABI checks passed. No duplicate build or extra execution tests were added.

## Shared controls and native task closure

Shared acceptance adds 1,800 unique code bytes, bringing the union to 43,045 / 930,756 (4.62474%). It closes shared aim/zoom/aspect/camera-mode/key synchronization plus the CTask family, including six complete seven-slot vtables, their deleting destructors, and the Jetpack destructor unwind thunk, EH entry, unwind map and FuncInfo. Only changed extraction/storage units were recompiled; the existing Ob2 Entity object was reused. All full-byte, target, PE relocation, mutation and existing ABI checks passed. Actual Jetpack exception unwinding is not exercised. Seven prior pending regions remain excluded. No additional direct 0.2.5 byte contribution in this batch; accepted direct transfers remain392 bytes.

## Menus, font, texture, RenderWare and radar helpers

Render acceptance adds 3,282 unique code bytes across78 selected functions. Certified union:46,327 /930,756 (4.97735%). All menu script descriptors/strings and the complete792-byte label array match, as do both4000-byte radar arrays and the complete52-byte rotation-axis source section including its emitted RakNet IDs/padding. Data is not counted as code. The native GTA-version scalar binds to26EB28. Full gate, six mutation controls and existing ABI checks passed. No rendering/deployment or redundant build. The older0.2.5 menu algorithm is already present in the current base; direct0.2.5 transfer credit remains392 bytes.

## Menu and gang-zone pools

UI-pool acceptance adds1,062 unique code bytes across13 ordinary-C++ functions. Union:47,389 /930,756 (5.09145%). Existing menu/render dependencies are reused. Complete phase/tick static objects and menu lookup strings are verified; malloc and GetTickCount are explicitly identified. Full gate, six mutation controls and existing ABI checks passed. MenuPool New/Process remain outside this batch. Direct0.2.5 contribution remains392 bytes.

## Complete extended model storage and getter

Model-storage acceptance adds51 unique code bytes, for47,440 /930,756 (5.09693%). GetModelInfo and its complete65535-pointer backing allocation, interior pointer at element32767 and separate BOOL mode flag pass original/linked full-region and relocation checks. Four changed/new units were compiled; all others reused. Six mutation controls and existing ABI exercise passed. The262,148 bytes of data are not code coverage. ModelInfoLoaded remains16 versus40 bytes and is excluded; the initializer and its two patching callees are still unimplemented. Direct0.2.5 accepted contribution remains392 bytes.

Next: explore missing object/pickup pool and object implementations from0.2.5 with existing R5 layouts; do not repeat costly register-only helper trials. Current link cp32-model-storage-linked, Ob1 cp32-model-storage-ob1, Ob2 cp32-player-ob2; seeds/contract model-storage.

## Object and pickup pool transfers

Object/pickup acceptance adds377 unique code bytes across six ordinary-C++ functions, reaching47,817 /930,756 (5.13744%). Four implementations missing from our R5 base were transferred from0.2.5: object lookup64/52 and pickup destruction87/95. Their298 bytes bring direct0.2.5 accepted contributions to690 bytes. The object constructor34 and pickup destructor45 already existed in our base. Initial unadapted trials failed three of four functions; R5 inclusive last-object-ID, invalid65535, full4096-slot pickup layout and network-ID reset resolve them.

Actual class definitions preserve existing R5 sizes; packed dropped-weapon records replace the opaque gap. CObject remains incomplete and unimplemented, with only its known Entity base accessed. Complete destroy_pickup opcode/argument descriptor and ScriptCommand target verified. Full gate, six mutations and existing ABI exercise passed. Shared header edits required one recompilation of consumers; no duplicate build. Seven older pending providers remain excluded.

Next: scan configuration, HTTP/run utilities and remaining pools for larger complete method families. Current link cp32-object-pickup-linked; matching Ob1/Ob2 runs; seeds/contract object-pickup. Model initialization patching callees and all unimplemented object methods remain outside coverage.

## Configuration, string utilities and labels

Acceptance adds 3,007 unique code bytes across 30 functions: all 19 CConfig methods (2,014 bytes), eight run utilities (780 bytes), and three label-pool methods (213 bytes). Certified union: 50,824 / 930,756 (5.46051%). Source implementations already exist in the R5 base; direct 0.2.5 contribution remains 690 accepted bytes. The full Base64 alphabet/pointer, file-mode/format strings and zero-float constant are checked as complete source objects and excluded from code coverage. Existing K_EncodeString/K_DecodeString symbolic rotate assembly is explicitly labeled.

The draft initially refused the multi-chunk CRT functions free and strchr. Tooling now checks every inventory chunk, including strchr's earlier five-byte tail, without adding any CRT coverage. Actual providers must still come from pinned vendor libraries. A seventh negative control corrupts a secondary CRT chunk and fails as required. All full-byte/relocation checks and the existing ABI exercise pass. Candidate objects were reused without recompilation, followed by one integration link.

Next: HTTP candidates and dependencies. Scan cp32-config-http-scan / evidence config-http-scan.json preserves eight matching HTTP candidates (CloseConnection has two possible identities) and three unmatched bodies. Before integration, pin Winsock imports/library and resolve the unmatched Util_stristr dependency. Current accepted link cp32-config-utilities-linked; Ob1 cp32-config-utilities-ob1 and Ob2 cp32-object-pickup-ob2; seeds/contract config-utilities. Seven prior pending regions remain excluded. Across this continuation's three accepted batches, gain is 3,435 exact code bytes. No push or GTA deployment.

## HTTP foundation and inline string helpers

HTTP acceptance adds 1,316 unique bytes (eight HTTP methods, 940 bytes; three string helpers, 376 bytes), reaching 52,140 / 930,756 (5.60190%). Marking the existing Util_toupper definition inline closes Util_stristr, Util_strupr and Util_strnicmp without changing their algorithms or any prior accepted body. Direct 0.2.5 transfer credit remains 690 bytes: these implementations already existed in the R5 base.

Five vendor six-byte Winsock import relays are now explicitly verified at original and linked addresses, including complete FF25 instructions, PE relocations, DLL/ordinal and pinned wsock32.lib/MAP provider. They are excluded from source coverage. All complete byte comparisons, nine mutation controls and existing ABI checks pass. One changed source unit compiled; other units reused. No network calls. Process is a new explicitly excluded dependency (448 source versus 473 original bytes); Connect/HandleEntity remain unselected.

Current accepted link cp32-http-linked; Ob1 cp32-http-ob1, Ob2 cp32-object-pickup-ob2; seeds/contract http. Next experiment cp32-http-inline-scan tests inline CloseConnection/Send/Recv: R5 Process visibly embeds Send while our initial source called it. Preserve the accepted source until those full-function results are known.

## Complete HTTP method family

HTTP-complete acceptance adds 1,453 exact bytes: HandleEntity 788, Connect 192 and Process 473. Union: 53,593 / 930,756 (5.75801%). All 11 HTTP source methods now match, together with their complete strings and source utility dependencies. Source changes are limited to inline annotations on CloseConnection, Send and Recv. The original user-agent string SAMP/0.2.0, parser behavior, cleanup behavior and 500000-byte content-length limit are preserved.

Four further Winsock relays bind gethostbyname/htons/socket/connect to their original DLL/ordinal identities and pinned import library. realloc's complete two-chunk boundary is verified. Vendor code remains outside coverage. Full comparisons, all targets, nine negative controls and existing ABI exercise pass; no live networking. The HTTP Process pending entry is removed; seven older pending regions remain. Direct 0.2.5 transfer contribution remains 690 bytes because these algorithms were already in our R5 base.

Current accepted link cp32-http-complete-linked, Ob1 cp32-http-complete-ob1, Ob2 cp32-object-pickup-ob2, seeds/contract http-complete. This continuation adds 2,769 unique code bytes across the two HTTP/string batches. Next: scan font/render overlays and statistics families for additional complete methods and transferable 0.2.5 implementations.

Overlay scan cp32-overlay-scan is complete; all eight source units compiled. Evidence overlay-scan.json lists 51 emitted functions, 50 with at least one candidate; ambiguous boilerplate is not coverage. Priority candidates: font methods 2,220 bytes and player tags 2,426 bytes before full dependency/string/data review. Spawn-screen candidates add 1,737 bytes including helpers. D3DX import-library identities and complete font-hook/metrics/vertex storage are the next integration work. All scan jobs are terminal; current worktree contains no live build process.

## Font renderer, player tags and font settings

Font/tag acceptance adds 5,018 unique code bytes, reaching 58,611 / 930,756 (6.29714%). All 10 font-renderer methods and the selected player-tag methods/helpers pass full comparison; the already covered 59-byte D3DXMatrixIdentity is not counted twice. Three configuration-based font helpers and 17 font-hook forwarding methods also match. The complete 448-byte metrics/vertex section, all constants/strings, the 18-slot font-hook vtable and pConfig storage are verified and excluded from code coverage.

The original 229-byte font-hook DrawTextA is still a 16-byte TODO forwarding provider and remains excluded, along with its unimplemented internals. Eight pending regions now exist in the linked contract; the font-hook subsystem is not complete. The 0.2.5 reference lacks this newer hook; direct transfer contribution remains 690 bytes. Three D3DX relays bind exports in d3dx9_25.dll through the import library pinned at CP3.1.

The first link failed because the full font object required its settings and hook providers; the failed artifact is retained. Adding original source providers resolved linkage. Full gate, nine mutation checks and existing ABI exercise pass; no live rendering. Current link cp32-font-tags-providers-linked, Ob1 cp32-font-tags-providers-ob1, Ob2 cp32-object-pickup-ob2; seeds/contract font-tags. Next: spawn-screen methods and the two already compiled extra font-setting helpers.

## Spawn screen and remaining font-size helpers

Spawn-screen acceptance adds 1,763 unique bytes, bringing coverage to 60,374 / 930,756 (6.48656%). All six spawn-screen methods and their complete vertex/array helpers match; existing coverage overlaps are removed. Death-window/UI font-size getters also match. The complete version string, pDefaultFont and pFileSystem pointer objects, and named D3DX texture-from-memory relay are verified. Virtual archive methods are only call boundaries and are not counted as implementations. Eight previous pending regions remain excluded.

Only the two new global-storage units compiled; other objects reused. Full gate, nine mutation controls and existing ABI exercise pass. Current link cp32-spawn-screen-linked; Ob1 cp32-spawn-screen-ob1, Ob2 cp32-object-pickup-ob2; seeds/contract spawn-screen. This continuation has accepted 6,781 new code bytes so far. Next isolated experiment transfers three missing command recall methods from0.2.5 using the existing R5 offsets and GetTextA API.

Command recall scan cp32-cmd-recall-scan completed: three previously absent0.2.5 methods compile to exact full-function candidates, AddToRecallBuffer90 at695D0, RecallUp100 at69630, RecallDown78 at696A0. These268 bytes are not accepted coverage yet. The trial preserves the verified R5 layout and uses GetTextA instead of the older GetText API. Actual class integration and complete DXUT GetTextA/SetText dependencies remain to do. The scan also preserves existing command-window and death-window candidates; no failed bodies are counted. Evidence command-recall-transfer-scan.json, source eval_cmd_recall.cpp. All build jobs are terminal.

## Command recalls and DXUT editing dependencies

Command-recall acceptance adds 2,132 unique code bytes, reaching 62,506 / 930,756 (6.71562%). Three previously absent0.2.5 recall functions contribute268 bytes, bringing direct accepted reference transfers to958 bytes. Actual CCmdWindow retains its0x1AFC layout; five existing command registration/layout methods also match.

The shared DXUT editing closure now includes text access, conversion, caret positioning, allocation, string copying, analysis and the R5 password mask helper. Complete original/linked code,2048/512-byte static buffers and the seven-pointer initialization section are checked. Seven actual SDK fallback bodies match; dynamic Uniscribe initialization/loading and DLL bodies are not counted. Initial Analyse181 versus298 and ANSI SetText stack-size mismatches were resolved by R5-specific source adaptations. The87-byte mask helper's initial loop form failed; the final ordinary C++ loop matches completely. All experiments are preserved.

Full gate, nine mutation checks and existing ABI exercise passed. Shared header changes required one rebuild of consumers; no live UI or redundant runtime tests. Eight earlier pending regions remain excluded. Current link cp32-command-recall-linked; Ob1 cp32-command-recall-ob1 and Ob2 cp32-command-recall-ob2; seeds/contract command-recall. Next: death-window family; SpriteIDForWeapon has a complete750-byte source COMDAT including jump tables, while its original code range is374 bytes, so the discovery tool's function-size filter did not report it. Review the full COMDAT through the strict integration gate.

## Death window and complete weapon-symbol tables

Death-window acceptance adds 2,450 unique code bytes, reaching 64,956 / 930,756 (6.97884%). The whole existing method family selected by the scan matches, including the374-byte SpriteIDForWeapon body and its complete compiler-generated tables. The original750-byte source COMDAT failed; adding the explicit drowning53 case expands it to754 bytes with44 jump targets and202 selectors, matching R5 exactly. Source0.2.5 corroborates the missing case. This correction does not receive an entire-function direct-transfer credit; accepted direct0.2.5 transfers remain958 bytes.

All complete strings, font names, constants, local table relocations and existing dependency identities pass the strict gate. Nine mutation controls and existing ABI exercise passed. Only changed deathwindow.cpp recompiled after the failed candidate; unchanged units reused. Eight prior pending regions remain excluded. Current link cp32-death-window-refined-linked; Ob1 cp32-death-window-refined-ob1, Ob2 cp32-command-recall-ob2; seeds/contract death-window. Across this continuation: +4,582 exact unique code bytes.

Next: continue command processing from0.2.5 now that recall and DXUT editing dependencies are accepted. Current ProcessInput remains a short TODO provider; transfer its full body with R5 APIs and inspect networking/Disable dependencies without counting missing providers. Current Disable source has a full72-byte candidate and calls the implemented CGame::ToggleKeyInputsDisabled(int,BOOL), which is available in game.cpp. ResetDialogControls and Enable still need separate full-range review. The font-hook DrawTextA229-byte original remains a16-byte TODO provider. No jobs remain running; no push or GTA deployment.


## Command processing transfer and dependency candidates

Three0.2.5 command functions now compile to complete discovery candidates: MsgProc57 at697D0, SendToServer200 at69900, ProcessInput365 at699D0. These622 bytes are NOT accepted coverage. Source adaptations preserve R5 layout/GetTextA, use the six-argument string-ID RPC, static IME handling and chat ResetPage on closure. ProcessInput requires a normal local strlen result; volatile variants377 bytes failed. The actual CCmdWindow still has its earlier short ProcessInput provider; integration remains next.

Discovery now understands FS:__except_list absolute-zero operands instead of mistaking them for image-base relocations. Valid FS operands and all full bytes remain checked; wrong segment and nonzero addend mutations are rejected. Strict gate is unchanged. SendToServer's EH/data and every target still need linked review.

BitStream dependency scan yields candidates for all38 emitted functions, including constructor33, destructor27, WriteBits163 and Write113 called by SendToServer. This is the pinned existing RakNet source, and overlaps with CP3.1 must not be counted again. Complete scrollbar Cap67 at85400 and UpdateThumbRect143 at85320 also have candidates. Chat ResetPage38 at67390 and SetTrackPos29 at670E0 match when the chat method is compiled separately underOb2; same-unitOb1/Ob2 failed due to different inlining. No proprietary bytes were added to source.

Evidence command-processing-transfer-scan.json preserves all successful and useful failed experiments. Current eval_cmd_process.cpp matches the final cp32-cmd-process-length-scan snapshot. eval_cmd_dependencies.cpp is the split chat-only source, final cp32-cmd-scroll-split-ob2-scan. closure_gui_scroll.cpp contains the two complete previously compiled DXUT helper definitions but needs its own unit compile for integration. closure_cmd_bitstream.cpp compiled in cp32-cmd-dependencies-refined-scan. All processes terminal. Accepted coverage remains64956 /930756, direct accepted0.2.5 transfers958; no new bytes accepted in this candidate-recovery turn. Next: actual command integration, complete BitStream/EH identities, chat scroll closure and Disable/key-input provider; defer larger IME closure if needed.


## Command processing and complete BitStream dependencies

Strict command-processing acceptance adds2,720 unique code bytes, reaching67,676 /930,756 (7.27108%). Actual CCmdWindow ProcessInput365 and SendToServer200 now match, along with Disable72, chat ResetPage38 and scrollbar helpers67/143. Complete BitStream method bodies identified by pinned decorated names pass, with CP3.1 overlaps subtracted. Complete21-byte EH cleanup/handler and36-byte unwind metadata pass without masking, including FS absolute-zero operands.

Direct accepted0.2.5 transfer contribution increases565 bytes to1,523. MsgProc57 remains an isolated candidate; its larger IME dependency is not integrated or counted. No RakNet source is misattributed to0.2.5. CGame::ToggleKeyInputsDisabled is an actual344-byte source provider for a407-byte original and remains excluded, as do its unselected mouse-patching dependencies. Nine pending providers now exist in the link; these are not the whole-project remaining count.

All910 full regions, targets, relocations, nine mutation controls and existing ABI exercise pass. Header consumers rebuilt once after actual class method declarations; only normal static matching ran, no UI/network/GTA execution. Current accepted link cp32-command-processing-linked; Ob1 cp32-command-processing-ob1, Ob2 cp32-command-processing-ob2 (closure_world plus closure_chat_scroll). Seeds/contract command-processing; source units build/command-processing-units.txt. Previous coverage64,956, new67,676.

Next: inspect key-input407-byte original versus344-byte existing implementation; mouse helpers already have full71/75-byte candidates but are not counted yet. Keep MsgProc IME closure as a later batch if key-input refinement stalls; other scoreboard/UI candidates remain available. No build process remains live.


## Close key-input disabling and mouse helpers

Key-input acceptance adds553 exact unique bytes: ToggleKeyInputsDisabled407, DisableMouseProcess71 and DisableMousePositionUpdate75. Coverage68,229 /930,756 (7.33049%). Existing branch logic was already correct; R5 embeds the complete DIResetMouse/UpdatePads helpers and their symbolic native-call assembly. The initial344-byte function used out-of-line calls.

An initial inline link failed with duplicate standalone helper symbols. Extraction now uses private static inline KeyInput-prefixed copies of the complete original helper definitions, preserving the existing closure_controls providers and native call boundaries. Explicit native-assembly exception is labeled in the contract. Full code, pD3DDevice26EB40 and dummy mouse state150330 scalar objects and all targets match. No memory-patching code was executed. Source contains no copied original function bytes.

All915 regions, nine negative controls and existing ABI checks pass. Only the changed key-input unit recompiled after the failed link; Ob2 objects reused. Eight earlier pending providers remain. Current accepted link cp32-key-input-local-linked; Ob1 cp32-key-input-local-ob1, Ob2 cp32-command-processing-ob2; contract key-input, seeds command-processing. Proof key-input-acceptance.json, preserved attempts key-input-experiments.json. Direct accepted0.2.5 contribution remains1,523 bytes. This goal turn gained3,273 bytes across two coherent accepted batches.

Next: CCmdWindow Enable246 is still partial; current source has GetClientRect and control enabled/visible setup but lacks the rest.0.2.5 Enable is much simpler, so recover R5-specific additions from original69480..69576. ResetDialogControls185 already has a candidate; its full dialog provider closure may be larger. MsgProc57 candidate needs IME StaticMsgProc659. Other scoreboard/UI scans remain available. All build processes terminal, no push or GTA deployment.


## Command opening and dialog focus

Command-enable acceptance adds303 unique bytes, reaching68,532 /930,756 (7.36305%). The selected methods total338 bytes: Enable246, RequestFocus57 and GetFont35, but GetFont was already covered and is not counted again. Enable's R5 sizing/placement/focus logic matched on the first compiled recovery. The chat offset12E four-byte gap is now typed as input-line y, with an inline getter; subsequent layout remains unchanged.

Full0.6/-1.5 float constants, static focus pointer142780, original native HWND boundaryC97C1C and named user32 GetClientRect import are verified. user32.lib now appears in the link and its hash matches the CP3.1 SDK pin. All921 full regions, nine negative controls and existing ABI checks pass. No live UI or game execution. Direct accepted0.2.5 transfer credit remains1,523 because its older Enable lacks this R5 logic. Eight earlier pending providers remain excluded.

Current accepted link cp32-command-enable-linked; Ob1 cp32-command-enable-ob1, Ob2 cp32-command-enable-ob2. Contract/seeds command-enable; full units build/command-enable-units.txt. Proof command-enable-acceptance.json; first candidate and deduplication command-enable-experiment.json. Next isolated scan cp32-ime-static-scan extracts the full pinned StaticMsgProc handler; evaluate complete code/tables before deciding whether its larger IME state/dependency closure is worthwhile.


IME follow-up scans completed. StaticMsgProc originally produced728 bytes underOb1 (727 underOb2), versus659 R5. The pinned source's Chinese fallback used tolower while the common fallback used towlower; R5 shares the wide-character path. Changing only that call to towlower yields a complete659-byte candidate at8C840.

Its providers now have full candidates: CheckToggleState217 at85DE0 after using the native game HWND accessor instead of DXUTGetHWND; SetupImeApi129 at85ED0 and ResetCompositionString42 at85F60 unchanged. Helpers and complete array templates also have candidates; ambiguous boilerplate is not counted. Evidence ime-candidates.json preserves the initial failed variants, successful candidates and inferred targets. Actual source eval_gui_ime.cpp corresponds to cp32-ime-static-wide-scan; eval_gui_ime_dependencies.cpp to cp32-ime-providers-hwnd-scan.

No IME bytes are accepted yet. Next: CheckInputLocale, complete IME state/callback initialization/indicator strings and array storage; then integrate the earlier MsgProc57 candidate and these providers through the strict gate. Dynamic imports/callback defaults need actual source identities, no invented placeholders. The accepted command-enable link remains current at68,532 bytes; all scans terminal, repository work preserved locally.


## IME dispatch, locale/version providers and Uniscribe lifetime

IME acceptance adds2,715 unique exact code bytes, reaching71,247 /930,756 (7.65475%). Actual CCmdWindow MsgProc57 and StaticMsgProc659 are integrated with CheckInputLocale247, CheckToggleState217, SetupImeApi129, ResetCompositionString42, GetImeId606 and full array helpers. CUniBuffer constructor53, destructor33 and Initialize333 also match. Existing StringCopyWorker regions are deduplicated.

Locale selection needed the same towlower correction as the static handler; GetImeId needed its original LCID_INVARIANT macro in extraction. Crucially, the initial ResetCompositionString42 candidate had matching instruction shape but called the wrong ANSI SetText overload. Draft closure rejected the inconsistent target. Correct source uses SetText(L""), binding the complete wide provider9BF20 and full Unicode empty string. Failed link/source snapshots remain preserved.

Complete136-byte initialized callback/indicator section passes, including25 real SDK fallback bodies,25 callback pointers, five-by-three WCHAR indicators, alignment and current-indicator pointer. These are original initial SDK callbacks, not invented substitutes for dynamic DLL APIs. Complete static1/4-byte flags/pointers,12-byte locale array,19-byte composition buffer,256-byte attributes and4/8-byte local caches are verified. Dynamic API DLL bodies and unselected module-wide startup/IME initialization are outside coverage. Full Uniscribe path and all7 exported procedure strings/pointers match. No live DLL/UI/GTA execution.

All995 regions, every target/relocation, nine negative controls and existing ABI exercise pass. Current link cp32-ime-wide-reset-linked; Ob1 cp32-ime-wide-reset-ob1, Ob2 cp32-ime-ob2. Seeds/contract ime; full units build/ime-units.txt. Source generator tools/prepare_gui_ime.py reproduces complete definitions and statics. Proof ime-acceptance.json; failed attempts ime-integration-experiments.json. Direct accepted0.2.5 transfer total stays1,523; no whole-wrapper credit for a R5-dispatch rewrite. Eight earlier linked providers remain pending.

Next: scan the whole pinned DXUT GUI source with include paths adapted to current R5 headers, seeking larger UI/control families for efficient integration. Existing accepted closures stay authoritative; whole-source scan is discovery only and must subtract overlaps. ResetDialogControls and remaining dialog/control initialization need those providers. All current build processes terminal; no push or game deployment.


Full DXUT scan cp32-gui-full-scan completed:434 functions emitted,380 have at least one discovery candidate. Ranked evidence gui-full-scan.json contains143 single-candidate functions with unaccepted bytes, potential union23963 bytes after all prior coverage, zero newly accepted. Raw discovery and source snapshot remain in build. eval_gui_full.cpp copies the complete pinned GUI source with only include paths adapted; it deliberately does not replace accepted adapted closures.

Highest-yield next families: InitDefaultElements2745 at8D780; RenderComposition1594 at90020; ComboBox Render1036 at8E850; ANSI/wide DrawTextA878 each at8D060/8D3D0; IME Initialize774 at8C240 and Uninitialize329 at86290; DrawSprite691 at873A0; DrawRect505 at84A70; ScrollBar Render507 at88A50; UpdateRects variants and shared font/control helpers. Review call identities, full vtables/static objects and transitive source dependencies. Do not count candidate sums as accepted or link the full object over existing definitions. All processes terminal; next step can start a larger coherent UI family directly from this ranked list.


## Complete dialog drawing family

GUI-drawing acceptance adds4,160 unique exact bytes, reaching75,407 /930,756 (8.10169%). Nine methods total4,195 bytes: ANSI/wide DrawText878 each, CalcTextRect378 each, DrawSprite691, DrawRect505, DrawPolyLine293, CreateFont159 and GetTexture35; already-covered GetTexture35 is subtracted. All1,011 regions and their complete constants/FVF/debug string/font-hook vtable pass. The earlier unmatched font-hook DrawTextA remains excluded, with all eight earlier pending providers unchanged.

The initial linked candidate exposed another target-only issue: without <new>, MSVC emitted scalar allocation/deallocation symbols for source array expressions, at original array-new/array-delete destinations. The draft correctly rejected this. Adding the actual standard declarations via <new> restores correct operator identities while leaving source operations unchanged. Source generation also needed CRLF normalization before lexical extraction; failed preparation/link snapshots retained.

OffsetRect user32 import and complete D3DXMatrixScaling FF25 relay through pinned d3dx9_25.dll are verified; vendor relay code excluded. Full comparison, nine negative controls and existing ABI exercise passed. Only changed closure_gui_drawing.cpp compiled; all prior Ob1/Ob2 units reused. Current link cp32-gui-drawing-new-linked; Ob1 cp32-gui-drawing-new-ob1, Ob2 cp32-ime-ob2; seeds/contract gui-drawing; units build/gui-drawing-units.txt. Generator tools/prepare_gui_drawing.py. Direct0.2.5 accepted transfer credit remains1,523 bytes.

Next high-yield family: InitDefaultElements2745 at8D780, UpdateFont60 at8D740, SetDefaultElement148 at8A230, SetFont110 at8CFD0, SetTexture100 at89E10, AddTexture185 at89D50, CreateTexture134 at87A30, AddFont233 and element/color initialization. All have full-source candidates from cp32-gui-full-scan. GetUIFontSize/GetFontWeight/GetFontFace providers already exist in closure_font_settings. CreateTexture calls DXUTFindDXSDKMediaFileCch at10097830 and D3DXCreateTextureFromFileExA; the search helper source is vendor/upstream/saco/d3d9/common/DXUTmisc.cpp, needs full provider review. Keep drawing unit definitions separate to avoid duplicate providers. All processes terminal; no push or GTA deployment.


## Default styles, resources and complete media search

GUI-defaults acceptance adds6,792 unique exact bytes, reaching82,199 /930,756 (8.83142%). InitDefaultElements2745, Refresh245, element font/texture setters, color initialization/blending, font/texture/default-element registration and typed array helpers all match. The complete texture-loading dependency includes DXUTFindDXSDKMediaFileCch567, typical directories346, parent directories306, path accessors/setter and string helpers. No new missing provider was introduced.

All1,060 full regions, source constants/path/asset literals and the complete260-byte media-search buffer pass, along with every original/linked target and PE relocation, nine negative controls and existing ABI exercise. Complete Windows import DLL/name identities and CRT providers were reviewed. Unchanged source from the pinned SDK supplies this batch; only extraction headers were added. No filesystem searching or graphics execution occurred during validation. Direct accepted0.2.5 contribution stays1,523 bytes; eight earlier linked providers remain excluded.

Only closure_gui_defaults.cpp and closure_gui_media.cpp compiled. Existing Ob1/Ob2 objects reused; one additional link exposes already compiled Refresh and media path functions. The ambiguous five-byte DXUTGetMediaSearchPath getter was selected by its unique actual target to verified DXUTMediaSearchPath93810, not by arbitrary trampoline shape. Current link cp32-gui-defaults-complete-linked; Ob1 cp32-gui-defaults-ob1, Ob2 cp32-ime-ob2. Seeds/contract gui-defaults, full units build/gui-defaults-units.txt, source generator tools/prepare_gui_defaults.py, proof gui-defaults-acceptance.json.

Next: coherent control rendering/update family from gui-full-scan. ComboBox Render1036, ScrollBar Render507, Button Render325, CheckBox Render240, Slider Render219 and Static Render75 have candidates. Most dependencies are now accepted draw/color/font/scroll helpers. ScrollBar requires DXUTGetTime at1007D9D0 plus Scroll; inspect the original clock provider before dragging the whole DXUT framework. ComboBox rendering uses one static Boolean and previously accepted Cap/UpdateThumbRect. Include matching UpdateRects methods where useful. All processes terminal; repository work preserved locally, no push or GTA deployment.


## Control rendering and rectangle updates

GUI-controls acceptance adds3,218 exact unique bytes, reaching85,417 /930,756 (9.17716%). Twelve new complete methods: ComboBox Render1036/UpdateRects526, Button Render325, EditBox UpdateRects304, CheckBox Render240/UpdateRects131, Slider Render219/UpdateRects149, ScrollBar UpdateRects116/ShowItem64, Static Render75 and Control UpdateRects33. Existing Cap67/UpdateThumbRect143 moved to the same source unit and verified unchanged; their210 bytes are not counted again.

Initial extraction omitted original inline RectWidth/RectHeight helpers; restored their exact complete source lines. With scrollbar helpers in a separate unit, ComboBox produced1052/528 bytes versus1036/526 R5 and was rejected. Including complete original Cap/UpdateThumbRect bodies in closure_gui_controls restores MSVC interprocedural register knowledge and exact output without behavior edits. The separate closure_gui_scroll unit is excluded from this link; its source and earlier proofs remain intact. No shared headers changed and only the changed control unit compiled.

All1,078 regions, complete one-byte bSBInit143C04, five float constants and all relocation/target identities pass. InflateRect is verified as user32; prior draw/color/font/scroll/CRT providers unchanged. All9 negative controls and existing ABI exercise pass. First acceptance used an invalid nested report path after successful checks; corrected basename produced authoritative proof. No live graphics or GTA execution. Direct accepted0.2.5 contribution remains1,523 bytes; pinned DXUT supplies this batch.

Current accepted link cp32-gui-controls-scroll-linked; Ob1 cp32-gui-controls-scroll-ob1, Ob2 cp32-ime-ob2. Contract/seeds gui-controls; units build/gui-controls-units.txt. Generator tools/prepare_gui_controls.py. Proof gui-controls-acceptance.json, trial details gui-controls-integration.json, local review build/review_gui_controls.py. Eight earlier pending providers remain excluded.

Next candidates: IME Initialize774/Uninitialize329, RenderComposition1594 and RenderIndicator332 from gui-full-scan. ScrollBar Render507 depends on DXUTGetTime12 at7D9D0: original calls GetDXUTState7D140 and tailcalls DXUTState::GetTime79220. Pinned DXUT.cpp uses a complete static DXUTState with lifetime initialization; inspect its closure before undertaking framework extraction. Do not invent a clock or count its unimplemented dependencies. All processes terminal, no push/deployment.


## IME lifetime and composition rendering

GUI-IME-render acceptance adds3,300 unique exact bytes, reaching88,717 /930,756 (9.53171%). Complete RenderComposition1594, Initialize774, RenderIndicator332, Uninitialize329 and TruncateCompString271 all match from pinned source with extraction headers only. Full POINT8 at14180C, first-target index4 at14181C, IMM32 handle4 at142774, version handle4 at141814 and wait tick4 at141808 match. Full imm32/version path strings,23 dynamic API-name literals and1/255 float verified.

Previously accepted callback/indicator136-byte object supplies all25 function-pointer identities. Original fallback bodies re-emitted in this unit and StringCopyWorker77 match prior regions and are deduplicated. No dynamic DLL implementation is counted. All1140 regions, every target/relocation,9 negative controls and existing ABI pass. Eight earlier pending providers remain excluded. No DLL loading, SendMessage execution, graphics or GTA deployment.

Compared these5 exact source definitions with local0.2.5. Uninitialize is identical; Initialize lacks R5 GetTickCount wait initialization; RenderComposition/Indicator use TCHAR instead of explicit WCHAR; TruncateCompString uses narrow strlen and DXUTGetHWND instead of R5 lstrlenW/native HWND. Our base already has these needed adaptations. No incremental direct0.2.5 transfer credit; direct accepted total remains1,523 bytes. Source comparisons and provenance stored in gui-ime-render-integration.json.

Initial minimal-header extraction failed on pGame declarations, corrected with main.h/extern without any shared header changes. Only new source units compiled; all prior objects reused. Current accepted link cp32-gui-ime-render-linked; Ob1 cp32-gui-ime-render-game-ob1, Ob2 cp32-ime-ob2. Contract/seeds gui-ime-render, units build/gui-ime-render-units.txt, generator tools/prepare_gui_ime_render.py. Proof gui-ime-render-acceptance.json. Local review build/review_gui_ime_render.py. All processes terminal.

Next: remaining IME GetReadingWindowOrientation349 (requires original advapi32 registry imports), SendCompString79, SendKey33, StaticOnCreateDevice41, then full ctor/vtable closure for EditBox256 and IMEEditBox256/AddIMEEditBox222/command ResetDialogControls185. Full scan has no accepted candidates yet for larger HandleMouse/MsgProc/reading-window renderer; assess R5 wide-character/native-HWND adaptations from existing base and full original targets. ScrollBar clock closure remains available as recorded above. No push or deployment.


## Complete timer foundation and standalone IME helpers

GUI-timer acceptance adds822 unique exact bytes, reaching89,539 /930,756 (9.62003%). Complete timer constructor77, Reset77, Start100, Stop70, Advance43, GetAbsoluteTime81, GetTime81, GetElapsedTime99, IsStopped4 and DXUTGetGlobalTimer37:669 total. IME SendKey33/StaticOnCreateDevice41/SendCompString79 add153. CUniBuffer::operator[]16 is now an actual linked source provider, but CP3.1 already covered it so it is not counted again.

All1158 complete regions,34-byte packed timer143D3C, compiler guard4 at143D60, default input-context4 at141818 and -1 double pass. Complete CRT __alldiv170 atC8800 and kernel32 performance counter/frequency/user32 keybd_event identities verified. Full9 negative controls and existing ABI pass. No actual timer/input/game execution. Eight prior pending providers unchanged.

Initial link failed for missing CUniBuffer::operator[]. An unreviewed draft classified the missing custom symbol as CRT; it was never accepted. Added the complete original C++ body and recompiled only closure_gui_ime_helpers; actual wide-character write access and analysis-required flag match. Complete10 timer methods and SendKey are identical to0.2.5; remaining helpers require existing base R5 wide/native-HWND adaptations. No new direct0.2.5 credit, total remains1,523 bytes.

Current accepted link cp32-gui-timer-buffer-linked; Ob1 cp32-gui-timer-buffer-ob1, Ob2 cp32-ime-ob2. Contract/seeds gui-timer, units build/gui-timer-units.txt, generator tools/prepare_gui_timer.py. Proof gui-timer-acceptance.json; failed trial and source comparisons gui-timer-integration.json; local review build/review_gui_timer.py. All build processes terminal; no push/deployment.


Full framework scan cp32-dxut-framework-scan completed:345 emitted functions,338 with any candidate,261 single-candidate functions with new bytes and30,112 potential unique bytes after current89,539-byte coverage. Zero new acceptance from this scan. Source eval_dxut_framework.cpp is complete pinned DXUT.cpp with only dxstdafx include path adapted. Ranked evidence dxut-framework-scan.json, raw candidates in build run.

Largest candidates: ParseCommandLine2260, BuildValidDeviceSettings1545, SetDeviceCursor1016, Render3DEnvironment924, CreateWindow792, UpdateDeviceStats781, CreateDevice730, DoesDeviceComboMatchPreserveOptions664, MainLoop580. Prioritize coherent provider closures, not raw ranking alone. Independent device compatibility/format helpers look inexpensive; SetDeviceCursor needs pinned gdi32 imports and correct new[] declarations (<new> when extracting). BuildValid/Optimal device settings need GetDXUTState providers.

GetDXUTState92 has2 candidates; original known target7D140 from DXUTGetTime12 at7D9D0 can disambiguate. DXUTState GetTime66 at79220 has a unique candidate; full class uses g_bThreadSafe/g_cs and Enter/LeaveCriticalSection. Constructor12 has30 candidates and destructor22 has3; select using complete calls and lifetime targets. Source Create90 calls DXUTGetGlobalResourceCache, initializes full STATE and critical section; Destroy22 calls DXUTShutdown. Do not replace the complete static object/lifetime with a simplified timer or count unimplemented framework shutdown/cache providers. All scan/build handles terminal; no push or deployment.


## Device compatibility and complete format tables

DXUT-device acceptance adds1,013 unique exact bytes, reaching90,552 /930,756 (9.72887%). Complete compatibility predicate664 and typed search helpers Contains49/IndexOf74/IndexOf74 match. Full Color128/Alpha120/Depth90/Stencil82 COMDATs match, including every table entry and PE relocation; actual code spans58/52/53/47, with Color58 already covered in CP3.1 and not counted again.

Initial single-chunk code-size discovery skipped the4 format functions because their COFF sections include attached tables. Full section comparison at original function starts gives unique candidates, verified again by strict linked gate; no truncation or masks. Complete source internal labels bind every table destination. All1166 regions,9 negative controls and existing ABI pass. No new import or pending provider. Eight previous pending functions remain excluded. No graphics/game execution.

Only2 new source units compiled, all previous objects reused. Current accepted link cp32-dxut-device-linked; Ob1 cp32-dxut-device-ob1, Ob2 cp32-ime-ob2. Contract/seeds dxut-device, units build/dxut-device-units.txt, generator tools/prepare_dxut_device.py. Proof dxut-device-acceptance.json; table candidates and0.2.5 source comparisons dxut-device-integration.json. Local review build/review_dxut_device.py. Direct0.2.5 accepted transfer credit stays1,523; current base supplies these definitions. All processes terminal.


## Enumeration lifecycle and full capability-list closure

DXUT-enumeration acceptance adds4,443 unique exact bytes, reaching94,995 /930,756 (10.20622%). This goal turn gains5,456 across two coherent batches. Complete19 source methods cover enumeration constructor/destructor, adapter/device destruction, ClearAdapterInfoList, EnumerateDevices228/DeviceCombos575,4 capability builders,3 list resetters,3 finders and vertex-processing get/set. All actual typed array alloc/free providers included; previously covered helper bytes deduplicated.

All1223 complete regions, every original/linked target and PE relocation,9 negative controls and existing ABI pass. EH objects are complete: cleanup+handlers54/38/24 and matching4/2/1-state unwind arrays+FuncInfo60/44/36, all internal labels and CxxFrameHandler identities checked. No new external import or pending provider. Eight old linked pending bodies remain excluded. No graphics/game execution or actual C++ exception unwinding.

First extraction added <new>, changing destructor exception emission (128/98/50 vs176/146/98); remove the extra header to preserve original include environment. Constructor still169 vs120 because SetPossibleVertexProcessingList was not present in the same translation unit. Including the complete original setter lets MSVC infer its original nonthrowing behavior and restores120 bytes. No exception instructions/behavior were hand-edited. All source snapshots preserved; only the changed enumeration unit compiled for each correction.

All19 definitions are identical to local0.2.5, but already present in our base; no incremental direct transfer credit. Accepted direct0.2.5 total remains1,523 bytes. Current accepted link cp32-dxut-enumeration-linked; Ob1 cp32-dxut-enumeration-setter-ob1, Ob2 cp32-ime-ob2. Contract/seeds dxut-enumeration, units build/dxut-enumeration-units.txt, generator tools/prepare_dxut_enumeration.py. Proof dxut-enumeration-acceptance.json; trials/source comparison dxut-enumeration-integration.json; local review build/review_dxut_enumeration.py.

Complete eval_dxut_enum.cpp source scan cp32-dxut-enum-scan is preserved alongside raw discovery. Most larger candidates now integrated. Top-level Enumerate1003 remains unselected: it needs DXUTGetD3DObject/global state, SortModesCallback and actual additional typed array helpers. DXUTFindValidResolution296 from framework scan was already CP3.1 covered, so no standalone incremental credit. Next prioritize full DXUT state/resource-cache lifetime to unlock framework functions, or independent device cursor1016 (requires pinned gdi32 imports). Retain exact original header environment; adding <new> is appropriate only when required to reproduce actual array-new identities, not as a universal extraction rule. All processes terminal; no push/deployment.


## Resource-cache lifetime and object-local symbol binding

DXUT-cache acceptance adds1,375 unique exact bytes, reaching96,370 /930,756 (10.35395%). Complete cache singleton95, destructor122, OnReset96/OnLost182/OnDestroy220 and typed array Remove/SetSize/SetSizeInternal/destructor providers integrated. Full36-byte cache143D64,4-byte guard143D88, cleanup10 atE43A0, EH cleanup+handler40 atE3D20 and3-state unwind+FuncInfo52 atFBC38 match. Complete pinned LIBCMT atexit18 atC8B05 verified and excluded from source coverage.

Initial strict gate stopped because the compiler's local _$E2 cleanup name occurs4 times in MAP (different source units and LIBCPMT). Gate now resolves COFF storage-class3 anchors only through a unique MAP owner equal to actual unit.obj. Public-symbol uniqueness and all whole-section/target checks unchanged. Added a10th negative test rejects missing correct owner, duplicate correct owner and shifted owned address. All1245 complete regions,10 negative controls and existing ABI pass; no rebuild required for verifier correction. All future acceptance reports now record10 controls.

Only closure_dxut_cache compiled once; earlier objects reused. All5 selected definitions match local0.2.5 text, already present in our base, so no new direct transfer credit; direct total remains1,523. Eight old linked pending providers unchanged. No graphics, cache destruction, input or actual C++ unwinding executed. Current accepted link cp32-dxut-cache-linked; Ob1 cp32-dxut-cache-ob1, Ob2 cp32-ime-ob2. Contract/seeds dxut-cache, units build/dxut-cache-units.txt, generator tools/prepare_dxut_cache.py. Proof dxut-cache-acceptance.json; trial/source comparison dxut-cache-integration.json; local review build/review_dxut_cache.py. All processes terminal.

Whole DXUTmisc scan cp32-dxut-misc-scan completed (197 emitted,190 with candidates). Source eval_dxut_misc.cpp is pinned DXUTmisc.cpp with only include path adapted. Larger future candidates include cache texture/cube/volume FileEx/ResourceEx499..616 each, camera FrameMove1319/601, AddLine625 and UpdateVelocity574. Actual source cache lifetime is now available for framework GetDXUTState Create; Destroy still requires complete DXUTShutdown, DXUTCleanup3DEnvironment, DXUTAllowShortcutKeys and Windows identities. Do not simplify state/lifetime or execute shortcut changes. No push/deployment.


## Complete resource creation family

DXUT-resources acceptance adds4,837 unique exact bytes, reaching101,207 /930,756 (10.87363%). All16 original cache creation methods match: Texture/CubeTexture/VolumeTexture File/Resource basic+Ex variants, Font/FontIndirect and EffectFile/EffectResource. Complete typed Add Texture58/Effect58/Font55 providers included; existing size/allocation helpers deduplicated.

All1271 complete regions, every original/linked target and PE relocation,10 negative controls and existing ABI pass. Complete4 GUID objects16 bytes each atEDF7C/EDF8C/EDF9C/EDFAC are compiled from original SDK DEFINE_GUID declarations via windows/initguid/d3d9, not copied from the original binary. Eight new6-byte FF25 D3DX relays have complete DLL/API/IAT/relocation identities, pinned d3dx9.lib MAP owner/hash and actual d3dx9_25.dll names. Vendor relay/DLL code excluded; existing TextureFileEx import retained. Kernel32 lstrcmpA identity reviewed. Eight old pending providers unchanged.

Initial GUID unit included excessive DXUT headers after initguid and failed on GUID_NULL in unrelated DXUTsound.h. Narrowing it to windows/initguid/d3d9 fixed compilation; resource-method object reused unchanged. All16 definitions are text-identical to local0.2.5 but already present in our base; no incremental direct transfer credit, total remains1,523 bytes. No actual resource creation/graphics/game execution.

Current accepted link cp32-dxut-resources-linked; Ob1 cp32-dxut-resources-guids-ob1, Ob2 cp32-ime-ob2. Contract/seeds dxut-resources, units build/dxut-resources-units.txt, generator tools/prepare_dxut_resources.py. Proof dxut-resources-acceptance.json; compilation trial/source comparison dxut-resources-integration.json; local review build/review_dxut_resources.py. All processes terminal; no push or deployment.

Next: complete DXUTState/global framework lifetime and shutdown path, or remaining camera/arcball/math candidates from dxut-misc-scan. Resource cache now has full lifetime and creation providers. Full framework state class/layout is already present in eval_dxut_framework.cpp; GetDXUTState original7D140 and GetTime79220 identities recorded earlier. GetDXUTState ctor Create requires cache singleton (now accepted), global critical section and complete state; dtor calls DXUTShutdown, which requires full cleanup and shortcut restoration. Keep original source bodies and true callback/module targets; never execute native shortcut changes. Remaining standalone cursor1016 requires pinned GDI32 import verification. Candidate rankings predate these accepted bytes and must subtract the current coverage union.


## Complete DXUT state and framework shutdown foundation

DXUT-state acceptance adds4,136 unique exact bytes, reaching105,343 /930,756 (11.31800%). Complete DXUTState/global lifetime, Shutdown100, Cleanup355, error-reporting796 COMDAT (758 code plus complete alignment/table), Windows mode query and public state wrappers/accessors match. Full1981-byte state140FD0, guard4 at141790, critical section24 at140F90 and initialized thread-safe flag1 at102C0C verified. Create90, atexit cleanup22 atE4340, EH cleanup+handler24 atE3B00 and unwind+FuncInfo36 atFB9F8 are complete.

All1359 full regions, every target/PE relocation,10 negative controls and existing ABI pass. All11 original error strings and6 new named kernel32/user32 imports reviewed. The wrapper candidates with identical12-byte call shapes are disambiguated through known GetDXUTState7D140 and exact typed member-provider targets, then fully compared in the linked capsule. Eight old pending bodies remain excluded; no new missing provider. No runtime window messages, graphics, state destruction or C++ unwinding executed.

Initial lexical extraction hit DXUTIsWindowed call sites; anchor its exact bool definition. Initial source compiled with multimon GetSystemMetrics macro and failed link/exactness on xGetSystemMetrics. Original R5 explicitly calls user32 GetSystemMetrics through IAT E53E4. Undefining only that macro restores exact796-byte error-handler COMDAT without behavior edits. Previous notes overestimated shortcut restoration work: actual source DXUTAllowShortcutKeys18 only calls the state flag setter. It does not install hooks or change OS accessibility settings here, regardless of adjacent comments.

All5 core function bodies are identical to0.2.5. Complete state/lock/global/singleton block differs only by a MATCH comment. Current base already supplies this source, so no new direct transfer credit; total stays1,523 bytes. Only changed closure_dxut_state.cpp recompiled; prior objects reused. Current accepted link cp32-dxut-state-metrics-linked; Ob1 cp32-dxut-state-metrics-ob1, Ob2 cp32-ime-ob2. Contract/seeds dxut-state, units build/dxut-state-units.txt, generator tools/prepare_dxut_state.py. Proof dxut-state-acceptance.json; source comparison/trials/candidate resolution dxut-state-integration.json; local review build/review_dxut_state.py. All processes terminal; no push or deployment.

Next: state foundation now unlocks top-level CD3DEnumeration::Enumerate1003, framework BuildOptimalDeviceSettings482/BuildValidDeviceSettings1545, ParseCommandLine2260 and other larger state users. Preserve single shared original DXUTState definition and provider identities when extracting additional methods: adding a second unit that independently owns GetDXUTState/state would be wrong. Best extend the current state generator with whole source functions and needed local helpers. ScrollBar Render507 now has actual DXUTGetTime12/GetTime66 providers; can add it to existing GUI controls with Scroll and original repeat/delay doubles. Remaining device cursor1016 and cameras also available. Candidate rankings are older than latest coverage and must subtract accepted overlaps. Whole DLL still far from complete.
