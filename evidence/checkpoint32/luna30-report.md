# Essai Luna high avec Astra intégrateur

+341 octets uniques acceptés, total429099/930756 (46,10%). Validation terminée à23:58:41 UTC, 27min05s après le départ. Sur la fenêtre prévue de30minutes, rendement682 octets/h. Aucun autre lot lancé; le checkpoint reste incomplet.

Trois chercheurs Luna high: un natif et deux via le CLI embarqué0.153.4, avec Astra moyen intégrateur. Les places natives occupées et deux lancements rejetés par le vieux CLI0.136.0 ont retardé le départ. Ce montage ne permet pas d'attribuer causalement le résultat au seul modèle. Aucun mode Fast explicitement vérifié pour cet essai.

Acceptés: roster101, noms100, acteur/caméra63, GUI31 et math46. 310octets réutilisent le source existant;31 proviennent d'une nouvelle unité C++ GUI. Aucun gain exclusif du0.2.5. Comparaisons entières, dépendances fermées, cibles COFF/PE réelles, contrôles négatifs et ABI requis: PASS.

Erreurs conservées: source roster périmé rejeté, propriétaire math déjà existant, proposition réseau dépendant d'une VFT incomplète et utilisant du C++ non supporté. Deux remises de manifeste ont aussi nécessité correction du schéma par l'intégrateur (roots dictionnaire et statut PASS générique). Le pipeline a bloqué les propositions invalides avant acceptation. La piste réseau de141octets reste non compilée et non comptée.

Bilan: rendement nettement inférieur au précédent essai Astra (+3935,10712octets/h), mais familles et conditions différentes. Les reprises et l'orchestration ont consommé du temps et des tokens Astra. Ne pas augmenter à10 chercheurs sur ce résultat. Prochaine expérience utile: départ natif propre, petit nombre de tâches bornées, schéma de remise validé automatiquement; comparaison sur tâches comparables. Compteurs par modèle et phases dans luna30-report.json; tokens cachés séparés et aucun coût facturé inventé.

Preuves et essais conservés. Aucun push ni déploiement GTA. Travail préexistant main.cpp conservé.
