# Workflow historique à 16 358,9 octets/h

Reconstitution en lecture des preuves et journaux locaux, le 2026-09-08. Aucun benchmark relancé.

## Mesure retrouvée

- Départ : 2026-09-05 20:11:21 UTC, couverture 173 141 octets, trois agents coordination comprise (`build/parallel-throughput-start.json`).
- Dernier jalon : couverture 402 825, soit +229 684 octets uniques, débit cumulé 16 358,9 octets/h (`PROGRESS.md`, jalon RakClient/Router/SystemAddressList).
- Durée déduite du compteur arrondi : environ 14 h 02 min. Ce débit n'est pas celui d'un essai de 30 minutes.

## Organisation confirmée

Deux sous-agents natifs persistants, tous deux `gpt-6-astra`, effort `medium`, confirmés par leurs enregistrements `turn_context` :

- `/root/textdraw_reconstruction`, créé à 20:12:05 UTC, espace `build/agent-textdraw` ; départ libération de ressources textdraw et dépendances, puis familles UI.
- `/root/independent_family`, créé à 20:12:19 UTC, espace `build/agent-independent` ; départ pickups avec source 0.2.5, puis familles indépendantes et bibliothèques.
- Root intégrateur : seul accès compilation/link VM, modifications actives, acceptation et commits. Propositions et diagnostics dans les espaces isolés ; tâches réattribuées selon les blocages.

Les notes initiales `build/agent-textdraw/AUDIT.md` et `build/agent-independent/README.md` distinguent explicitement propositions non compilées, dépendances et absence de crédit. Elles exigent déjà la déduction de l'union acceptée, les tailles/types réels et les fonctions entières.

## Mécanique de production

- Travail continu et maturation de dépendances sur plusieurs heures ; préparation des propositions pendant l'intégration des lots précédents.
- Priorité aux ensembles de fonctions dont les dépendances deviennent implémentées, puis intégrations regroupées.
- Source vendor existant utilisé directement lorsqu'il suffit ; 0.2.5 utile aux sources client manquantes.
- Compilations ciblées, réutilisation des profils ; modifications de headers traitées comme des jalons avec recompilation des profils concernés.
- Comparaisons originales ET liées de fonctions/données/EH entières, vraies cibles COFF/PE, contrôles négatifs et ABI maintenus. Aucun crédit pour données, CRT, doublons ou dépendances non implémentées.

Exemples de gros lots consignés : Database/IME/keys +23 173 ; FileList/DatabaseClient/SHA1 +14 988 ; ConnectionGraph +10 510 ; Replica/NetworkID/navigation +12 131 ; RSA/parser/commandes +17 345. Le journal indique que le source vendor fournit l'essentiel de plusieurs de ces gains.

## Limites et reproduction

Les arguments textuels historiques des appels spawn sont chiffrés dans le journal local : le prompt exact n'est pas reproduit. Les paramètres de modèle, noms, heures, espaces et preuves de lots sont lisibles. Aucun réglage Fast particulier n'est établi ici.

Pour reproduire l'organisation : deux Astra moyen persistants, périmètres évolutifs client/UI et réseau/vendor, root intégrateur, propositions préparées en continu et lots regroupés à dépendances fermées. Retirer l'objectif spécifique d'économie de tokens, conserver les vérifications et corrections de propriété/déduplication.

Le contexte de rendement diffère : les grands lots déjà acceptés sont consommés, et la moyenne historique bénéficie d'environ 14 heures de maturation. Reprendre l'organisation ne garantit donc pas 16,4k/h sur les fonctions restantes ni sur une fenêtre de 30 minutes. Les résultats récents ne permettent pas d'attribuer toute baisse au modèle ou aux seules optimisations de contexte.
