# Reprise avec deux Astra moyen et pipeline

Deux lots acceptés : **+3935 octets uniques**, total **428758/930756 (46,07 %)**. Commits code `5f1da1f`, `6c4f531`. Aucun push ni déploiement. Travail préexistant préservé.

Durée jusqu'aux commits code et relevé final : **22.04 minutes**, soit **10712 octets/h**, coordination, essais rejetés et contrôles compris. Aucune durée fixe de30 minutes demandée. Tous les candidats acceptés ont été préparés pendant cette reprise.

| Lot | Gain unique |
| --- | ---: |
| UI texte120, glyphes146, contrôles réseau143 |409|
| EmailSender3395, dialogue95, slot36 |3526|

Le pipeline complet a accepté les deux lots sans collision de propriétaires ni dépendance manquante au contrôle avant lien. Les sources, données/EH, relocations et cibles liées ont passé les contrôles inchangés, dont10 contrôles négatifs et ABI par lot. Les providers déjà couverts ne reçoivent aucun doublon de crédit.

Compteurs disponibles : **254,298 tokens d'entrée non cachés**, **12,956,672 cachés**, **47,816 de sortie**. Le raisonnement est inclus dans la sortie. Proxy entrée non cachée + sortie : **76.78 tokens/octet**. Ce proxy n'est pas un coût facturé ; les compteurs précèdent la réponse finale et peuvent être légèrement retardés.

Comparaison indicative : essai C12518 octets/h et78,15 tokens/octet selon ce proxy ; essai F7287 octets/h et104,68 ; ancienne moyenne16359 octets/h sur environ14 heures. Les tâches diffèrent. EmailSender représente3395 des3935 octets et provient du vendor existant : aucune accélération causale due au seul pipeline ou modèle n'est démontrée. Le source0.2.5 n'apporte pas de gain exclusif dans ces lots.

Temps des étapes du pipeline : compilation74.3s, audit8.2s, lien39.9s, revue43.8s, acceptation392.0s. Ces sous-totaux excluent les probes isolés et revues des chercheurs ; le temps global les inclut. Le contrôle final reste coûteux par rapport aux petits lots : regrouper les propositions fermées demeure prioritaire.

Entity reste rejeté (289/398 contre225), Cell174 non identifié sans ambiguïté. Le dialogue104/95 a été corrigé par des setters inline reproduisant l'évaluation observée, puis accepté en entier. Piste suivante documentée : IME4324, dépendances réelles encore à reconstruire. Reprise arrêtée après ces deux lots, aucune commande VM laissée en cours par l'intégrateur.
