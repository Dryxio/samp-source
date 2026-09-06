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


## Full adapter enumeration and device-settings construction

DXUT-settings acceptance adds3,321 unique exact bytes, reaching108,664 /930,756 (11.67481%). Complete Enumerate1003, GetEnumeration58, SortModesCallback69, BuildOptimalDeviceSettings482 and BuildValidDeviceSettings1545 are linked with all actual providers. FindValidResolution296 is now in the closure but was CP3.1 covered and is not counted again. Typed display-mode/adapter Add providers, formatting and integer-array helpers verified; prior source providers deduplicated.

All1384 whole regions, every target/PE relocation,10 negative controls and existing ABI pass. Complete93-byte enumeration singleton141798 and4-byte guard1417F8; original adapter-name string; atexit cleanup10 atE4360; EH cleanup+handlers21 atE3BD0 and18 atE3B20; both36-byte one-state unwind+FuncInfo objectsFBACC/FBA1C checked. Pinned LIBCMT stricmp105 and qsort643 fully verified and excluded from source coverage. No new import/missing source provider. Eight old linked pending bodies excluded. No D3D enumeration, callbacks or exception unwinding executed.

One incremental compile of changed enumeration unit and new settings unit; all others reused. All6 source definitions compared to local0.2.5 are text-identical, already available in our base, so no additional direct transfer credit; direct accepted total1,523 unchanged. No failed compile/link trial in this batch. Current accepted link cp32-dxut-settings-linked; Ob1 cp32-dxut-settings-ob1, Ob2 cp32-ime-ob2. Contract/seeds dxut-settings, units build/dxut-settings-units.txt. Generator tools/prepare_dxut_settings.py and extended tools/prepare_dxut_enumeration.py. Proof dxut-settings-acceptance.json; source comparison/notes dxut-settings-integration.json; local review build/review_dxut_settings.py. All processes terminal; no push/deployment.

Next high-yield family: ParseCommandLine2260 and its state override setters. Extend the single tools/prepare_dxut_state.py source unit with complete DXUTGetCmdParam (define before parser), DXUTSetConstantFrameTime and parser. Debug output provider DXUTOutputDebugStringA lives in DXUTmisc.cpp, not DXUT.cpp; extract its real body separately. CRT atoi/strtod and GetCommandLineA import need full identity review; no parsing execution necessary. Top-level enumeration/settings providers are now available for FindValidDeviceSettings and later device creation. Keep source original include environment and shared singleton; use ranked framework/misc candidates after subtracting current accepted coverage.


## Command-line options and constant-frame configuration

DXUT-commandline acceptance adds3,599 unique exact bytes, reaching112,263 /930,756 (12.06149%). Parser2260, SetConstantFrameTime83,19 typed state setters60 each and2 getters55/60 match. Actual GetCmdParam126 is linked but CP3.1 already covered it; no duplicate credit. All17 complete option/diagnostic strings verified. Named kernel32 GetCommandLineA and pinned LIBCMT strtod142 atCA47D checked, prior atoi/strnicmp providers retained.

DXUTOutputDebugStringA uses its complete original conditional source. The configured Release body is1-byte RET, bound to exact parser call target95660; it is not an invented stub or a missing dependency. Debug branch remains in source. All1426 complete regions, every target and PE relocation,10 negative controls and existing ABI pass. Eight old pending providers unchanged; no parser, input or state-change execution.

Initial GetCmdParam extraction matched call sites. First probe compiled debug unit and reused unchanged state, never linked/accepted. Anchoring actual bool definition fixed generation; only changed state unit recompiled. All4 new function definitions are identical to local0.2.5 and already in base, so direct accepted0.2.5 transfer total stays1,523. Current accepted link cp32-dxut-commandline-linked; Ob1 cp32-dxut-commandline-extract-ob1, Ob2 cp32-ime-ob2. Contract/seeds dxut-commandline, units build/dxut-commandline-units.txt; generators tools/prepare_dxut_state.py and tools/prepare_dxut_debug.py. Proof dxut-commandline-acceptance.json; source/trials dxut-commandline-integration.json; local review build/review_dxut_commandline.py. All processes terminal; no push/deployment.

Next: DXUTInit553 and associated callbacks/settings are now attractive because parser, timer, global state and cache are accepted. Extend existing state unit for true state users; preserve exact GetSystemMetrics import selection and original header environment. Framework FindValidDeviceSettings482/PrepareEnumerationObject and device creation are next larger closures, though RankDeviceCombo783 lacked a full-scan candidate and needs investigation. ScrollBar Render507 and camera/arcball families also remain. No whole-DLL claim; coverage counts only exact accepted unique code regions.


## DXUT initialization and dynamic Direct3D API closure

DXUT-init acceptance adds1,433 unique exact bytes, reaching113,696 /930,756 (12.21545%). Complete Init553 at7F690, loader282 at991E0, eight typed API wrappers at99300..993E0, six state setters and StringCopyWorker77 integrated. Duplicate helper instances deduplicated. Complete36-byte nine-pointer block143C14 and11 DLL/API strings verified. Ambiguous wrappers resolved through named GetProcAddress assignments in the complete loader and matching actual pointer members.

All whole regions, original/linked relocations and targets,10 negative controls and existing ABI pass. New pinned comctl32.lib included in link/provenance. Initial review assumed named InitCommonControls, but original and linked use COMCTL32 ordinal17; MAP confirms actual SDK library owner. SystemParametersInfoA and full D3DX25 CheckVersion6-byte relay/IAT/library verified. No workaround or behavior edits. Eight prior pending providers unchanged. No module loading, accessibility changes, initialization or game execution.

All10 compared source bodies are identical to0.2.5 but already in current base, so direct transfer credit remains1,523 bytes. One incremental compile of changed state/new dynamic unit; all other objects reused. Current accepted link cp32-dxut-init-linked; Ob1 cp32-dxut-init-ob1, Ob2 cp32-ime-ob2. Contract/seeds dxut-init, units build/dxut-init-units.txt; generators prepare_dxut_state.py and prepare_dxut_dynamic.py. Acceptance/integration proofs dxut-init; review build/review_dxut_init.py; retained initial failed named-import assumption in build/make_init_review.py. All processes terminal; no push/deployment.

Next: FindValidDeviceSettings/PrepareEnumerationObject and device creation now have complete initialization providers. RankDeviceCombo783 still needs exactness investigation. Alternatively ScrollBar Render507 with existing time providers or remaining camera/arcball family. Preserve one shared DXUTState unit and incremental source snapshots. Whole DLL still incomplete.


## Complete DXUT device selection and ranking

DXUT-selection acceptance adds1,521 unique exact bytes, reaching115,217 /930,756 (12.37886%). PrepareEnumerationObject96 at7E580, RankDeviceCombo783 at7CD70, FindValidDeviceSettings482 at7F8C0,3 state getters,typed IndexOf74 and original GetAdapterInfoList4 now integrated. Seven complete source float constants match. All1471 whole regions, every target/PE relocation,10 negative controls and existing ABI pass. Eight old pending remain excluded.

Initial full original Rank783 had same size but different call/register evaluation order only around the first adapter-format subtraction. Normal C++ local sequences combo conversion before optimal conversion, matching original full instruction sequence; no assembly or copied bytes. Recompile only settings. First link failed on GetAdapterInfoList; actual original getter4 added to enumeration and only that unit recompiled. Draft's prior CRT guess for this custom getter was never accepted. Final review uses successful link metadata; early review before link completion stopped on missing metadata and was rerun after terminal success. All failed snapshots/logs retained.

All4 original definitions compared with0.2.5 identical and already in base; no new direct transfer credit, total remains1,523. Current accepted link cp32-dxut-selection-provider-linked; Ob1 cp32-dxut-selection-provider-ob1, Ob2 cp32-ime-ob2. Same full units build/dxut-init-units.txt. Contract/seeds dxut-selection. Proofs dxut-selection-acceptance/integration; review build/review_dxut_selection.py. Generators prepare_dxut_state/settings/enumeration.py. Original/source rank disassembly build/dxut-rank-original/source.txt. No runtime enumeration/device creation, push or deployment; all processes terminal.

Next: DXUTCreateDevice730 and CreateDeviceFromSettings246 have full scan candidates but need DXUTChangeDevice2719 (no complete candidate yet) and transitive device/window callbacks. Inspect bounded instruction differences before investing in this larger closure. Callback registration35/52 can be resolved through exact typed state member identities; do not choose arbitrary identical-shaped candidate. Remaining ScrollBar507/camera/arcball are alternative useful closures. Whole DLL remains incomplete.


## Complete DXUT create/reset lifecycle and device statistics

DXUT-lifecycle acceptance adds3,737 unique exact bytes, reaching118,954 /930,756 (12.78036%). Complete Create3DEnvironment572, Reset390, Pause160, BackBuffer102, GetDeviceSettings73, DeviceStats781, StaticStats411 with all actual state/cache/formatting providers. FormatToString1036 COMDAT contains738 code bytes plus complete verified alignment/switch table. All75 literals validated against actual original source strings and target data, not guessed pointers.

All1566 whole regions, every actual target/PE relocation,10 negative controls and existing ABI pass. Actual cache OnCreateDevice5 comes from full original source returning S_OK; not an invented stub. No new external; eight old pending unchanged. Initial Reset definition extraction matched call sites; state generator now anchors actual top-level definitions. First probe only compiled cache with previous state. Corrected state compiled next; first link missing FormatToString; extracted full helper into format unit and only that unit recompiled. Every failed snapshot retained. No device creation/reset, callback, window or game execution.

All10 compared original definitions (including future ChangeDevice) identical to0.2.5 and already present in base; no extra direct transfer credit, total1,523 unchanged. Current accepted link cp32-dxut-lifecycle-formats-linked; Ob1 cp32-dxut-lifecycle-formats-ob1, Ob2 cp32-ime-ob2. Full units still build/dxut-init-units.txt. Contract/seeds dxut-lifecycle, acceptance/integration evidence dxut-lifecycle. Review build/review_dxut_lifecycle.py. Generators prepare_dxut_state/cache/device.py. All processes terminal; no push/deployment.

Next: ChangeDevice original7FCF0/source both2719; full-framework diagnostic found only3 non-relocation byte differences at1CA/1E8/2C6 (CMP operands reversed). Full original/source disassembly build/dxut-change-original/source.txt. Diagnose actual call ordering and source comparison expressions; do not patch bytes. Still requires UpdateDeviceSettingsWithOverrides, InitHWCursor/SetDeviceCursor1016 (GDI imports), monitor compatibility providers and static accessors. Create/Reset/Pause/Stats foundations now accepted. CreateDevice730 and CreateDeviceFromSettings246 candidates depend on this closure. No ChangeDevice acceptance yet. Whole DLL remains incomplete.


## Hardware cursor and device override providers

DXUT-cursor acceptance adds2,079 unique exact bytes, reaching121,033 /930,756 (13.00373%). Complete SetDeviceCursor1016 at7BA80, InitHWCursor105 at7DC30, UpdateDeviceSettingsWithOverrides282 at7D440, DesktopResolution132 at7F600 and10 typed override getters integrated. Original color/monochrome bitmap loops, watermark constants and cleanup preserved as normal C++.

All1580 whole regions, actual targets/PE relocations,10 negative controls and existing ABI pass. New gdi32.lib pinned in link metadata/contract. All13 named GDI32/user32 imports bound to actual PE IAT entries and SDK MAP owners. One incremental compile of changed state/new cursor, successful first link. No cursor, desktop, graphics or game execution. Eight old pending unchanged.

All4 definitions identical to0.2.5 and already present in base, no new direct transfer credit; direct total1,523 unchanged. Current accepted link cp32-dxut-cursor-linked; Ob1 cp32-dxut-cursor-ob1, Ob2 cp32-ime-ob2. Full units build/dxut-cursor-units.txt. Contract/seeds dxut-cursor; acceptance/integration evidence dxut-cursor; review build/review_dxut_cursor.py. Generators prepare_dxut_state.py and prepare_dxut_cursor.py; linker now includes pinned gdi32. All processes terminal, no push/deployment.

Next: ChangeDevice2719 closure now has real cursor/override/Create/Reset/Pause/Stats providers. Remaining monitor wrappers xGetMonitorInfo/xMonitorFromWindow originate from MSVC PlatformSDK multimon.h with COMPILE_MULTIMON_STUBS; original DXUT.cpp includes this after dxstdafx. Preserve single actual monitor storage or verify native R5 imports if applicable. Full scan original ChangeDevice3 CMP operand differences at1CA/1E8/2C6; original call order windowed7D940 first then fullscreen7D930, compares EAX with saved ESI/EDI. Source expression currently FullScreen() ==/!= Windowed(); use normal source evaluation-order investigation, never instruction patches. Complete targets still require review, no ChangeDevice coverage yet. Larger CreateDevice730/CreateDeviceFromSettings246 remain dependent on it. Whole DLL incomplete.


## Complete DXUTChangeDevice and multimon providers

DXUT-change acceptance adds4,409 unique exact bytes, reaching125,442 /930,756 (13.47743%). Full ChangeDevice2719 at7FCF0,17 state accessors, multimon Init257 at78240, IsPlatformNT62 at78200, Window130 at78480, Rect82 at78420 and Info189 at78510 accepted. Full36-byte9-member pointer/flag block140FA8 and9 API strings verified. All1613 whole regions, every actual target/PE relocation,10 negative controls and existing ABI pass. Eight old pending unchanged.

Normal C++ explicit Windowed handle locals restore3 comparison encodings while retaining original Windowed-first call order. No assembly/byte patching. Original SDK multimon.h compiled, then materialized fully into source snapshot for exact source provenance (pinned input SHA in prepare_dxut_multimon.py). First monitor Info191 failed original189: original imports kernel32 lstrcpyA for fixed DISPLAY, SDK used lstrcpyn. Changed only this source call, recompiled only monitor. Import review then caught SetWindowLongA vs true R5 SetWindowLongW despite otherwise complete candidate bytes. Explicit W variant compiled in state only, final actual targets pass. All15 new imports and SDK owners reviewed. Failed trials retained; none accepted early. No native window/monitor/device execution.

Original ChangeDevice already in base and identical to0.2.5; no new transfer credit, direct total1,523 unchanged. SDK source was already implied by original framework include, not an extra0.2.5 transfer. Current accepted link cp32-dxut-change-wide-linked; Ob1 cp32-dxut-change-wide-ob1, Ob2 cp32-ime-ob2. Units build/dxut-change-units.txt. Contract/seeds dxut-change, evidence dxut-change-acceptance/integration, review build/review_dxut_change.py. Generators prepare_dxut_state.py and prepare_dxut_multimon.py. SDK original local private/checkpoint31/multimon.h, SHA checked before generation; materialized source tracked. All processes terminal, no push/deployment.

Next: CreateDevice730 and CreateDeviceFromSettings246 now have real ChangeDevice providers but may require CreateWindow and static window procedure. Inspect transitive dependencies before selecting large window closure. Callback registration35/52 and SetWindow/SetDevice helpers are other coherent framework closures. Preserve one state and one actual multimon global owner. Whole DLL incomplete.


## Window/device entry-point closure — candidates in progress

No new accepted coverage: remains125,442 /930,756. Added12 complete original functions to existing state generator: CreateWindow792, SetWindow328, CreateDevice730, CreateDeviceFromSettings246, SetDevice360, FindAdapterFormat114, StaticWndProc2649 full COMDAT (2235 code), CheckSize192, CheckMonitors, GetAdapterOrdinal106, ToggleFullScreen and ToggleREF. All12 originals identical to0.2.5 and already in base; no new transfer credit.

VM was confirmed stopped before first compile; restarted successfully, no live job was restarted. Baseline cp32-dxut-window-ob1 compiled only state. Its exact source SHA restored after three unsuccessful experiments, verified against probe.sources. CheckMonitors356 vs353 original (stackD0 vs94), ToggleFullScreen444 vs441, ToggleREF273 vs270 remain. Full original framework scan has correct-size candidates with same Ob1 flags; source extraction context remains an open cause. Moving finder alone caused488 vs482, moving full selection family restored finder but not3 mismatches; scoped temporary assignment also unchanged. All experiments reverted in current source; source/build snapshots preserved. Read evidence/checkpoint32/dxut-window-trials.json. Do not repeat these exact experiments.

WndProc original81030 has2235 code but fullCOMDAT2649 with table. Diagnostic found0 nonfixup differences across full2649, not acceptance. Initial seed-generation assertion stopped on CheckMonitors, so no dxut-window-seeds persisted and no new exports added. Initial link failed on ExtractIconA; linker now adds shell32.lib and its hash, awaiting actual link/import review. No acceptance attempted. Current accepted link/proof remains dxut-change-wide/ dxut-change; full units build/dxut-change-units.txt, separate closure_dxut_settings retained. build/dxut-window-units.txt belongs to reverted merged-family trial; do NOT use it. All processes terminal; no push/deployment. Current source candidate work intentionally exceeds accepted contract; do not run old acceptance against these changed sources.

Next: diagnose3 stack-slot reuse differences with baseline and complete-framework objects/disassembly, then create seeds/exports and link with actual shell32 provider. Alternatively integrate a coherent independent window-helper subset while preserving unaccepted closure separately. Maintain whole matches and real callback providers, never count missing window procedure/dependencies. No whole-DLL claim.


## Complete DXUT window/device creation closure accepted

DXUT-window acceptance adds8,882 unique exact bytes, reaching134,324 /930,756 (14.43171%). Twelve complete functions and48 state accessors accepted. Includes CreateWindow792, SetWindow328, CreateDevice730, SetDevice360, CreateDeviceFromSettings246, FindAdapterFormat114, AdapterOrdinal106, CheckSize192, CheckMonitors353, ToggleFullScreen441, ToggleREF270. WndProc2649 full COMDAT contains2235 code bytes plus all tables verified. SDK SwapChain GUID16,2 strings,zero double8 and full60-byte contiguous critical-section/monitor globals checked; no duplicate coverage from migrated prior regions.

All1674 whole regions, every actual target/PE relocation,10 negative controls and existing ABI pass.14 new Windows imports/pinned shell32 verified. Earlier extractions had353/441/270 functions3 bytes larger from temporary stack placement. Sorting core definitions did not help and headers were unchanged. Restoring complete original DXUT.cpp context resolved all3, without source stack hacks. Exact internal compiler mechanism not proved; full-context result directly verified. Prior settings/cursor/multimon/predicate providers now in same original state unit; exclude their standalone units from this link. Full prior ranges and target sequences reviewed across unit migration. All12 originals already in base and identical to0.2.5, no new transfer credit; direct total1,523 unchanged.

Initial full generator definition extraction failed on ChangeDevice call-site ambiguity, anchored actual definition. First full link found duplicate standalone predicate and missing true DXTraceWrapper; excluded replaced unit and compiled real conditional Debug/Release wrapper in debug unit. Next target review caught SetWindowLongPtrA/DefWindowProcA vs native R5 W identities; explicit W forms compiled in state only. All failed runs/snapshots kept. No actual window creation, input processing, D3D/device/game execution; eight old pending unchanged. No push/deployment.

AUTHORITATIVE generator is now tools/prepare_dxut_full_state.py (uses body adaptations from prepare_dxut_state.py and pinned prepare_dxut_multimon.py). Do not run partial prepare_dxut_state.py directly to regenerate current accepted state: it is retained as extraction/adaptation utility and historical trial generator. Current accepted link cp32-dxut-window-full-wide-linked; Ob1 cp32-dxut-window-full-wide-ob1, Ob2 cp32-ime-ob2. Full units build/dxut-window-full-units.txt excludes closure_dxut_settings,closure_dxut_cursor,closure_dxut_multimon,closure_dxut_device. Contract/seeds dxut-window; acceptance/integration evidence dxut-window; review build/review_dxut_window.py. Every process terminal. Source regeneration can safely materialize full source only through the full generator.

Next: full DXUT unit already compiled makes callback registrations/timer/frame-loop/context getters candidates available without recompiling, select coherent missing families and verify every target. Full runtime Render3DEnvironment may need DXUTHandleTimers and actual native message/timing imports; use exact compiled candidates minus current coverage union. Alternatively return to other subsystems/0.2.5 missing implementations. Whole DLL incomplete.


## Main/render loop, frame statistics and timer lifecycle

DXUT-loop acceptance adds3,567 unique exact bytes, reaching137,891 /930,756 (14.81495%). Full Render3DEnvironment924, MainLoop580, UpdateFrameStats238, SetTimer199, KillTimer117, HandleTimers159 plus20 state accessors and3 complete timer-array providers. Prior overlaps deduplicated. Full double1 and2^32 unsigned-conversion constants match.

All1705 whole regions, every original/linked actual target and PE relocation,10 negative controls and existing ABI pass. Five new user32 imports (PeekMessageA, TranslateAcceleratorA, TranslateMessage, DispatchMessageA, DestroyAcceleratorTable) checked against original/linked IAT and SDK MAP owners. No source changes or compilation: reused full DXUT object, linked only. Eight old pending unchanged. No main/render loop, Windows message, timer callback or game execution. All6 original source bodies identical to0.2.5 and already in base, no additional transfer credit; total1,523 unchanged.

Current accepted link cp32-dxut-loop-linked; Ob1 cp32-dxut-window-full-wide-ob1, Ob2 cp32-ime-ob2. Contract/seeds dxut-loop; evidence dxut-loop-acceptance/integration; review build/review_dxut_loop.py. Full units build/dxut-window-full-units.txt, full state generator prepare_dxut_full_state.py remains authoritative. All processes terminal; no push/deployment.

Next: callback registration family already compiled; public35-byte wrappers have many shape candidates, resolve through actual typed setter providers/field offsets before selecting. Full compiled discovery build/cp32-dxut-window-full-extract-ob1/discovery.json predates only two reviewed W API adjustments; verify against actual final object/linked targets, not scan inference alone. Remaining framework settings/getters or GUI ScrollBar/camera/arcball families can proceed without inventing dependencies. Whole DLL incomplete.


## Complete callback registration family

DXUT-callbacks acceptance adds1,507 unique exact bytes, reaching139,398 /930,756 (14.97686%). Ten callbacks registered by9x35-byte wrappers and mouse52 at7D1A0..7D360,19 new60-byte typed setters;2 existing ModifyDevice setters reused. Ambiguous35-byte candidates resolved through actual setter targets. Each setter unique complete candidate includes real member-offset literals and accepted thread-safe/critical-section/import identities; final actual linked targets fully verified.

All1734 whole regions, every target/relocation,10 negative controls and existing ABI pass. No new data/imports and no compilation: prior full object reused. First resolver assumed GetDXUTState inside instance setters and stopped; corrected to actual globals/locking targets before selecting wrappers. Initial link with previous exports was never accepted. No callback registration or execution; eight old pending unchanged. All10 original bodies identical to0.2.5 and already in base, no direct transfer credit; total1,523 unchanged.

Current accepted link cp32-dxut-callbacks-resolved-linked; Ob1 cp32-dxut-window-full-wide-ob1, Ob2 cp32-ime-ob2. Full units build/dxut-window-full-units.txt. Contract/seeds dxut-callbacks; evidence dxut-callbacks-acceptance/integration; resolver build/resolve_dxut_callbacks.py, review build/review_dxut_callbacks.py. Full state generator prepare_dxut_full_state.py remains authoritative; do not replace accepted full state with partial generator. All processes terminal, no push/deployment.

Next: broaden beyond framework to remaining GUI/camera/arcball or source0.2.5 missing subsystem bodies. Existing misc full-scan candidates include camera FrameMove1319/601, UpdateVelocity574, AddLine625; many resource methods from same older scan already accepted, subtract coverage. GUI ScrollBar Render507 can use now-accepted clock providers. Framework additional settings/getters remain available in existing object for coherent batches, but prioritize larger useful sets. Whole DLL incomplete.


## Complete arcball and SDK camera family

DXUT-cameras acceptance adds7,454 unique exact bytes, reaching146,852 /930,756 (15.77771%).35 complete source methods/constructors for CD3DArcBall, CBaseCamera, CFirstPersonCamera, CModelViewerCamera. Includes both FrameMove1319/601, UpdateVelocity574, constructors482/555/228/28, all actual reset/input/projection/rotation/boundary providers. Three whole7-slot vtables match correct class overrides and real targets. Four full floats (+/-pi/2,-2,5) verified.

All1780 whole regions, actual targets/PE relocations,10 negative controls and existing ABI pass. Message/key COMDATs include complete tables: Arcball388 includes346 code, BaseHandle608 includes588 code, MapKey221 includes95 code. Entire tables verified; no table/alignment coverage credit.11 new actual D3DX25 relays checked against DLL/API/IAT/pinned lib owner,6 native user32 imports and complete LIBCMT purecall18 checked; vendor code excluded. No new pending provider, eight old pending unchanged.

One compile of new closure_dxut_cameras only; prior objects reused. First link included32 unique seeds and transitive vtable providers. Added nonvirtual Arcball HandleMessages explicit93C00 seed after inventory346/fullCOMDAT388 distinction; full388 matched, link only, no recompile. No camera/input/graphics/game execution. Entire original contiguous source block identical to0.2.5 but already in base, no extra transfer credit; direct total1,523 unchanged.

Current accepted link cp32-dxut-cameras-arcball-linked; Ob1 cp32-dxut-cameras-ob1, Ob2 cp32-ime-ob2. Full units build/dxut-cameras-units.txt. Contract/seeds dxut-cameras, evidence dxut-cameras-acceptance/integration, review build/review_dxut_cameras.py. New generator prepare_dxut_cameras.py; full state generator remains prepare_dxut_full_state.py. All processes terminal, no push/deployment.

Next: cubemap helper compiled alongside cameras but unselected: fullCOMDAT352, original likely950E0 code328 (function interval ends near format converter95240). Verify whole body/table/targets before seed/credit; prior no-candidate result is code-size/table distinction, not proof of mismatch. DirectionWidget/line-manager families remain in full misc scan and can reuse camera/math/import foundations. GUI ScrollBar507 still available. Subtract current coverage from older scans; whole DLL incomplete.


## Complete line-manager and cubemap view helper

DXUT-lines acceptance adds1,822 unique exact bytes, reaching148,674 /930,756 (15.97347%). Eleven complete CDXUTLineManager methods (AddLine625,AddRect288,OnRender225 included), actual typed array/vector providers, complete EH cleanup/handlers and two36-byte unwind/FuncInfo structures verified. Cubemap helper352 fully matches at950E0, including328 code and all attached table bytes; tables excluded from code credit. Prior overlapping providers deduplicated.

All1805 whole regions, every actual target/PE relocation,10 negative controls and existing ABI pass. One new real D3DXCreateLine relay verified with DLL/API/IAT and pinned SDK MAP owner; vendor code excluded. Eight old pending unchanged. Only closure_dxut_lines compiled once, all prior objects reused. First link lacked explicit ctor/dtor exports; second link added them without recompile. All products/snapshots retained. No graphics/input/game execution, push or deployment.

Whole line block identical to local0.2.5 but already in base, so no new direct transfer credit; total1,523 unchanged. No source adaptations. Generator tools/prepare_dxut_lines.py. Current accepted link cp32-dxut-lines-complete-linked; Ob1 cp32-dxut-lines-ob1, Ob2 cp32-ime-ob2. Full unit list build/dxut-lines-units.txt. Contract/seeds dxut-lines; evidence acceptance/integration dxut-lines; review build/review_dxut_lines.py. Full-state generator remains prepare_dxut_full_state.py. All processes terminal.

Next: CDXUTDirectionWidget contiguous block DXUTmisc.cpp lines2520..2745. StaticOnCreateDevice uses existing accepted DXUTFindDXSDKMediaFileCch and real D3DXCreateEffectFromFileA / D3DXLoadMeshFromX imports; source paths UI/DXUTShared.fx and UI/arrow.x are original external assets, not fabricated providers. Assess full source closure including actual globals, math relays, effect parameter strings. Older misc-scan candidates ctor302 at97380, OnRender529 at95AF0, UpdateLightDir470 at95D10, StaticOnCreate235 at990F0, OnReset95 at95A30, OnLost19 at95A90, OnDestroy51 at95AB0. HandleMessages fullCOMDAT289 requires explicit inventory/table mapping. Reuse accepted arcball/cameras and media helpers. Whole DLL incomplete.


## Complete direction-widget family

DXUT-direction acceptance adds1,962 unique exact bytes, reaching150,636 /930,756 (16.18426%). Eight complete CDXUTDirectionWidget methods, including OnRender529, UpdateLightDir470, ctor302, StaticOnCreate235. HandleMessages at974B0 fully matches289-byte COMDAT (261 code plus attached table); tables excluded from code credit. Twelve-byte original device/effect/mesh static pointer storage and4 complete source strings checked. Previously accepted math/camera/media/effect creation providers reused.

All1820 whole regions, actual targets/PE relocations,10 negative controls and existing ABI pass. Three new real D3DX25 relays LoadMeshFromXA,MatrixRotationX,Vec3TransformNormal verified with DLL/API/IAT and pinned SDK MAP owner; vendor code excluded. Eight old pending unchanged. One compile of new direction unit, one successful link; no source adaptations. Local0.2.5 differs only by commenting an unused HRESULT hr declaration in StaticOnCreateDevice, otherwise identical to base; no direct transfer credit, total1,523 unchanged. No graphics/input/game execution, push or deployment.

Current accepted link cp32-dxut-direction-linked; Ob1 cp32-dxut-direction-ob1, Ob2 cp32-ime-ob2. Full units build/dxut-direction-units.txt. Generator prepare_dxut_direction.py; contract/seeds dxut-direction; evidence acceptance/integration dxut-direction; review build/review_dxut_direction.py. Full-state generator remains prepare_dxut_full_state.py. All processes terminal.

Next: GUI ScrollBar Render507 plus Scroll actual helper can reuse accepted timer/PtInRect/drawing and UpdateThumbRect providers. Existing authoritative tools/prepare_gui_controls.py extracts ComboBox/Button/CheckBox/Slider/Static Render and rectangle/cap/thumb helpers, original <new> include retained. Add ScrollBar Render and Scroll there, plus original SCROLLBAR_ARROWCLICK_DELAY/REPEAT defines (.33/.05 confirm source); compile changed controls only. Keep same-unit Cap/UpdateThumbRect required. Alternatively select larger untouched subsystem via current union minus older scans. Whole DLL incomplete.


## GUI scrollbar rendering and timed scroll

GUI-scroll-render acceptance adds538 unique exact bytes, reaching151,174 /930,756 (16.24207%). Complete Render507 at88A50 and Scroll31 at887B0; full original double0.33/0.05 constants verified. Existing Cap/UpdateThumbRect/clock/PtInRect/Blend/DrawSprite providers all reused.1824 whole regions, every actual target/PE relocation,10 negative controls and existing ABI pass. No new imports or pending providers; eight old pending unchanged. Only existing controls unit recompiled, one link, no source adaptations or runtime GUI/input/game execution. Both source functions identical to0.2.5 but already in base; no direct transfer credit, total1,523 unchanged.

Current accepted link cp32-gui-scroll-render-linked; Ob1 cp32-gui-scroll-render-ob1, Ob2 cp32-ime-ob2. Same full units build/dxut-direction-units.txt. Generator prepare_gui_controls.py now includes Render and Scroll with original delay/repeat defines. Contract/seeds gui-scroll-render; evidence acceptance/integration gui-scroll-render; review build/review_gui_scroll_render.py. Full state generator unchanged. All processes terminal; local commit only.

Next: GUI input handlers remain large potential families. Older cp32-gui-full-scan has no simple candidate for ListBox HandleMouse1155, ComboBox HandleMouse811/HandleKeyboard507, ScrollBar HandleMouse632, Slider HandleMouse372, often because fullCOMDAT includes tables and inventory code-only sizes differ. Source candidate discovery currently compares only equal inventory code size, so no candidate is not proof of mismatch. Inspect complete original function interval/full attached tables and actual providers; e.g ScrollBar HandleMouse likely887D0 preceding Render88A50. Broaden discovery to attached table candidates if useful, but acceptance must retain full bytes/targets/PErelocsets. Misc TextHelper DrawTextLine379/361 and REF warning856 remain candidates. Whole DLL incomplete.

### Attached-table discovery enabled

Enhanced tools/discover_source_functions.py with optional --include-attached-data. Keeps every default exact-size candidate, additionally compares full COMDATs beyond code-only inventory size without crossing next original function. All bytes, consistent inferred targets and exact original PE relocation set still required; never acceptance. Writes discovery-attached.json separately, preserving existing discovery.json. Replaced repeated full PE relocation walks with sorted index/bisect; indexed and unindexed scans produce identical434 GUI rows, all original exact-size candidates retained. Optimized full scan command finished within0.85 seconds (includes validation); prior unindexed scan took over150 seconds. Old scan finished naturally just before attempted termination (kill returned no such process); terminal handle returned0, no duplicate live scans. Initial wrong unit gui_DXUTgui missing-object error retained; real unit eval_gui_full.

Seven new whole-COMDAT GUI candidates: ComboBox HandleKeyboard507 at87F70, Slider HandleKeyboard148 at88400, Slider HandleMouse372 at884A0, ScrollBar HandleMouse632 at887D0, EditBox HandleKeyboard614 at88EF0, EditBox HandleMouse428 at89160, ComboBox HandleMouse811 at8A370. Evidence gui-attached-discovery.json; raw outputs build/cp32-gui-full-scan/discovery-attached.json, build/gui-attached-unindexed-result.json, build/gui-new-attached-candidates.json. Four known accepted camera/table functions rediscovered (arcball38893C00,base60893EE0,key22194560,cubemap352950E0), validates prior blind spot. New candidates receive NO coverage credit yet.

Next integrate coherent GUI input family, potentially all7 if actual dependencies close. ScrollBar HandleMouse has function-local static int ThumbOffsetY and bool bDrag (complete source storage layout must verify); can compile separate input unit with original dxstdafx, <new> if needed and RectHeight inline. Source from original DXUTgui.cpp; helpers may need to remain same source unit for codegen. Keep verified controls Render/Scroll unit unchanged unless new source context required. No build in progress; latest accepted link remains cp32-gui-scroll-render-linked, coverage151,174. Whole DLL incomplete.


## Seven GUI input handlers with complete provider closure

GUI-input acceptance adds4,398 unique exact bytes, reaching155,572 /930,756 (16.71458%). Seven full COMDATs: Combo keyboard507 at87F70, Combo mouse811 at8A370, Slider keyboard148 at88400/mouse372 at884A0, ScrollBar mouse632 at887D0, EditBox keyboard614 at88EF0/mouse428 at89160. All attached tables verified; tables excluded from code credit. Twelve real providers SendEvent49,ClearFocus31,SetValueInternal63,ValueFromPos73,ResetCaretBlink30,CopyToClipboard185,Paste122,DeleteSelection79,RemoveChar86,GetPriorItem143,GetNextItem200,InsertString196. Actual source local ThumbOffsetY4 at143C00 and bDrag1 at143BFD complete zero storage checked. Old code overlaps deduplicated.

1846 whole regions, every actual target/PE relocation,10 negative controls and existing ABI pass.10 new real user32/kernel32 imports including clipboard/global memory/GetKeyState verified, vendor code excluded. Eight old pending unchanged, no new incomplete provider. New input unit only compiled per changed-source trial; all other objects reused. Initial compile missing original WHEEL_DELTA120 and DXUT_MAX_EDITBOXLENGTH0xFFFF defines, restored. Initial link seven handlers matched but CopyToClipboard179/185,RemoveChar84/86, Paste122 clipboard operand differed. R5 disassembly confirms WCHAR buffers,2-byte allocation/copy,CF_UNICODETEXT13 and lstrlenW. Explicit WCHAR/sizeof(WCHAR)/CF_UNICODETEXT source adaptations resolved all, including transitive InsertString196. No assembly or copied bytes. All trials/snapshots retained.

0.2.5 comparison14/19 bodies identical to base. CF_UNICODETEXT corroborated there, but its buffer operations are ANSI versus R5 wide; its Slider HandleMouse omits SetValueInternal(0,false) on release, whereas original R5 full body retains base call. Preserve actual R5 behavior despite surprising slider reset. No new direct whole-function transfer credit; total1,523 unchanged. Detailed comparison build/gui-input-025-comparison.json. No clipboard, input, GUI or game execution; no push/deployment.

Current accepted link cp32-gui-input-wide-linked; Ob1 cp32-gui-input-wide-ob1, Ob2 cp32-ime-ob2. Full units build/gui-input-units.txt. New generator prepare_gui_input.py, contract/seeds gui-input, evidence acceptance/integration gui-input, review build/review_gui_input.py. Full-state and controls generators unchanged. All processes terminal.

Next: GUI remaining input families CheckBox/RadioButton/Button can reuse SendEvent/ClearFocus and actual state providers. Existing full-scan candidates Radio mouse200 at87C20/keyboard134 at87B90, CheckBox mouse173 at84FF0. Larger remaining ListBox keyboard480/mouse1155 still no full attached-table candidate, so diagnose true differences if worthwhile. Other potential SetTextFloatArray230 at8B100, control SetElement147 at8A2D0, dialog OnCycleFocus142 at879A0. Prioritize coherent larger groups and0.2.5 useful differences; no duplicate table/code credit. Latest scanner --include-attached-data is fast and preserves old exact-size candidates. Whole DLL incomplete.


## Complete Button/CheckBox/RadioButton input family

GUI-buttons acceptance adds899 unique exact bytes, reaching156,471 /930,756 (16.81117%). Nine full functions total1081, prior-proof overlaps deduplicated to899 new bytes. Button keyboard96 at84E50/mouse199 at84EB0, CheckBox keyboard99 at84F80/mouse173 at84FF0, Radio keyboard134 at87B90/mouse200 at87C20, CheckBox SetCheckedInternal32 at850A0, Radio SetCheckedInternal65 at87CF0, ClearRadioButtonGroup83 at87340. Existing events/focus/typed providers reused.

1855 whole regions, every actual target/PE relocation,10 negative controls and existing ABI pass. No new data/imports or incomplete providers; eight old pending unchanged. One compile of new buttons unit and one link. Original full-scan Button HandleMouse194 failed; R5 disassembly shows no source OutputDebugString trace, but extra SendEvent(0x102,true,this) immediately after capture before optional focus. Removed trace and added explicit source press event constant; complete199 body and actual targets match immediately. No copied instruction bytes. Both base and0.2.5 have trace and omit event; no direct transfer credit, total1,523 unchanged.

Current accepted link cp32-gui-buttons-linked; Ob1 cp32-gui-buttons-ob1, Ob2 cp32-ime-ob2. Full units build/gui-buttons-units.txt. New generator prepare_gui_buttons.py; contract/seeds gui-buttons; evidence acceptance/integration gui-buttons; review build/review_gui_buttons.py. All processes terminal; no input, GUI or game execution, push or deployment.

Next: larger ListBox HandleKeyboard480/HandleMouse1155 still do not match even with attached-table discovery. Inspect actual item/member layout before chasing instruction selection: base DXUTListBoxItem contains TCHAR strText[256],void*pData,RECT rcActive,bool bSelected; ANSI TCHAR build may differ from R5 item size/offsets, currently unverified. Do not blindly modify shared headers or accept pointer-layout assumptions. Other remaining family candidates TextHelper DrawTextLine379/361, REF warning856, GUI dialog lifecycle/focus and edit text formatting. Use new input foundations and measured0.2.5 differences for prioritization. Whole DLL incomplete.


## ListBox keyboard with verified partial field views

GUI-listbox acceptance adds437 unique exact bytes, reaching156,908 /930,756 (16.85812%). ListBox keyboard full480 at8AA80 includes432 code and all attached switch data; ScrollBar keyboard5 at853F0 is original complete false-return implementation, identified via actual original ctor8ECD0 storing vtableEB528 and virtual keyboard slot+14, not arbitrary shape. No placeholder provider.1857 whole regions, every actual target/PE relocation,10 negative controls and existing ABI pass. Eight old pending unchanged, no new imports/data. One new unit compiled once, one link; no GUI/input/game execution.

Earlier full source keyboard differed13 operand bytes: scrollbar member address7D vs R5 5D, its page-size10F vs EF, and item bSelected114 vs298. Typed partial C++ views of actual known offsets resolve full function with all targets/table/PE relocations. Prefix bytes explicitly unknown and unaccessed, no allocation or sizeof(full-item) claims, no data coverage. Shared headers untouched; accepted other ListBox fields retained where original full instructions match. Keyboard source body identical to0.2.5 and base; layouts absent from both, no direct transfer credit, total1,523 unchanged. Diagnostic mismatch locator ignored COFF operands only to rank candidates, explicitly not matching/coverage (build/gui-listbox-diagnostic.json); final acceptance integral.

Current accepted link cp32-gui-listbox-linked; Ob1 cp32-gui-listbox-ob1, Ob2 cp32-ime-ob2. Full units build/gui-listbox-units.txt. New generator prepare_gui_listbox.py; contract/seeds gui-listbox; evidence acceptance/integration gui-listbox; review build/review_gui_listbox.py. All processes terminal; local-only work.

Next: extend same verified views to ListBox HandleMouse at8AC60. Original code1140, likely fullCOMDAT1171 including table, versus old source1155 full. Initial exact instruction comparison identifies scrollbar5D, rcSelection address11F (old6D), rcText.top113 => rcText10F (old5D). Actual scrollbar sizeofB2 =>5D+B2=10F, rectangles follow scrollbar instead of preceding it; later fields unchanged. Add RECT text/selection to partial scroll view AFTER scrollbar with offsetof asserts10F/11F, replace source m_rcText/m_rcSelection accesses, and all pItem/pSelItem/m_Items[...] bSelected with selection view bool298. These additional offsets are diagnostic evidence only until full matching. Mouse handling may then match once operand length changes accounted for; no behavior edits yet justified. Source UpdateRects252 may also benefit from same reordered view, but original item/render layout still incomplete and Render source is tiny incomplete129. Do not claim whole ListBox structure or renderer. Whole DLL incomplete.


## Complete ListBox mouse handler

GUI-listbox-mouse acceptance adds1,140 unique exact bytes, reaching158,048 /930,756 (16.98061%). Full1171-byte COMDAT at8AC60 includes1140 code and31 attached table bytes, every byte checked; table excluded from coverage.1858 whole regions, all actual targets/PE relocations,10 negative controls and existing ABI pass. No new imports/data/providers, eight old pending unchanged.

Partial R5 scroll view now confirms rectangles after scrollbar: scrollbar5D, text10F, selection11F. All item selection accesses use bool298. Compile-time offsetof checks retain these placements; unknown prefixes remain unknown and no allocations/sizeof-full-item claims. Original mouse source also differed behavior: R5 does not set m_bDrag=true after capture; on release it checks m_nSelected < m_Items.GetSize() before m_nSelected != -1. Views-only source1155 failed; behavior additions produced correct1171 but condition order differed23 bytes; swapping normal C++ && operand order resolves full code/table with actual targets. No assembly or copied bytes. Other accepted keyboard remains exact. All failed source snapshots/products retained; only changed listbox unit compiled each time. Base/0.2.5 mouse source identical but both lack R5 changes, no direct transfer credit; total1,523 unchanged.

Current accepted link cp32-gui-listbox-mouse-linked; Ob1 cp32-gui-listbox-mouse-order-ob1, Ob2 cp32-ime-ob2. Full units build/gui-listbox-units.txt. Generator prepare_gui_listbox.py; contract/seeds gui-listbox-mouse; evidence acceptance/integration gui-listbox-mouse; review build/review_gui_listbox_mouse.py. All processes terminal; no mouse/GUI/game execution, push or deployment.

Next: extend same views to ListBox SelectItem134 (original8A9F0), GetSelectedIndex85, UpdateRects252 and possibly RemoveItem/RemoveAllItems if actual providers close. SelectItem only selected bool298 + scrollbar5D differences likely; GetSelectedIndex only bool298. UpdateRects accesses verified text/selection and calls SetPageSize; existing full-scan SetPageSize23 had no candidate and may need original behavior inspection, do not force provider. Current generator adaptation regex covers pItem,pSelItem,m_Items[index],m_Items.GetAt(...) selected accesses. Add original RectHeight inline if UpdateRects included. Original GetItem33 at8A9C0 can be included if needed. Do not implement allocations/AddItem/InsertItem with unknown item full layout; actual renderer remains unimplemented. Whole DLL incomplete.


## ListBox selection, removal and rectangle operations

GUI-listbox-ops acceptance adds909 unique exact bytes, reaching158,957 /930,756 (17.07827%). Seven full methods plus three typed array providers: SetTrackRange33 at88C50,UpdateRects256 at88C80,GetSelectedIndex85 at88E70,RemoveItem113 at88D80,SelectItem134 at8A9F0,GetItem33 at8A9C0,RemoveAllItems73 at8C7F0,Remove68 at868F0,SetSize5 at89B40,SetSizeInternal142 at86C60.942 code bytes across new regions,909 unique after prior-proof overlap.1868 whole regions, actual original/linked targets and PE relocations,10 negative controls and existing ABI pass. No new imports/data/pending; eight old pending unchanged.

Same partial R5 layout views retained, no whole-item allocation/layout claim. Unsized delete providers match real targets. Initial listbox-only extraction GetSelectedIndex/RemoveItem/SelectItem/GetItem/RemoveAllItems matched; SetTrackRange41 vs33 and UpdateRects266 vs256 did not. R5 UpdateRects calls accepted CDXUTDialog::GetFont870C0 instead of direct GetManager()->GetFontNode. Moved UpdateRects/SetTrackRange to controls unit containing actual Cap/UpdateThumbRect definitions; complete original code and register preservation now match. Unselected SetPageSize23 not credited; UpdateRects uses exact compiled inline operations. No assembly or byte patches. Only changed controls/listbox units recompiled. All seven source bodies identical to0.2.5/base; R5 differences not provided, no direct transfer credit, total1,523 unchanged.

Current accepted link cp32-gui-listbox-ops-linked; Ob1 cp32-gui-listbox-ops-context-ob1, Ob2 cp32-ime-ob2. Full units build/gui-listbox-units.txt. Generators: prepare_gui_listbox.py provides adapted() and shared layout_header(); prepare_gui_controls.py now imports those and materializes UpdateRects plus original SetTrackRange. Run both when shared view changes. No global header changes. Contract/seeds gui-listbox-ops, evidence acceptance/integration gui-listbox-ops, review build/review_gui_listbox_ops.py. All processes terminal; no input/GUI/game execution, push/deployment.

Next: ListBox allocation/renderer still lacks complete R5 item layout, do not use old small source Render or guessed sizeof. Prioritize other coherent source families: SDK TextHelper two DrawTextLine379/361 and formatted wrappers (original DXUTmisc.cpp), REF warning856 with actual dialog callback, remaining GUI dialog lifecycle/focus/edit rendering. Existing complete drawing/font/state providers now broad. Full-scan source bodies may differ ANSI vs Unicode, R5 events and layouts; verify actual targets. Whole DLL incomplete.


## Complete SDK TextHelper family

DXUT-text acceptance adds912 unique exact bytes, reaching159,869 /930,756 (17.17625%). Seven complete methods/constructor total1027, prior-proof overlaps deduplicated. Constructor99 at956B0,DrawTextLine379/361 at95720/958A0,Begin16 at95A10,End14 at95A20,formatted wrappers71/87 at972D0/97320.1879 whole regions, actual targets/PE relocations,10 negative controls and existing ABI pass. Existing complete formatting/import/CRT helpers reused; no new imports/data or incomplete providers, eight old pending unchanged. Only new text unit compiled once, one link, no source adaptations.

End14 has byte-identical alternate6BC80. Chosen95A20 in contiguous independently matched SDK TextHelper block and actual sprite pointer+4/virtual End operation agree with constructor/Begin. No direct caller found; record this attribution basis rather than claiming call-site proof. Only95A20 credited here, alternate excluded. Full source block identical to0.2.5 and already in base, no direct transfer credit; total1,523 unchanged. No text drawing, GUI, graphics or game execution; no push/deployment.

Current accepted link cp32-dxut-text-linked; Ob1 cp32-dxut-text-ob1, Ob2 cp32-ime-ob2. Full units build/dxut-text-units.txt. Generator prepare_dxut_text.py; contract/seeds dxut-text; evidence acceptance/integration dxut-text; review build/review_dxut_text.py. All processes terminal.

Next: SDK REF warning family in DXUTmisc.cpp lines42..~150 (DXUTDisplaySwitchingToREFWarning856 candidate963E0 and complete DisplaySwitchToREFWarningProc344 with attached table). Real source creates stack dialog template and uses RegOpenKeyEx/RegQueryValueEx/RegSetValueEx/RegCloseKey, so actual advapi32.lib dependency likely needed in linker/pinned SDK metadata; never execute dialog or registry APIs. Existing DXUT state/window/title/shutdown providers accepted. Source uses ANSI TCHAR and mixed API behavior: preserve actual original bytes/targets, do not fix apparent template encoding without evidence. Alternatively remaining GUI lifecycle/edit rendering or renewed0.2.5 source gaps; prioritize larger coherent sets. Whole DLL incomplete.


## Complete SDK REF warning and dialog callback

DXUT-ref-warning acceptance adds1,173 unique exact bytes, reaching161,042 /930,756 (17.30228%). Warning856 at963E0 and actual callback344 at933E0 (317 code plus27 attached table). Entire callback table and stack dialog template initializer in emitted warning code verified; table excluded from code coverage. Seven full source strings checked.1888 whole regions, every actual target/PE relocation,10 negative controls and existing ABI pass.13 real user32/advapi32 imports reviewed, including4 registry APIs. advapi32.lib added to tools/link_actor_closure.ps1 link command and SDK hash metadata; existing SDK hashes unchanged. Actual callback target933E0 verified, no fake provider.

Only new warning unit compiled once and linked once, no source adaptations. Both complete bodies identical to0.2.5/base, no direct transfer credit; total1,523 unchanged. Original ANSI APIs and stack dialog template preserved despite apparent encoding oddity; no functional cleanup permitted by byte-match objective. No dialog, registry, graphics or game execution. Eight old pending unchanged, no new incomplete provider. Local commit only, no push/deployment.

Current accepted link cp32-dxut-ref-warning-linked; Ob1 cp32-dxut-ref-warning-ob1, Ob2 cp32-ime-ob2. Full units build/dxut-ref-warning-units.txt. Generator prepare_dxut_ref_warning.py; contract/seeds dxut-ref-warning; evidence acceptance/integration dxut-ref-warning; review build/review_dxut_ref_warning.py. All processes terminal.

Next: remaining larger GUI dialog lifecycle/render and edit/IME methods or renewed source0.2.5 missing subsystem bodies; SDK misc families largely closed. Full GUI scan has remaining EditBox Render846, dialog OnRender899/MsgProc1140, IME MsgProc2428 etc, some source variants differ from R5 and need actual layout/target review. Existing events/fonts/window/timer/buffer dependencies now broad. Current acceptance always re-runs same actor ABI and10 negative controls even for SDK-only additions (~minute); if optimizing workflow, preserve fresh full gate for every batch and only reuse checks with explicit immutable evidence of unchanged verification logic and relevant providers. Do not merely skip or report reused checks as freshly run. Whole DLL incomplete.


## Complete dialog rendering, refresh and default focus

GUI-dialog-render acceptance adds1,125 unique exact bytes, reaching162,167 /930,756 (17.42315%). OnRender899 at908E0,FocusDefaultControl60 at87960,ControlRefresh43 at87AE0,DialogRefresh123 at89CD0, complete time8 at142778 and pressed-pointer4 at142784 zero owners.1894 whole regions, all actual targets/PE relocations,10 negative controls and existing ABI pass. No new imports/pending; eight old pending unchanged. Actual Refresh call identifies render function908E0.

Measured0.2.5 trial: OnRender source differs sprite Begin ordering and enables minimized title suffix; compiled963 bytes has no full candidate. Base old full scan899 has29 diagnostic byte differences at caption construction/arguments. R5 caption DrawText shadow=false versus base true. Isolated base+false extraction886 still differs register/constant propagation across actual GetTexture/DrawSprite/DrawText calls. Moving OnRender to same drawing unit with those definitions yields complete899 and all actual targets. Source behavior otherwise preserved, no assembly or copied bytes. Initial nearby-address diagnostic89E80 was wrong (message-handler-shaped function830, not render); discarded and located correct908E0 by actual direct Refresh call. No wrong candidate credited.

Final source based on base plus reviewed false caption parameter and original TU context.0.2.5 did not accelerate this function; no direct transfer credit, total1,523 unchanged. Only changed/new units compiled across trials; all failed snapshots/logs retained. No dialog, graphics or game execution; no push/deployment.

Current accepted link cp32-gui-dialog-render-linked; Ob1 cp32-gui-dialog-render-context-ob1, Ob2 cp32-ime-ob2. Full units build/gui-dialog-render-units.txt. Generator prepare_gui_dialog_render.py writes actual Refresh/FocusDefault/ControlRefresh and two separate static stores; render_body() imported by prepare_gui_drawing.py, which owns OnRender beside verified drawing providers. Run both generators for shared changes. Contract/seeds gui-dialog-render; evidence acceptance/integration gui-dialog-render; review build/review_gui_dialog_render.py. All processes terminal.

Next: remaining dialog MsgProc and focus/hover helpers can reuse current event/render/refresh/window providers. Original function89E80 has830 code and message-handler signature ret16; confirm identity/full attached tables before using. Source MsgProc1140 differs substantially;0.2.5 only whitespace changes here, so actual R5 behavior needed. Alternatively EditBox Render846 still incomplete match, source/base0.2.5 identical with caret blinking commented out; determine actual original function by calls/provider identities rather than adjacency. Larger source0.2.5 missing subsystem bodies remain priorities where useful. Whole DLL incomplete.


## Complete dialog message routing and focus/hover family

GUI-messages acceptance adds1,030 unique exact bytes, reaching163,197 /930,756 (17.53381%). MsgProc full880 at89E80 includes830 code and50 attached table bytes, all verified and table excluded from credit. GetControlAtPoint113 at87120,OnMouseMove58 at871A0,GetNextControl68 at872B0,GetPrevControl57 at87300,OnCycleFocus142 at879A0.1268 code bytes across new regions,1030 unique after prior-proof overlap.1900 whole regions, actual targets/PE relocations,10 negative controls and existing ABI pass. One actual new user32 GetAsyncKeyState import checked. No new data or pending providers, eight old pending unchanged.

R5 original MsgProc entry verified by actual automation call7DA80, visible/minimized checks and source message dispatch. Base/0.2.5 has caption drag/minimize click block absent from R5 and mouse OutputDebugString trace absent from R5. Removed exactly those in normal C++, retaining caption coordinate subtraction and all other routing. Full880 matches first compile, all5 original helpers match. Five helper bodies identical to0.2.5, MsgProc only whitespace/comment differs; no direct transfer credit, total1,523 unchanged. No stub/copy/masking. Only new message unit compiled once, one link; no message, input, GUI or game execution, push/deployment.

Current accepted link cp32-gui-messages-linked; Ob1 cp32-gui-messages-ob1, Ob2 cp32-ime-ob2. Full units build/gui-messages-units.txt. Generator prepare_gui_messages.py; contract/seeds gui-messages; evidence acceptance/integration gui-messages; review build/review_gui_messages.py. Full prior drawing/render/control generators unchanged. All processes terminal.

Next: GUI dialog lifecycle ctor206 at8CB60/dtor203 at8CC30 (original full-scan candidates) may now close through broad drawing/event/resource foundations. Inspect actual RemoveAllControls/RemoveDialog/RegisterDialog providers before counting. EditBox Render846 and MsgProc565 remain source variants with R5-specific behavior; source0.2.5 mostly same. Larger missing0.2.5 subsystem bodies remain priorities where actual added implementation offers gain; avoid guessing full ListBox allocation/renderer layout. Whole DLL incomplete.


## Complete dialog lifecycle and removal

GUI-lifecycle acceptance adds967 unique exact code bytes, reaching164,164 /930,756 (17.637705%). Constructor206 at8CB60, destructor203 at8CC30, RemoveControl130 at86EF0, RemoveAllControls126 at8C5D0; complete typed array providers and66-byte compiler EH funclet region atE3BF0,60-byte compiler unwind map/FuncInfo atFBAF0.1917 whole regions, full actual targets/PE relocations,10 negative controls and existing ABI pass. EH data excluded from code credit; real exception unwinding not executed. Eight old pending unchanged, no new imports/unimplemented providers.

All four bodies identical in pinned base/local0.2.5, no adaptations or direct transfer credit (total1,523 unchanged). New lifecycle unit compiled once. Initial link omitted unexported roots and acceptance correctly rejected unresolved regions; added four real capsule exports, relinked only, complete gate then passed. First failed link/log preserved. No GUI/game execution, push or deployment.

Current accepted link cp32-gui-lifecycle-linked2; Ob1 cp32-gui-lifecycle-ob1, Ob2 cp32-ime-ob2. Full units build/gui-lifecycle-units.txt; generator tools/prepare_gui_lifecycle.py; contract/seeds gui-lifecycle; review build/review_gui_lifecycle.py; acceptance/integration gui-lifecycle. All processes terminal. Local0.2.5 still prioritized where it provides missing implementations; recent SDK bodies identical to base.

Next candidates confirmed by existing full scan (discovery only): Control SetElement147 at8A2D0 calls actual element-array Add89C10; EditBox SetTextFloatArray230 at8B100 uses ANSI formatter/StringCopyWorker/SetText, check actual R5 wide buffer contract before transfer; IME GetReadingWindowOrientation349 at8B1F0 needs horizontal-reading static and actual registry API identities/strings. Broader missing0.2.5 subsystem implementations remain useful alternatives. No matches credited for these candidates. Whole DLL remains incomplete.


## Complete GUI element setter, numeric formatting and IME orientation

GUI-elements acceptance adds908 unique exact code bytes, reaching165,072 /930,756 (17.73526%). SetElement147 at8A2D0, SetTextFloatArray230 at8B100, GetReadingWindowOrientation349 at8B1F0, actual element-array Add40 at89C10 and SetSizeInternal142 at86BD0. Five full source strings and one-byte horizontal-reading state at14252A verified, excluded from code credit. 1931 whole regions, all actual targets/PE relocations,10 negative controls and existing ABI pass. No new imports/unimplemented providers; eight old pending unchanged.

All three bodies identical to pinned base/local0.2.5, no adaptation/direct transfer credit; total1,523 unchanged. Original numeric ANSI formatting preserved and calls actual accepted ANSI SetText; no speculative conversion to wide strings. IME source compatibility defines preserved. Registry imports already accepted and reused; no registry/GUI/game execution. Only two new units compiled,147 reused, one link. Snapshot preparation was slow (~minutes) but observed live PowerShell/cl and terminal success; no duplicate launch. No push/deployment.

Current accepted link cp32-gui-elements-linked; Ob1 cp32-gui-elements-ob1, Ob2 cp32-ime-ob2; full units build/gui-elements-units.txt. Generator tools/prepare_gui_elements.py, review build/review_gui_elements.py, contract/seeds and acceptance/integration gui-elements. All processes terminal.

Next full-scan discovery-only candidates: DialogResourceManager destructor181 at8CD20, ComboBox AddItem196 at8A6A0 and FindItem126 at881D0, ParseFloatArray171 at89580, EditBox/IME constructors256 at8F280/8F8F0 and their complete vtable closure (Render/MsgProc unresolved). ScrollBar constructor166 at8ECD0 may now close using accepted input/render providers, then dialog AddSlider165 at90F20 etc. D3DXCOLOR conversion317 at84390 is a candidate but check actual prior credit/identity. Continue broader0.2.5 missing bodies when actual source adds information; SDK unchanged transfers no longer provide a unique advantage. Whole DLL incomplete.


## Complete control and scrollbar lifetime / virtual tables

GUI-control-lifetime acceptance adds474 unique code bytes, reaching165,546 /930,756 (17.78619%). Control constructor80 at8E240, destructor66 at8E380, ScrollBar constructor166 at8ECD0, destructor11 at8ED80, SetTextColor19 at87AC0, full compiler deleting destructors30 each at91220/913D0 and original inline virtual defaults plus array SetSize5 at89B00.503 total new-region code bytes,474 unique after overlap. Complete80-byte vtables atEB338/EB528,40 actual slot targets resolved; data excluded from code coverage. 1959 whole regions, full target/PE relocation check,10 negative controls and existing actor ABI pass. Eight old pending unchanged, no new imports/provider gaps.

Five unchanged original bodies identical to0.2.5/base; direct transfer credit unchanged1,523. Source normal C++, original no-op/default virtual bodies verified by actual constructor table identities, not fabricated stubs. Compiler weak vector/scalar deleting destructor aliases explicitly checked. Base destructor isolated compile66 matches, prior fullGUI114 does not; no semantic edits, compiler context is relevant. Initial link reported missing SetTextColor and ScrollBar destructor; added actual definitions, only lifetime unit recompiled and link2 passed. Failed artifacts retained. No GUI/game execution, push/deployment.

Current accepted link cp32-gui-control-lifetime-linked2; Ob1 cp32-gui-control-lifetime-ob1b, Ob2 cp32-ime-ob2. Full units build/gui-control-lifetime-units.txt. Generator tools/prepare_gui_control_lifetime.py, review build/review_gui_control_lifetime.py, contract/seeds and acceptance/integration gui-control-lifetime. All processes terminal.

Next: close constructor families now that base control and scroll dependencies are complete. Full-scan candidates Button50 at8E4A0, CheckBox43 at8E6A0, RadioButton36 at8E800, Slider65 at8EC60 and AddSlider165 at90F20. Actual full virtual tables must determine tiny method/destructor identities; do not select arbitrary identical short-body candidates (Slider dtor has many matches). Static/Button render providers may need additional closure. ResourceManager destructor181 at8CD20 requires CUniBuffer::Uninitialize original placeholders. Broader0.2.5 missing implementation families remain alternatives; no speculative coverage. Whole DLL incomplete.


## Complete static/button/check/radio/slider constructors and virtual closure

GUI-widget-lifetime acceptance adds760 unique exact bytes, reaching166,306 /930,756 (17.86784%). Static constructor96 at8E3D0, Button50 at8E4A0, CheckBox43 at8E6A0, Radio36 at8E800, Slider65 at8EC60. Five complete tables atEB388/EB3D8/EB428/EB480/EB4D8, sizes80/80/84/88/80, all103 actual slot targets.24 full new code regions760 bytes include original inline virtual focus/hotkey/point methods and compiler deleting destructors. Actual tiny destructor targets recovered through deleting-destructor calls, not arbitrary identical candidates. 2004 whole regions, all original/linked targets and PE relocations,10 negative controls and existing actor ABI pass. Eight old pending unchanged, no new imports/provider gaps.

Six of seven extracted bodies identical to0.2.5/base. Slider existing field_76=false absent0.2.5 and required by complete65-byte R5 constructor. No changes to current base bodies, no direct0.2.5 credit (total1,523 unchanged). First link missing Slider/CheckBox ContainsPoint; added complete original bodies, only widget unit recompiled and link2 passed. Weak vector/scalar deleting aliases explicitly checked for all five types. No GUI/game execution, push/deployment. All failed snapshots retained.

Current accepted link cp32-gui-widget-lifetime-linked2; Ob1 cp32-gui-widget-lifetime-ob1b, Ob2 cp32-ime-ob2; units build/gui-widget-lifetime-units.txt. Generator tools/prepare_gui_widget_lifetime.py; review build/review_gui_widget_lifetime.py; contract/seeds and acceptance/integration gui-widget-lifetime. All processes terminal.

Next: AddSlider165 at90F20 now has constructor closure, needs original AddControl47 at8C6C0, InitControl98 at8C650 and actual SetRange88620 (inspect size/source). Full-scan AddStatic216/AddButton223/AddCheckBox237/AddRadio261 have no candidates even attached-data mode; inspect R5 allocation size and source/TU context before accepting. These factories may unlock app-level GUI initialization. Other GUI/0.2.5 families remain open. Whole DLL incomplete.


## Complete dialog control factories and shared initialization

GUI-factories acceptance adds1,041 unique exact bytes, reaching167,347 /930,756 (17.979685%). AddSlider165 at90F20, AddStatic146 at90C70, AddButton153 at90D10, AddCheckBox167 at90DB0, AddRadioButton191 at90E60; SetRange34 at88620, InitControl98 at8C650, AddControl47 at8C6C0, StaticSetText69 at87B40, control-array Add40 at89A10.1110 new-region code bytes,1041 unique after prior overlap. 1999 full regions, every relocation and actual original/linked target,10 negative controls and existing ABI pass. No new pending/data/imports; eight old pending unchanged.

Nine complete factory/shared/SetText bodies identical to0.2.5/base; no semantic changes/direct transfer credit, total1,523 unchanged. Initial isolated factories sizes235/216/223/237/261 fail; placing derived constructors in same TU alone unchanged. Including full base-control constructors/destructors in same TU yields exact165/146/153/167/191 (70 fewer bytes each), consistent with removal of compiler exception overhead after complete constructor knowledge. No flags/stubs/masks used. Full previous-region hashes/fixup targets preserved after owner migration. First link missing StaticSetText; complete original69-byte provider added to separate helpers TU, incremental compile/link2 passes. All failed snapshots retained.

IMPORTANT current ownership: prepare_gui_widget_lifetime.py imports BASE_NAMES from prepare_gui_control_lifetime.py and emits all base/scroll lifetime definitions followed by derived constructors and factories. Do NOT link standalone closure_gui_control_lifetime anymore (file retained for provenance); its seeds migrated into closure_gui_widget_lifetime. Run widget generator if BASE_NAMES changes. prepare_gui_factories.py emits InitControl/AddControl/SetRange/StaticSetText only. Current full units build/gui-factories-units.txt excludes standalone control unit.

Current accepted link cp32-gui-factories-linked2; Ob1 cp32-gui-factories-text-ob1, Ob2 cp32-ime-ob2. Contract/seeds gui-factories; review build/review_gui_factories.py; acceptance/integration gui-factories. All processes terminal. No GUI/game execution, push/deployment.

Next: GUI factory foundations can unblock app-level dialog initialization. Remaining ComboBox/ListBox/EditBox allocation paths need verified full layouts and virtual closure; don't fabricate unknown layout. Resource manager destruction/UniBuffer Uninitialize and ComboBox items or ParseFloatArray remain candidates. Broader missing0.2.5 implementation families still worth prioritizing when they add actual source information. Whole DLL incomplete.


## Complete ComboBox item ownership, lookup and selection

GUI-combo-items acceptance adds947 unique exact bytes, reaching168,294 /930,756 (18.08143%).12 complete original methods include AddItem196 at8A6A0, RemoveItem92 at8A770, RemoveAllItems95 at8C790, FindItem126 at881D0, ContainsItem28 at8A7D0, selected data/item29/23 at88250/88270, GetItemData string/index49/39 at88290/882D0, select index/text/data53/78/101 at88300/88340/88390. Four complete actual typed array providers Remove68 at86940, Add40 at89BF0, SetSize5 at89BE0, SetSizeInternal142 at86CF0.1164 new-region code bytes,947 unique after prior overlap. 2015 whole regions, all actual target/PE relocations,10 negative controls and existing ABI pass. No new pending/data/imports; eight old pending unchanged.

All12 source bodies identical to0.2.5/base, including both GetItemData overloads. No adaptations/direct transfer credit, total1,523 unchanged. Original AddItem allocates0x115 bytes and zeroes0x45 dwords+1 byte: packed ComboBoxItem text256+data pointer4+RECT16+bool1. This verifies actual used allocation layout, separate from unresolved ListBox item layout. One new unit compiled once, one link. Initial source-comparison script ambiguous overload fixed by explicit overload index; failed early command log retained. No GUI/game execution, push/deployment.

Current accepted link cp32-gui-combo-items-linked; Ob1 cp32-gui-combo-items-ob1b, Ob2 cp32-ime-ob2; full units build/gui-combo-items-units.txt. Generator tools/prepare_gui_combo_items.py; review build/review_gui_combo_items.py; contract/seeds and acceptance/integration gui-combo-items. Retain previous owner migration: standalone closure_gui_control_lifetime excluded, base definitions owned by closure_gui_widget_lifetime. All processes terminal.

Next: ComboBox constructor initializes embedded scrollbar (already complete), destructor calls now accepted RemoveAllItems. Add constructor/destructor and actual virtual-table providers (OnFocusOut,OnHotkey,SetTextColor) using complete source context; full R5 layout still require exact constructor/table comparison. Then dialog AddComboBox may close and broaden app GUI init. Remaining IME/EditBox or resource teardown and broader missing0.2.5 sources still open. Whole DLL incomplete.


## Complete ComboBox lifetime, factory and virtual table

GUI-combo-lifetime adds464 unique exact bytes, reaching168,758 /930,756 (18.13128%). Constructor112 at912D0, destructor46 at91370, AddComboBox141 at91450, OnFocusOut12 at851A0, SetTextColor32 at87D40, OnHotkey96 at88170, deleting destructor30 at914E0, OnInit16 at91360, CanHaveFocus23 at91340.508 new-region code bytes,464 unique after overlap. Complete80-byte vtable atEB678 and20 actual slots; compiler weak alias checked. 2025 whole regions, all full original/linked target/relocation checks,10 negative controls and existing ABI pass. Eight old pending unchanged, no new imports/unimplemented providers.

Six original bodies identical to0.2.5/base; no semantic changes or direct transfer credit (total1,523 unchanged). Destructor trials95 with lifetime context,108 with item context alone,95 with both; full known RemoveAllItems->SetTrackRange->Cap/UpdateThumbRect chain yields46. This removes compiler exception overhead with source visibility alone. First narrow track trial lacked RectHeight definition, fixed original helper; preserve-caller trial lacked RectWidth, fixed original helper. Failed objects/snapshots/logs preserved. Moving track helpers out of controls TU changed ComboRender,ComboUpdateRects,Scroll,ListBoxUpdateRects; moving those callers alongside helpers restores all old full hashes/target sequences. No arbitrary masking, annotations or source behavior substitutions.

IMPORTANT ownership: closure_gui_widget_lifetime now owns full original ComboBox item block and destructor; standalone closure_gui_combo_items excluded from current units/seeds (old generator/file retained as experiment/provenance). Also widget owns Cap,UpdateThumbRect,SetTrackRange,ComboBoxRender,ComboBoxUpdateRects,Scroll and adapted ListBoxUpdateRects, removed from closure_gui_controls generator. Widget imports layout_header/adapted from prepare_gui_listbox; run widget as well if these change. Existing standalone closure_gui_control_lifetime remains excluded. Two affected consumer generators must stay coordinated; avoid reintroducing duplicate strong definitions.

Current accepted link cp32-gui-combo-lifetime-linked3; Ob1 cp32-gui-combo-lifetime-preserve2-ob1, Ob2 cp32-ime-ob2. Full units build/gui-combo-lifetime-units.txt. Review build/review_gui_combo_lifetime.py; contract/seeds and acceptance/integration gui-combo-lifetime. All processes terminal. No GUI/game execution, push/deployment.

Next: complete GUI control families now broadly available; prioritize app-level dialog initialization or resource manager destruction/UniBuffer Uninitialize and remaining EditBox/IME matching. ListBox allocations still need unknown layout, no speculation. Broader missing0.2.5 implementations remain candidates where they add actual information. Whole DLL incomplete.


## Complete wide edit-buffer numeric parser

GUI-parse-float adds171 unique exact bytes, reaching168,929 /930,756 (18.14965%). Entire ParseFloatArray171 at89580 matches with existing StringCopyWorkerW and actual new CRT wcstod provider. Original wcstod379-byte inventory chunk atCABF0 and full hash checked, linked provider LIBCMT:wcstod.obj at1002C0CE, CRT excluded from reconstructed code coverage. 2027 whole regions, all original/linked relocations/targets,10 negative controls and existing ABI pass. Eight old pending unchanged.

Source0.2.5 uses TCHAR/StringCchCopy/strtod, while pinned base already uses WCHAR/StringCchCopyW/wcstod matching actual R5 wide buffer. No adaptation to base and no direct0.2.5 transfer credit (total1,523 unchanged). One new unit compiled once, one link. No GUI/game execution, push/deployment.

Current accepted link cp32-gui-parse-float-linked; Ob1 cp32-gui-parse-float-ob1, Ob2 cp32-ime-ob2; units build/gui-parse-float-units.txt. Generator tools/prepare_gui_parse_float.py; review build/review_gui_parse_float.py; contract/seeds and acceptance/integration gui-parse-float. Previous owner migrations preserved. All processes terminal.

Next concrete app-level lead: client/saco/unkclass4.cpp CUnkClass4::ResetDialogControls creates3 sliders and sets colors; AddSlider now accepted. Compile constructor/reset with existing base headers and check full R5 candidates; GetSlider/SetColor dependencies likely tractable. unkclass3 ResetDialogControls still requires actual ListBox and IME constructors, avoid premature coverage. Resource-manager teardown and remaining EditBox/IME sources still open. Whole DLL incomplete.


## App-level three-slider dialog initialization

GUI-slider-panel adds420 unique exact bytes, reaching169,349 /930,756 (18.19478%). CUnkClass4 constructor54 at71660, ResetDialogControls366 at716A0, actual GetControl(ID)46 at87240 and GetControl(ID,type)60 at87270.526 new-region bytes,420 unique after prior overlap. 2031 whole regions, all original/linked targets/PE relocations,10 negative controls and existing actor ABI pass. Eight old pending unchanged; no new import/data/provider gaps. Full setup includes three AddSlider calls and inline GetSlider/SetColor with exact original float constants.

No counterpart located in local0.2.5 by filename or distinctive calls/colors; no direct transfer credit (total1,523 unchanged). Both lookup overloads identical to0.2.5/base. Existing unkclass4.cpp/header unchanged, no fabricated members in unknown gap. Initial link missing typed GetControl, original overloads added in new closure_gui_lookup unit. Only these two new units compiled. Review script now preserves previous explicit single-chunk wcstod proof in addition to usual discontiguous CRT chunks; stale review failures retained, no gate weakening. No GUI/game execution, push/deployment.

Current accepted link cp32-gui-slider-panel-linked2; Ob1 cp32-gui-slider-panel-lookup-ob1, Ob2 cp32-ime-ob2; units build/gui-slider-panel-units.txt. New generator tools/prepare_gui_lookup.py; class uses existing client/saco/unkclass4.cpp. Contract/seeds gui-slider-panel, review build/review_gui_slider_panel.py, acceptance/integration gui-slider-panel. All processes terminal. Prior widget/control/combo owner migrations unchanged.

Next: broader app-level GUI sources now viable; unkclass3 reset still gated by actual ListBox/IME constructors/layout. Evaluate resource-manager teardown/UniBuffer Uninitialize or remaining EditBox Render/MsgProc against R5. Wider0.2.5 missing implementations remain prioritized when available. Whole DLL incomplete.


## Complete R5 EditBox rendering including password display

GUI-edit-render adds1,253 unique exact bytes, reaching170,602 /930,756 (18.32940%). Entire Render1253 at8F400 identified by actual GetFont870C0/PlaceCaret85470/CPtoX9BCF0/drawing calls. Original field126 branch splits plain/password rendering; two actual MaskPasswordString85920 calls wrap text substrings in password path. Base and0.2.5 render bodies identical846 and omit that branch. Added normal C++ conditional, duplicating original text/selection block and wrapping both DrawText strings; all other logic unchanged, including disabled blink and visible/enabled caret condition. Full1253 matches first compile.

2033 whole regions, all original/linked target/PE relocation checks,10 negative controls and existing actor ABI pass. New actual user32 IntersectRect import identity checked. No new pending/data; eight old pending unchanged. No direct0.2.5 transfer credit (total1,523 unchanged). No GUI/game execution, push/deployment. R5 diagnostic disassembly retained build/edit-render-r5.asm.

Current accepted link cp32-gui-edit-render-linked; Ob1 cp32-gui-edit-render-ob1, Ob2 cp32-ime-ob2; units build/gui-edit-render-units.txt. Generator tools/prepare_gui_edit_render.py; review build/review_gui_edit_render.py; contract/seeds and acceptance/integration gui-edit-render. All processes terminal; previous owner migrations unchanged.

Next: EditBox MsgProc original at89320 has545 inventory code bytes (source565 may include attached tables); identified by actual PlaceCaret/InsertChar calls. Base body has unfinished overwrite else branch and does not use bPlaceCaret after insert, suspicious cast-direction logic; recover actual behavior from R5, do not fix by intuition. Actual InsertChar WCHAR at9BA10 and CHAR at9BAA0 still need complete providers; source fullscan candidates143/175. Constructor256 at8F280 now closer to full virtual closure once MsgProc is accepted. IME and ListBox remaining gaps still excluded. Whole DLL incomplete.


## Complete wide and DBCS InsertChar

GUI-insert-chars adds318 unique exact bytes, reaching170,920 /930,756 (18.36357%). Full WCHAR overload143 at9BA10 and CHAR/DBCS overload175 at9BAA0 accepted. 2035 whole regions, all actual target/PE relocation checks,10 negative controls and existing actor ABI pass. New kernel32 IsDBCSLeadByteEx import checked. Eight old pending unchanged. Original base bodies unchanged, restore original DXUT_MAX_EDITBOXLENGTH0xFFFF macro after initial missing-define compile failure.0.2.5 only one InsertChar definition, no direct transfer credit (total1,523 unchanged). No GUI/game execution, push/deployment.

Current accepted link cp32-gui-insert-chars-linked; Ob1 cp32-gui-insert-chars-ob1b, Ob2 cp32-ime-ob2; units build/gui-insert-chars-units.txt. Generator tools/prepare_gui_insert_chars.py; review build/review_gui_insert_chars.py; contract/seeds and acceptance/integration gui-insert-chars. All processes terminal.

IMPORTANT next MsgProc evidence: original89320 code545 plus attached switch data. R5 default branch rejects strlen(GetTextA())>=128; overwrite WCHAR>255 uses actual operator[]9B9F0 and updatescaret. Overwrite byte<=255 calls missing method9C170; insert WCHAR>255 calls9BA10, else9BAA0 (base source condition reversed). Both bool-returning paths test result then advancecaret and synchronize selection. Original9C170 full234 code, debug capture build/edit-overwrite-r5.asm. It resembles CHAR InsertChar DBCS conversion, BUT single-byte path constructs temporary CUniBuffer via9C130 with converted WCHAR as constructor size, copies19 bytes to this+nIndex*19, destroys temp via9B9C0! Both actual ctor53 and dtor33 already accepted. Inference to test: typo-style `this[nIndex] = WideCharStr[0]` invoking implicit CUniBuffer(int)/assignment, not ordinary character replacement. Preserve actual behavior, do not silently fix to buffer assignment. DBCS-pending path still calls wide InsertChar9BA10 and clears field8. Missing method not declared in current header; adding method declaration must preserve layout and compile all affected consumers. Alternative explicit typed source view needs rigorous ABI identity, no fabricated shortcut. No bytes credited for this helper or MsgProc yet. Main MsgProc capture build/edit-msg-r5.asm. Whole DLL incomplete.


## Recovered exact byte-overwrite helper

GUI-overwrite adds234 unique bytes, reaching171,154 /930,756 (18.38871%). Complete helper9C170 exact first compile when final single-byte branch uses `this[nIndex] = WideCharStr[0]`: implicit CUniBuffer(int) temporary construction at9C130,19-byte object assignment indexed on this, temporary destruction9B9C0. Pending-DBCS branch still calls actual InsertChar(WCHAR)9BA10. Descriptive recovered name OverwriteChar; original symbol name unknown. No behavioral correction of apparent source bug. Nonvirtual method declaration added to vendor/upstream/saco/d3d9/common/DXUTgui.h, no layout change. Generator tools/prepare_gui_overwrite.py reproduces declaration/body.

2036 whole regions, full actual target/PE relocations,10 negative controls and existing actor ABI pass. All affected header consumers recompiled in Ob1 and Ob2, previous complete hashes/fixup target sequences preserved. Eight old pending unchanged, no new import/data/provider gaps. No direct0.2.5 transfer credit (total1,523 unchanged). No GUI/game execution, push/deployment.

Current accepted link cp32-gui-overwrite-linked; Ob1 cp32-gui-overwrite-ob1, **Ob2 cp32-gui-overwrite-ob2** (old cp32-ime-ob2 has stale header). Units build/gui-overwrite-units.txt. Contract/seeds and acceptance/integration gui-overwrite; review build/review_gui_overwrite.py. All processes terminal.

UNACCEPTED MsgProc work preserved: tools/prepare_gui_edit_messages.py/client/saco/closure_gui_edit_messages.cpp not in accepted units/seeds. Corrected R5 branches: switch/cut comparisons WCHAR (base TCHAR wrong), wide insert when>255 elseCHAR, byte overwrite calls recovered helper, bool result controls caret advance, directwide overwrite advancescaret. First duplicated-if form609 bytes, shared initializedbool605; adding both InsertChar definitions sameTU did not change605; WCHAR switch fixes register usage but remains605; ternaryinsert latesttrial5 still605. Full original code545 at89320 with attached tables. Latest candidate snapshot cp32-gui-edit-messages-trial5, discovery-only NOT ACCEPTED. Current generator emits both InsertChar definitions at end; don't link alongside standalone insertion unit without owner migration.

Diagnostic sequence comparison trial5 to original shows all significant instruction differences concentrated around insert call argument loading (at original offsets1A1..1BF): R5 loads m_nCaret once before conditional jump and shares push/lea this, candidate loads it separately in branches, leading to8 extra total bytes/register differences. Hypothesis next: explicit local index before insert condition may reproduce shared load; alternatively function-pointer/type/code context. Preserve complete byte checks, no masks. Old overwritten trial disassembly file build/edit-msg-trial.asm corresponds trial1, regenerate forlatest if needed. Do not count MsgProc until exact including attachedtables/dependencies. Currenthelper acceptance is real progress but fullgoal remains active.


## Bounded EditBox MsgProc argument organization experiment (no acceptance)

Trial6 explicit local caret index changes candidate605 to full597, equal to R5 code545+attached52. Still no exact match. Trial7 if/else vs ternary, trial8 removes both InsertChar definitions from candidate TU, trial9 declares index beforebool: all597, all fail. Three additional pointer/reference/character-local variants also597 and fail. Diagnostic disassembly shows register choices only after operand substitutions; this is NOT byte matching and zero bytes credited. Full experiment evidence edit-message-argument-experiment.json explicitly distinguishes diagnostics from acceptance. No accepted units/contracts/source changed; accepted coverage remains171,154 and latestlink cp32-gui-overwrite-linked (Ob1 cp32-gui-overwrite-ob1,Ob2 cp32-gui-overwrite-ob2).

Current tools/prepare_gui_edit_messages.py emits only MsgProc, no InsertChar duplicates anymore; local index declared beforebool, assigned insideinsert branch, both insert overloads useindex, WCHARswitch retained. Unaccepted source not in liveunits/seeds. Latest canonical candidate cp32-gui-edit-messages-trial9;3 ignored evalsources/snapshots cp32-gui-edit-messages-arg-trials. All processes terminal. No push/deployment. Avoid repeating these7 source-organization trials without new evidence; pivot to resource teardown or another larger open family. Whole goal remains active.


## Resource teardown closure accepted

Adds336 unique code bytes, reaching171,490 /930,756 (18.424807%). Uninitialize97 at9B750 and manager destructor181 at8CD20 compiled unchanged, both definitions identical in local0.2.5 and pinned base. No new direct0.2.5 credit (total1,523 unchanged). Actual original Dummy_Script functions and pointer stores already accepted; no fabricated placeholders. Complete32-byte EH regionE3C40 (handler+22),44-byte unwind/FuncInfoFBB2C (anchor+16), four array wrappers26 bytes accepted. Full gate,10 negative controls and actor ABI pass; eight pending unchanged. No GUI/game execution, push/deployment.

Current accepted link cp32-gui-resource-teardown-linked; Ob1 cp32-gui-resource-teardown-ob1, Ob2 cp32-gui-overwrite-ob2. Units build/gui-resource-teardown-units.txt; generator tools/prepare_gui_resource_teardown.py; review build/review_gui_resource_teardown.py. Contract/seeds/acceptance/integration gui-resource-teardown. All processes terminal. Next useful candidates resource manager constructor, OnCreateDevice, OnResetDevice, OnLostDevice, OnDestroyDevice (existing font/texture providers now closed). Preserve untracked three EditBox diagnostic eval sources; MsgProc still not accepted. Goal remains active.


## Resource device lifecycle closure accepted

Five complete unmodified SDK functions412 bytes: constructor31 at8CD00, OnCreateDevice97 at8CDE0, reset81 at86F80,lost82 at86FE0,destroy121 at87040. All five definitions identical to local0.2.5. Full union deduplication yields only97 NEW unique bytes;315 already covered by earlier proofs/configs. Coverage171,587 /930,756 (18.435229%). No direct0.2.5 credit (1,523 total unchanged).2058 regions, full gate,10 negative controls and actor ABI pass; eight pending unchanged. All actual CreateFontA/CreateTexture/D3DXCreateSprite/StaticOnCreateDevice providers already accepted.

Latest accepted link cp32-gui-resource-device-linked, Ob1 cp32-gui-resource-device-ob1,Ob2 cp32-gui-overwrite-ob2. Units build/gui-resource-device-units.txt; generator tools/prepare_gui_resource_device.py,review build/review_gui_resource_device.py. All processes terminal, no push/deployment. Three untracked prior EditBox eval files preserved.

Discovery prioritization MUST subtract full accepted union (initial configs manifest/checkpoint2/checkpoint31/checkpoint32 plus all previous proof code_ranges), not just latest contract regions. Fullscan candidate317 D3DXCOLOR conversion at84390,98 constructor715A0,73 D3DXColorLerp844D0,68 GetDefaultElement871F0,43 SetBackgroundColors84DA0,38 SetNextDialog84DD0 may already be globally covered; check first. EditBox constructors/factories still blocked on nonmatching MsgProc597; ListBox factory requires full layout. Consider larger non-GUI0.2.5 families after cheap global coverage filter. Goal remains active.


## SDK color conversions accepted; full-union discovery ranking

Adds415 unique bytes: D3DXCOLOR operator DWORD317 at84390 from closure_gui_drawing, DWORD ctor98 at715A0 from closure_gui_defaults. Both original SDK inline definitions identical to0.2.5 after newline normalization; no direct0.2.5 credit (1,523 total unchanged). No recompilation: unchanged cp32-gui-resource-device-ob1 and cp32-gui-overwrite-ob2 reused, only exports/link changed. Actual float constants and __ftol2 external CRT identity already accepted, CRT bytes excluded.2060 whole regions, full gate,10 negative controls and actor ABI pass,8 pending unchanged. Coverage172,002 /930,756 (18.479816%).

Current accepted run cp32-gui-color-conversion-linked; same units build/gui-resource-device-units.txt and Ob1/Ob2 as above. Contract/seeds/acceptance/integration gui-color-conversion; review build/review_gui_color_conversion.py. No source generation changes. All processes terminal, no push/deployment. Prior three untracked EditBox eval files preserved.

New tools/rank_uncovered_candidates.py --proof evidence/checkpoint32/gui-color-conversion-acceptance.json --discovery <discovery.json> --output <build/path.json> reproduces full union and checks proof/artifact hashes; subtracts only inventory code, excludes attacheddata. Ranking is potential only (ambiguous same-body matches must still resolve real identities). Initial scan artifacts build/gui-uncovered-ranked.json and build/broad-uncovered-ranked.json use previous proof (before415 accepted). Re-rank with latest proof. Remaining promising existing fullDXUT source candidates: xEnumDisplayMonitors316 at785D0,xMonitorFromPoint97 at783B0; DXUTSetShortcutKeySettings101 at7D3D0,LowLevelKeyboardProc51 at7D390, several state methods. Framework original scan build/cp32-dxut-framework-scan/discovery.json; authoritative source closure_dxut_state generated by prepare_dxut_full_state.py, do not reintroduce old partial units. No new meaningful unique game candidates from old vehicle/pool/http scans; these require further adaptations, not accepting ambiguous identical tiny bodies. Goal remains active.


## Additional multimon functions accepted

Adds413 unique code bytes: xMonitorFromPoint97 at783B0 and xEnumDisplayMonitors316 at785D0. Original pinned PlatformSDK multimon source unchanged for both; no direct0.2.5 credit (1,523 total remains). Existing full closure_dxut_state object reused, no compile. Dynamic API pointer stores and InitMultipleMonitorStubs already accepted. Two new actual GDI imports GetDCOrgEx/GetClipBox verified in original and linked IAT.2062 full regions, full gate,10 negative controls and actor ABI pass;8 pending unchanged. Coverage172,415 /930,756 (18.524% approximately; see exact coverage-current.json).

Current accepted run cp32-dxut-monitor-extra-linked, Ob1 cp32-gui-resource-device-ob1,Ob2 cp32-gui-overwrite-ob2; units build/gui-resource-device-units.txt. Contract/seeds/acceptance/integration dxut-monitor-extra; review build/review_dxut_monitor_extra.py. All processes terminal; no push/deployment/GUI execution; three prior EditBox untracked eval files preserved. Goal remains active.

Fresh full-state discovery: build/cp32-gui-color-conversion-linked/discovery-attached.json (closure_dxut_state only; originally this run had no other discovery). Potential next ranking build/dxut-next-shortlist.json from previous proof,18 unique candidates excluding newlyaccepted monitors. xGetSystemMetrics full92 at78350 includes66 inventorycode+26attachedtable and requires full comparison. DXUTSetShortcutKeySettings101 at7D3D0,LowLevelKeyboardProc51 at7D390,DXUTIsKeyDown75 at7F540,DXUTGetPresentParameters74 at7DB60 plus actual state method dependencies. Source definitions already compiled in fullDXUT context. Treat GetDXUTState candidate9A9C0 as ambiguous separate staticstate ownership, not another mainstate credit. Re-rank latest proof before choosing.


## DXUT input/settings closure accepted

Thirteen functions757 code bytes integrated,726 new after31 duplicate bytes removed. Coverage173,141 /930,756 (18.602190%). Includes systemmetrics66+26 attached table at78350; LowLevelKeyboardProc51,shortcut settings101,mouse down31,cursor settings35,present parameters74,key down75; keyboardhook getter53,four state setters240,mouse mapping31. Actual global DXUTState identity preserved; only new import CallNextHookEx verified original+linked IAT.2075 regions, full gate,10 negative controls and actor ABI pass;8 pending unchanged. Seven free DXUT definitions identical0.2.5; no new directcredit(total1,523). No recompilation; unchanged Ob1/Ob2 snapshots, link roots only.

Current accepted run cp32-dxut-state-extra-linked; Ob1 cp32-gui-resource-device-ob1,Ob2 cp32-gui-overwrite-ob2; units build/gui-resource-device-units.txt. Contract/seeds/acceptance/integration dxut-state-extra; review build/review_dxut_state_extra.py. All processes terminal,no push/deployment/GUI or hook execution. Prior three EditBox untracked diagnostic evals preserved. Goal remains active.

Next broader0.2.5 lead: client/net/textdrawpool.cpp in current base onlyconstructor+unnamed_100B2AF0;0.2.5 provides destructor/New/Delete/Draw, plus entire client/game/textdraw.cpp/.h absent from current client. R5 base textdrawpool header has field_0[MAX_TEXT_DRAWS],field_2400[MAX_TEXT_DRAWS] and ctor sets both arrays thencallsunnamed;0.2.5 pool lacks call, different capacity/layout must verify.0.2.5 CTextDraw ctor initializes TEXT_DRAW_DATA and copies256text,SetText and substantialDraw use game/font helpers. Do NOT assume0.2.5 structures are R5. Inventory contains no textdraw names; identify via poolconstructor/stringcopy/fontcall evidence and current IDB. This is a meaningful source delta to investigate after SDK sweep. No textdraw source imported or accepted yet.


## 0.2.5 textdraw pool transfer experiment (no accepted coverage)

Meaningful source transfer found:0.2.5 Delete61 at1E7F0 and pooldestructor64 at1E8D0 reproduce full R5 COFF functions after actual destructorB2F50 and accepted operator deleteC648A call identities resolved. Base poolconstructor43 at1E7A0 also matches with actual unnamed_100B2AF0B2AF0. Total168 candidatebytes,125 specifically missing0.2.5 implementations. NO accepted coverage or direct accepted0.2.5 credit. Coverage remains173,141; currentaccepted run cp32-dxut-state-extra-linked,Ob1 cp32-gui-resource-device-ob1,Ob2 cp32-gui-overwrite-ob2.

Snapshots cp32-textdraw-pool-anchor-trial1(net_textdrawpool) and cp32-textdraw-pool-transfer-trial1(eval_textdraw_pool). Isolated client/saco/eval_textdraw_pool.cpp has pointer-only CTextDraw destructor declaration (no implementation/no fakeemptydestructor), verified pool2304flags then2304pointers@2400. Never allocates/indexes CTextDraw objects. Complete bytes andPErelocsets checked, source/headerhashes current, explicittargets in evidence/checkpoint32/textdraw-pool-transfer-experiment.json. Both methods copied unmodified from0.2.5; classfieldnames mapped via verifiedarraylayout. Preserve candidate and previous3EditBoxevals. All processes terminal.

Next required actual dependency: CTextDraw::~14 atB2F50 loadsfield9A3 and calls resource releaseB2B50(57bytes), which needsdisassembly and originalproviderimplementation. R5 New155 at1E910 allocates0x9D6 then calls ctor402 atB36E0. Itsdata starts963,size91,transmitflagsat0,floatwidthat1;025 layouts incompatible (text256,string1024,etc.). R5 ctor copies many new fields inclpreviewmodel/rotations,usesSetText298B2F60, textureallocateB2B20 and textureload630B2CD0. Existing game/unnamed_1.cpp contains initializerB2AF0 and slotallocateB2B20, not releaseB2B50. Existing full captures ignored build/textdraw-pool-r5.asm and build/textdraw-lifetime-r5.asm. PoolDraw50 at1E830 checks scoreboardglobal1026EB4C firstfield, not025 GetAsyncKeyState(VK_TAB); then callsB3480. Additional poolpass36 at1E870 callsB34A0(513),pass36 at1E8A0 callsB36B0(44). Pooldtor at1E8D0 and Delete1E7F0 recovered; deletingwrapper30 at1E7D0 identity requires actual CTextDrawdtor. No textdraw code integrated yet; close cleanup chain beforecredit.


## Three-agent restart and textdraw cleanup acceptance

User explicitly authorized2 subagents, integrator alone edits active source/compiles VM/accepts/commits. Start2026-09-05T20:11:21Z,173141 baseline; VM/process check found no stale CL/link command, pendingOb1 complete. Agents textdraw_reconstruction and independent_family write ONLY build/agent-textdraw and build/agent-independent. Active root owns allbasechanges. Full goal recreated after get_goal returnednull; scope retained, no budget.

Accepted196 unique code: release57 B2B50,dtor14 B2F50,poolDelete61 1E7F0,pooldtor64 1E8D0.125 direct025 accepted credit, total1648. Stores textures800@26B568 andused800@26B888; split into separate TUs after combined1600 COFF reordered and draft failedconflictingplacement. Failedlink cp32-textdraw-cleanup-linked preserved. Actual DestroyTextureB3960 alreadyaccepted. Partial R5TextDrawCleanupView field9A3 only; no allocations/indexing/sizeof claims. Distinct R5TextDrawPool names avoid baseODR conflict.2081 complete regions,10 controls and actorABI PASS;8 pendingunchanged. Coverage173337/930756=18.623248%.

Current accepted link cp32-textdraw-cleanup-linked2; Ob1 cp32-textdraw-cleanup-ob1b,Ob2 cp32-gui-overwrite-ob2,units build/textdraw-cleanup-units.txt. Contract/seeds/acceptance/integration textdraw-cleanup; review build/review_textdraw_cleanup.py. Throughput initial interval in integration proof includes allcoordination+priorcandidate resume; not causal speedup estimate. VM allterminal. No push/deployment.

Agent1 auditedcleanup and proposed next194byte source build/agent-textdraw/closure_textdraw_cleanup_proposed.cpp with style987 slot9A3 releasepreview44 B36B0/poolpass36 1E8A0,init34 B2AF0/allocate37 B2B20/poolctor43 1E7A0. Not compiled/accepted. Agent researches larger loader630 and actualTXD providers, do not fabricate dependencies. Agent2 supplies build/agent-independent/eval_pickup_more.cpp: FindNetworkId37 133E0/PickedUp217 13440/Process334 13520/New236 13180.824 potential, no compileyet. New depends unresolvedCreateWeaponPickupA12D0; others purportedclosedBitStream/script but independentlyverify. All agentproposals remain isolated. Next integrate these serially, no VM concurrency.


Parallel proposal compile results after cleanupacceptance: cp32-parallel-proposals-trial1 contains eval_pickup_more andeval_textdraw_more. Pickups New236 at13180,PickedUp217 at13440,Process334 at13520 candidate full matches; FindNetworkId37 failed singleJL vsJNE, agentfixed forloop < to != in isolatedproposal (notcopied/recompiledyet). CreateWeaponPickup151 A12D0 requires originalbody game.cpp1024 appended to closure_models generator context; notattemptedyet. Agent2 researchingsecondNew176. Textdraw releasepreview44 exactB36B0,poolpass36 ambiguous1E870/1E8A0 resolves1E8A0 bycallB36B0,init34/allocate37 exact. Poolctor42 vs43 due sameTU init allowsEDX this; agentpreparing separatedconstructorTU torestoreESI. cp32-textdraw-txd-trial1 eval_textdraw_txd: ReadDictionaryTexture60 exactB2C60; EnsureDictionary194 proposal fails (agentcheckingbuffer/literals). All candidates NOT ACCEPTED. Active copies client/saco/eval_pickup_more.cpp,eval_textdraw_more.cpp,eval_textdraw_txd.cpp are untracked experiments and must preserve. Agentisolatedproposals may advance; integratorsolecompiler. All rootVM commands terminal at this note. Candidate results retained build/parallel-proposals-status.json and respectiveprobe/discoverylogs. No additionalcoverage since196accepted.


## Preview/init textdraw194 accepted; parallel trials continue

Fivefunctions194 newunique accepted, coverage173531/930756=18.644091%. Previewrelease44B36B0,poolpass36 1E8A0,init34 B2AF0,slotallocate37 B2B20,poolctor43 1E7A0. Ctor separateTU restoresESI original; sharedviewstyle987/slot9A3 only noallocation/fullsizeclaim.2086regions,10negativecontrols,actorABI PASS.0 newdirect025(total1648 unchanged). Throughput since3agentstart173141 recordedin textdraw-preview-integration.json.

Latestlink cp32-textdraw-preview-linked;Ob1 cp32-textdraw-preview-ob1,Ob2 cp32-gui-overwrite-ob2,units build/textdraw-preview-units.txt. Contract/seeds/acceptance/integration textdraw-preview; review build/review_textdraw_preview.py. RootVM processes terminal. Agents workingisolated. No push/deployment.

Model dependency probe original CreateWeaponPickup appendedclosure_models andgenerator selection (unaccepted/notseeded, discardedlink); trial cp32-pickup-model-trial1=132 vs151 original because Ob1doesnotinline unmarked helpers. Subsequent expandedC++ candidate eval_pickup_weapon_expanded.cpp trial3 also fails; agent2 diagnosing. Remainingnewmodel function notcredited. Alloldmodelmatches preservedcurrentacceptance.

cp32-parallel-proposals-trial2: separatedtextdrawctor43exact; corrected FindNetworkId37 nowexact133E0; Pickup New236/PickedUp217/Process334 exact asbefore. Currentproposal eval_pickup_more2.cpp NOTintegrated. Need actualmodelhelper151 or omitNewtemporarily.

cp32-parallel-proposals-trial3: EnsureTextdrawDictionary194 exactB2B90 and ReadTextdrawDictionaryTexture60 exactB2C60, source client/saco/eval_textdraw_txd2.cpp, readyfor fulltargetreview/integration. Query GetTextureId59 exactD280 only, othersfail. Agent1 auditing TXDtargets andquerydiff; agent2 modelhelper. Othertrial3sourcefiles eval_custom_model_queries.cpp,eval_pickup_weapon_expanded.cpp untracked, preserve. Large textdrawloader630 stilldependscustommodelmanagerproviders; nofalsecoverage. Alltrialsource activecopies are experimentsnotliveunits.


## Pickup/TXD integration blocked on descriptor placement (no new credit)

Trial4 cp32-pickup-weapon-trial4: guardedandinline CreateWeaponPickup both151 exactA12D0. Selected guardedstandalone client/saco/closure_pickup_weapon.cpp. Removedunmatchedoriginalbody fromclosure_models andgeneratorselection. Newactivecandidates closure_pickup_more.cpp (copiedcorrectedtrial2),closure_textdraw_txd.cpp (exactTXDtrial2), allNOTaccepted. Combinedunits build/pickup-texture-units.txt; seeds config/checkpoint32/pickup-texture-seeds.json, rootsaddedexports.

Longprlctl dispatch cp32-pickup-texture-ob1 hungbeforeguestprocess/directorycreation. Readonlyguestprocesschecks showed onlyqueryprocess, noCL/probe; MacwrapperPID71592 remained. TERMdidnotstopit,KILLfinallyterminal137. No guestcompilationduplicated. Replacement shortlauncher build/run_pickup_texture.ps1 readsunitsfile andsuccessfullycompiled cp32-pickup-texture-ob1b; useshortlaunchersforlongunitlists. Prematurefailedlink cp32-pickup-texture-linked preservedmissingprobejson. Successfulcomplete link cp32-pickup-texture-linked2. AllVM/rootprocessesterminalnow.

Draftforlinked2 FAILS conflicting originalplacement closure_pickup_weapon section46 rdata78: _request_model@0,_load_requested_models@20,_is_model_available@40,_create_pickup_with_ammo@60. These fourdescriptors have noncontiguousR5 placement; needexternalactualsplitstores or sourcecontextrestoration, nevermask. Stale build/actor-contract-draft.json is PREVIOUSpreviewdraft, doNOTreviewituntilnewdraftsuccess. Agentindependent_family explicitlyassigneddescriptorarrangement isolated. Code151 exact but noacceptedcredit. Candidatepotential allroots1229(824pickup+151weapon+254TXD) excludesEHuniqueextras.

Currentacceptedunchanged textdraw-preview-linked/proof,173531bytes. No commitacceptedcoverage thisturn. Needpreserveactivependingedits and allfailedsnapshots. Agenttextdraw querytrial2proposal ready170potential; agentpickupsecond419proposalready. Limitnewresearchuntilintegrationqueueclears. Fullgoalactive.


## Pickup/TXD combined lot1271 accepted

Coverage174802/930756=18.780647%,+1271unique. PrincipalpickupNew236/PickedUp217/Process334=787 direct025credit,total2435. FindNetworkId37/CreateWeapon151/TXD254/EH42 notdirect025. Codeallimplemented, actualBitStream/ScriptCommand/model/textureproviders verified; no missingprovidercredit.2107 complete regions,10negativecontrols and actorABI PASS;8 oldpendingunchanged.

Descriptorplacement fixedwithoutcompile: narrowSCRIPT_COMMAND rule addedto draftandgate specificallyclosure_pickup_weapon and4names, eachcomplete18-bytetypedobject. Existingmodelcommands samehandling; noinstructiontruncation, paddingnotclaimed. Old3descriptors aliasesdedup, newcreateammoEC74C18. Review boundlocalidentities toselecteddataobjectoffsetrange (notallsectionmembers). FullnewRPCbytes2/strings6,11,10/twoEHmaps36 eachverified.

Latestaccepted cp32-pickup-texture-linked2; Ob1 cp32-pickup-texture-ob1b,Ob2 cp32-gui-overwrite-ob2,units build/pickup-texture-units.txt. Contract/seeds/acceptance/integration pickup-texture; review build/review_pickup_texture.py. LongunitCLI use shortPS1 launcher readingunitsfile (build/run_pickup_texture.ps1) toavoidParallelsdispatchhang. Allrootprocessesterminal. Accepted changes include closure_pickup_more/weapon/closure_textdraw_txd; previousunmatchedweaponbodyremovedfromclosure_models/generator. Throughputbaseline173141 at20:11:21Z; total1661since3agentstart, allcoordination/waits included; exactrateinintegrationjson.

Nexttrials cp32-parallel-next-trial1: CreatePickup209exactA11F0,IsPickupModelValid34exactB4570; poolNew176fails (agent2diagnosing). All3queries58D150/53D190/59D280 exactfromeval_custom_model_queries2.cpp; helperGetAtChecked22 emitsbutmaybeinline atactualcalls, mustresolvebeforecredit. Agent1 auditeffects/callersviewsready inbuild/agent-textdraw/custom-model-views-audit.md. Agent2 secondidentityaudit readybuild/agent-independent/pickup-second-identity-audit.json. Candidatefiles eval_pickup_create.cpp,eval_pickup_model_valid.cpp,eval_custom_model_queries2.cpp untrackedpreserve. No nextlotcreditedyet. Agentsremainisolated,nocompile/writebase. Fullgoalactive, no push/deployment.


## Second pickup/model query lot589 accepted

Coverage175391/930756=18.843929%,+589unique.025adaptedpoolNew176+CreatePickup209=385 directcredit,total2820; helpervalidity34 andqueries170 R5reconstruction. Newcorrection useslocalnetworkIndex preservingoriginalslot insteadofoutputoverwrite. Exactfullfunctions; descriptorcreatepickupEC73818 and narrow5symbolSCRIPT_COMMANDrule closure_pickup_create. Threequeries58D150/53D190/59D280 rawexact nofixups, emittedGetAtChecked22 notcredited/exported.2118regions,10negativecontrols and actorABI PASS;8 pendingunchanged.

Latestaccepted run cp32-pickup-model-linked;Ob1 cp32-pickup-model-ob1,Ob2 cp32-gui-overwrite-ob2; units build/pickup-model-units.txt; shortlauncher build/run_pickup_model.ps1. Contract/seeds/acceptance/integration pickup-model; review build/review_pickup_model.py. Allrootprocessesterminal,nopush/deployment. Throughputsincebaseline173141 savedinintegration.

Nexttrial cp32-parallel-rpc-model-trial1: eval_pickup_rpc.cpp three181/160/159exact F080/F140/F1E0 (500 principalbytes). Needsfullexceptiontable/providerclosure; agent2 nowresearchingnextRPCfamily. SourcefullviewscloneCPickupPoolCreate exactlysameclassasaccepted, includes main, netgameviewpools3DE->pool8 pointeractual.

Modeltrial1: IsSupportedCustomModelInfo58 exactB44E0,CloneCustomModelInfo91 exactA7AD0,LoadCustomModelTexture100 exactA7B60;setterB4660/29nonmatch. Activeevalsources eval_custom_model_kind/clone/texture/texture_slot.cpp untrackedpreserve. Agent1diagnosticsetter andactualcalloc/fullstores; nextlargerC770281 proposaldependsA7C30 stillunimplemented. Noextra modelcreditbeforeclosure. Agentproposalsisolated inbuild/agent-textdraw; userauthorized2agents only, integratorsoleVM/sourcewriter. Goalactive.


## Pickup RPC/model clone lot712 accepted

Coverage176103/930756=18.920426%,+712unique (RPC500 +modelhelpers149 +EH63). Direct025credit500,total3320.2129regions,10negativecontrols and actorABI PASS,8oldpendingunchanged. FullthreeEHmaps36each included. Nativevtableclassifier comparesconstants only,no vtablecontents/nativeclass reconstruction claimed. Clonecopies32bytes asactualR5,existingcallocC6982 reviewed/excludedCRT.

OriginalRPCconstructoridentity corrected: source025unsignedchar*cast selected1F7C0,but R5callschar*overload1F840; bothalreadyimplemented separately. Removedcast onPCHARData in activeclosure_pickup_rpc.cpp. Firstlink cp32-pickup-rpc-linked draftfailedproviderplacement; preserved. Newcorrectlinked2 fullgatepasses, no dualmapping/masks.

Currentaccepted cp32-pickup-rpc-linked2;Ob1 cp32-pickup-rpc-ob1b,Ob2 cp32-gui-overwrite-ob2; units build/pickup-rpc-units.txt; launcher build/run_pickup_rpc.ps1. Contract/seeds/acceptance/integration pickup-rpc; review build/review_pickup_rpc.py. Allrootprocessesterminal,no push/deployment. Throughputbaseline173141 at20:11:21Z,total2962new since3agentstart,exactrateinintegration.

Nextcp32-rpc-game-model-trial1:6RPCgame exactprincipal1117: CameraPos20219AA0,LookAt24219B70,Money1471A500,Gravity1471ACD0,Weather168F370,TimeEx211F420. Money/Gravity identicalbodyshape mustreviewactualGivePlayerMoney vsgravityproviders; no ambiguouscredit. Sourceclient/saco/eval_rpc_game.cpp useschar*constructorfix already. Agent2identityaudit underway.

SetterC++ trials2and3bothfailed; latest cp32-rpc-game-model-trial1/eval_custom_model_texture_slot3.obj,agent1diagnosing. ActualLoadCustomModelTexture100 A7B60 exacttrial1 butsetterB4660missing =>excluded. Sourceviewkind58andclone91nowaccepted; custommodelpathloader281proposalandInstallCustomModelA7C30remainunimplemented. Preservealluntrackedevals andisolatedagentsources. Fullgoalactive.


## Game RPC / model texture lot1372 accepted

Coverage177475/930756=19.067833%, +1372unique (sixRPC1117, texture100, slot29, compilerEH126). Direct025credit1117,total4437.2149whole regions, six36-byte unwind maps/FuncInfo,10negativecontrols and actorABI PASS. Eightoldpending unchanged. Actual constructorchar*1F840, MoneyA0F70 versusGravityA1400 verified; R5LookAt cutbyte and Weather/Time settings adaptations preserved.

TextureSlotB4660 accepted29 with explicitly labeled source-derived symbolic field-access assembly after three C++ mismatches. Uses GetModelInfo then WORD offset0xA; no copied instruction bytes. LoadCustomModelTexture100 now has complete actual dependency closure. Other model/textdraw loaders still excluded. Failed C++ trials retained.

Latest cp32-rpc-game-linked;Ob1 cp32-rpc-game-ob1,Ob2 cp32-gui-overwrite-ob2. Units build/rpc-game-units.txt; launcher build/run_rpc_game.ps1; review build/review_rpc_game.py. Contract/seeds/acceptance/integration rpc-game. Total4334unique gained since20:11:21Z baseline173141; measured throughput in integrationjson includes coordination. No push/deployment, goalactive.

Next agent2proposal build/agent-independent/eval_rpc_zones_player.cpp: nineRPC1671potential bytes, awaiting compile/actualtarget review. Agent1 pursuing custommodel InstallCustomModelA7C30/entryC770/managerD320 chain in isolated directory. Root retains sole activewriter/compiler role.


## Nine gangzone/player RPCs lot1860 accepted

Coverage179335/930756=19.267671%,+1860unique principal1671+compilerEH189. Direct0251671,total6108.2176whole regions,nine36-byte unwindmaps/FuncInfo,10negativecontrols and actorABI PASS. FourzoneRPCs,health/armour,weapons/ammo,wantedlevel. Firsttrialallmatch; actualtarget identity distinguishes Remove2240/StopFlash2220 and HealthABD70/ArmourABDB0. Gettersinline verified existingR5offsets. Eightpendingunchanged.

Latestcp32-rpc-zones-linked;Ob1cp32-rpc-zones-ob1,Ob2cp32-gui-overwrite-ob2;unitsbuild/rpc-zones-units.txt,launcherbuild/run_rpc_zones.ps1,reviewbuild/review_rpc_zones.py. Contract/seeds/acceptance/integration rpc-zones. Total6194newuniquesince20:11:21Z baseline173141; rateinintegrationjson, coordinationincluded. No push/deployment,fullgoalactive.

Concurrentnative-modelresearch: cp32-custom-install-trial1 normalC++Clump127B1FC0 andInstall133A7C30 exact, wrappersOpen/Close20vs61/57,Read24vs32,SetRef26vs36 unaccepted. Agentdocumentedsource-derivedsymbolicexceptions inbuild/agent-textdraw/custom-native-exceptions-audit.md. Trial2compilefailedbecauseasmreservedkeywordtype, preserved. Trial3renamestreamType compiling symbolicwrappers+modelLoadFiles/EnsureReady386candidates; nonecredited. Agent2researchesnextRPCbatch; agent1customloadchain,rootsolecompiler.


## Custom model install closure446 accepted

Coverage179781/930756=19.315588%,+446unique. No direct025credit,total6108unchanged. Four native stream/refcount wrappers186 explicitlysource-derivedsymbolicasm after failednormalC++20/20/24/26; C++Clump127/Install133. AllrealGetModelInfo/TXD/chat/UnFuck/iGtaVersion providers included. Two fullBSSstoresbyte15181C andDWORD1A25B0 justifiedbyrealread/writewidths, sourceerrors36/48withpaddingverified.2186regions,10negativecontrols and actorABI PASS;8oldpendingunchanged. Preservesoriginalfailurepaths, noGTA/nativeexecution.

Latestcp32-custom-install-linked;Ob1cp32-custom-install-ob1,Ob2cp32-gui-overwrite-ob2;unitsbuild/custom-install-units.txt,launcherbuild/run_custom_install.ps1,reviewbuild/review_custom_install.py. Contract/seeds/acceptance/integration custom-install. Total6640newuniquesincebaseline173141; throughputinintegrationjson. Localcommitonly,fullgoalactive.

Nextcp32-model-player-trial1: EnsureReady105D320 exactafterstoreorderfix; LoadModelFiles281stillnonmatchdespitefailurelabel, agent1diagnosing. FiveRPC+helperexact: Reset28,Behind13,FindZ270,Explosion258,Widescreen174,SetStatePair23; Pos279stillnonmatchagent2diagnosing. Two SCRIPT_COMMAND objects18 atE61E4/E620C needwhole-objectselection, notwholecombined38section; sourceRadiusfloat/formatfffii preserved. No pendingcandidatecredit. Alltrialprocessesterminal, evalsourcespreserved.


## Player position/effects RPC lot1129 accepted

Coverage180910/930756=19.436888%,+1129unique principal1022+helper23+EH84. Direct0251022,total7130.2205whole regions,4EHmaps36each,2completeSCRIPT_COMMAND18,float1.5/debugstring16;10negativecontrols and actorABI PASS. Pos279 matchedusingnull-localplayer earlyreturn insteadnestedpositivebranch (prior283). Actual statepair3710 fields2DA/2DE, reset2E2; partialviewonly. Radiusfloatvarargs/fffii descriptor and Widescreendebug preserved. Narrow two-symbol SCRIPT_COMMAND rule addedtoolsdraft/gate, full18bytes notcombined38section, no codetruncation/masks. Eightoldpendingunchanged.

Latestcp32-player-effects-linked;Ob1cp32-player-effects-ob1,Ob2cp32-gui-overwrite-ob2;unitsbuild/player-effects-units.txt,launcherbuild/run_player_effects.ps1,reviewbuild/review_player_effects.py. Contract/seeds/acceptance/integration player-effects. Total7769newunique sincebaseline173141 at20:11:21Z; rateinintegrationjson includescoordination. Localcommitonly,goalactive.

ModelC770281temporarilydeprioritized: variants1/2same281wrongfailureblockposition,variant3=285. EnsureReady105D320exactbutdependencyC770unmatchedsoexcluded. Agent1nowresearchestextdrawDrawText727/DrawSprite272/dispatch19 independentofC770; agent2vehicle/menuRPC nextfamily. Currentcp32-rpc-vehicle-menu-trial1 liveprobe ofeval_rpc_vehicle_menu (5RPC+poolprovider1031potential); waitterminal before nextVMjob. Preserveevalsources andallfailedtrials.


## Vehicle/menu RPC lot1136 accepted

Coverage182046/930756=19.558939%,+1136unique principal995+pool36+EH105. Direct0251031,total8161; LinkToInterior36 source025previouslymissingbase genuinelytransferredwithR5viewoffsets.2221whole regions,5EHmaps36each,10negativecontrols and actorABI PASS. Netgame3DE poolsvehicle0/menu20, vehicles1134/states3074/capacity2000, Healthnullguardsverified; ShowMenu7FC0/HideMenu8020 truetargets. No newimports/constants,nofullallocatedclassclaim. Eightoldpendingunchanged.

Latestcp32-vehicle-menu-linked;Ob1cp32-vehicle-menu-ob1,Ob2cp32-gui-overwrite-ob2;unitsbuild/vehicle-menu-units.txt,launcherbuild/run_vehicle_menu.ps1,reviewbuild/review_vehicle_menu.py. Contract/seeds/acceptance/integration vehicle-menu. Total8905newunique sincebaseline173141 at20:11:21Z; measuredrateinintegrationjson includescoordination. Localcommitonly,fullgoalactive.

Nexttextdrawtrial cp32-textdraw-render-trial1: DrawTextA727B31A0 exact (WindowsDrawTextmacro appendsA), dispatch19B3480exact; sprite278vs272nonmatch. NativeCppfailedMSVC2003C4234__thiscallfunctionpointerkeyword; no probejson butfirsttwoobjectspreserved. Agent1fixingsprite/nativeABI, sourceviewembeddedinCPPavoidsnewsharedheaderandunrelatedrecompile. RenderState26 proposedfromexistingbase. Agent2preparing6ActorRPC1512potentialusingacceptedActorPool/ActorPed/Entity, noSpawnbecauseproviderB3DD0unimplemented.


## Actor RPC / textdraw font lot2431 accepted

Coverage184477/930756=19.820125%,+2431unique (ActorRPC1512+EH126+DrawText727+native66). Direct025727,total8888. Actoranimationserializationborrows025knowledgebutconservative0directcredit; RenderState26alreadyinbase/nodirect025credit.2244whole regions,6EHmaps36each,complete448/640 floats;10negativecontrols and actorABI PASS. Destroycorrectedearlyreturn, Rotation9C570vsHealth9C5D0 actualprovideridentity. AnimationcallsboolRead8420/charRead1FEA0 accepted. Eightoldpendingunchanged.

TextDrawfont727full29fixups verified, pointerviewoffsets963/9A3/9C1/9D1 compileasserted,noallocation/fullclassclaim. R5selection/keycodes/bounds/FPUorderpreserved; WindowsmacroDrawTextA. Native40 symbolicthiscallbridge afterMSVC2003C4234functionpointerrejection,26unchangedbaseRenderState; realGTAaddressliteralsnotnativecoverage. DrawSprite278/272/272 variantsremainnonmatch(twoFIADD/storeorderingdifferences), dispatch19excludedbecauseSpriteunimplemented. NoGTAexecution.

Latestcp32-actors-render-linked;Ob1cp32-actors-render-ob1,Ob2cp32-gui-overwrite-ob2;unitsbuild/actors-render-units.txt,launcherbuild/run_actors_render.ps1,reviewbuild/review_actors_render.py. Contract/seeds/acceptance/integration actors-render. Total11336newunique sincebaseline173141; exactrateinintegrationjson includescoordination. Localcommitonly,fullgoalactive.

NextSpawn: cp32-actor-spawn-trial1 validation46B3DD0 exact, Spawn213vs240: aggregateinit+2byte vscontiguousmemset27; unusedpostNewreadeliminated29bytes. Trial2eval_rpc_actor_spawn2.cpp usesmemset and explicitlyqualifiedvolatileactorpointerarray toretainobservedread; uniqueRPCActorPoolSpawnViewavoidsODRcollisionwithacceptednonvolatileactorRPCview. Variantcurrentlycompiling; noSpawn/providercredityet. Agent1researchingothertextdrawselection/hittest functions; C770andSpritesboundedpaused.


## Textdraw selection and actor validation526 accepted

Coverage185003/930756=19.876638%,+526unique (selection480,actorvalid46). No direct025credit,total8888unchanged.2251whole regions,10negativecontrols and actorABI PASS. SelectionUpdateHover exactaftercounterinitbeforeselectedId andGetAt nullfallthroughbranchorder. ActualGetCursorPos/ScreenToClient/PtInRectimports unchanged andverified. GetAt17CC0/35 rawuniquezero COFF/PEfixups, structuralidentityfrom2304flags/pointeroffset2400; no directCALL/JMP/absolutePErefsfound, nocalleridentityclaimed. Fullviewpointeronly,noallocation. Actorvalidationrange0..30000 and literal85BDC0classifier, novtablecontents/nativeclassclaim. Eightoldpendingunchanged.

Latestcp32-textdraw-selection-linked;Ob1cp32-textdraw-selection-ob1,Ob2cp32-gui-overwrite-ob2;unitsbuild/textdraw-selection-units.txt,launcherbuild/run_textdraw_selection.ps1,reviewbuild/review_textdraw_selection.py. Contract/seeds/acceptance/integration textdraw-selection. Total11862newunique sincebaseline173141; measuredrateinintegrationjsonincludescoordination. CorrectedpreviousPROGRESSactor-renderpercentage toactual19.820125. Localcommitonly,fullgoalactive.

Spawnparkedafterthreeboundedtrials:213first,240second/thirdbutoneModRM44vs4Cdifferenceremains;0Spawncredit. Helper46acceptedseparatelynormalC++, no volatilequalificationinacceptedhelper. Diagnostics actor-spawn-trial2/3-diagnostic.json. Agent2nextRPCstates/clock808+providers239potential. Agent1textdrawSendClick151/Disable77/MsgProc41plusEH21 andRPC83scalar4 proposed, currentcp32-textdraw-click-trial1 eval_textdraw_click running; waitterminalbeforeVMnextjob. SourceclassR5TextDrawSelectorViewtokendeclarationidenticalwithowner, ClickViewdistinct. NoGTA/network/desktopexecution.


## World state/object RPC and textdraw click lot2800 accepted

Coverage187803/930756=20.177469%,+2800unique. Direct0251791 (state708+worldobjects1083),total10679; priorarithmetic808stateRPC correctedto708beforeacceptance.2301whole regions,231EHcode/11maps36each,10negativecontrols and actorABI PASS. Fullsource functions alltrueprovidersclosed, eightoldpendingunchanged.

Worldstate947+EH84: RakServerStats296 completeBSS12E968 fromexisting296-byte sourceobject, fourEHmaps. ResetMoney18090actualpGame26EBAC/A20C0/RPC20; unrelatedE650pNetGame/A540excluded. Clock187/Money52frombase,nodirect025credit. MoneydescriptorEC64818alreadyacceptednowemittedwhole18-byte section, no truncation/newcredit.

Worldobjects1267+EH126: firstsourcefailedincompleteCObject; nonexistentgame/object.hattemptpreserved. FinalsourceusesCEntityoffset0 pointerview, nofullobjectclaim. AudioGetAudioinline fixesPlaySound218->219evaluationorder. R5read-onlyRespawn132 preserved, notstub. Bounds227/Team233/ObjectPos272/NoCamera184 fullmatches; last184R5reconstructionnot025.

Textdrawclick/update/HUD355+EH21: SendClick149directint,158WORDaliasbothfailed; realRakNetWrite<WORD>producesexact151. Disable77/MsgProc41; HUD40viaforceinlineknownradarbody,Update46. RPC83scalarE6274/4 actualregistrationreferences andsendoperandverified. Firstlinkcp32-world-state-click-linked failedmissingpCmdWindow; real4-bytezeroowner26EB84addedfrommain.cpp18, constructor693D0/allocation1AFC/storeC4923and70PErefsconfirmpointeridentity; allocationnotcredited. No native/game/networkAPIexecution.

Latestcp32-world-state-click-linked2;Ob1cp32-world-state-click-ob1c,Ob2cp32-gui-overwrite-ob2;unitsbuild/world-state-click-units.txt,launcherbuild/run_world_state_click.ps1,reviewbuild/review_world_state_click.py. Contract/seeds/acceptance/integration world-state-click. Total14662uniquegainedsincebaseline173141; direct025gain9156; measuredrateinintegrationjsonincludescoordination. Localcommitonly,wholeDLLfalse/fullgoalactive.

Nextcp32-input-animation-trial1live: sixanimation/cameraRPC1925potential+EH126;HideTextdraw164(char*ctorcorrectedfromagentunsignedcast); ProcessInputDisabling283+RestoreMouse70 base-sourcefoundationswiththree5-byteexistingnativeGTApatchdataarrays, noR5codecopyorcoverageforthosearrays. Agent1auditingremainingUIcontroller/editoridentities; formerguessesScoreBoard/Dialogwithdrawn,donotassertthemunproven. Agent2awaitingprobe. Preservealluntracked evalsources/failedtrials.


## Input restoration and animation/camera RPC lot accepted

190392/930756 (20.455630%), +2589 unique code: input353, HideTextDraw164, animation/camera1925, seven EH147. Direct0251434 (Hide164 + animation1270), cumulative12113. All complete linked regions/actual providers, seven full36-byte EH maps, ten negative controls and actor ABI pass. Three five-byte source GTA patch arrays receive no code credit and were never executed. Input symbolic native-call helpers labeled; no instruction blobs. Eight old pending unchanged.

Run cp32-input-animation-linked; Ob1 cp32-input-animation-ob1, Ob2 cp32-gui-overwrite-ob2. Seeds/contract/acceptance/integration input-animation; review build/review_input_animation.py; units build/input-animation-units.txt. Throughput from real union and elapsed coordination recorded in integration. World-extra/object-selection trial1 compiled next, discovery under review. Full goal active, no push/deploy.


## World RPC, editor and menu initialization lot accepted

193401/930756 (20.778915%), +3009 unique code. Direct0251408 (attachment453,spawn159,menu796), cumulative13521. Twenty complete code sections including eight21-byte EH handlers, eight36-byte maps, scalar117/4, descriptor069B/18, bool1, managerpointer4 and file-backed257-byte sharedmenu buffer.2361whole regions; ten negativecontrols and actorABI PASS. Eightoldpending unchanged.

Worldextra trial1/2 Attachment455 failed, trial3 source-edit error preserved, trial4 restored two025ScriptCommandbranches exact453. Editor515 corrected with threefloatlocals and direct Write1/Write0 exact513. Objectselection ctor15/toggle117 exact. MenuInit796/New188 exact firsttrial.

Firstdraft rejected wrong pChatWindow identity26EB98; real pChatWindow26EB80 retained. Independent caller evidence RPC11EB0 -> ctorDE20 at11F39 -> store11F4B identifies custom model download manager. Field217 filters modelentrytype2 world+8 and flags220/221 reset. Corrected RPC descriptive name ScrSetCustomModelWorldTransfer and distinct pointerowner, no constructor/managerallocation credit. Failed linkedrun/draft and original proposals preserved.

Latest cp32-world-extra-editor-linked2; Ob1 cp32-world-extra-editor-ob1b; Ob2 cp32-gui-overwrite-ob2. Seeds/contract/proofs world-extra-editor; units build/world-extra-editor-units.txt; review build/review_world_extra_editor.py; throughput inintegration includesallcoordination. Currentnext memoryreadertrial2 fullRead126 exact, ReadInt mapsBAA70 viaactualcallerBC030; ambiguousEnded4 excluded. CheckpointRPCtrial1 running. Fullgoal active; no push/deploy.


## Memory reader and checkpoints accepted

194861/930756 (20.935777%), +1460unique code: reader310, checkpointRPC608/providers500/EH42. Direct025608, cumulative14129.2384whole regions; ten negativecontrols and actorABI PASS; eightoldpendingunchanged.

Reader firstRead126 same-size failed due return0 block order; explicit earlyreturn trial2exact. ReadIntBAA70 same-reader callerBC030 atBC21F/BC229 verified; homologueB3A80 not credited. Ended4 has three identical candidates/no caller and is excluded; source remainsunexported/unselected. ReadWord27 included, allwholebodies.

Checkpointfive functions exactfirsttrial. Six18-byte SCRIPT_COMMAND complete sourceobjects shareCOFFdata118 withpadding, originalobjectsnoncontiguous. Extended existingtyped-object draft/gate rule onlysixnamedsymbols in closure_rpc_checkpoints; all18bytes compared,no relocations, sourcebounds checked. Fourdescriptorspreviouslycovered; onlytwonewdataobjects and no data-as-codecredit. Fullcodefunctions remainwholeCOFFonly.

Latestcp32-reader-checkpoints-linked; Ob1cp32-reader-checkpoints-ob1, Ob2cp32-gui-overwrite-ob2; seeds/contract/proofsreader-checkpoints; unitsbuild/reader-checkpoints-units.txt, reviewbuild/review_reader_checkpoints.py. Total21720unique sincebaseline173141; direct025gain12606; fullcoordinationrateinintegrationjson.

Next: RWframeparsertrial1 principal258/129/299 notmatched, constructor46/nestedconstructor28/dtor35 rawcandidates only; agentdiagnosing. Spectatorvehicletrial1 running. Preservealltrials. Fullgoal active; no push/deploy.


## Spectator and vehicle closure accepted

Coverage: 196090 / 930756 code bytes (21.067820%). This batch adds 1229 unique bytes: 1145 principal code and four 21-byte EH handlers. Direct contribution from adapted 0.2.5 source is 960 bytes (771 RPC, 189 spectator providers), bringing the cumulative reference contribution to 15089. ExitCurrentVehicle138 is base source and the angle provider47 is reconstructed from R5. Four complete 36-byte EH maps and both complete 18-byte command descriptors are verified without code credit. The fresh full gate, ten negative controls and actor ABI passed. Eight old pending regions remain unchanged.

Seven principal functions matched in trial1. ExitCurrentVehicle had the right size and targets but different register allocation. Trial2 uses an inline member view for the first two vehicle-index reads, retaining the two real B3C00 calls and final ABC90 call. Its entire 138-byte body now matches. No global or import was introduced; pNetGame remains 26EB94. Typed-object rules were extended only to the two named complete SCRIPT_COMMAND objects in closure_rpc_spectator_vehicle.

Latest run: cp32-spectator-vehicle-linked. Ob1: cp32-spectator-vehicle-ob1; Ob2: cp32-gui-overwrite-ob2. Seeds, contract and proof prefix: spectator-vehicle. Units: build/spectator-vehicle-units.txt. Review: build/review_spectator_vehicle.py. Throughput measurement includes coordination and failed trials. All commits are local; the full goal remains active.

Next work: frame-parser trial2 matches ReadFrameList299 but its ReadChunk/Validate providers remain unmatched, so the principal receives zero credit. Three independent foundations (reader ctorBB20/28, chunk ctorB9570/46, dtorBB70/35) have whole-body and identity audits in build/agent-textdraw. BB20 has a unique structural identity but no direct caller found; do not invent one. Agent1 is preparing explicitly labeled symbolic helper proposals after two bounded C++ trials, preserving real allocator/reader targets without wrappers or instruction byte blobs. Agent2 is preparing object operations and RPC rotation/stop, reducing object-lifetime dependencies without claiming the missing constructor/vtable chain. No integrator VM command remains active after this batch.


## Object operations and RenderWare frame parser accepted

Coverage: 197750 / 930756 code bytes (21.246170%). This batch adds 1660 unique bytes: object operations/RPC/EH865 and parser795. Direct contribution from adapted 0.2.5 source is451, cumulative15540. Existing Entity::SetTurnSpeedVector9EE60/33 is now a linked provider but was already covered and receives no second credit. 2423 complete regions, two complete21-byte RPC EH handlers and36-byte maps, ten standard negative controls and actor ABI pass. Eight old pending regions remain unchanged.

All nine object principal functions matched in trial1. Add/Remove22 are distinguished by actual WorldAddEntityB3B20/WorldRemoveEntityB3B40 providers, never by convenient homonymous bytes. GetModelReferenceCount47 preserves the existing025 symbolic implementation and its real GetModelInfo target. The rotation SCRIPT_COMMAND is a complete18-byte section.

FrameList299 matches normal C++ after inline Reset restores the original pointer evaluation boundary. ReadChunk258 and ValidateFrameExtensions129 required explicitly labeled symbolic reconstruction after two bounded C++ layouts failed. Named offsetof/sizeof fields represent complete reader23, chunk35 and frame56 structures. Their aliases contain no wrappers: the actual MAP and every linked CALL resolve OperatorNew to libcpmt:newop.obj and Read to closure_memory_reader.obj. The symbolic trial matched both complete bodies on its first compilation. Reader ctor28, chunk ctor46 and destructor35 are independently matched; reader ctor has unique structural identity, with no invented caller. ReadChunkB9630 is selected through actual FrameList callers; homologues remain excluded.

First link failed because unused inline constructors were not emitted after separating symbolic helpers. Both current TUs now have identical dllexport annotations on the constructors, forcing their complete capsule bodies. No dummy creator or wrapper was introduced. Link2 succeeds; original bytes and all targets remain verified.

The shared alias resolver now reads undefined /alternatename source directives and resolves their canonical providers. Existing COFF weak aliases are unchanged. Duplicate aliases, aliases with local implementations and chained aliases all reject in three additional fast controls. Aliases grant no code coverage. Evidence of actual alias addresses belongs specifically to cp32-object-parser-linked2; future contracts retaining that metadata must label its originating run or regenerate it for their new MAP, rather than present old linked addresses as current.

Latest run: cp32-object-parser-linked2; Ob1cp32-object-parser-ob1b; Ob2cp32-gui-overwrite-ob2. Contract/seeds/proofs: object-parser. Units: build/object-parser-units.txt. Review: build/review_object_parser.py, followed by explicit linked alias proof in the reviewed contract and integration. Source hashes, failed trials and identity captures are preserved. No push or deployment; full goal remains active.

Next: cp32-release-clump-trial1 compiled object release dependencies279, clump dispatch/setters395 plus full50-byte switch tables, memory buffer/file360 and clump-file parsers301; discovery/acceptance still pending. New fread CRT provider must receive actual library identity verification and no code credit. Object destructor300/deleting30/EH18 proposal is isolated in build/agent-independent, with four globals and actual vtableECD74 ownership still to integrate; no opaque constructor/allocation claim. Preserve all proposal and trial files.


Release/clump dependencies accepted: +1335 unique code bytes, total199085/930756 (21.389602%). Object release279, clump dispatch/setters395, buffer/file360 and clump-file301. Pool getter18 was already covered and receives no new credit. Complete dispatcher310 includes50 table bytes excluded from code credit. Empty D1D0 Release3 is a genuine whole original provider, separated to prevent compiler elimination of the original call; no missing dependency is represented by a stub. All full comparisons, ten negative controls and actor ABI pass. New fread76 has actual LIBCMT:fread.obj ownership and no source credit. Fresh alias evidence belongs to this link. No new whole-function025 contribution; cumulative15540.

Run cp32-release-clump-linked; Ob1cp32-release-clump-ob1; Ob2cp32-gui-overwrite-ob2. Review and independent identity audits retained with evidence. Measured 25944 new unique bytes over 3.114h = 8331.9 bytes/hour including coordination and failed trials. Next: integrate exact object destructor348 plus full vtable/stores, consolidate existing Add/Remove identities, then player lifetime dependencies. Goal remains active; no push/deployment.


Object destructor accepted: +348 unique bytes; total199433/930756 (21.426991%). Exact dtor300/deleting30/EH18, full vtable12/descriptor18/unwind36 and four independent4-byte globals. No complete CObject allocation or size claim. Add/Remove22+22 migrate from removed nonvirtual pointer-view implementations to the true derived virtual methods; original hashes and WorldAdd/WorldRemove targets remain identical, zero duplicate credit. Model/material release dependencies are now accepted. Ten negative controls and actor ABI pass.

Run cp32-object-destructor-linked; Ob1cp32-object-destructor-ob1b (first launcher rejected appended path syntax after cache processing; preserved), Ob2cp32-gui-overwrite-ob2. No new025 whole-function credit, cumulative15540. Measured 26292 new bytes since baseline, 8240.5 bytes/hour including coordination. Agent audit confirms original textdraw125 already accepted, no re-count/recompile. Next: eight player lifetime bodies and57EH, true ED140 vtable and exact32D allocation; header method-only additions. Goal remains active.


Player lifetime accepted: +1126 unique code, total200559/930756 (21.547967%). Seven principals matched first C++trial; RemoveAllAttachedObjects86 matched whole symbolic exception on first symbolictrial, after three C++ induction-base experiments failed. Trial3 initially used DWORD& on intfield and was corrected in separate preservedtrial3b; not an additional algorithm variant. Exact32D allocation and all layout assertions, real ED140 table B13D0/9F300/9F400 and accepted object attachment destruction. Header adds only5 methods, no fields. Five SCRIPT_COMMAND18 objects include ctorECE84 distinct from ActorPedEC280, stringNULL5, EH21+18+18 and three full36-byte maps. No fabricated complete CObject allocation or dead factory credit. Fullgate, ten negative controls and actorABI pass.

Run cp32-player-lifetime-linked; Ob1cp32-player-lifetime-ob1, Ob2cp32-player-lifetime-ob2 (new common header requires fresh objects). Contracts/seeds/proofs/review: player-lifetime. Added312 direct adapted025 Destroy bytes; cumulative15852. Symbolic native GTA deletion protocol already present in025 remains explicitly labeled. Measured 27418 accepted bytes since baseline at 8137.2 bytes/hour including coordination, failed trials and header rebuild. Next ready: selection/editor639, sprite341, custom model loader386; all still candidates. Goal active, no push/deployment.


Selection/model/motion accepted: +2131 unique bytes, total202690/930756 (21.776921%). Selection/editor639, custom loader386, five playerRPC1106. Added580 direct025 bytes (Velocity248 and Drunk166 twice); cumulative16432. Other new functions are R5. C770281 uses explicitly labeled symbolic reconstruction after three failed C++ branch layouts; true InstallCustomModelA7C30/EnsureReadyD320 fix identity, C650 homologue excluded. Three format strings are full independent source objects. Close109 requires explicit if, Shop247 full memset33, Armed174 early-return guard.

Raw discovery of Shop247 initially mapped swapped source method names to the two real targets. Strict placement review rejected it before acceptance: SetShopNameAE380 must precede LoadShoppingDataSubsectionAE300. Corrected source is exact with actual targets. First link also exposed three absent interface globals26EB60/64/68; all have independently audited real DoInitStuff writers and are now separate4-byte zero-fill stores. No constructor/allocation or data bytes credited.

Two repeated inline FindPlayerPed copies caused absent object-local EH MAP owners. Gate now defers only COFF selection5 associative sections with an identical accepted parent COMDAT and its actual selected source owner. All final code/data/relocation comparisons remain intact, duplicates add zero coverage. Four targeted controls reject non-associative sections, unknown selected owner, changed original parent identity and altered actual EH bytes. Standard ten negative controls and actor ABI pass. No second build required for this verifier-only fix.

Run cp32-selection-model-motion-linked3, Ob1cp32-selection-model-motion-ob1c, Ob2cp32-player-lifetime-ob2. Failed links/drafts/trials preserved; review/identities/controls retained. Measured 29549 accepted bytes since baseline at 7586.4 bytes/hour, coordination and diagnostics included. Next candidates: textdraw construction1506, Vehicle/GameText831, one bounded Sprite barrier experiment. Original125textdraw bytes already accepted and never recounted. Goal active; no push/deployment.


Textdraw/init accepted: +3158 unique bytes, total205848/930756 (22.116215%). Construction1506, Vehicle/GameText831, pregame/init821. Direct025 contribution824, cumulative17256; constructor402 hybrid qualitative contribution, existing-base pregame/init and DisplayGameText receive zero incremental025 credit. Original125textdraw bytes are already accepted and not recounted.

SetText declaration order and loader align8 arrays plus explicit memset65 close the first trial differences. Full0x9D6 construction layout asserted. Actual InitGame calloc513 closes GameText pointer ownership, four-byte clear preserved. Pregame patch data remains existing source-owned GTA data, never executed. New strncmp provider is actual pinned LIBCMT:STRNCMP.OBJ; imports retain actual identities. Full31 new placements, all previous regions, relocations, ten negative controls and actor ABI pass. No redundant second build.

Run cp32-textdraw-init-linked1, Ob1cp32-textdraw-init-ob1, Ob2cp32-player-lifetime-ob2. Measured 32707 unique new bytes at 7657.6 bytes/hour including coordination/trials. Sprite272 scheduling/absolute-address encoding remains parked; dispatch/pool dependents are excluded. Next: remaining high-yield textdraw and independent families. Goal active; no push/deployment.


Textdraw/vehicle RPC accepted: +1848 unique bytes, total207696/930756 (22.314763%). Show/Edit/Info724 and appearance/interior1124, nine principal functions and five21-byte EH with full36-byte maps, three typed18-byte commands and literal41. Existing getters, FindPlayerPed copies and old125textdraw bytes add zero new credit. Direct025 contribution524, cumulative17780: TireRPC218 and Interior306; other functions receive zero strict direct source credit.

Edit268 trial1 differed only in length initialization scheduling, corrected before pool retrieval. Plate279 vs280 corrected by initializing length before text buffer memset33. These motivated C++ changes preserve protocol conditions, argument identities and original boundary behavior. Show297/Info117 and five vehicle/interior bodies exact first trial. Info is real type4/color126, not Debug type8/color12A. No header change, no new globals/allocations. All full gate, ten negative controls and actor ABI pass.

Run cp32-textdraw-vehicle-rpc-linked1, Ob1cp32-textdraw-vehicle-rpc-ob1, Ob2cp32-player-lifetime-ob2. Measured 34555 new unique bytes at 7795.7 bytes/hour, coordination/trials included. Preview requires >=6393 bytes before complete closure and is deferred; Sprite encoding remains unmatched. Next PlayerName/std::string and another independent family. Goal active; no push/deployment.


Name/distance accepted: +1713 unique code bytes, total209409/930756 (22.498807%). PlayerName337+string893+EH31, Entity distances379 and independent model predicates73. Full_Copy316 includes267 primary and49-byte catch section; its final1-byte INT3 padding is fully compared but excluded from code coverage, leaving315 unique code bytes; fullEH10 and132metadata retained. True Xran/Xlen from pinned LIBCPMT:STRING.OBJ and CxxThrowException from LIBCMT:THROW.OBJ outside sourcecoverage. RPC EH21+36 map complete. Header-instantiated string code is actual source, not copied original instructions. Added337 direct adapted025 bytes, cumulative18117; other sources alreadybase or R5-only.

Historical distance code-only matches were insufficient: initial linked draft rejected source10000.0f against originalEC54C100000.0f. Corrected both return literals, recompiled onlychangedTU throughcache and relinked, then reviewed actual bytes including constantidentity. Existing accepted100000constant gives no new data/codecredit. All2599 regions, old8pending excluded, full gate, ten negative controls and actor ABI pass. No exception-path native execution or redundant secondbuild.

Run cp32-name-distance-linked2, Ob1cp32-name-distance-ob1b, Ob2cp32-player-lifetime-ob2. Measured 36268 unique new bytes at 7913.2 bytes/hour including coordination/trials. Next ScoreBoard209 candidate and independent std::string consumers/common wrappers. Goal active, no push/deployment.


Screenshot/model accepted: +1546 unique code bytes, total210955/930756 (22.664909%). Filename/string988, ScoreBoard209, common model utilities349 (seven sizes sum349, initial agent449 arithmetic corrected beforeacceptance). Added255 direct025 filename bytes, cumulative18372. Fullcppstdappend/operator+/dtor/EH and originalformat24; realSAMPDir261 owner/writersidentified but SetupDirectories remains notimplemented/notcredited. No filesystem or screenshot operation executed.

ScoreBoard209 constants checked including0.5, providersactual. Collision getter C++trials1 were33bytes short; trial2 gotfullsizes74/93 but EDX vsECX pointer transfer differed. Three named symbolic MOVs isolate that compiler-allocation difference; remainingC++ unchanged. RenderAtomic C++41 vs59 requires actual frame/local/cdeclcallback bridge, explicitlysymbolic59; fullactualgetter/callbackslot48 ABI reviewed, no fakevtable/rawbytes/nops. Otherfour utilityfunctions remainC++. Whole2626regions, allrelocations, tennegativecontrols and actorABI pass; old8pendingunchanged.

Run cp32-screenshot-model-linked1, Ob1cp32-screenshot-model-ob1, Ob2cp32-player-lifetime-ob2. Measured 37814 unique new bytes at 7857.0 bytes/hour includingcoordination/trials. Nexttrue SetupDirectories and nativefactory81, then otherproviders; fullpreviewnotclaimed. Goalactive,no push/deployment.


Directory/pool accepted: +1110 unique code bytes, total212065/930756 (22.784167%). Actual SetupDirectories/SetupCacheDirectories438 now close the SAMP/cache bufferwriters; sourcebase preserves registryhandle behavior,261-byte realbuffers and sixfullstrings. No filesystem/registry functions executed. Missing game/util.h declaration fixed after preservedfailedtrial. LocalPlayer ctor223/reset263 and PlayerPool ctor154 first C++trial exact, plus fullEH32/map44; allocator324h and existingwholeheaders324h/2F3Eh asserted. No destructor closure fabricated.

All2644regions, actualproviders, everyCOFF/PErelocation, tenstandardnegativecontrols and actorABI pass. Direct025 new0, cumulative18372. Sourcefunctions alreadycompletebase, newlyclosed dependencies are the demonstrated accelerator.

Run cp32-directory-pool-linked1, Ob1cp32-directory-pool-ob1, Ob2cp32-player-lifetime-ob2. Measured 38924 unique newbytes at 7568.3 bytes/hour includingcoordination/trials. Next actualType1 custompedchain823 and modelinstancefactory81 candidates; destruction/spectating/spawn remainexcluded. Goalactive,no push/deployment.


Ped chain accepted: +1186 unique code bytes, total213251/930756 (22.911590%). Type1 customped chain823, factory81, Remote/Net destruction282 including EH21. GetModelInfo51 moved to a sole new TU owner with unchanged exact bytes; real EDX preservation yields clone76, explicit getter guard yields Ensure94. Factory81 symbolic after two documented C++ failures; C650281 uses accepted resolver pattern with actual PED installer A7BD0, never object homologue A7C30. Existing globalowners and formats fully verified; actual native slots16 SetClump and11 CreateInstance. Remote509/Net48 actual classes, CPlayerPed virtual deletion through real slot0 and flags1, real usedslots840. Two genuine nonvirtual declarations require fresh Ob1/Ob2.

All2661regions, everyCOFF/PErelocation, tennegativecontrols and actorABI pass. Direct025 +79 adapted remote teardown, cumulative18451. Measured 40110 unique newbytes at 7429.1 bytes/hour coordination included. Eightoldpending excluded; ModelInfoLoaded extent under audit. Next objectrenderer694 and construction/clock649 candidates. Goalactive; no push/deployment.


Construction/clock/providers accepted: +1265 unique code bytes, total214516/930756 (23.047501%). Remote ctor122/reset244/Net ctor160+EH32 and actual RakNet clocks86/155+NetGame wrapper5, shared true staticfrequency8/init1 (whole9). ColorsRPC233+setters33/21+EH21; ped modelsetter113 from025, real R5 fields44/2A4 and native audio bridge. ModelInfoLoaded original40 now fullyexact after C++16/volatile40 temporaryregister failure and bounded three-MOV field bridge; sevenpendingremain. Constructor source base/vendor and clocks firsttrialexact, true Remote allocation509 and Net layout48 preserved.

All2684regions and actualprovider bindings/COFF/PErelocations pass, tencontrols/actorABI pass. CRT allmul52 and alldvrm223 full original/linkbody equality with actual LIBCMTmembers, no CRTcodecredit. Direct025 +346,cumulative18797. Measured 41375 newbytes at 7431.5 bytes/hour coordination included. Previewobject694 remains excluded after ABIcorrection and bounded C++ trials; no padding inserted. Next matrix/animation137 and source025trailers/mapicons/objectdelete. Goalactive,no push/deployment.


Map/trailer/object/matrix accepted: +1959 unique code bytes, total216475/930756 (23.257975%). Trailers693, mapicons674, objectpoolDelete455, rotation72/animation65. RPC accessors and map helper expansion restore C++ codegen; uniquely separate true CVehicle methods from RPC avoids cross-function ECX clobber optimization and matches Attach315. Actual trailer pointer48 typed in same-size class field, three nonvirtual declarations require one fullOb1/Ob2 refresh. Real flag1517444, caller/writer/reader audit does not credit unimplemented hook. Map disableE5958 and game disableEC5BC remain distinct samecontent18-byte objects. Actual CObject deleting destructorAA100 via real slot0/flags1 accepted; views access-only never allocated.

All2715regions plus all trueCOFF/PEtargets/data, fiveEH21/maps36, sourceflag and matrix40 pass; tencontrols/actorABI pass. Firstprobe terminalfailure was filename typo after eightcompiledobjects, preserved; actual full successfulrun used. Direct025 +1434,cumulative20231. Measured 43334 newbytes at 7443.6 bytes/hour coordination included. Next EntityRender138 and quaternionutilities171 candidates, broader025 RPC work. Goalactive,no push/deployment.


Vehicle/entity/death providers accepted: +1266 unique code bytes,total217741/930756 (23.393994%). VehicleParams/RemoveComponent652 with true SetDoorStateB7840 distinct from oldSetLockedState; normalRPCGetAt and actualcomponentdescriptor18. Death name72/color55/AddMessage5 providers132, existingDeathWindow owner reused after rejectedduplicate link; RPCDeath493 remainsparked490. EntityRender138 and quaternion171, plus previouslyblocked Update82/SetMatrixAndUpdate91 nowcomplete173. Symbolic exceptions after boundedC++codegen failures, native PreRender17 vsRender18, Remove3/Add2, CMatrixUpdateRW59AD70 and EntityUpdateRwFrame532B00 verified. Matrix64 argument/ret64 forwarded to real SetMatrix177; two aliases bind soleactualmembers, no wrappers. Fivependingremain, AdvancePosition113 staysunchangedexact.

All2735regions, wholedata/EH maps, everyCOFF/PErelocation and trueprovider, tencontrols/actorABI pass. Direct025 +549,cumulative20780; measured44600 newbytes at 7231.5 bytes/hour coordinationincluded. No newheaders, no native/runtime execution. NextHuffman2300+EH and remotePedctor712 candidates; goalactive,no push/deployment.


Huffman/ped accepted: +6155 unique code bytes, total223896/930756 (24.055284%). Complete Huffman/StringCompressor source closure +4788 after all previous coverage; original ctor26760/9 has actual Huffman callers but already present in historicalCP31 union, zero duplicate credit. Complete English frequencies1024 isolated without sentinels or truncation, actual static state8; true Queue/LinkedList/Map allocations/lifetimes and every EH region. Real CPlayerPed813 remote constructor694 and ped preview634 plus EH39 =1367. First normal C++ probes exact, actual vtable/lifetime/matrix/render/model providers closed. Four whole script descriptors explicitly whitelisted. Header refresh both compiler profiles, one successful full link.

All2836 whole regions, ten controls and actor ABI pass; five old pending excluded. Direct025 increment0, cumulative20780: useful remote-ped lifecycle input is hybrid reconstruction, not entire-body transfer credit. Measured50755 unique newbytes at7986.7 bytes/hour, coordinationincluded. Vehiclepreview/lifetime and independentnetwork next; goalactive,no push/deployment.


Vehicle/statistics accepted: +5503 unique code bytes,total229399/930756 (24.646524%). Full actualCVehicleB8 constructor1308/lifetime2180, scalarB8B50/AddB8A10/RemoveB8A40 and vtableED728/12, plus vehiclepreview686/EH21 =707. Newconstructor needed bounded C++ float conditional fix retaining true float4; no symbolicbody or forceddata. Whole descriptors/separate realstores, actualtrain/carriagepaths and COMRelease retained. Existing anglecommand exposed with oneowner, no second body. Statistics2338 semanticR5levels1/2/4 and true differential counters, Entityqueries278 all firstprobeexact. Speedhistory225 trial146/235/150 excluded.

All2905 whole regions, all COFF/PE targets, tencontrols and actorABI pass; fiveoldpending excluded. Direct025 numericcredit conservatively332,cumulative21112; missingvehiclelifecycle source was materially useful but hybrids notblanketcredited. Measured56258 newunique bytes at8423.4 bytes/hour coordinationincluded. Bothprofiles refreshed forheader; no push/nativeexecution/deployment. AES/MT6367 thenDataBlockEncryptor987 candidatesnext;goalactive.


AES/DataBlock accepted: +7354 unique code bytes,total236753/930756 (25.436634%). Full Rijndael/MT199376367 plus DataBlockEncryptor/CheckSum987, 182 previouslycovered bytes deducted. Whole actual AES tables13712 contiguous, MT state/next2504 (raw296+virtual2208), ROUNDS4 and left4=-1, floating literals. Si/S5 duplicate table contents retain caller-specific identities, no invented aliases. True key282/cipher21/encryptor586/checksum10 structures, entire methods including bounds/modes, no fake providers. Ambiguous getter4 and dtor1 excluded. All sources normalvendor C++, exact first probes.

All2939 complete regions, everyCOFF/PE target, tencontrols/actorABI pass. No headers changed, all unchanged objects reused. Direct025increment0/cumulative21112. Measured63612 newunique bytes at9350.9 bytes/hour coordinationincluded. Real previewowner361 and nextnetwork family follow; fivepending and object694/sprite272 unresolved, goalactive. No push/network/game/native execution.


Reliability/preview accepted: +2203 unique code bytes,total238956/930756 (25.673324%). Reliability serialization/pool1008 and DataCompressor622/EH42=1672, true packet55/pool16 and offsets403/6F2, complete actual dynamic buffer allocation/release. Existing BitStreamRead<bool>89 reused, no duplicatecredit. Real previewowner28-byte constructor28/Initialize333 =361, all actual initialization calls and failurebehavior; startup/global notcredited. RW firstAtomic53/geometrySkin53/clumpSkin25/hierarchy39 =170, primary SDK native identities/version ABI. All normal C++ firstprobeexact.

All2957 complete regions, everyCOFF/PEtarget, tencontrols/actorABI pass. No headerchange, cachedunchangedobjects. Direct025increment0/cumulative21112; measured65815 newunique bytes at9436.4 bytes/hour coordinationincluded. Fiveoldpending and Object694/Sprite272 stillunmatched, outer549 excluded. NextTEA1358 and largerinterface/renderfamily;goalactive,no push/game/network/nativeexecution.


TEA/health accepted: +3330 unique code bytes, total 242286/930756 (26.031097%). TEA1358, seven health methods1894, actual matrix hook78. Normal C++; preserve TEA key no-op, genuine Ob2 for health Begin, actual12-byte owner and COM lifetime. Entire vertex/matrix stores and constants verified. D3DXVec3Project import thunk has true pinned library/slot identity and no source credit. DataBlock base lifetime reused with zero duplicate credit.

All 2996 complete regions, every COFF/PE relocation, ten controls and actor ABI pass. Initial acceptance rejected missing import-library metadata; corrected manifest and reran without source change. Direct025 increment0/cumulative21112. Measured 69145 new unique bytes at 9413.5 bytes/hour including coordination and failed trials. Five old pending, Object694/Sprite272 and enclosing549 remain excluded. Next packet-family extraction and label785; goal active, no push/deployment/live networking.


Reliability packet family accepted: +5919 unique code bytes, total 248205/930756 (26.667032%). Split/reassembly, queues, ordering, statistics, ping and TEA setup5901 plus EH18, complete unwind map36. Normal vendor C++ and real allocation/lifetime; failed first extraction preserved, corrected full source74/74 candidate emissions. Selected57 sections/190 bindings,558 previously covered bytes excluded. IsDataWaiting/AreAcksWaiting actual distinct callers confirmed. External alloca61 from LIBCMT chkstk.obj checked whole, zero source credit.

All 3053 linked regions, every COFF/PE target, ten controls and actor ABI pass. No header change,348 cached objects reused. Direct025 increment0/cumulative21112. Measured 75064 unique bytes at 10077.5 bytes/hour, coordination included. Label757/756/755 still unmatched, not counted; old pending and parked preview unchanged. Next chat851 and independent network family. Goal active; no push, deployment or live networking.


Lifecycle/chat accepted: +5532 unique code bytes, total 253737/930756 (27.261388%). Network lifecycle4667, chatRenderText851, independent menu14. Real1794/16/55 class sizes and all actual allocations/releases. Sole SetPing/UpdateNextActionTime146 moved without extra credit; genuine same-TU visibility restored Initialize405. Full typed selectany double8 preserves compiletime value while pooled16 and extern-only split rejected. No slicing/copied instructions. All complete EH/unwind maps, true vector CRT boundaries excluded from source coverage.

All 3151 linked regions, COFF/PE targets, ten controls and actor ABI pass. Direct02514/cumulative21126; chat851 hybrid0, vendor lifecycle0. Measured 80596 unique bytes at 10441.1 bytes/hour including coordination and failed attempts. Label755 and Draw662 parked, dependent wrappers excluded. Resources536 candidate next. Goal active; no push/deploy/game/live network.


Chat resources accepted: +536 unique bytes, total 254273/930756 (27.318975%). CreateFonts419/MeasureDimensions117 normal C++ first probe exact. Whole six strings,17 COFF/PE bindings; Y2 reused. Actual D3DXCreateTexture/CreateRenderToSurface import thunks and pinned library identities, no source credit for imports. Real CChatWindow owner/nonallocating view, original failure paths. All 3159 linked regions, ten controls and actor ABI pass.

Direct0250/cumulative21126; measured 81132 unique bytes at 10342.6 bytes/hour incl coordination. Constructor279 only LEA/MOV scheduling difference, parked with full diagnostic; Draw662 and dependent wrappers excluded. Next ACK/resend trees and scoreboard actions; goal active, no push/game/network execution.


Tree/scoreboard accepted: +2834 unique bytes, total 257107/930756 (27.623459%). BPlusTree removal/RangeList serialization2393 (EH42) +Scoreboard441 (EH21), complete unwind maps and RPC23 scalar4. Actual Serialize/Deserialize/RemoveResend callers, no duplicate666-byte credit. Insert505/SendAck22 excluded. Normal scoreboard snapshot of ListBox pointer restores Click282; no unknown owner allocated. Whole TU Ob2 for Hide, other objects unchanged. All 3190 linked regions, full COFF/PE targets, ten controls and actor ABI pass.

Direct0250/cumulative21126. Measured 83966 unique bytes at 10476.0 bytes/hour including coordination. Next full item400 and datagram static3718 pending callbacks. Goal active, no push/deploy/game/live network.


Tree insertion/items accepted: +2524 unique bytes, total 259631/930756 (27.894636%). Static insertion/queue2124 plus full ListBox item400. Real29E allocation and all fields identified/asserted, exact zero64/128 and textcapacity256 preserved; outer ListBox not allocated. Normal source actual pointer-reference identity and reload after memset, no invented padding. Whole functions/all COFF/PE targets, all 3204 linked regions, ten controls and actor ABI pass.

Direct0250/cumulative21126, measured 86490 unique bytes at 10494.1 bytes/hour coordination included. Datagram remains unmatched and callbacks open; Update892 parked while queries432 match. Next logger base/file and playerqueries. Goal active; no push/game/live network.


Logger/scoreboard accepted: +2480 unique code bytes, total 262111/930756 (28.161086%). Logger1184+real server ABI bridge8; population892+queries396 unique (GetRemote36 already accepted in base manifests). Full class tables, all original targets and relocations; IDTOString complete172 checked and only118 code counted. Actual compiler deleting aliases and server target provider qualified; unused entry alias removed after draft rejection with unchanged gate. Real row45 and scratch260, no fabricated allocation/padding. 3271 regions, ten negative controls and bounded actor ABI pass.

Direct0250/cumulative21126; source025 helps scoreboard reasoning but hybrid implementation has no direct byte credit. 88970 total new unique bytes since parallel start, 10215.8 bytes/hour including coordination/failures. Next graph/Dijkstra and scoreboard Draw/request; plate renderer296/295 still excluded. Goal active, no push/GTA/live networking.


Plate/request accepted: +402 unique code bytes, total 262513/930756 (28.204277%). Full plate owner lifecycle230 and genuine periodic request151/EH21; whole unwind36 and scalars4+4. All 3280 regions, COFF/PE bindings and actual providers, ten negative controls and actor ABI pass. Existing aliases freshly qualified at final link. Direct0250/cumulative21126 (hybrid request adaptation). PlateRender296/295 and ScoreboardDraw790 scheduling mismatches excluded; no guessed register permutations.

Since parallel start 89372 new unique bytes, 10124.3 bytes/hour coordination included. Next graph context split, socket leaves and labels3D. Goal active; no push/deployment/network execution.


Graph/labels/socket accepted: +9624 unique code bytes, total 272137/930756 (29.238275%). Graph8633 (476 EH), labels564, socket leaves427. Every selected function/COMDAT copy, all 3456 regions, original COFF/PE bindings and complete EH maps804 verified; ten controls and bounded actor ABI pass. Real compilation contexts separated, comparator44 emitted explicitly to eliminate absent fallback name without gate changes. Duplicate copies give no extra coverage. High graph algorithms and dependent SpanningTree remain excluded, as does SocketSendTo137 with extra transformation.

Direct0250/cumulative21126; missing025 descriptor18 provides concrete semantic/data input to labels, not direct code credit. Since parallel start 98996 new unique bytes at 10871.4 bytes/hour coordination included. LabelPool and StringTable next; goal active. No push, deployment, native UI or live sockets.


AVL/StringTable/label leaves accepted: +4132 unique code bytes, total 276269/930756 (29.682215%). AVL2359 (deleting60+EH18), StringTable1474 (EH36), LabelPool New270+IsActive29. Complete real objects, VFTs, state and EH; all 3526 regions/COFF/PE targets, ten controls and actor ABI pass. PlayerID sole definitions transferred into NetworkTypes comparison context restores real ECX preservation and84-byte NetworkID comparisons; no fake barriers or alias/gate changes. Old comparisons and assignment29 gain no duplicate credit.

Direct0250/cumulative21126; 103128 new unique bytes since parallel start, 10870.3 bytes/hour including coordination/failures. Draw643/Transform92 and unconsumed pLabel store remain excluded. Next ListBox deliberate layout/lifetime milestone and Actor context diagnosis. Goal active; no push/deploy/native game or networking.


ListBox/roster accepted: +1250 unique code bytes, total 277519/930756 (29.816515%). ListBox Insert195+arrayInsert92 and player roster900+EH63, all full regions and actual providers. Shared header milestone rebuilt all profiles,111 old GUI regions preserved; full gate, ten controls and Actor ABI pass. Real pScoreBoard4 storage transferred separately; prior pDeathWindow/pLabel main changes remain pending.

Source025821 adapted principal bytes/cumulative21947; 104378 new unique bytes since parallel start, 10485.9 bytes/hour including coordination/failures. Ctor144/Render1014 and ServerJoin remain unmatched, their dependencies excluded. Agents reassigned after bounded trials. Goal active; no push/deploy.


Database/IME/keys accepted: +23173 unique code bytes, total 300692/930756 (32.306211%). Whole DatabaseServer/Table/Common/Serializer and extra sort/serialization operations, PlayerID string120, IME2460 and SetKeys655. All source/data/EH/COFF/PE providers, ten controls and Actor ABI pass. Actual compiler CRT initializer pointer4 now bound only inside pinned LIBCMT initializer array with null sentinels, unique local provider and PE relocations; full comparison unchanged, dedicated negative controls retained. No data or duplicate code credited.

Direct0250/cumulative21947: existing vendor source produced the major gain here. 127551 new unique bytes since parallel start, 12189.7 bytes/hour coordination/failures included. Prior85 player regions remain identical after transfer77. Unmatched GetKeys and later IME message/reading candidates excluded. Goal active; no push/deploy.


FileList/DatabaseClient/SHA1 accepted: +14988 unique code bytes, total 315680/930756 (33.916515%). Whole functions and attached data/EH, all COFF/PE targets, genuine CRT providers and weak destructor alias, ten controls and Actor ABI pass. FileList12324, Client1592, SHA11072; old SHA1 core excluded from gain. No data, duplicate or CRT code credited.

Direct0250/cumulative21947: existing vendor source supplies this batch. 142539 new unique bytes since parallel start, 12945.1 bytes/hour coordination/failures included. Parked IME message/reading bodies excluded. Goal active; no push/deploy.


DirectoryDelta/IME/menu accepted: +3238 unique code bytes, total 318918/930756 (34.264404%). Whole functions and attached data/EH, all COFF/PE targets, genuine providers and whole state, ten controls and Actor ABI pass. DirectoryDelta2164 including actual FileList/List constructors25, IME711, Menu363. No data, duplicate or CRT code credited.

Direct0250/cumulative21947: existing vendor source and R5 UI adaptations supply this batch. 145777 new unique bytes since parallel start, 13067.9 bytes/hour coordination/failures included. Parked IME message/reading bodies excluded. Goal active; no push/deploy.


NAT/chat/mesh accepted: +5027 unique code bytes, total 323945/930756 (34.804503%). Whole functions and attached data/EH, all COFF/PE targets, genuine providers and whole state, ten controls and Actor ABI pass. Nat4016, Mesh630, Chat381; local R5 failure IDs52/53, whole dispatch tables and EH included in comparison. No data, duplicate or CRT code credited.

Direct02541/cumulative21988: FilterInvalidChars41 has actual missing source in025, other code uses existing vendor and R5 adaptations. 150804 new unique bytes since parallel start, 13280.1 bytes/hour coordination/failures included. Parked IME message/reading bodies excluded. Goal active; no push/deploy.


ConnectionGraph accepted: +10510 unique code bytes, total 334455/930756 (35.933693%). Whole functions and attached data/EH, all COFF/PE targets, genuine providers and whole state, ten controls and Actor ABI pass. ConnectionGraph and actual AllocPacket32, all101 former node_identity regions conserved, graph_algorithms57 intact with four physical shared copies exact. No data, duplicate or CRT code credited.

Direct0250/cumulative21988: full vendor source supplies the new code. 161314 new unique bytes since parallel start, 13998.4 bytes/hour coordination/failures included. Parked IME message/reading bodies excluded. Goal active; no push/deploy.


Replica/NetworkID and chat navigation accepted: +12131 unique code bytes, total 346586/930756 (37.237042%). All complete functions and attached data/EH, actual COFF/PE targets, owner transfer35 unchanged, true aliases, ten inherited controls and Actor ABI pass. CRT binder now validates entire aligned initializer blocks without splitting source data; two positive cases8/4, eight new fault cases and five old scalar faults pass. Navigation helper distinct name fixes a real COMDAT conflict without forcing a wrong target. No duplicate, data, CRT or unimplemented callback code counted.

Direct0250/cumulative21988. 173445 new unique bytes since parallel start, 14366.8 bytes/hour coordination/failures included. TCP and font helper candidates under review. Goal active; no push/deploy.


TCPInterface/RakSleep4306 and font helpers331 accepted: +4637 unique code bytes, total 351223/930756 (37.735239%). Whole worker/lifetimes, EH/data, all COFF/PE targets and imports; true CRT thread wrappers and scope, ten controls and Actor ABI pass. All4266 prior regions preserved. True static font scratch/scalars40012 verified, but startup heap allocation remains unimplemented and receives no credit. No data/CRT/duplicate credit.

Direct0250/cumulative21988. 178082 new unique bytes since parallel start, 14612.4 bytes/hour coordination/failures included. Font proxy/hook candidate next. Goal active; no push/deploy.


Console/parser/Telnet/logger8195 and Font439+ANSI271 accepted: +8905 unique code bytes, total 360128/930756 (38.691988%). All4317 previous regions preserved, 170 new complete sections, all targets/COFF/PE, three true deleting aliases and whole VFTs, CRT strtoul23 to true strtoxl boundary qualified. Ten controls and Actor ABI pass. Two former pending bodies closed; three remain. ANSI source change one predicate, no header edits. Font real static object4/VFT56, no synthetic init; original startup allocator still outside coverage. No data/CRT/duplicate code credit.

Direct0250/cumulative21988. 186987 new unique bytes since parallel start, 15144.5 bytes/hour coordination/failures included. Next RakPeer and newly unlocked text consumer. Goal active; no push/deploy.


RSA11013/RakNetCommandParser5090, commands1032 and ANSI201/ctor9 accepted: +17345 unique code bytes, total 377473/930756 (40.555527%). Entire source/data/EH, all actual targets and COFF/PE, true VFT/weak alias, pinned CRT wrappers and whole locale archive12, ten controls and Actor ABI pass. One positive/seven negative CRTdata controls. Duplicate FindPlayerPed/EH and prior RSA1833 excluded from gain. Three real SCRIPT_COMMAND18 objects stored independently, no typed slicing. No data/CRT/fake-storage or virtual concrete-implementation credit.

Direct025+280/cumulative22268, verified entire Weather/Time/Interior definitions; Save/Raw adapted with zero direct credit. 204332 new unique bytes since parallel start, 16026.7 bytes/hour coordination/failures included. Chat descendants and font startup remain outside coverage. Goal active; no push/deploy.


RakPeer policy/mutex2112, UI/Rcon/Skin/CreateVehicle1556 and NewVehicle163: +3831 unique accepted code bytes, total381304/930756 (40.967128%).128 new complete regions and all4669 prior preserved. Whole typed settings1297 transferred, two bool stores verified; complete CVehicle184 allocation and existing constructor. Full original/linked COFF/PE, true providers/aliases/duplicate owners, ten negative controls and Actor ABI PASS. No duplicate/data/CRT or unimplemented dependency credit.

Direct025+0/cumulative22268; adapted commands/factory are hybrid, vendor already in base. 208163 newunique bytes since parallel start, 15855.6 bytes/hour including coordination and failed trials. Remote-state trial compiled; goal active. No push or deployment.


RakPeer remote state/RPCMap/reliability setters: +2816 unique accepted code bytes, total384120/930756 (41.269678%).38 new whole regions, all4797 previous preserved. Actual RPCNode6 allocation and full RPCMap1024, whole static POD296 statistics and original3177 remote stride. Entire original/linked COFF/PE, true providers/aliases, ten negative controls and Actor ABI PASS. Prior956 code excluded. No data/CRT/unimplemented virtual dependency credit. Direct025+0/cumulative22268; source already present in vendor base. 15934.3 bytes/hour including coordination and failures. Command leaves next; goal active, no push/deploy.
