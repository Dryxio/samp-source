# Essai de 30 minutes — trois agents

Départ 2026-09-06T12:07:17Z, arrêt 2026-09-06T12:37:17.002540+00:00 : 1800.00 secondes, coordination et finalisation comprises.

Couverture 402825 →408507/930756 (43.889806%). Gain exact unique **5682**, débit **11364.0 octets/heure**. Dont3161 issus de candidats préparés avant l’essai et2521 issus de travaux commencés pendant (débit frais5042.0/heure). Aucun benchmark solo comparable.

Les gains constatés des trois lots locaux sont2247,2217,1218. IsActive11 et simulateur53 sont des providers intégrés mais déjà couverts, zéro gain supplémentaire. Trois acceptations complètes PASS, chacune avec10 contrôles négatifs et ABI Actor. Pas de push/déploiement.

Temps des commandes instrumentées : compilation124.9s ; lien+draft16.2s ; revue+acceptations218.3s. Première acceptation supplémentaire observée pendant au moins90.2s. Le début n’a pas de chronométrage détaillé : ces sous-totaux sont incomplets, et chevauchent la recherche des agents. Le temps exact de l’intégration manuelle n’est pas isolable du reste analyse/coordination/finalisation ; ne pas inventer une ventilation exclusive.

0.2.5 : aide adaptée démontrée sur le codec et les handlers sync ; aucun nouveau crédit direct. Le nouveau réseau vient du vendor déjà disponible.

Pistes restantes hors couverture : trailer1441 octets cibles (modèles, quaternion, store et packet ; dépendances/divergences), paquet marqueurs510 octets cibles. Leurs tailles attendues ne constituent pas des matches. SendTo137 hérité demeure exclu faute d’étendue du buffer prouvée. Sources/essais conservés.

Goulot : intégration et résolution des dépendances, puis hypothèses de compilation infructueuses. Garder3 agents, renforcer la remise des déclarations/stockages/seedsCRT et la déduplication complète, regrouper les candidats prêts. Ne pas conclure à un gain chiffré contre le solo. Checkpoint toujours incomplet, travail en pause.
