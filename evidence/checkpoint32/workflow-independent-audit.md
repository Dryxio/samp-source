# Audit indépendant du workflow R5 — 8 septembre 2026

## Décision recommandée

Garder Astra moyen pour les décisions de reconstruction, mais remplacer le travail répétitif de l'intégrateur par des commandes déterministes. Le meilleur changement à essayer n'est ni « tout Luna », ni « tout Astra faible », ni un retour aveugle aux longs contextes historiques : c'est **Astra pour les hypothèses, un pipeline pour leur exécution et leur preuve**.

Conserver initialement deux chercheurs Astra moyen persistants et un intégrateur Astra. Les deux chercheurs travaillent sur des familles dont les dépendances sont connues, pas sur un flux de petites fonctions indépendantes. L'intégrateur intervient sur les exceptions et les dépendances critiques ; il ne doit plus réécrire à la main exports, seeds, unités, manifests et commandes entre chaque tentative. Cette recommandation est une hypothèse d'amélioration, pas un rendement déjà mesuré.

L'objectif « minimum de tokens » et l'objectif « terminer au plus vite » peuvent diverger. Politique proposée : minimiser le coût mesuré par progrès accepté **sous contrainte d'un débit acceptable**, et suivre séparément le chemin critique vers la DLL entière. Un énorme gain de petits getters ne compense pas une fin de projet bloquée sur le linker ou le startup.

## Ce que les preuves établissent

| Essai | Gain accepté | Durée finale | Octets/h | Entrée non cachée/octet | Entrée cachée/octet | Sortie/octet |
|---|---:|---:|---:|---:|---:|---:|
| C, Astra moyen | 6 590 | 31 min 35 s | 12 518 | 64,65 | 5 022,84 | 13,50 |
| E, Luna High natif | 993 | 36 min 11 s | 1 646 | 1 067,43 | 45 639,93 | 132,31 |
| F, Astra moyen ciblage tokens | 4 247 | 34 min 58 s | 7 287 | 83,14 | 7 205,19 | 21,54 |

Sources : `evidence/checkpoint32/trial30c-report.md`, `trial30e-report.md`, `trial30f-report.md`, `trial30f-results.json`. Les compteurs sont des deltas de sessions, pas une facture ni une mesure directe du quota Codex. Les derniers instants du rapport ne sont pas tous inclus dans la télémétrie.

F coûte davantage **dans chacune des trois composantes par octet** que C. Au même modèle et à pondérations positives identiques, F serait donc moins rentable pour toute combinaison de prix de ces composantes : ce résultat n'a pas besoin d'inventer un tarif. Cependant cela ne prouve pas que son ciblage de contexte a causé la dégradation. Les fonctions, dépendances, tentatives et sources disponibles diffèrent ; 1 363 octets F étaient déjà émis dans des sources existantes. Ces octets sont un gain légitime de production, mais doivent être séparés lors d'une comparaison de capacité de reconstruction.

Le chiffre historique 16 358,9/h vient de +229 684 octets sur environ 14 h 02, avec de grands lots vendor. Le début de cette même séquence avançait autour de 7,7–10,5k/h avant la maturation des gros ensembles. Le workflow n'a donc pas à lui seul un débit constant ; le mélange de travaux et la durée de maturation comptent fortement. Source : `historical-16k-workflow.md`, `PROGRESS.md` autour des jalons 1320, 1373–1508.

### Deux corrections aux conclusions précédentes

1. « Le bottleneck est vraiment le modèle » est trop fort. Les workers E ont fourni 140 octets, contre 2 860 en F ; c'est un signal réel contre le remplacement général par Luna, mais le travail n'était pas contrôlé et l'intégrateur a commis des erreurs dans les deux essais. On sait quel système a gagné ces essais, pas quelle cause unique explique l'écart.
2. « Annuler les optimisations » n'est pas fondé causalement. Des sorties filtrées peuvent réduire le coût sans nuire à la recherche ; enlever des détails nécessaires peut faire l'inverse. Il faut distinguer réduction du bruit et suppression d'information. L'index de propriété existait déjà dans C ; ce n'est pas un ajout responsable de la régression F.

## Où se trouve le gaspillage vérifiable

F consomme 91 494 tokens de sortie, dont 18 013 de raisonnement : environ 19,7%. Le reste, 73 481, inclut code, arguments d'outils et messages. Ce ratio ne prédit pas l'économie de faible, mais réfute l'intuition que presque toute la sortie est le raisonnement. Réduire l'effort ne traite pas automatiquement le coût des commandes générées et des corrections d'intégration. Le root produit à lui seul 38 731 tokens de sortie, soit 42,3% du total F ; optimiser uniquement les chercheurs manque une part substantielle du système.

Les 30,6 millions d'entrées cachées F sont principalement du contexte relu par les appels successifs, pas 30,6 millions de tokens nouveaux. Un contexte long mais stable et peu de tours peut battre un contexte court reconstruit constamment. La documentation officielle confirme l'intérêt des préfixes stables pour le cache ; elle ne permet pas de convertir nos compteurs en quota d'abonnement : https://developers.openai.com/api/docs/guides/prompt-caching . Les compteurs de raisonnement et de sortie doivent rester non doublés : https://developers.openai.com/api/docs/guides/reasoning . Aucun tarif ou Fast effectif n'est supposé dans cet audit.

Erreurs récurrentes constatées dans les rapports : sélection déjà couverte sous un autre nom, propriétaire existant manqué, visibilité C++ privée incorrecte, extern absent, source modifiée pendant la preuve, exports restant après exclusion d'une unité, et COMDAT choisi au lien différent de l'objet source exact. Elles produisent du temps et des tokens évitables **sans rien apprendre du binaire**.

`tools/index_symbol_owners.py` sait déjà comparer les propriétaires COFF et l'union couverte ; son existence n'a pas empêché ces erreurs. Le problème est le passage facultatif autour d'un outil, pas l'absence d'un nouveau document d'instructions.

F chronomètre 207 s de compilation sur 17 commandes, 34 s d'intégration sur 5 et 386 s de vérification sur 4 (une en échec). Ces catégories ne couvrent pas toute la préparation et peuvent se chevaucher. C avait 477 s d'union des commandes pour 1 895 s au total. L'hypothèse « accélérer la VM résoudra l'essentiel » n'est donc pas étayée. L'amélioration majeure doit réduire les cycles humains/LLM entre les commandes.

## Pipeline proposé, sans baisse de preuve

### 1. Une proposition exécutable, pas une remise en prose

Chaque lot fournit un fichier de proposition unique : base/proof hash, fichiers modifiés, propriétaire prévu, RVAs et chunks entiers, hypothèse par changement, profils de compilation, dépendances attendues, provenance, et états explicitement `proposé`, `objet exact`, `lié exact`, `accepté`. Les annotations de RVA ne sont jamais prises pour des preuves.

Une commande root valide automatiquement la base, la couverture réelle, les collisions, les déclarations et la liste des unités ; après compilation elle établit la fermeture COFF réellement émise et le propriétaire sélectionné au lien. Exports, seeds et manifeste de revue proviennent de cette même proposition. En cas de divergence, réponse courte et structurée avec lien vers le dossier complet. Aucun patch non accepté ne reste accidentellement dans le profil de lien du lot accepté.

Coût initial : consolidation des scripts existants et fixtures de non-régression, plus important qu'un changement de prompt mais limité en ne construisant pas un framework générique. Mesure : erreurs d'intégration évitables par lot, tours root par octet, nombre de lignes/arguments régénérés à la main. Premier test discriminant : rejouer en diagnostic les erreurs historiques E/F et vérifier qu'elles échouent avant le lien coûteux ; ce n'est pas du nouveau crédit RE.

### 2. Un registre de candidats relié aux dépendances

Élargir le classement actuel par taille à un graphe de fermeture : octets encore uniques, fournisseurs déjà prouvés, fournisseurs manquants, CRT/init/EH/table/vtable, collisions, source exploitable, et tentative précédente. Un petit fournisseur peut déverrouiller plusieurs gros appelants ; ces gains potentiels ne sont pas comptés avant leur acceptation et ne sont pas additionnés plusieurs fois entre familles.

Priorités : ancien source déjà émis mais non certifié et fermé ; gros vendor fermé ; petite dépendance bloquant un gros ensemble ; reconstruction nouvelle avec source apparentée ; inconnues profondes et layout sur une file dédiée. Router F montre pourquoi « 963 octets source exacts » n'est pas « un lot prêt » : 95 régions émises supplémentaires et 9 divergences nécessitent une décision de fermeture explicite.

Coût initial : joindre propriétaires/couverture/relocations déjà produits, sans cartographier parfaitement toute la DLL avant de reprendre. Mesure : taux proposition→acceptation, octets uniques acceptés par famille incluant recherches infructueuses, coût de fermeture. Ne pas remplacer une exploration par un score pseudo-précis sans données.

### 3. Déléguer les boucles mécaniques à la machine

L'agent propose un ensemble limité de variantes motivées par le binaire : type signé, lifetime d'une locale, portée, ordre d'un guard, contexte inline, classe d'optimisation déjà justifiée. Le root soumet la série à une seule file VM séquentielle. Le compilateur/comparateur produit pour chaque variante taille, hash, première divergence, catégorie relocation/target/control-flow et régions annexes ; l'agent analyse la synthèse en un tour.

C'est particulièrement adapté aux quatre essais DeathMessage E. Le but n'est pas une recherche brute de milliers de permutations ni des octets originaux injectés, mais moins d'allers-retours LLM pour des hypothèses déjà formulées. Le tri diagnostique ne remplace jamais la comparaison entière ni la revue des cibles à l'acceptation.

Coût initial : petit exécuteur avec cache par source/outil/flags/dépendances, essai isolé sur une famille réelle. Risque : volume de variantes, cache incomplet ou faux score de proximité. Mesure : hypothèses utiles par tour LLM, tentatives inutiles et coût total d'une fonction acceptée. Escalade après un échec seulement si l'agent fournit une nouvelle hypothèse fondée ; éviter de relancer une permutation déjà réfutée.

### 4. Amortir les preuves immuables, conserver les preuves sensibles au lien

`accept_actor_closure.py` fait une vérification, l'exercice ABI puis dix contrôles négatifs. Les tests instancient chacun `Gate(fresh=False)` et beaucoup relancent la vérification globale. Il existe donc une piste de cache/fixtures, mais il serait incorrect de retirer simplement ces tests ou de réutiliser un PASS historique après un changement de linker.

Première amélioration sûre : un lot cohérent par snapshot gelé, parsing d'objets partagé, préparation immutable, moins de validations avortées. Ensuite seulement, certification de source inchangée indexée par tous ses inputs et tests du vérificateur sur fixtures gelées, invalidés dès que le vérificateur/parsers/contrats pertinents changent. Les bytes liés, vraies cibles, sélection COMDAT et relocations PE sont revérifiés à chaque nouveau lien : ils peuvent changer sans changement du source de la fonction. Les cas négatifs pertinents pour une règle nouvelle restent requis.

Coût initial/risque : supérieur aux deux premiers changements, car il touche la confiance de la preuve. À différer tant que des erreurs de profil/export consomment encore des lots. Condition : démontrer l'équivalence ancien/nouveau verdict sur les cas historiques et mutants, sans crédit RE pour ce travail. Aucune modification du gate n'a été faite durant cet audit.

## Répartition des modèles et du contexte

**Par défaut maintenant : deux Astra moyen et root Astra, natifs.** Préfixe stable de règles et pack de famille ; accès aux preuves complètes sur disque à la demande. Ni historique entier imposé à chaque chercheur, ni plafonds arbitraires qui retirent les preuves nécessaires. Les grandes sorties sont consultables, mais les retours par défaut sont des diagnostics. Maintenir un agent sur une famille tant que ses connaissances sont utiles ; transfert explicite quand il change de domaine, pas de renaissance à chaque lot ni de contexte sans limite.

**Astra faible** est le prochain test modèle raisonnable sur les tâches bornées, pas un changement global déjà justifié. **Luna, Sol ou Terra** n'ont pas démontré ici une rentabilité de reconstruction. Les métadonnées des outils exposent ces options ; cela ne constitue pas une mesure comparative. Luna peut éventuellement remplir des tâches documentaires non critiques, mais si elles sont déterministes Python est souvent un meilleur candidat : pas de tokens de raisonnement, résultat reproductible. Ne pas ajouter un quatrième agent pour transformer en texte ce qu'un script sait lire.

**Moins d'agents peut gagner** si la file d'intégration grossit : suspendre les nouvelles recherches d'un worker quand plusieurs lots complets attendent, ou le réaffecter à la dépendance bloquante plutôt qu'accumuler des candidats non acceptables. Les octets préparés par agent ne sont pas le rendement système. Critère observable : âge du plus ancien lot prêt, temps root par lot, occupations VM, fraction des candidats finalement intégrés. Aucun seuil universel n'est encore démontré ; commencer par une file de un prochain lot par worker, puis ajuster.

## Atteindre 100% : ne pas optimiser le mauvais compteur

424 823 / 930 756 est de la couverture régionale de code. Les capsules relocalisent les fonctions et certifient leurs cibles ; elles ne constituent pas une DLL entière identique. Le contrat exige explicitement de distinguer exact brut, exact résolu et exact fichier entier. L'inventaire de fonctions n'est pas non plus une partition parfaite de `.text` ; padding, code CRT exclu du crédit, trous et données intercalées demandent une catégorie propre.

Vérification parallèle du parent : la section `.text` couvre 930 756 octets et l'union des chunks inventoriés 867 772, tous inclus dans `.text`. Il reste donc 62 984 octets hors de cet inventaire, à classifier sans supposer qu'ils sont tous du padding. Le gate exige une correspondance des `code_ranges` avec l'inventaire : à inventaire inchangé, ce compteur est plafonné à environ 93,23% avant même d'exclure les autres catégories. Ce n'est pas une impossibilité du projet ; c'est une impossibilité de promettre 100% de cette métrique sans d'abord en corriger la partition.

Il faut maintenir maintenant deux tableaux : (a) source/corps/fournisseurs acceptés et gain unique ; (b) sections, ordre des objets et COMDAT, layout/alignment, données/vtables/initialisations, EH, CRT/imports, entrypoint, relocations et métadonnées PE nécessaires au fichier final. Le CRT peut être indispensable au fichier sans devenir un gain de source. Pas de remplissage final par copie du binaire pour atteindre le compteur.

Réserver périodiquement une tâche bornée au risque de composition du binaire complet, même si elle apporte zéro octet de score immédiat. Sinon le scheduler qui maximise uniquement octets/tokens abandonne rationnellement les tâches indispensables à la livraison. Un profil de lien de production à faire converger en parallèle des capsules est une piste, à préciser après inventaire du travail déjà effectué ; je ne prétends pas qu'un changement de script rendra immédiatement ce lien possible.

## Expérience suivante et règle de décision

1. Instrumenter chaque lot dès maintenant : modèle/effort, famille, provenance, préparé avant/après, gains uniques, dépendances débloquées, tokens non cachés/cachés/sortie, phases, erreurs évitables. Prendre les snapshots finaux au même instant logique pour tous. Sans tarifs/quota, publier le vecteur de consommation, pas un prix fictif.
2. Mettre seulement la proposition exécutable et les filtres obligatoires en place, avec les mêmes Astra moyen. Reprendre une session de production assez longue pour voir plusieurs lots mûrir ; des fenêtres glissantes de 30 min sont informatives sans arrêter/recréer tous les agents toutes les 30 min. Le coût d'installation est enregistré séparément, puis amorti, jamais effacé du coût total projet.
3. Tester faible sur un petit ensemble de cas appariés : familles, taille, provenance et fermeture comparables, allocation préétablie ; proposer au besoin le même cas en replay isolé sans montrer la solution à l'autre effort. Compiler en file root identique, comptabiliser toute l'intégration. Un doublon de replay ne compte jamais en production. Alterner l'ordre pour limiter l'effet de cache/contexte et de sélection. Un essai court apporte un signal, pas une preuve universelle.
4. Conserver le changement s'il améliore le coût pondéré/octets sans dégrader excessivement le délai des familles critiques. Si les prix ou le quota ne sont pas accessibles, préférer les améliorations qui dominent sur les trois composantes, ou présenter clairement le compromis débit/consommation.

Le retour arrière recommandé concerne un changement mesuré qui échoue, pas toutes les automatisations utiles. Le meilleur pari immédiat est de réduire le nombre de cycles LLM nécessaires à une acceptation, plutôt que demander au même modèle de « réfléchir moins » à chaque cycle.

## Périmètre de l'audit

Lecture de `AGENTS.md`, rapports C/E/F et historique 16k, résultats/compteurs F, progrès, continuation actuelle, outils d'index/classement/acceptation et tests négatifs. Le parent a vérifié en parallèle les coûts de copie/cache de `probe_client.ps1` et la différence inventaire/section `.text`. Aucun accès VM, aucune compilation, aucun changement de base active, aucune acceptation, aucun push et aucun sous-agent supplémentaire. Ce fichier est uniquement une proposition d'organisation ; les gains futurs restent à mesurer.
