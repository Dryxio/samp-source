# Itérations observées

Les snapshots intermédiaires restent localement dans les dossiers ignorés
`build/cp2-*`. Les essais incomplets ne sont pas des acceptations.

1. Les méthodes de stockage/destruction dans une seule unité de compilation
   permettaient à MSVC d’inférer l’absence d’exception et de supprimer les
   prologues EH attendus. Répartir Storage, Buffer et Layer dans des unités
   distinctes a reproduit les corps, helpers et données EH du R5.
2. Le switch écrit avec regroupement de valeurs identiques produisait deux
   tables compressées. Des cases séparés avec leurs retours, suivant la forme
   DXUT consultée comme piste, ont reproduit la table directe complète.
3. Les premiers essais de capsule ont corrigé la syntaxe de l’assembleur inline
   de test puis les alias du fichier DEF pour le linker 2003. Cet assembleur
   lit seulement FS:[0] dans les tests ; aucun octet R5 n’y est recopié.
4. L’allocation native à l’adresse fixe GTA de gravité échouait dans le processus
   hôte autonome. Le test a été séparé en exécution de la chaîne liée dans Unicorn
   et test natif de la protection sur une page allouée par Windows.
5. Le journal initial PowerShell était en UTF-16. Sa production a été fixée en
   UTF-8 ; ce problème de lecture n’a pas été traité comme un succès de gate.
6. Un contrôle négatif initial renommait seulement un symbole COFF défini sans
   changer sa section/adresse, donc sans changer la cible. Le contrôle actuel
   substitue réellement une autre définition et vérifie son rejet.
7. Les builds finaux `cp2-baseline` et `cp2-repeat` passent la commande complète
   aux bases distinctes, avec snapshots et hashes des sources/tests/artefacts.
