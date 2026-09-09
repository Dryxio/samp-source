# Reprise de 40 minutes et finalisation

**+4,597 octets uniques acceptés**, total **435,895/930756 (46.8324%)**. Preuve finale : [astra40-ten-acceptance.json](astra40-ten-acceptance.json). Le checkpoint et la reconstruction intégrale de samp.dll restent incomplets.

Fenêtre demandée :20:40:48–21:20:48 UTC. Travail et finalisation observés : **49.75 minutes**, dont **9.75 minutes de dépassement** pour terminer l’intégration engagée. Débit global **5544 octets/h**, coordination, essais rejetés, contrôles et dépassement compris. À la fin des 40 minutes demandées, 3170 octets étaient acceptés ; les 1427 octets du second lot ont été finalisés ensuite. L’interruption précédente est exclue. La moyenne historique16359 octets/h portait sur environ 14h, avec d’autres familles ; ce résultat ne démontre aucun effet causal de modèle ou du seul pipeline.

Quota partagé : **50% → 56% consommés**, **44% restants**. Extrapolation au rythme arrondi de cette fenêtre : environ **6.1h**, indicative et sensible aux autres tâches. Aucun reset consommé.

| Agent | Entrée non cachée | Entrée cachée | Sortie |
| --- | ---: | ---: | ---: |
| root | 409,304 | 16,279,808 | 49,898 |
| /root/client_game_ui | 404,080 | 12,871,040 | 45,709 |
| /root/network_common | 392,751 | 15,259,136 | 59,959 |

Compteurs observés par agent, avec remise à zéro des compteurs de continuation détectée. Le raisonnement est inclus dans la sortie ; ce tableau ne représente pas un coût facturé. Root et les deux chercheurs natifs étaient Astra xhigh ; le réglage medium demandé pour root n’était pas modifiable ici, adaptation signalée dès le démarrage. Aucun changement de modèle ni expérience d’économie de tokens.

Débit récent entre les deux acceptations : **3306 octets/h**, intégration et essais rejetés compris.

Temps cumulés du pipeline : compilation88.4s, audit16.8s, lien29.4s, qualification2.8s, revue22.8s, acceptation205.9s. Probes isolés :75.3s. Ces sous-totaux ne couvrent pas toute la coordination et les recherches ; le débit global les inclut.

Les sources UI 267 sont nouvelles pendant la fenêtre. Les1123 octets SDK/UI supplémentaires proviennent de sources et objets déjà présents, dont les dépendances sont maintenant fermées. Les candidats Transport/Router avaient été en partie préparés avant l’interruption. GenerateDatagram 1594 retrouve entièrement les octets source attendus, mais reste NON ACCEPTÉ : conflits de linkage et émission des définitions canoniques non résolus. Ses variantes sont conservées, les trois sources actives restaurées depuis les sauvegardes antérieures, et aucun de ses 1594 octets ne compte dans ce bilan. Aucun crédit exclusif attribué au source 0.2.5.

Les échecs sont conservés sous leurs identifiants d’origine : comparateur Router absent, format d’alias parser, régions de propriétaires alternatifs non sélectionnées, export `_powf`, conflits NODUPLICATES/ANY puis fonctions inline non émises. Les comparaisons n’ont jamais été masquées et aucun `/FORCE` n’a été utilisé. Les preuves de tous les propriétaires sont séparées des manifestes des régions effectivement atteintes. Les contrôles finaux portent sur les fonctions/données/EH entières, toutes les cibles COFF/PE, les dix contrôles négatifs et les vérifications ABI par lot accepté. Données, padding, CRT et doublons ne donnent aucun crédit.

Les propositions, sources, snapshots, sauvegardes et logs restent sous build/astra40 et build/astra-resume ; les revues utiles et les rapports des chercheurs sont aussi conservés dans astra40-support. Ces rapports intermédiaires peuvent encore indiquer une acceptation en attente : la preuve finale ci-dessus fait autorité. main.cpp est préservé, commits locaux seulement, aucun push ni déploiement GTA.

Suite documentée : Draw statistiques et IME restent divergents ; `_powf14` nécessite la qualification de son nom d’export C. Renforcer le préflight des sélections COMDAT évitera des rejets tardifs, sans remplacer le linker ni l’acceptation complète.
