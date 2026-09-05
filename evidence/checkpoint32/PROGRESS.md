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
