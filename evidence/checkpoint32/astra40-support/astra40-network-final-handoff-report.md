# Reprise réseau Astra — remise finale du 9 septembre 2026

**2 903 octets réseau sont acceptés dans astra40-three**, sur une preuve globale de434468/930756 (commit fdc201b). **304 octets réseau supplémentaires, exacts et fermés sur objets frais, restent soumis au dernier lot indépendant de l’intégrateur.** Leur acceptation porterait le gain réseau de cette fenêtre à3207 octets.

**GenerateDatagram1594 est SOURCE EXACT MAIS LIEN NON ACCEPTÉ : zéro crédit.** La recherche est arrêtée sur demande de l’intégrateur. L’intégration déjà engagée a dépassé la fenêtre20:40:48–21:20:48 UTC ; le rapport global doit prendre le temps réel jusqu’à la dernière validation.

| Famille | Octets nouveaux | État |
| --- | ---: | --- |
| Transport13 méthodes +Push<PlayerID> +EH |1542|Acceptés astra40-three|
| Factory deletions, Console destructor existant, PacketFile ctor existant |151|Acceptés astra40-three|
| Router Send +EH +SystemAddressList dtor |963|Acceptés astra40-three|
| Cycle de vie parseur Transport +EH |182|Acceptés astra40-three|
| Pop<PlayerID>, déjà émis dans Transport |65|Acceptés astra40-three|
| Router lifecycle/factory +Queue ctor/dtor +DestroyPeer |304|Objets frais exacts ; dernier pipeline indépendant requis|
| GenerateDatagram |1594|Source exact ; lien rejeté ; zéro crédit|

## Lot indépendant304

Remise : router-queue304-batch-handoff.json. Dix racines unit/symbol/RVA/taille entiers,24 régions complètes,0 collision au préflight astra40-three. Les manifestes couvrent toutes les régions locales atteintes, leurs données/EH, cibles COFF/PE et propriétaires réels. Les anciens fournisseurs ne reçoivent aucun crédit nouveau.

Router a sa vraie taille29, confirmée par allocation0x1d et offsets du constructeur. Ses trois tables virtuelles40+4+4 sont complètes. Les trois callbacks vides sont identifiés par leurs vrais slots, sans choisir arbitrairement parmi les fonctions identiques. Les deux appels Queue<PlayerID> du destructeur Transport117 fixent son destructor17 à0x37160 ; le même appelant fixe DestroyRakPeerInterface15 à0x37df0. Cet appelant et son lifecycle restent hors couverture.

## GenerateDatagram : résultat exact, échec d’intégration

Le corps R5 préexistant avait1614 octets contre1594 dans l’original. Le nouveau travail a restauré dans son unité trois définitions déjà acceptées : GetBitStreamHeaderLength54, WriteToBitStreamFromInternalPacket288 et InsertPacketIntoResendList134. Le probe-seven reproduit alors les1594 octets entiers à0x48070 et le frame0x2c, contre0x30 précédemment. Les24 régions atteintes, toutes les relocations et données/EH sont exactes. Aucune modification du corps GenerateDatagram ni des headers/profils n’était nécessaire pour ce match source.

Le linker a cependant rejeté les trois définitions fortes en double. L’audit initial de collisions comparait le drapeauCOMDAT et les corps, sans distinguer sélection1 NODUPLICATES et sélection2 ANY ; il avait donc laissé passer cette collision réelle.

La variantev2 ajoute uniquement inline __declspec(noinline) aux trois copies dans Datagram. Probe-eight conserve1594 et les trois providers54/288/134, avec véritables sélectionsANY vérifiées dans les enregistrements auxiliairesCOFF. Le lien reste rejeté face aux copies canoniquesNODUPLICATES.

L’intégrateur a ensuite essayé d’aligner les définitions canoniques sur inline/noinline. Probe-nine omet alors Insert134 et Write288, inutilisés dans leurs unités ; notre audit refuse une revuePASS. Le dernier essai force leur émission via dllexport, ces fonctions étant déjà exportées dans la capsulebaseline. Probe-ten échoueC2375 : déclarationheader non-dllexport incohérente. Aucun header/profil n’a été modifié pour contourner ce rejet. L’intégrateur arrêteDatagram, conserve les essais et restaure les trois sources actives antérieures avant le lot indépendant.

Les fichiers datagram-context-v2-reviewed-manifest.json et datagram-context-v2-owner-audit.json prouvent uniquement le match source et les trois copiesANY de Datagram ; ils ne prouvent pas une fermetureODR avec les canoniques ni une acceptation liée. Aucun de ces1594 octets ne doit être ajouté à la couverture.

## Reprise distincte prescrite

Une repriseDatagram doit traiter explicitement le contrat de déclaration et l’émission des propriétaires, dans un workflow séparé si elle touche les headers. Avant tout nouveau lien : présence physique de chaque canonique exigée, sélectionCOMDAT auxiliaire inspectée sur chaque copie, maintien des frontières d’appel et des1594 octets, puis comparaison de toutes les régions déjà acceptées affectées. Des corps identiques ne suffisent pas à autoriser leur coalescence. Aucun /FORCE, alias de propriétaire inventé ou effacement silencieux d’un canonique.

L’auditisolé audit_datagram_canonical.py est prêt pour cette vérification :60 régions canoniques complètes et six copies attendues des trois méthodes. Il n’a jamais émis dePASS sur probe-nine ni probe-ten. L’auditpermanent devra distinguer COMDAT ANY et NODUPLICATES ; aucun outilactif n’a été modifié ici.

## Autres corrections, provenance et exclusions

Transport réel56/parser16 : assertions60/20 rejetées puis corrigées sur preuve de l’allocation0x38 et des offsets réels. Router v1/v2 ont34 différences de frame ; v3 a été rejeté pour comparateur de repli absent ; v4 instancie le vrai comparateur44 déjà accepté et ferme77 régions sans nouveau crédit pour ce fournisseur.

Le parseur a un véritable alias localE→G30 dans sa VFT60. La revue suit le canonique complet. Le premier schéma weak_aliases àplat a été rejeté puis corrigé par l’intégrateur en actual_weak_aliases={unit:{alias:canonical}} ; les nouvelles remises utilisent le schéma correct.

Transport ctor97/dtor117 et factories restent ouverts : VFT36 →Start68 →AutoAllocate20 →GetRakPeerInterface89 →lifecycleRakPeer manquant. SendTo137 reste exclu : les deux relocations identifient seulement le début119A38 ; le hint0.2.5 de4092 ne prouve pas l’étendue du bufferR5. Aucun stockageBSS arbitraire proposé. NodeE77 reste écarté face au fournisseurgraph84.

Scan de39 anciens objetsvendor et336 découvertes : aucune nouvelle familleHTTP/MD5/fichiers unique. std::string a été laissé au chercheurUI après identification du chevauchement ; un octet de paddingCopy horsinventaire a été signalé pour éviter un gain artificiel.

Le vendor existant suffit aux lots acceptés ; aucun apport exclusif0.2.5 nouveau revendiqué. Pop65, Console dtor25 et PacketFile ctor18 provenaient de fournisseurs déjà émis. Les autres sources isolées sont nouvelles ou corrigées, avec algorithmesvendor/préexistants réutilisés. Tous les essais et faussespréparations sont conservés : Routerv1/v2/v3, assertionsTransport rejetées, Datagram1614, v1forte, v2ANY, probe-nine et probe-ten côtéintégrateur. La fauteRVA initiale0x47ef0 a été rejetée par revue et corrigée depuis295024=0x48070 avant toute remise.

Toutes les écritures du chercheur restent sous build/astra-resume/network. Aucun accèsVM, compilation, commit, modification de source/header/outil actif par ce chercheur. Deux chercheurs au total ; aucun sous-agent supplémentaire.
